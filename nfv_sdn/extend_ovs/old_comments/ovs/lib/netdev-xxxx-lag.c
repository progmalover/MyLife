struct netdev_xxxx_lag {
    struct netdev up;

    /* Protects all members below. */
    struct ovs_mutex mutex;

    uint32_t port_no;           /* UINT32_MAX if unknown. */
    uint32_t bound_vport;
    ctype_speed_type_t speed;
    flow_control_type_t flow_ctl_para;
    lag_type_t lag_type;
    ctype_lag_psc_t hash_mapping;
    struct lacp *lacp;
    struct lacp_settings lacp_set;
    struct hmap queues;
    bool enable_loopback;
    uint32_t members_count;
    uint32_t lag_selected_min;
    uint32_t *members;
    bool members_changed;
};

struct xxxx_lag {
    struct list node; /* In 'all_lags'. */
    uint32_t port_no;  /* lag port ifindex, started from LAG_PORT_NUM_START. */
    uint32_t pvid;
    unsigned long *trunks;  /* Bitmap of trunked VLANs. */
    bool state;  /* True if lag is up. */
    uint32_t mtu;
    ctype_xxxx_cosq_schedule_mode_t mode;  /* Qos data. */
    int32_t weights[XXXX_QOS_QUEUES];      /* Qos data. */

    struct ovs_mutex mutex;       /* Protects members below. */
    struct netdev_xxxx_lag *dev;  /* NULL if port is not created. */
};

// All lags of both chipset and openflow support. lags is all lag, all_lags is
// all lags we have added.
static struct xxxx_lag lags[MAX_LAG_NUM];
static struct list all_lags = LIST_INITIALIZER(&all_lags);

// This is for select group lag.
struct select_group_lag {
    uint32_t lag_id;
    bool used;
    uint16_t *members;
    uint32_t members_count;
    uint32_t select_group_id;
};
#define MAX_SELECT_GROUP_LAG (MAX_LAG_NUM - MAX_USER_CONFIG_LAG)
static struct select_group_lag select_group_lags[MAX_LAG_NUM]; //MAX_SELECT_GROUP_LAG
static int xxxx_set_lag_balancing_strategy(uint32_t lag_no,
            ctype_lag_psc_t hash_mapping);

int
construct_select_group_lag(uint16_t *members, uint32_t members_count,
                uint32_t select_group_id, uint32_t *lag_id)
{
    // Addition of normal lag, just set lag type into select.
}

// Add/delete lag member.
static int
xxxx_set_lag_members(struct netdev_xxxx_lag *lag,
                     uint32_t *members, uint32_t members_count)
{
    // remove old members from lag
    if (lag->members_count) {
        ports = xmalloc(lag->members_count * sizeof(ctype_usp_t));
        for (i = 0; i < lag->members_count; i++) {
            // to remove a port from lag, first change port info of lag.
            set_phy_port_lag_info(lag->members[i], UINT32_MAX);
            // Then delete port from lag lacp group.
            lacp_delete_slave(lag->lacp, &lag->members[i]);
        }
        // Then delete port from lag group.
        lcmgr_delete_ports_from_lag(&lag_id, ports, lag->members_count);
        free(ports);
    }

    // add new members to lag
    if (members_count) {
        for (i = 0; i < members_count; i++) {
            // To add port to lag, first clear port vlan member.
            xxxx_clear_phy_port_vlan(members[i]);
            // Then set port tobe a lag.
            set_phy_port_lag_info(members[i], lag->port_no);
            // add port into lacp group.
            lacp_add_slave(lag->lacp, &members[i]);
            // If port is link up and use dynamic lacp:
            if (get_phy_port_link_up(members[i])
                && get_phy_port_lacp_enable(members[i])) {
                // Then add port to lag group.
                ret = lcmgr_add_ports_to_lag(&lag_id, &port_usp, 1, OVS_USER);
                if (!ret) {
                    VLOG_WARN("Failed to add port %d to ae%d.\n", members[i], lag_id.port);
                }
                // enable port STP.
                lcmgr_set_port_stp_state(&port_usp, CT_STP_STATE_FORWARDING);
            }
            // If port is link down, no need to add port into lag group.
            // If lag group of this port use static lacp, whether add port to lag is
            // determined by hardware.
        }
    }

    // reconfigure deleted physical ports
    for (i = 0; i < lag->members_count; i++) {
        if (phy_port_is_lag_member(lag->members[i])) {
            continue;
        }

        // Reset physical port vlan config, like pvid and vlan mode.
        xxxx_reset_phy_port_vlan(lag->members[i]);
        // For OVS, reset some files in netdev of this port.
        netdev_xxxx_reconfigure(lag->members[i]);
    }

    return 0;
}

// lag QoS operation.
static int
xxxx_ops_lag_queues(struct netdev_xxxx_lag *lag)
{
    // For lag QoS operation, do it on all members of lag.
}

// lag port could bond to vport, which means lag port could assign to
// gre/vxlan/etc tunnel vport, for packet, a packet go through vport,
// then output through lag, that's bond means.
static void
xxxx_set_lag_port_bound_vport(uint32_t lag_no, uint32_t vport)
{
}

// register callback function for send lacp packet.
static void
send_lacpdu_cb(void *port_, const void *pdu, size_t pdu_size)
{
    // Process lacp packet (pdu and pdu_size) follow protocol.
    // Then send it back.
    lcmgr_send_packet_by_port(NULL, 0, &usp, 1, &pkt);
}

// set config
static int
netdev_xxxx_lag_set_config(struct netdev *dev_, const struct smap *args)
{
    // Same as netdev_xxxx_set_config, to set lag param
    SMAP_FOR_EACH (node, args) {
        if (strcmp(node->key, "flow_ctl") == 0) {
            // lag port flow control mode, rx tx rx_tx
        } else if (strcmp(node->key, "lag_selected_min") == 0) {
            // minimum lag menber select number
        } else if (strcmp(node->key, "members") == 0) {
            // lag member
        } else if (strcmp(node->key, "lag_type") == 0) {
            // whether to use lacp
        } else if (strcmp(node->key, "lacp-system-id") == 0) {
            // system id
        } else if (!strcmp(node->key, "mtu")) {
            // lag mtu
        } else if (strcmp(node->key, "lacp-system-priority") == 0) {
            // lacp system priority
        } else if (strcmp(node->key, "lacp-time") == 0) {
            // fast lacp or slow lacp
        } else if (strcmp(node->key, "lacp-mode") == 0) {
            // active lacp or pasive lacp
        } else if (strcmp(node->key, "hash-mapping") == 0) {
            //
        } else if (strcmp(node->key, "loopback") == 0) {
            // whether could loopback
        } else {
            VLOG_WARN("Unsupported options:%s=%s for lag %s\n",
                      node->key, node->value, dev_->name);
        }
    }

    // Then set these configure by lcmgr api.
}

// send packet
static int
netdev_xxxx_lag_send(struct netdev *netdev_,
                     struct ofpbuf *pkt_ofpbuf,
                     bool may_steal)
{
    // Same as netdev_xxxx_send, send by lag usp.
}

static const struct netdev_class netdev_xxxx_lag_class = {
    NETDEV_TYPE_LAG,
    netdev_xxxx_lag_init,
    netdev_xxxx_lag_run,
    netdev_xxxx_lag_wait,

    netdev_xxxx_lag_alloc,
    netdev_xxxx_lag_construct,
    netdev_xxxx_lag_destruct,
    netdev_xxxx_lag_dealloc,

    NULL,                       /* get config */
    netdev_xxxx_lag_set_config,
    NULL,                       /* get_tunnel_config */
    netdev_xxxx_lag_send,
    NULL,                       /* send_wait */
    NULL,                       /* set_etheraddr */
    netdev_xxxx_lag_get_etheraddr,
    netdev_xxxx_lag_get_mtu,
    netdev_xxxx_lag_set_mtu,
    netdev_xxxx_lag_get_ifindex,
    netdev_xxxx_lag_get_carrier,
    NULL,                       /* get_carrier_resets */
    NULL,                       /* set_miimon_interval */
    netdev_xxxx_lag_get_stats,
    netdev_xxxx_lag_set_stats,
    netdev_xxxx_lag_get_features,
    netdev_xxxx_lag_get_speed,
    NULL,                       /* set_advertisements */
    NULL,                       /* set_policing */
    netdev_xxxx_lag_get_qos_types,
    netdev_xxxx_lag_get_qos_capabilities,
    netdev_xxxx_lag_get_qos,
    netdev_xxxx_lag_set_qos,
    netdev_xxxx_lag_get_queue,
    netdev_xxxx_lag_set_queue,
    netdev_xxxx_lag_delete_queue,
    NULL,                       /* get_queue_stats */
    netdev_xxxx_lag_queue_dump_start,
    netdev_xxxx_lag_queue_dump_next,
    netdev_xxxx_lag_queue_dump_done,
    NULL,                       /* dump_queue_stats */

    NULL,                       /* get_in4 */
    NULL,                       /* set_in4 */
    NULL,                       /* get_in6 */
    NULL,                       /* add_router */
    NULL,                       /* get_next_hop */
    NULL,                       /* get status */
    NULL,                       /* arp_lookup */

    netdev_xxxx_lag_update_flags,

    NULL,                       /* rx_alloc */
    NULL,                       /* rx_construct */
    NULL,                       /* rx_destruct */
    NULL,                       /* rx_dealloc */
    NULL,                       /* rx_recv */
    NULL,                       /* rx_wait */
    NULL                        /* rx_drain */
};

/* Protect all_crossflow_lags. */
static struct ovs_mutex mutex_crossflow = OVS_MUTEX_INITIALIZER;
/* Store all crossflow lags. */
static struct hmap all_crossflow_lags = HMAP_INITIALIZER(&all_crossflow_lags);

struct netdev_crossflow_lag {
    struct netdev up;
    struct hmap_node node;      /* In 'all_crossflow_lags'. */
    uint32_t port_no;           /* UINT32_MAX if unknown. */

    /* Protects all members below. */
    struct ovs_mutex mutex;
    uint32_t bound_vport;       /* UINT32_MAX if none. */
    uint32_t members_count;
    uint32_t *members;
    bool admin_state;
    bool link_state;            /* True if lag is up. */
};

static const struct netdev_class netdev_crossflow_lag_class = {
    NETDEV_TYPE_LAG,

    NULL,                       /* init */
    NULL,                       /* run */
    NULL,                       /* wait */

    netdev_crossflow_lag_alloc,
    netdev_crossflow_lag_construct,
    netdev_crossflow_lag_destruct,
    netdev_crossflow_lag_dealloc,

    NULL,                       /* get config */
    NULL,                       /* set_config */
    NULL,                       /* get_tunnel_config */
    netdev_crossflow_lag_send,
    NULL,                       /* send_wait */
    NULL,                       /* set_etheraddr */
    netdev_xxxx_lag_get_etheraddr,
    NULL,                       /* get_mtu */
    NULL,                       /* set_mtu */
    netdev_crossflow_lag_get_ifindex,
    netdev_crossflow_lag_get_carrier,
    NULL,                       /* get_carrier_resets */
    NULL,                       /* set_miimon_interval */
    netdev_crossflow_lag_get_stats,
    netdev_crossflow_lag_set_stats,
    NULL,                       /* get_features */
    NULL,                       /* get_speed*/
    NULL,                       /* set_advertisements */
    NULL,                       /* set_policing */
    NULL,                       /* get_qos_types */
    NULL,                       /* get_qos_capabilities */
    NULL,                       /* get_qos */
    NULL,                       /* set_qos */
    NULL,                       /* get_queue */
    NULL,                       /* set_queue */
    NULL,                       /* delete_queue */
    NULL,                       /* get_queue_stats */
    NULL,                       /* queue_dump_start */
    NULL,                       /* queue_dump_next */
    NULL,                       /* queue_dump_done */
    NULL,                       /* dump_queue_stats */

    NULL,                       /* get_in4 */
    NULL,                       /* set_in4 */
    NULL,                       /* get_in6 */
    NULL,                       /* add_router */
    NULL,                       /* get_next_hop */
    NULL,                       /* get status */
    NULL,                       /* arp_lookup */

    netdev_crossflow_lag_update_flags,

    NULL,                       /* rx_alloc */
    NULL,                       /* rx_construct */
    NULL,                       /* rx_destruct */
    NULL,                       /* rx_dealloc */
    NULL,                       /* rx_recv */
    NULL,                       /* rx_wait */
    NULL                        /* rx_drain */
};

void
netdev_xxxx_lag_register(void)
{
    if (is_xovs_enabled()) {
        netdev_register_provider(&netdev_crossflow_lag_class);
    } else {
        netdev_register_provider(&netdev_xxxx_lag_class);
    }
}