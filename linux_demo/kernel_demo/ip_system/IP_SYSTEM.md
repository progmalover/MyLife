### Architecture?

Basic packet flow is:
Socket Layer(Supply socket and its option for use)
    |
L4 Layer(For TCP supply buffer and FMT, For UDP do nothing. All is file-like by fd.)
    |
IP System(Iptables, route, tc)
    |
Net Device(port, vport, tunnel port, etc)


### How to use?

For user space app, if you treat kernel as datapath, important is: net device operate; UC route, MC route, multi-instance rout, NAT route; iptables; tc.

For socket app, important is cs demo and socket opt.
