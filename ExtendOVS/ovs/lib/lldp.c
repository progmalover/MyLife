bool
lldp_should_process_flow(const struct flow *flow,
                struct flow_wildcards *wc OVS_UNUSED)
{
    /* Most packets are not CFM. */
    if (OVS_LIKELY(flow->dl_type != htons(ETH_TYPE_LLDP))) {
        return false;
    }

    return true;
}

// This is called by type_run --> dpif_class->run in main thread.
void
lldp_run(struct lldp *lldp) OVS_EXCLUDED(mutex)
{
    if (!lldp) {
        return;
    }

    ovs_mutex_lock(&mutex);
    if (timer_expired(&lldp->timer)) {
        struct lldp_port *lldp_port;

        lldp->timer.t += 1000;
        if (timer_expired(&lldp->timer)) {
            timer_set_duration(&lldp->timer, 1000);
        }

        HMAP_FOR_EACH (lldp_port, hmap_node, &lldp->ports) {
            // For each lldp port to process its packets.
            lldp_port_tick(lldp_port);
        }
    }
    ovs_mutex_unlock(&mutex);
}
