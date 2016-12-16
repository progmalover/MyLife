// This is how to use netdev in NFV and kernel.
// Refer to:
// 1. http://www.cnblogs.com/mosp/p/3553398.html

// Netdev is net device, There are kinds of netdev in both kernel and nfv.
// Eth0, tunnel, virtual eth are both netdev.
//
// For use of netdev, we could get from ovs's netdev-provider.h or vrouter's
// or kernel's netdevice.h.
//
// Take ovs's netdev-provider.h for example. "struct netdev" is main struct of
// a net device, contains "const char *name" for "eth0",
// "const struct netdev_class netdev_class" for operation function of a net
// device, "struct shash_node node" to link all netdev for search, and others.
//
// "struct netdev_class" is main class to operate this netdev, contains
// "const char *type" to present which type does this class belongs to, such
// as "linux", "ABC-company"; a serials of call back fuction is used for
// init or destroy or send etc.
//
// Use of the netdev is netdev->send or something.
//
// For vrouter, it's like ovs, refer to "vr_interface.h", "struct vr_interface"
// is represent for L3 interface of a router, but it's not like ovs' struct,
// as vrouter is router, its interface should be L3 interface not net device.
// Vrouter use "vr_vxlan_req_process" or sth to process vxlan packet. As
// vrouter is a router, its packet could be send by "nh_output", as for router
// , all it's known is L3 interface from route table as next hop. Rx process
// is unknown ????
//
// For linux kernel, it's like ovs, "struct net_device" equals "struct netdev"
// , "struct net_device_ops" equals "struct netdev_class".

// Intruduce basic net device(eth0) fuction of ovs, kernel and vrouter:
//
// How to send in user space ovs? Use "sendmsg" of POSIX API, as user space ovs
// send the packet to datapath by netlink, so here use socket(may be unix
// socket) is OK. Notice that user space could only use socket, could not use
// netdev of eth0 directly, so use soclet to send packet to datapath by
// netlink, and then datapath will process it.
//
// How to receive in user space? It's handled by handler thread using packet
// in method, by netklink API.
//
// How to send in datapath? It's like kernel doing, "datapath/vport-netdev.X"
// implements it, thay all include "linux/netdevice.h". For netdev_send, it
// calls dev_queue_xmit at last.
//
// How to receive in datapath? Use "netdev_frame_hook" call back fuction from
// kernel TCPIP stack, and then use "netdev_port_receive" to trigger the
// receive process.
//
// How to send in vrouter? Use "nh_output" in tx function.
//
// How to receive in vrouter? Use "netdev_rx_handler_register" to register
// "vhost_rx_handler".

