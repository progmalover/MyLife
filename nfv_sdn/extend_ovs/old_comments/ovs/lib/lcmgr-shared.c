// RPC library.

static ssize_t
xovs_sock_recv(int fd, uint8_t *buffer, size_t n)
{
    return read(fd, buffer, n);
}

static ssize_t
xovs_sock_send(int fd, const uint8_t *buffer, size_t n)
{
    int bytes_left;
    int written_bytes;
    uint8_t *ptr;

    ptr = (uint8_t *)buffer;
    bytes_left = n;
    while (bytes_left > 0)  {
        written_bytes = write(fd, ptr, bytes_left);
        if (written_bytes <= 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                usleep(100000);
                continue;
            } else {
                return -errno;
            }
        }
        bytes_left -= written_bytes;
        ptr += written_bytes;
    }

    return 0;
}

static uint32_t
xovs_sock_allocate_seq(unsigned int n)
{
    xovs_sock_seq += n;

    if (xovs_sock_seq >= UINT32_MAX / 2) {
        xovs_sock_seq = 1;
    }

    return xovs_sock_seq;
}

// Socket RPC。
// @Param cmd: command of RPC, like "create vlan".
static int
xovs_sock_transact(int fd, int cmd, bool must_reply,
                   uint8_t *request, size_t send_len,
                   uint8_t *reply, size_t *recv_len)
{
    int ret;
    xovs_packet_header_t *ph = (xovs_packet_header_t *)(void *)request;

    ovs_mutex_lock(&xovs_mutex);

    ph->seq_no = xovs_sock_allocate_seq(1);
    ph->type = XOVS_PACKET_TYPE_REQUEST;
    ph->flag = must_reply ? XOVS_PACKET_FLAG_MUST_REPLY : XOVS_PACKET_FLAG_NO_REPLAY;
    ph->cmd = cmd;
    ph->len = send_len - XOVS_PACKET_HEADER_SIZE;

    ret = xovs_sock_send(fd, request, send_len);
    if (ret < 0) {
        VLOG_ERR("sending error (%s)", ovs_strerror(-ret));
        goto EXIT;
    }

    VLOG_DBG("xovs_sock_send: fd=%d, cmd=%d, bytes=%" PRIuSIZE,
             fd, cmd, send_len);

    if (must_reply && reply) {
        xovs_packet_header_t *reply_header;

        // If wait response, use do-while
        do {
            ret = xovs_sock_recv(fd, reply, *recv_len);
        } while (ret < 0 && errno == EAGAIN);

        VLOG_DBG("xovs_sock_recv: ret=%d", ret);

        if (ret < 0) {
            VLOG_ERR("receiving error (%s)", ovs_strerror(errno));
            goto EXIT;
        }
        if (ret < XOVS_PACKET_HEADER_SIZE) {
            VLOG_ERR("received bad reply len (%d)", ret);
            ret = -1;
            goto EXIT;
        }
        reply_header = (xovs_packet_header_t *)(void *)reply;
        if (reply_header->seq_no != ph->seq_no) {
            VLOG_ERR("received bad reply seq no (%u)", reply_header->seq_no);
            ret = -1;
            goto EXIT;
        }
        if (reply_header->type != XOVS_PACKET_TYPE_RESPONSE) {
            VLOG_ERR("received bad reply type (%u)", reply_header->type);
            ret = -1;
            goto EXIT;
        }
        if (reply_header->flag != XOVS_PACKET_FLAG_SUCCESS) {
            VLOG_DBG("received bad reply flag (%u)", reply_header->flag);
            ret = -1;
            goto EXIT;
        }
        if (reply_header->cmd != cmd) {
            VLOG_ERR("received bad reply cmd (%u)", reply_header->cmd);
            ret = -1;
            goto EXIT;
        }
        *recv_len = ret;
        ret = 0;
    }

EXIT:
    ovs_mutex_unlock(&xovs_mutex);
    return ret;
}

// The 'main' function for the sif handler thread.
// Sync program:
// client send its configure to server, do not wait;
// server run this configure, notice client by result.
static void *
sif_handler_main(void * args OVS_UNUSED)
{
    uint8_t packet[RECV_BUFF];
    size_t n = RECV_BUFF;
    int head_room = 0;
    int ret;

    // first check alive by hello packet.
    if (!xovs_sock_active_sif()) {
        VLOG_WARN("sif handler terminated");
        return NULL;
    }

    // exit_latch for exit. Latch is fds[2] wrapper in unix system, which is
    // double-pipe.
    while (!latch_is_set(&exit_latch)) {
        if (exited) {
            break;
        }

        // As socket is STREAM mode, we will receive hole packet.
        ret = xovs_sock_recv(xovs_sif_fd, &packet[head_room], n - head_room);
        if (ret > 0) {
            int offset = 0;
            int resovled_len;
            int left = head_room + ret;

            // Typical way to process packet, first parse header, then parse
            // body through header.
            do {
                resovled_len = process_sif_xovs_packet(&packet[offset], left);
                if (resovled_len < 0) {
                    head_room = 0;
                    break;
                }
                if (resovled_len == 0) {
                    if (offset == 0) {
                        head_room = 0;
                        VLOG_ERR("bad xovs packet len received from sif");
                    } else {
                        head_room = left;
                        memcpy(&packet[0], &packet[offset], left);
                    }
                    break;
                }

                head_room = 0;
                left -= resovled_len;
                offset += resovled_len;
            } while (offset < ret);
        } else if (ret < 0) {
            head_room = 0;
            if (errno != EAGAIN && errno != EOPNOTSUPP) {
                VLOG_ERR("error receiving data from sif: %s",
                         ovs_strerror(errno));
            }
        }

        if (ret > 0) {
            poll_immediate_wake();
        } else {
            poll_fd_wait(xovs_sif_fd, POLLIN);
            latch_wait(&exit_latch);
        }
        poll_block();
    }

    VLOG_INFO("sif handler terminated");
    return NULL;
}

// This is main entry for start sif_handler for RPC.
int
shared_lcmgr_init(void)
{
    static bool initialized = false;
    int fd;
    uint32_t port_mode;

    if (initialized) {
        return 0;
    }

    // Use these latch to notice other thread.
    latch_init(&xovs_latch);
    latch_init(&exit_latch);

    // Create unix socket to do RPC with lcmgr.
    fd = make_unix_socket(SOCK_STREAM, true, NULL, lcmgr_unix_socket_addr);
    if (fd < 0) {
        VLOG_ERR("%s: connection failed (%s)",
                 lcmgr_unix_socket_addr, ovs_strerror(-fd));
        return -fd;
    }
    VLOG_DBG("%s: connected to fd(%d)", lcmgr_unix_socket_addr, fd);
    xovs_lcmgr_fd = fd;

    // Create unix socket to do RPC with sif.
    fd = make_unix_socket(SOCK_STREAM, true, NULL, sif_unix_socket_addr);
    if (fd < 0) {
        VLOG_ERR("%s: connection failed (%s)",
                 sif_unix_socket_addr, ovs_strerror(-fd));
        close(xovs_lcmgr_fd);
        return -fd;
    }
    VLOG_DBG("%s: connected to fd(%d)", sif_unix_socket_addr, fd);
    xovs_sif_fd = fd;

    // This is to use RPC to get system info from sif(xovs_sif_fd).
    if (!xovs_sock_get_sys_info(system_mac, serial_num, &port_mode,
                                flow_limit)) {
        goto EXIT;
    }

    // This is to devide chipset resource between xovs and router.
    set_max_xxxx_table_entries(XXXX_STAT_ICAP, flow_limit[0]);
    // others are same.

    // This is to devide port management between xovs and router.
    set_port_mode(port_mode);
    crossflow_port_map_init(port_mode, NULL, 0);

    // These are check resource by RPC, talk to sif.
    if (!xovs_sock_get_xovs_ports(&xovs_ports, cmd_get_xovs_openflow_ports)) {
        goto EXIT;
    }
    VLOG_DBG("got xovs openflow ports");

    if (!xovs_sock_get_xovs_ports(&hybrid_ports, cmd_get_xovs_hybrid_ports)) {
        goto EXIT;
    }
    VLOG_DBG("got xovs hybrid ports");

    vlan_bitmap = bitmap_allocate(MAX_VLAN_NUMBER);
    if (!xovs_sock_get_vlans(vlan_bitmap)) {
        goto EXIT;
    }
    VLOG_DBG("got xovs vlans");

    // Init xovs mode.
    if (xxxx_xovs_init()) {
        goto EXIT;
    }

    // Create sif_handler thread to process RPC from sif or lcmgr.
    sif_handler_thread = ovs_thread_create("sif_handler",
                                           sif_handler_main,
                                           NULL);
    initialized = true;
    return 0;

EXIT:
    close(xovs_lcmgr_fd);
    close(xovs_sif_fd);
    return -1;
}

// Exit handler of xovs, called by main.
void
shared_lcmgr_exit(void)
{
    exited = true;

    // set exit_latch to make sif_handler thread stop.
    latch_set(&exit_latch);
    // Wait for sif_handler thread exit.
    xpthread_join(sif_handler_thread, NULL);

    // clean work.
    close(xovs_lcmgr_fd);
    close(xovs_sif_fd);
    latch_destroy(&exit_latch);
    latch_destroy(&xovs_latch);
    return;
}

// This is demo of create_vlan, others are same.
static bool
shared_lcmgr_create_vlan(uint32_t vlan_id)
{
    uint8_t *request = NULL;
    uint8_t *reply = NULL;
    size_t send_len = XOVS_PACKET_HEADER_SIZE;
    size_t recv_len = XOVS_PACKET_HEADER_SIZE;
    size_t offset = XOVS_PACKET_HEADER_SIZE;
    int ret;

    send_len += sizeof vlan_id;
    request = (uint8_t *)xmalloc(send_len);
    memcpy(&request[offset], &vlan_id, sizeof vlan_id);

    if (must_reply) {
        reply = (uint8_t *)xmalloc(recv_len);
    }

    ret = xovs_sock_transact(xovs_lcmgr_fd, cmd_create_vlan, must_reply,
                             request, send_len, reply, &recv_len);
    if (ret) {
        VLOG_ERR("Failed to create vlan (%u) in shared lcmgr", vlan_id);
    }

    free(request);
    free(reply);
    return ret ? false : true;
}

const struct lcmgr_class shared_lcmgr_class = {
    "shared",
    NULL, /* lcmgr_api_init */
    NULL, /* lcmgr_api_terminate */
    shared_lcmgr_create_vlan,
    // others are same.
};
