// Reference:
// 1. http://man7.org/linux/man-pages/man7/rtnetlink.7.html
// 2, route netlink(RTN): http://lxr.free-electrons.com/source/include/linux/netlink.h
// 3. netlink(NET): http://man7.org/linux/man-pages/man7/netlink.7.html

// Types of netlink: NETLINK_XXX(<2>),
// Two side of netklink process: one is in user space for send/recv netlink message;
// the other one is kernel side for register netlink.

// This is user space for send/recv netlink message.
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

void
netlink_create_socket(void)
{
    struct sockaddr_nl sa;

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    bind(fd, (struct sockaddr *) &sa, sizeof(sa));
}

void
netlink_send(void)
{
    struct nlmsghdr *nh;    /* The nlmsghdr with payload to send */
    struct sockaddr_nl sa;
    struct iovec iov = { nh, nh->nlmsg_len };
    struct msghdr msg;

    msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    nh->nlmsg_pid = 0;
    nh->nlmsg_seq = ++sequence_number;
    /* Request an ack from kernel by setting NLM_F_ACK */
    nh->nlmsg_flags |= NLM_F_ACK;

    sendmsg(fd, &msg, 0);
}

void
netlink_recv(void)
{
    int len;
    char buf[4096];
    struct iovec iov = { buf, sizeof(buf) };
    struct sockaddr_nl sa;
    struct msghdr msg;
    struct nlmsghdr *nh;

    msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
    len = recvmsg(fd, &msg, 0);

    for (nh = (struct nlmsghdr *) buf; NLMSG_OK (nh, len);
        nh = NLMSG_NEXT (nh, len)) {
       /* The end of multipart message */
       if (nh->nlmsg_type == NLMSG_DONE)
           return;

       if (nh->nlmsg_type == NLMSG_ERROR)
           /* Do some error handling */
       ...

       /* Continue with parsing payload */
       ...
    }
}

// This is used for blog titled with "OVS转发面分析".
// This is code in kernel side for register netlink fuinction.

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
