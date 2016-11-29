/**
*   This is demo for linux kernel notifier chain
*   Used for blog titled with "OVS转发面分析"
*   Refer to http://www.cnblogs.com/mosp/p/3551006.html
*/

// Include "struct notifier_block"
// int notifier_chain_register(struct notifier_block **nl, struct notifier_block *n)
// int notifier_chain_unregister(strut notifier_block **nl, struct notifier_block *n)
// int notifier_call_chain(struct notifier_block **nl, unsigned long val, void *v)
#include "notifier.h"

/*
*   notifier chain used in linux kernel network system
*   1. inetaddr_chain used in register_netdevice_notifier function in net/core/dev.c, is used to notify when address of interface change.
*   2. netdev_chain is used to notify when register status change of netdev, for example, netdev register or unregister.
*/
