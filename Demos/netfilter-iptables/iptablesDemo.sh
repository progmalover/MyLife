#!/bin/bash
# 1. insert kernel module
# 2. cases:

# 1) drop all icmp packets from local
iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP

# 2) delete rule 1
iptables -D INPUT 1

# 3) delete rule
iptables -D INPUT -s 127.0.0.1 -p icmp -j DROP

# 4)
iptables -A FORWARD -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j ACCEPT
