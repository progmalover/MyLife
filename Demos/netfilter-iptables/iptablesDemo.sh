#!/bin/bash
# 1. insert kernel module
# 2. cases:

# Basic usage


# Use mangle table
iptables -t mangle -A PREROUTING -i eth0 -p tcp -dport 80 -j MARK --set-mark 1
# this command add rule into mangle table, hook point is PREROUTING, match is interface=eth0 protocol=tcp L4port=80, actions is set skb->mark=1
# As you see, all iptables' action is to replace skb's field, just like packet flow in hardware.



# 1) drop all icmp packets from local
iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP

# 2) delete rule 1
iptables -D INPUT 1

# 3) delete rule
iptables -D INPUT -s 127.0.0.1 -p icmp -j DROP

# 4)
iptables -A FORWARD -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j ACCEPT
