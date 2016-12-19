### Architecture?

Basic packet flow is:
Socket Layer(Supply socket and its option for use)
    |
L4 Layer(For TCP supply buffer and FMT, For UDP do nothing. All is file-like by fd.)
    |
IP System(Iptables, route, tc)
    |
Net Device(port, vport, tunnel port, etc)




