### Reference?

1. http://man.chinaunix.net/network/iptables-tutorial-cn-1.1.19.html


### Introduce?

Netfilter is ip system hook point, which is hook point around route table. Iptables use these hook points to match SKB using kinds of tables.

5 hook in netfilter and 2 tables in iptable, with 2 way: 
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
|table: nat   |       |         ****************
|chain: OUTPUT|       |               +
+-----+-------+       |               |
      |               |               v
      v               |      +-------------------+
+--------------+      |      | table: nat        |
|table: filter | +----+      | chain: POSTROUTING|
|chain: OUTPUT |             +--------+----------+
+--------------+                      |
                                      v
                               XXXXXXXXXXXXXXXXXX
                             XXX    Network     XXX
                               XXXXXXXXXXXXXXXXXX


