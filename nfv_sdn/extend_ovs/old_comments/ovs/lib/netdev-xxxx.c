// main struct used for netdev.
struct netdev_xxxx {
    // extend netdev defined in OVS.
    struct netdev up;

    /* Protects all members below. */
    struct ovs_mutex mutex;

    unsigned int cache_valid;
    /* unsigned int change_seq; */

    int ifindex;                     /* -1 if unknown. */
    uint32_t kbits_rate;             /* Policing data. */
    uint32_t kbits_burst;
    ctype_speed_type_t speed;
    bool use_default_speed;
    flow_control_type_t flow_ctl_para;
    uint32_t tpid;
    enum sfp_port_lfs_state lfs_state;
    bool up_mode;
    bool enable_loopback;
    bool is_avp;
    ctype_use_fec_t use_fec;
    struct lacp_slave_settings lacp_set;
    struct hmap queues;

    enum netdev_features current;    /* Cached from lcmgr. */
    enum netdev_features advertised; /* Cached from lcmgr. */
    enum netdev_features supported;  /* Cached from lcmgr. */
    enum netdev_features peer;       /* Cached from lcmgr. */
};

// Physical port struct.
struct xxxx_phy_port {
    uint32_t port_no; /* phy port ifindex, 0 is not used, started from 1. */
    uint32_t lag_id;  /* UINT32_MAX if port is not a lag member,
                       * else start from LAG_PORT_NUM_START. */
    uint32_t bound_vport; /* UINT32_MAX if port is not bound to a GRE vport,
                           * else start from GRE_PORT_NUM_START. */
    uint32_t pvid;
    unsigned long *trunks;  /* Bitmap of trunked VLANs. */
    bool state;   /* True if port is enable. */
    bool linkup;  /* True if port is up. */
    bool hw_link; /* True if port is up. Only for communication
                   * between main thread and hw thread. */
    uint32_t real_speed; /* The current speed received from lcmgr. */
    unsigned int change_seq;  /* Change if port link changed. */
    bool lacp_enable;  /* True if port can be added to lag. */
    uint32_t mtu;
    ctype_xxxx_cosq_schedule_mode_t mode;  /* Qos data. */
    int32_t weights[XXXX_QOS_QUEUES];      /* Qos data. */
    struct netdev_xxxx *dev;  /* NULL if port is not created. */
    struct ovs_mutex mutex;   /* Protects 'hw_link' and 'change_seq'. */

    struct netdev_stats cached_stats; /* Stats cached from lcmgr. */
    struct netdev_queue_stats cached_queue_stats[XXXX_QOS_QUEUES]; /* Queue stats cached from lcmgr. */

    struct ovs_mutex sfp_mutex; /* Protects sfp_info. */
    struct netdev_module_info sfp_info; /* Sfp into cached from lcmgr. */
};

// This is set pretty low because we probably won't learn anything from the
// additional log messages.
static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(5, 20);

// QoS queue, name:rate, which is "Q1":10000. Used by port QoS.
struct simap xxxx_qoss = SIMAP_INITIALIZER(&xxxx_qoss);

// Number of physical port is MAX_PHYSICAL_PORT_NUM.
static struct xxxx_phy_port phy_ports[MAX_PHYSICAL_PORT_NUM + 1];

// main ops of netdev, use by "netdev->ops".
const struct netdev_class netdev_xxxx_class;

// This is reset function of port, which means a port add/delete of lag, will
// call this.
int
netdev_xxxx_reconfigure(uint32_t port_no)
{
    // Reset all value to default.
}

// Use this to set port configure, like speed 1G/10G/100G.
// Upper caller don't know netdev_xxxx, they set config by args.
static int
netdev_xxxx_set_config(struct netdev *netdev_, const struct smap *args)
{
    // cast "struct netdev" to "netdev_xxxx"
    struct netdev_xxxx *dev = netdev_xxxx_cast(netdev_);

    // In xovs mode, port speed is managed by router.
    if (is_xovs_enabled()) {
        return 0;
    }

    SMAP_FOR_EACH (node, args) {
        if (strcmp(node->key, "link_speed") == 0) {
            // set link_speed to dev
        } else if (strcmp(node->key, "flow_ctl") == 0) {
            // flow control
        } else if (strcmp(node->key, "lacp-port-id") == 0) {
            // lacp port id
        } else if (strcmp(node->key, "lacp-port-priority") == 0) {
            // lacp port priority
        } else if (strcmp(node->key, "lacp-aggregation-key") == 0) {
            // lacp aggregation key
        } else if (!strcmp(node->key, "mtu")) {
            // port mtu
        } else if (strcmp(node->key, "tpid") == 0) {
            // vlan tpid, notice default vlan tag, double tag
        } else if (strcmp(node->key, "access-vport") == 0) {
            // access vport
        } else if (strcmp(node->key, "is_dac") == 0) {
            /* Nothing to do. DAC configuration is removed. */
        } else if (strcmp(node->key, "use_fec") == 0) {
            // whether use fec
        } else if (strcmp(node->key, "link-fault-signaling") == 0 &&
                   port_type_is_optical(port_type)) {
            // lfs for optical port
        } else if (strcmp(node->key, "up-mode") == 0 &&
                   port_type_is_optical(port_type)) {
            // up mode for optical
        } else if (strcmp(node->key, "loopback") == 0) {
            // whether could use loopback
        } else {
            VLOG_WARN("Unsupported options:%s=%s for port %s\n",
                      node->key, node->value, netdev_->name);
        }
    }

    // If configure changed, set it to chipset through lcmgr_api
    return 0;
}

// Send packet by physical port through lcmgr_send_packet_by_port.
// Also upper do not need to know netdev_xxxx.
//
// When datapath is kernel, send packet by sendmsg.
// When use chipset, send packet by lcmgr_send_packet_by_port api.
static int
netdev_xxxx_send(struct netdev *netdev_, struct ofpbuf *pkt_ofpbuf,
                 bool may_steal)
{
    if ((*(uint16_t *)((void *)&pkt.data[12])) == htons(ETH_TYPE_VLAN)) {
        // If it's ethernet packet, notice vlan tag. First get pvid.
        lcmgr_get_port_pvid(&usp, &pvid);

        // Get packet vlan tag.
        vlanid = vlan_tci_to_vid((*(uint16_t *)((void *)&pkt.data[14])));

        // Check if port is the vlanid's member.
        if (!is_port_in_vlan(usp, vlanid)) {
            goto EXIT;
        }

        // Refer to lcmgr_send_packet_by_port, if packet's vlan id equals pvid,
        // send it as untagged packet, else send it as tagged packet.
        if (pvid == vlanid) {
            lcmgr_send_packet_by_port(NULL, 0, &usp, 1, &pkt);
        } else {
            lcmgr_send_packet_by_port(&usp, 1, NULL, 0, &pkt);
        }
    } else {
        // If it's not ethernet packet, send it as untagged packet.
        lcmgr_send_packet_by_port(NULL, 0, &usp, 1, &pkt);
    }

EXIT:
    if (may_steal) {
        // If may_steal is set, pkt_ofpbuf will be free here, or it's free outside.
        ofpbuf_delete(pkt_ofpbuf);
    }
    return ret;
}

// Get features from lcmgr_api, and then
// Stores the features supported by 'netdev' into each of '*current',
// '*advertised', '*supported', and '*peer' that are non-null.  Each value is a
// bitmap of "enum ofp_port_features" bits, in host byte order.  Returns 0 if
// successful, otherwise a positive errno value.
static int
netdev_xxxx_get_features(const struct netdev *netdev_,
                         enum netdev_features *current,
                         enum netdev_features *advertised,
                         enum netdev_features *supported,
                         enum netdev_features *peer)
{
    struct netdev_xxxx *netdev = netdev_xxxx_cast(netdev_);
    int ret;

    ret = netdev_xxxx_read_features(netdev);
    if (ret) {
        return ret;
    }

    *current = netdev->current;
    *advertised = netdev->advertised;
    *supported = netdev->supported;
    *peer = netdev->peer;

    return 0;
}

// Attempts to set input rate limiting (policing) policy.
static int
netdev_xxxx_set_policing(struct netdev *netdev_,
                         uint32_t kbits_rate, uint32_t kbits_burst)
{
    // kbits_rate = rate, kbits_burst = burst, refer to leaky bucket.
    // if kbits_rate == 0, then burst = 0;
    // else if burst == 0, burst = default burst;
    // else burst = kbits_burst.
    // Last, use lcmgr_api to set it.
}

// All QoS related is get from xxxx_qoss set.
// In linux, use TC to do this, and at last, TC ops will change netlink ops
// and send by socket to kernel, Demo will add this.

const struct netdev_class netdev_xxxx_class = {
    NETDEV_TYPE_OPENFLOW,
    netdev_xxxx_init,
    netdev_xxxx_run,
    netdev_xxxx_wait,

    netdev_xxxx_alloc,
    netdev_xxxx_construct,
    netdev_xxxx_destruct,
    netdev_xxxx_dealloc,

    NULL,                       /* get config */
    netdev_xxxx_set_config,
    NULL,                       /* get_tunnel_config */
    netdev_xxxx_send,
    NULL,                       /* send_wait */
    NULL,                       /* set_etheraddr */
    netdev_xxxx_get_etheraddr,
    netdev_xxxx_get_mtu,
    netdev_xxxx_set_mtu,
    netdev_xxxx_get_ifindex,
    netdev_xxxx_get_carrier,
    NULL,                       /* get_carrier_resets */
    NULL,                       /* set_miimon_interval */
    netdev_xxxx_get_stats,
    netdev_xxxx_set_stats,
    netdev_xxxx_get_features,
    NULL,                       /* get speed*/
    netdev_xxxx_set_advertisements,
    netdev_xxxx_set_policing,
    netdev_xxxx_get_qos_types,
    netdev_xxxx_get_qos_capabilities,
    netdev_xxxx_get_qos,
    netdev_xxxx_set_qos,
    netdev_xxxx_get_queue,
    netdev_xxxx_set_queue,
    netdev_xxxx_delete_queue,
    netdev_xxxx_get_queue_stats,/* get_queue_stats */
    netdev_xxxx_queue_dump_start,
    netdev_xxxx_queue_dump_next,
    netdev_xxxx_queue_dump_done,
    NULL,                       /* dump_queue_stats */

    NULL,                       /* get_in4 */
    NULL,                       /* set_in4 */
    NULL,                       /* get_in6 */
    NULL,                       /* add_router */
    NULL,                       /* get_next_hop */
    NULL,                       /* get status */
    NULL,                       /* arp_lookup */

    netdev_xxxx_update_flags,

    NULL,                       /* rx_alloc */
    NULL,                       /* rx_construct */
    NULL,                       /* rx_destruct */
    NULL,                       /* rx_dealloc */
    NULL,                       /* rx_recv */
    NULL,                       /* rx_wait */
    NULL                        /* rx_drain */
};

const struct netdev_class netdev_hybrid_class = {
    NETDEV_TYPE_HYBRID,
    NULL,                       /* init */
    NULL,                       /* run */
    NULL,                       /* wait */

    netdev_xxxx_alloc,
    netdev_xxxx_construct,
    netdev_xxxx_destruct,
    netdev_xxxx_dealloc,

    NULL,                       /* get_config */
    NULL,                       /* set_config */
    NULL,                       /* get_tunnel_config */
    netdev_xxxx_send,
    NULL,                       /* send_wait */
    NULL,                       /* set_etheraddr */
    netdev_xxxx_get_etheraddr,
    netdev_xxxx_get_mtu,
    NULL,                       /* set_mtu*/
    netdev_xxxx_get_ifindex,
    netdev_xxxx_get_carrier,
    NULL,                       /* get_carrier_resets */
    NULL,                       /* set_miimon_interval */
    netdev_xxxx_get_stats,
    netdev_xxxx_set_stats,
    netdev_xxxx_get_features,
    NULL,                       /* get speed*/
    NULL,                       /* set_advertisements */
    NULL,                       /* set_policing */
    NULL,                       /* get_qos_types */
    NULL,                       /* get_qos_capabilities */
    NULL,                       /* get_qos */
    NULL,                       /* set_qos */
    NULL,                       /* get_queue */
    NULL,                       /* set_queue */
    NULL,                       /* delete_queue */
    netdev_xxxx_get_queue_stats,/* get_queue_stats */
    netdev_xxxx_queue_dump_start,
    netdev_xxxx_queue_dump_next,
    netdev_xxxx_queue_dump_done,
    NULL,                       /* dump_queue_stats */

    NULL,                       /* get_in4 */
    NULL,                       /* set_in4 */
    NULL,                       /* get_in6 */
    NULL,                       /* add_router */
    NULL,                       /* get_next_hop */
    NULL,                       /* get status */
    NULL,                       /* arp_lookup */

    netdev_xxxx_update_flags,

    NULL,                       /* rx_alloc */
    NULL,                       /* rx_construct */
    NULL,                       /* rx_destruct */
    NULL,                       /* rx_dealloc */
    NULL,                       /* rx_recv */
    NULL,                       /* rx_wait */
    NULL                        /* rx_drain */
};

// When system init, register net device ops, which is netdev_xxxx_class
// and netdev_hybrid_class.
void
netdev_xxxx_register(void)
{
    static struct ovsthread_once once = OVSTHREAD_ONCE_INITIALIZER;

    if (ovsthread_once_start(&once)) {
        netdev_register_provider(&netdev_xxxx_class);
        netdev_register_provider(&netdev_hybrid_class);

        // register virtual port like gre, is same as netdev-vport.x. Refer
        // to Demos/kernel_demo/netdev_vport.c
        netdev_xxxx_vport_register();

        // register lag port netdev_class, refer netdev-xxxx-lag.x.
        netdev_xxxx_lag_register();

        ovsthread_once_done(&once);
    }
}