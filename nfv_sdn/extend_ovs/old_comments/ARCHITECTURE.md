### Reference?

1. http://vinllen.com/ovs-facethe-subfacetde-guan-xi/


### Introduce?

This is OVS-2.3.0 .

### Files on box?

root      9153  0.0  0.0  21344  2092 ?        S<   02:03   0:03 ovsdb-server /etc/openvswitch/conf.db -vconsole:emer -vsyslog:err -vfile:info --remote=punix:/var/run/openvswitch/db.sock --private-key=db:Open_vSwitch,SSL,private_key --certificate=db:Open_vSwitch,SSL,certificate --bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert --no-chdir --log-file=/var/log/openvswitch/ovsdb-server.log --pidfile=/var/run/openvswitch/ovsdb-server.pid --detach --monitor
root      9163  0.1  1.2 390864 48832 ?        S<Ll 02:03   1:25 ovs-vswitchd unix:/var/run/openvswitch/db.sock -vconsole:emer -vsyslog:err -vfile:info --mlockall --no-chdir --log-file=/var/log/openvswitch/ovs-vswitchd.log --pidfile=/var/run/openvswitch/ovs-vswitchd.pid --detach --monitor
root      9566  0.0  0.0  62096  2040 ?        S    02:04   0:00 sudo /usr/bin/neutron-rootwrap /etc/neutron/rootwrap.conf ovsdb-client monitor Interface name,ofport --format=json
root      9568  0.0  0.2  39280  9488 ?        S    02:04   0:00 /usr/bin/python /usr/bin/neutron-rootwrap /etc/neutron/rootwrap.conf ovsdb-client monitor Interface name,ofport --format=json
root      9570  0.0  0.0  17444  1580 ?        S    02:04   0:00 /usr/bin/ovsdb-client monitor Interface name,ofport --format=json
root     14566  0.0  0.1  50248  5296 pts/1    S+   02:57   0:00 vi tests/unit/services/l2gateway/agent/ovsdb/test_ovsdb_monitor.py

 9163 root      10 -10  390864  48832   6552 S  0.3  1.2   0:46.17 ovs-vswitchd                   
 9409 root      10 -10  390864  48832   6552 S  0.3  1.2   0:32.60 revalidator16                  
 9221 root      10 -10  390864  48832   6552 S  0.0  1.2   0:00.00 urcu4                          
 9407 root      10 -10  390864  48832   6552 S  0.0  1.2   0:00.00 handler14                      
 9408 root      10 -10  390864  48832   6552 S  0.0  1.2   0:00.00 handler15                      
 9410 root      10 -10  390864  48832   6552 S  0.0  1.2   0:05.98 revalidator17

ovs-vswitchd: main task, response for ovsdb configure and reconfigure(bridge_reconfig), and openflow task(type_run).
revalidator: response for flow revalidate and flow counter.
urcu: response for user mode RCU.
handler: response for packet upcall and then send back to kernel.

$ ls /usr/bin/ovs*
/usr/bin/ovs-appctl     /usr/bin/ovs-dpctl            /usr/bin/ovs-pcap       /usr/bin/ovs-vsctl
/usr/bin/ovs-benchmark  /usr/bin/ovs-dpctl-top        /usr/bin/ovs-pki
/usr/bin/ovsdb-client   /usr/bin/ovs-ofctl            /usr/bin/ovs-tcpundump
/usr/bin/ovsdb-tool     /usr/bin/ovs-parse-backtrace  /usr/bin/ovs-vlan-test

$ ls /etc/openvswitch
conf.db  conf.db.backup7.3.0-2483452374  system-id.conf

$ ls /var/log/openvswitch/
ovs-ctl.log    ovsdb-server.log    ovs-vswitchd.log
ovs-ctl.log.1  ovsdb-server.log.1  ovs-vswitchd.log.1

$ ls /var/run/openvswitch/
br-ex.mgmt   br-int.snoop  db.sock                ovs-vswitchd.9163.ctl
br-ex.snoop  br-tun.mgmt   ovsdb-server.9153.ctl  ovs-vswitchd.pid
br-int.mgmt  br-tun.snoop  ovsdb-server.pid


### Source code?
.
├── boot.sh // second run `boot.sh` to autoconf
├── configure.ac // first run `configure` to configure
├── datapath // datapath
├── include // include files, used for user space program
├── lib // libs used for user space program
├── Makefile.am // third `make` and `make install`
├── ofproto // openflow protocol related, including parse and construct message
├── ovn
├── ovsdb // ovsdb server
├── python // python libs
├── utilities // ovs-XXX command
├── vswitchd // ovs-vswitchd
├── vtep // vtep demo


### ovs-vswitchd?

init chain:
ovs-vswitchd.c main():
    bridge_init
    while(run):
        bridge_run
        netdev_run
        bridge_wait
        netdev_wait
    bridge_exit

bridge.c bridge_init():
    create ovsdb connection
    some init work

bridge.c bridge_exit():
    for each br:
        destroy br
    destroy ovsdb connection

bridge.c bridge_run():
    get cfg from ovsdb server
    bridge_run__
    if (idl changed or vlan_splinters_changed):
        bridge_reconfigure // reconfig bridge according cfg in new idl
    refresh STP or PORT counter into ovsdb
    run_system_stats // get system stats and write to ovsdb
    
bridge.c bridge_run__():
    for each datapath type:
        ofproto_type_run(type)
    for each bridge:
        ofproto_run(br->ofproto)

ofproto/ofproto.c ofproto_type_run(const char *datapath_type):
    // datapath_type: 'normal', 'internal', 'XXXX' ...
    // each datapath is for 'ofproto->backer'
    backer = shash_find_data(&all_dpif_backers, type)
    dpif_run(backer->dpif) 
    // Then 'dpif_run' call 'dpif->dpif_class->run(dpif)'. All bridge belongs
    // to one 'backer' use one backer.
    process 'backer->recv_set_enable'
    if (backer->recv_set_enable):
        udpif_set_threads(backer->udpif, n_handlers, n_revalidators)
        // This is start revalidate thread and handler thread
    if (backer->need_revalidate):
        HMAP_FOR_EACH (ofproto, all_ofproto_dpifs_node, &all_ofproto_dpifs):
            if ofproto not belong to backer:
                continue
            for each tunnel port in ofproto:
                handle tunnel garbage collection
        set revalidate lable
        HMAP_FOR_EACH (ofproto, all_ofproto_dpifs_node, &all_ofproto_dpifs):
            xlate_ofproto_set // set ofproto options
            HMAP_FOR_EACH (bundle, hmap_node, &ofproto->bundles):
                xlate_bundle_set // set bundle options
            HMAP_FOR_EACH (ofport, up.hmap_node, &ofproto->up.ports):
                xlate_ofport_set // set port options
        udpif_revalidate(backer->udpif) // seq_change(udpif->reval_seq) to notice revalidate
        process_dpif_port_changes(backer) // poll to check port of dpif

ofproto/ofproto.c ofproto_run(struct ofproto *p):
    error = p->ofproto_class->run(p) // call ofproto_class->run, which is ofproto_dpif_class->run
    run_rule_executes(p) // call ofproto->ofproto_class->rule_execute(e->rule, &flow, e->packet)
    // This is process ofproto->rule_executes which is queued before.
    process eviction rule of ofproto
    process p->ofproto_class->port_poll
    if (p->change_seq changed):
        Update OpenFlow port status whose netdev has changed
    switch(p->state){   // so p is statefull
        case S_OPENFLOW: 
            connmgr_run(p->connmgr, handle_openflow) // handle openflow message from controller
            break
        case S_EVICT: 
            connmgr_run(p->connmgr, NULL) // no openflow message
            ofproto_evict(p) // handle too much flow in oftable
            break
        case S_FLUSH:
            connmgr_run(p->connmgr, NULL)
            ofproto_flush__(p) // flush flows of p, if connect to controller
            ...
            break
    }
    
lib/netdev.c netdev_run():
    netdev_initialize
    HMAP_FOR_EACH (rc, hmap_node, &netdev_classes):
        rc->class->run

lib/netdev.c netdev_initialize():
    if (ovsthread_once_start(&once)):
        fatal_signal_add_hook // new
        netdev_vport_patch_register
#ifdef __linux__
        netdev_register_provider(&netdev_linux_class);
        netdev_register_provider(&netdev_internal_class);
        netdev_register_provider(&netdev_tap_class);
        netdev_vport_tunnel_register();
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__)
        netdev_register_provider(&netdev_tap_class);
        netdev_register_provider(&netdev_bsd_class);
#endif
        netdev_dpdk_register();

vswitchd/bridge.c bridge_wait():
    sset_init(&types)
    ofproto_enumerate_types(&types) // get types from all dpif, including new dpif
    SSET_FOR_EACH (type, &types) {
        ofproto_type_wait(type) // call dpif->wait
    }
    sset_destroy(&types)
    for br in all_bridges:
        ofproto_wait(br->ofproto)
    system_stats_wait

lib/netdev.c netdev_wait():
    rc->class->wait


### Upcall handler?

udpif_upcall_handler():
    while():
        read_upcalls // use nl_sock_recv to recv from datapath.
        handle_upcalls // important, Refer to <1>, notice 'resubmit' action.

handle_upcalls():
    HMAP_FOR_EACH (miss, hmap_node, misses):
        handle flow miss // add flow into datapath by dpif_linux_flow_put(miss->xout, ...)
    for (i = 0; i < n_upcalls; i++):
        handle packet in // send packet into datapath by dpif_linux_flow_excute(op->packet, ...)
    if (fail_open):
        ofproto_dpif_send_packet_in(miss->ofproto, pin) // send packet to controller
    dpif_operate(udpif->dpif, opsp, n_ops) // excute ops, include flow_add and packet_in


### Revalidate?


