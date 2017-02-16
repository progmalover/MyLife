### Reference and intruduce
1. https://www.kernel.org/doc/htmldocs/kernel-api/
2. http://blog.csdn.net/collide/article/details/125965
3. TCPIP详解
4. Linux设备驱动开发详解
5. Linux设备驱动程序
6. <linux内核源代码剖析 TCPIP 下>

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


### socket layer?

include/linux/net.h
include/net/socket.h
net/socket.c
net/ipv4/af_net.c
- socket data: struct socket and its data, include: 'socket_state state' for tcp; 'unsigned long flags' for socket option; 'const struct proto_ops ops' for socekt operate function; 'struct file *file' for socket fd; 'struct sock *sk' for socket control; 'struct fasyn_struct *fasyn_list' for ???; 'wait_queue_head_t wait' for hold thread waiting this socket; 'short type' for socket type like SOCK_STREAM/SOCK_DGRAM etc.
- socket operate: socket/bind/listen/accept/connect/select/etc system call, and what these system call do.

include/linux/socket.h
net/core/iovec.c : implement io operate.
net/socket.c : api layer.
- socket io:

net/socket.c : api layer.
net/ipv4/af_inet.c : ip/tp layer implement.
- socket option: all this saved in socket.flag .

include/net/sock.h
- socket control block: This is used for 'struct sock' in 'struct socket'. These sock_common sock inet_sock ... refer to <1>, is used for this.
- socket control block is used for tcp/udp/ip layer control. For TCP, also control asyn IO(epoll), refer to <1>.


### TCP ?

Basic architecture:
include/linux/tcp.h: TCP message struct/type/macro.
include/net/sock.h: sock struct/type/macro.
include/net/inet_connection_sock.h: TCP transport_connection struct/type/macro.
include/net/inet_hashtables.h
net/ipv4/af_inet.c: implement APIs between TCP(af) and INET(inet) layer.
net/ipv4/tcp_ipv4.c: implement APIs between transport_control block and INET.
net/ipv4/tcp.c: APIs between INET and SOCKET.
net/core/stream.c: TCP stream.
- TCP layer entry: receive from ip_local_deliver() in IP layer -> ... -> send by ip_queue_xmit() in IP layer.
- TCP inet_protosw: TCP block in 'struct inet_protosw inetsw_array', this is TCP INET layer implement.
- TCP net_protocol
- TCP control block: tcp_request_sock, tcp_sock, tcp_timewait_sock.
- TCP FMT: two path(client, server).
- TCP header and checksum.
- TCP init.
- TCP control block: Structure: inet_hashinfo. Init: tcp_v4_init_sock(). Memory management for sending and receive data: 
- TCP socket option: tcp_setsockopt().
- TCP error process: tcp_v4_err().

TCP Timer:
net/ipv4/tcp_timer.c
net/ipv4/inet_connection_sock.c
net/ipv4/tcp_output.c
net/ipv4/tcp_input.c
- TCP use 7 timer for every TCP connection: connection creation timer, re-send timer, wait-ack timer, continous timer, keep-alive timer, FIN_WAIT_2 timer, TIME_WAIT timer.
- Timer init: tcp_init_xmit_timer()

- connection creation timer:
    --used in SERVER-SIDE in PASIVE-OPEN for TCP 3-hand-shake for create connection, as below:
    server recv first SYN;
    server send first SYN+ACK;
    start connection creation timer;
    waiting for recv ACK;
    after SEVAREL times of TIMEOUT of this timer, connection will TIMEOUT(send RST), refer to TCP FMT.
    -- tcp_keepalive_timer(): TIMEOUT process function. This function is for 3 timer: connection creation, keepalive, FIN_WAIT_2. Refer to code.
    -- connection creation timer active: inet_csk_reqsk_queue_hash_add(), inet_csk_reqsk_queue_hash_added().

- Re-send timer:
    -- re-send timer is used when socket send data but not recv ACK. If re-send timer TIMEOUT, then TCP of this socket will re-send these data.
    -- tcp_write_timer(): TIMEOUT process function.
    -- tcp_retransmit_timer(): re-transmit function.
    -- timer activate: inet_sck_reset_xmit_timer(sk, ICSK_TIME_RETRANS, inet_sck(sk)->icsk_rto, TCP_RTO_MAX)

- Delay-ACK timer:
    -- Delay-ACK and Incidentally confirm, refer to <6>.
    -- TIMEOUT process function: tcp_delack_timer()
    -- timer activate: inet_csk_reset_xmit_timer(sk, ICSK_TIME_DACK, TCP_DELAY_MAX, TCP_RTO_MAX)

- Continuous timer:
    -- When peer annonce peer's window is 0, this will block TCP transmition, then this timer is set. After this timer TIMEOUT, TCP will transmit 1b data to peer, if peer is active, then TCP will recv ACK with new window value. Then contimue TCP session.
    -- TIMEOUT process function: tcp_probe_timer()
    -- timer activate: tcp_ack().

- Keekalive timer:
    -- When socket set SO_KEEPALIVE, and TCP connection is free over 2 hours, then use this timer, detail rule refer <6>P763.
    -- TIMEOUT process function: tcp_keepalive_timer()
    -- timer activate: inet_csk_reset_keepalive_timer()

- FIN_WAIT_2 timer: TBD'

TCP connection process:
include/linux/tcp.h: TCP message related struct/type/macro.
include/inet/request_socket: TCP connection_request block related struct/type/macro.
include/inet/inet_sock.h: ipv4 transport_control block related struct/type/macro.
include/inet/inet_connection_sock.h: TCP transport_control block related struct/type/macro.
net/ipv4/inet_connection_sock.c: TCP transport_control block implement.
net/ipv4/af_inet.c: implement interface between inet layer(ipv4 or ipv6) and transport(TCP or UDP) layer.
net/core/request_sock.c: TCP connection_request block implement.
net/ipv4/tcp_ipv4.c: implement interfaces between transport_control block(ipv4 or TCP??? maybe ipv4) and inet layer
net/ipv4/tcp_input.c: APIs used when recv message by TCP.
net/ipv4/tcp_output.c: APIs used when send message by TCP.
net/ipv4/tcp_minisocks.c: when recv request, create TCP trnasport_contrl block.
net/ipv4/inet_hashtable.c: implement of hash table used in TCP transport_control block.

- TCP transport_control block: struct inet_connection_sock *icsk.
TCP connection_request block: struct request_sock_queue icsk_accept_queue.
So transport_control block contains connection_request block.

- SERVER-SIDE connection creation process related struct:
    -- <6>P768 F28-1:
        struct inet_connection_sock.icsk_accept_queue
        struct request_sock_queue
        struct listen_sock
        ...

- some system call implement:
    -- create socket: ???
    -- bind: 
        --- struct inet_bind_hashbucket, inet_bind_bucket_create, inet_bind_bucket_destroy.
        --- bind_sys_call() -> inet_bind() -> tcp_v4_get_port() -> inet_csk_get_port()
    -- listen:
        --- inet_listen(), inet_csk_listen_start(), reqsk_queue_alloc()
    -- accept:
        --- inet_accept(), inet_csk_accept()
        ---  TCP SERVER pasive open:
            ---- SYN cookie
            ---- TCP message process entry: tcp_v4_do_recv(), tcp_rcv_established(), tcp_v4_hnd_req, tcp_rcv_state_process()
                ----- 1st handshake: tcp_rcv_state_process(), SYN=???
                ----- 2nd handshake: tcp_v4_send_synack(), SYN=1 ACK=1
                ----- 3rd handshake: tcp_v4_hnd_req()
    -- connect:
        --- inet_stream_connect(), tcp_v4_connect()
        --- TCP CLIENT active open:
            ---- 1: tcp_v4_connect()
            ---- 2: tcp_rcv_state_process()
            ---- 3: tcp_rcv_synsent_state_process()

