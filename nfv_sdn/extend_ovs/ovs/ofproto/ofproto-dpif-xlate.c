// How to deal with resubmit action?
//
// Tables in ofproto layer is user's view, which means multi-table. But in
// datapath, there is only one exact match table. So when flow adding from
// ofproto to kernel, flows in multi-table have to combine or merge into
// one flow and NEW/SET to kernel datapath.
//
// The resubmit action process is in xlate_actions in
// ofproto/ofproto-dpif-xlate.c

// defined in ofp-actions.h . For resubmit([port],[table])
struct ofpact_resubmit {
    struct ofpact ofpact;
    ofp_port_t in_port; // use in_port is specified, or use OFPP_IN_PORT
    uint8_t table_id;   // use table_id is specified, or no resubmit action
};

struct xlate_ctx {
    struct xlate_in *xin;
    struct xlate_out *xout;

    const struct xbridge *xbridge;

    /* Flow at the last commit. */
    struct flow base_flow;

    /* Tunnel IP destination address as received.  This is stored separately
     * as the base_flow.tunnel is cleared on init to reflect the datapath
     * behavior.  Used to make sure not to send tunneled output to ourselves,
     * which might lead to an infinite loop.  This could happen easily
     * if a tunnel is marked as 'ip_remote=flow', and the flow does not
     * actually set the tun_dst field. */
    ovs_be32 orig_tunnel_ip_dst;

    /* Stack for the push and pop actions.  Each stack element is of type
     * "union mf_subvalue". */
    union mf_subvalue init_stack[1024 / sizeof(union mf_subvalue)];
    struct ofpbuf stack;

    /* The rule that we are currently translating, or NULL. */
    struct rule_dpif *rule;

    /* Resubmit statistics, via xlate_table_action(). */
    int recurse;                /* Current resubmit nesting depth. */
    int resubmits;              /* Total number of resubmits. */
    bool in_group;              /* Currently translating ofgroup, if true. */

    uint32_t orig_skb_priority; /* Priority when packet arrived. */
    uint8_t table_id;           /* OpenFlow table ID where flow was found. */
    uint32_t sflow_n_outputs;   /* Number of output ports. */
    odp_port_t sflow_odp_port;  /* Output port for composing sFlow action. */
    uint16_t user_cookie_offset;/* Used for user_action_cookie fixup. */
    bool exit;                  /* No further actions should be processed. */

    bool use_recirc;            /* Should generate recirc? */
    struct xlate_recirc recirc; /* Information used for generating
                                 * recirculation actions */

    /* OpenFlow 1.1+ action set.
     *
     * 'action_set' accumulates "struct ofpact"s added by OFPACT_WRITE_ACTIONS.
     * When translation is otherwise complete, ofpacts_execute_action_set()
     * converts it to a set of "struct ofpact"s that can be translated into
     * datapath actions.   */
    struct ofpbuf action_set;   /* Action set. */
    uint64_t action_set_stub[1024 / 8];
};

static void
xlate_ofpact_resubmit(struct xlate_ctx *ctx,
                      const struct ofpact_resubmit *resubmit)
{
    ofp_port_t in_port;
    uint8_t table_id;
    bool may_packet_in = false;
    bool honor_table_miss = false;

    if (ctx->rule && rule_dpif_is_internal(ctx->rule)) {
        /* Still allow missed packets to be sent to the controller
         * if resubmitting from an internal table. */
        may_packet_in = true;
        honor_table_miss = true;
    }

    in_port = resubmit->in_port;
    if (in_port == OFPP_IN_PORT) {
        in_port = ctx->xin->flow.in_port.ofp_port;
    }

    table_id = resubmit->table_id;
    if (table_id == 255) {
        table_id = ctx->table_id;
    }

    // For GOTO action, is same like resubmit(,table)
    xlate_table_action(ctx, in_port, table_id, may_packet_in,
                       honor_table_miss);
}

static bool
xlate_resubmit_resource_check(struct xlate_ctx *ctx)
{
    // To check resubmitable.
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);

    if (ctx->recurse >= MAX_RESUBMIT_RECURSION + MAX_INTERNAL_RESUBMITS) {
        VLOG_ERR_RL(&rl, "resubmit actions recursed over %d times",
                    MAX_RESUBMIT_RECURSION);
    } else if (ctx->resubmits >= MAX_RESUBMITS + MAX_INTERNAL_RESUBMITS) {
        VLOG_ERR_RL(&rl, "over %d resubmit actions", MAX_RESUBMITS);
    } else if (ofpbuf_size(&ctx->xout->odp_actions) > UINT16_MAX) {
        // There are too many action asigned into odp_actions.
        VLOG_ERR_RL(&rl, "resubmits yielded over 64 kB of actions");
    } else if (ofpbuf_size(&ctx->stack) >= 65536) {
        // stack is overflow.
        VLOG_ERR_RL(&rl, "resubmits yielded over 64 kB of stack");
    } else {
        return true;
    }

    return false;
}

tatic void
xlate_table_action(struct xlate_ctx *ctx, ofp_port_t in_port, uint8_t table_id,
                   bool may_packet_in, bool honor_table_miss)
{
    // To check is ctx could be resubmit.
    if (xlate_resubmit_resource_check(ctx)) {
        ofp_port_t old_in_port = ctx->xin->flow.in_port.ofp_port;
        bool skip_wildcards = ctx->xin->skip_wildcards;
        uint8_t old_table_id = ctx->table_id;
        struct rule_dpif *rule;
        enum rule_dpif_lookup_verdict verdict;
        enum ofputil_port_config config = 0;

        ctx->table_id = table_id;

        /* Look up a flow with 'in_port' as the input port.  Then restore the
         * original input port (otherwise OFPP_NORMAL and OFPP_IN_PORT will
         * have surprising behavior). */
        ctx->xin->flow.in_port.ofp_port = in_port;
        verdict = rule_dpif_lookup_from_table(ctx->xbridge->ofproto,
                                              &ctx->xin->flow,
                                              !skip_wildcards
                                              ? &ctx->xout->wc : NULL,
                                              honor_table_miss,
                                              &ctx->table_id, &rule,
                                              ctx->xin->xcache != NULL);
        ctx->xin->flow.in_port.ofp_port = old_in_port;

        if (ctx->xin->resubmit_hook) {
            ctx->xin->resubmit_hook(ctx->xin, rule, ctx->recurse);
        }

        switch (verdict) {
        case RULE_DPIF_LOOKUP_VERDICT_MATCH:
           goto match;
        case RULE_DPIF_LOOKUP_VERDICT_CONTROLLER:
            if (may_packet_in) {
                struct xport *xport;

                xport = get_ofp_port(ctx->xbridge,
                                     ctx->xin->flow.in_port.ofp_port);
                config = xport ? xport->config : 0;
                break;
            }
            /* Fall through to drop */
        case RULE_DPIF_LOOKUP_VERDICT_DROP:
            config = OFPUTIL_PC_NO_PACKET_IN;
            break;
        case RULE_DPIF_LOOKUP_VERDICT_DEFAULT:
            if (!ofproto_dpif_wants_packet_in_on_miss(ctx->xbridge->ofproto)) {
                config = OFPUTIL_PC_NO_PACKET_IN;
            }
            break;
        default:
            OVS_NOT_REACHED();
        }

        choose_miss_rule(config, ctx->xbridge->miss_rule,
                         ctx->xbridge->no_packet_in_rule, &rule,
                         ctx->xin->xcache != NULL);

match:
        if (rule) {
            /* Fill in the cache entry here instead of xlate_recursively
             * to make the reference counting more explicit.  We take a
             * reference in the lookups above if we are going to cache the
             * rule. */
            if (ctx->xin->xcache) {
                struct xc_entry *entry;

                entry = xlate_cache_add_entry(ctx->xin->xcache, XC_RULE);
                entry->u.rule = rule;
            }
            xlate_recursively(ctx, rule);
        }

        ctx->table_id = old_table_id;
        return;
    }

    ctx->exit = true;
}

// Bellow is for "Special action excuted?" section of lldp.
static void
xlate_actions__(struct xlate_in *xin, struct xlate_out *xout)
    OVS_REQ_RDLOCK(xlate_rwlock)
{
    // ...
    // For SLOW_LLDP/SLOW_LACP/etc...
    special = process_special(&ctx, flow, in_port, ctx.xin->packet);
    if (special) {
        ctx.xout->slow |= special;
    } else {
        // ...
    }
    // ...
}

static enum slow_path_reason
process_special(struct xlate_ctx *ctx, const struct flow *flow,
                const struct xport *xport, const struct ofpbuf *packet)
{
    struct flow_wildcards *wc = &ctx->xout->wc;
    const struct xbridge *xbridge = ctx->xbridge;
    // ...
    // If configure lldp in ovsdb as lldp_enable, and if lldp_should_process_flow,
    // then the packet should be processed by xbridge->lldp.
    // lldp_should_process_flow: get whether process this packet by "flow" and "wc"
    if (xbridge->lldp && lldp_should_process_flow(flow, wc)) {
        if (packet && xport) {
            // Let lldp module(xbridge->lldp)'s lldp port(xport->ofp_port) to
            // process packet(packet).
            // This is just copy these value to lldp memory, wait lldp_run(lldp.c)
            // to process.
            lldp_process_packet(xbridge->lldp, xport->ofp_port, packet);
        }
        return SLOW_LLDP;
    }
    // ...
}


