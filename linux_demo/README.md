### Reference?

1. https://www.gnu.org/software/libc/
2. /usr/include  /usr/local/include
3. http://man7.org/linux/man-pages/man7/pthreads.7.html
4. <APUE>
5. http://linux-ip.net/
6. TCPIP详解
7. http://netfilter.org/projects/iptables/index.html
8. https://www.kernel.org/doc/man-pages/
9. <linux kernel development>
10. https://github.com/MintCN/linux-insides-zh.git
11. <linux 内核源码剖析 TCPIP 实现>
12. https://github.com/y123456yz/Reading-and-comprehense-linux-Kernel-network-protocol-stack.git


### What /usr/include and /usr/local/include contains?

/usr/include contains POSIX API signature, it's different from kernel include file, it's inline files of linux os. BTW, what's POSIX API? The easy way to remember is what <4> introduce is POSIX API.

/usr/local/include contains user installed library API signature, such as openvswitch or python2.7 or etc.

/usr/include/i386-linux-gnu may be only used by GCC, /usr/include/i386-linux-gnu/sys contains signature of kernel API and struct.


### Introduce to LIBC?

LIBC is C library in unix-like os, the C library means C's library just like "printf", "malloc", "assert", "lock" etc. We could get LIBC's path through `ldd /bin/ls`.


### Kernel Networking Achitecture and Tools in user view?

First we talk about command tools, refer to <5> for iproute2 command suite. There are kinds of command: 1)`ip xxx` to control net device, route, neighbour, tunnel, counter etc; 2)`ss` to dump TCP/UDP socket; 3)`tc` for traffic class; 4)`iptables` for firewall. So basically there L3, L4, TC, FW suites.

<ul>
    <li>ip: ip command to instead `ifconfig`.
        <ul>
            <li>ip link: network device config.</li>
            <li>ip address: protocal address management.</li>
            <li>ip neighbour: arp table management.</li>
            <li>ip route: route table management.</li>
            <li>ip rule: routing policy management.</li>
            <li>ip maddress: multicast address management.</li>
            <li>ip mroute: multicast cache management.</li>
            <li>ip tunnel: tunnel configure, support ipip(tunl0), sip(ipv6 over ip, sit0), gre(any over gre over ip, gre0).
                <ul>
                    <li>command:
                    create tunnel: ip tunnel add <NAME> mode <MODE> [ local <S> ] [ remote <D> ]
                    use tunnel interface: ip route add 3FFE::/16 via ::193.233.7.65 dev sit0
                    </li>
                </ul>
            </li>
            <li>ip monitor: monitoring <remote ip address, netdev, remote MAC address>.</li>

            <li>rtacct: associate tool for counting route realm packet/byte count.</li>
            <li>ifstat: interface counter.</li>
            <li>nstat: network counter.</li>
            <li>rtstat: router counter.</li>
        </ul>
    </li>
    <li>ss: TCP/UDP socket command utilities to instead `netstat`.
        <ul>
            <li>ss: will list "Netid  State  Recv-Q Send-Q  Local Address:Port  Peer Address:Port".</li>
        </ul>
    </li>
    <li>tc: traffic class command, refer to ./kernel_use_demo/tc_demo.c .</li>
    <li>iptables: refer to ./kernel_use_demo/netfilter_iptables/ .</li>
</ul>

Then we discuss TCPIP stack, thay are: 1) init chain; 2) config system; 3) packet flow in L2/L3/L4. For packet flow, thay are: interface layer refer to <6>; iptables module refer to <7>; ip layer refer to <6>; tcp/dup layer refer to <6>; socket layer refer to <6>; tc module refer to <5>.

Others refer to DATAPATH.md .


### difference between `ip route` and route NAT?


