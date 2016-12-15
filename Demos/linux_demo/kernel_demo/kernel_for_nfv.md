### Reference and intruduce
1. https://www.kernel.org/doc/htmldocs/kernel-api/
2. http://blog.csdn.net/collide/article/details/125965
3. TCPIP详解
4. Linux设备驱动开发详解
5. Linux设备驱动程序

This is mainly record data types and apis for NFV. So we will read ovs and vrouter code.


### Kernel Directory Tree

Documentation/ : Document.
arch/ : hardware such as CPU include file.
block/ : block device include file and source file.
certs/ : ???
crypto/ : ???
drivers/ : drivers.
firmware/ : old folder of drivers.
fs/ : file system.
include/ : include files for all.
init/ : ???
ipc/ : ipc use it.
kernel/ : ???
lib/ : library.
mm/ : memory management.
net/ : network use it.
samples/ : ???
scripts/ : ???
security/ : security related.
sound/ : old folder of sound drivers.
tools/ : ???
usr/ : ???
virt/ : new folder of linux virtual and namespace related.


### How to build .ko module?

For writing .ko module, refer to <2>, three point have to notice: wirting module, compiling module, installing and removing module.

For writing and compiling module, 1) have to define "#define MODULE"(but this is not defined in vrouter, dont know why); 2) have to include "linux/module.h"; 3) define "int init_module" and "void cleanup_module"; 4) when compiling, have to include "-I /usr/src/linux" and -D__KERNEL__ to enable (1); Notice EXPORT_SYMBOL.


### how to get and send packet with kernel TCPIP stack?

<4>P364 F16.1 is key point of netdev driver, so if you want to send packet into netdev, call dev_queue_xmit(struct skb_buff *skb) and the skb is the packet you want to send. If you want to receive packet from netdev, use "int netif_rx(struct skb_buff *skb)", the skb is the packet you get from net device. So all you need to do to process packet is to fill netdev struct defined in include/linux/netdevice.h .

For "vrouter", linux_if_rx funciton of linux/vr_host_interface.c is this.
