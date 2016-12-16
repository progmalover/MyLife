/**
*   This is used for blog titled with "OVS转发面分析"
*/

// User side code
// define family
// first define attribute
enum doc_exmpl_a {
    DOC_EXMPL_A_UNSPEC,
    DOC_EXMPL_A_MSG,
    __DOC_EXMPL_A_MAX,  // this is common define method used in MACRO
};
#define DOC_EXMPL_A_MAX (__DOC_EXMPL_A_MAXi - 1) // I don't know what's this standards for

// second define attribute policy
static struct nla_policy doc_exmpl_genl_policy[DOC_EXMPL_A_MAX + 1] = {
    [DOC_EXMPL_A_MAX] = {.type = NLA_NUL_STRING},
};

// define family
static struct genl_family doc_exmpl_genl_family = {
    ...
};

// define handler
int doc_exmpl_echo(struct skb *skb, struct genl_info *info)
{
    ...
}

// register netlink
int register_netlink()
{
    // register family above
    int rc = genl_register_family(&doc_exmpl_gnl_family);
    if (rc != 0) {
        return EINVAL;
    }

    // register ops to family
    rc = genl_register_ops(&doc_exmpl_gnl_family, &doc_exmpl_echo);
    if (rc != 0) {
        return EINVAL;
    }
}


// kernel side code, which is coding in linux LKM
// you have to include these two files
#include "net/netlink.h"
#include "net/genetlink.h"

// use C style function name
int func_kernel()
{
    // 1. new nlsmsg
    struct sk_buff *skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
    if (skb == NULL) {
        return EINVAL;
    }

    // 2. fill payload
    // fill header
    msg_header = genlmsg_put(skb, ...);
    if (msg_header == NULL) {
        return EINVAL;
    }

    // fill body
    rc = nla_put_string(skb, ...);

    genlmsg_end(skb, ...);
}
