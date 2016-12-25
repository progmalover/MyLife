### Reference?

1. http://man.chinaunix.net/network/iptables-tutorial-cn-1.1.19.html
2. http://www.groad.net/bbs/thread-7091-1-1.html
3. http://conntrack-tools.netfilter.org/manual.html


### Introduce?

Netfilter is ip system hook point, which is hook point around route table. Iptables use these hook points to match SKB using kinds of tables.

5 hook in netfilter and 3 tables in iptable, with 2 way: 
1) PREROUTING/(mangle -- nat) -- routing -- FORWARD/(mangle -- filter) -- routing -- POSTROUTING/(mangle -- nat)
2) PREROUTING/(mangle -- nat) -- routing -- INPUT/(mangle -- filter) -- local process -- routing -- OUTPUT/(mangle -- nat -- filter) -- POSTROUTING/(mangle -- nat)

                               XXXXXXXXXXXXXXXXXX
                             XXX     Network    XXX
                               XXXXXXXXXXXXXXXXXX
                                       +
                                       |
                                       v
 +-------------+              +------------------+
 |table: m -> f| <---+        | table: m -> n    |
 |chain: INPUT |     |        | chain: PREROUTING|
 +-----+-------+     |        +--------+---------+
       |             |                 |
       v             |                 v
 [local process]     |           ****************          +--------------+
       |             +---------+ Routing decision +------> |table: m -> f |
       v                         ****************          |chain: FORWARD|
****************                                           +------+-------+
Routing decision                                                  |
****************                                                  |
       |                                                          |
       v                        ****************                  |
+-------------+       +------>  Routing decision  <---------------+
|table: m -> n|       |         ****************
|chain: OUTPUT|       |               +
+-----+-------+       |               |
      |               |               v
      v               |      +-------------------+
+--------------+      |      | table: m -> n     |
|table: filter | +----+      | chain: POSTROUTING|
|chain: OUTPUT |             +--------+----------+
+--------------+                      |
                                      v
                               XXXXXXXXXXXXXXXXXX
                             XXX    Network     XXX
                               XXXXXXXXXXXXXXXXXX

mangle: TOS, TTL, MARK.
nat: DNAT, SNAT, MASQUERADE, with conntrack module.
filter: any match filed defined in SKB, action is DROP or ACCEPT.


### conntrack?

Connection tracking:  it's a module to track flow using <SIP, DIP, SPORT, DPORT>, this make firewall STATE-FULL, so conntrack could listen TCP/UDP/ICMP etc protocols. For a flow, there are NEW/ESTABLISHED/RELATED/INVALID state. Packet from local app send to net device will tracked by OUTPUT chain, packet from net device to local app or forward will be tracked by PREROUTING chain, no matter where flow tracked, infomation will be share.

For example, packet<sip-A, dip-B, sp-C, dp-D> from local app to net device will be NEW state in OUTPUT chain. Then packet<sip-B, dip-A, sp-D, dp-C> from net device to local app or forward will be ESTABLISHED state. These track history could be recorded and state by `conntrack -L` refer to <2> <3>.

<1> shows NEW/ESTABLISHED/RELATED/INVALID state.


### How to construct rules?

Tables in iptables:
<ul>
    <li>nat: it's used for NAT for a flow.</li>
    <li>mangle: set TTL or TOS or MARK of a flow, MARK is used for `tc`. Notice hook point and its use, refer to <1>.</li>
    <li>filter: FORWAD chain is used for packet not through local app. INPUT chain is used for packet to locla app. OUTPUT is used for packet from local app.</li>
</ul>


