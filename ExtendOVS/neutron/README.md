# Refer to

1. openstack_understand_neutron
2. http://linux-ip.net/pages/documents.html
3. https://github.com/yeasy/openstack_code_Neutron
4. http://www.cnblogs.com/sammyliu/p/4633814.html


### Notice?

IR: in neutron, IR means route instance of hypervisor, which means kernel TCPIP route module with namespace.


### What's neutron?

Neutron is focus on "NAAS", which means network as a service, which means network could be shaped and supply service like QoS as customer needed like through openstack.

For SDN use case, there are CMS and CONTROLLER both, CMS response for setting database like create a bridge, CONTROLLER response for packet/flow/group/meter/etc processing. Basic use of neutron is act as CMS.

For supporting neutron in linux, some network skills are needed in transport layer as bellow. First is "linux bridge", it's a L2 kernel module, which could connect netdev(like eth0 or vm0). it's just like a hub, No need to develop linux bridge.

Second is "openvswitch" as ovs, for a hypervisor, there are commonly three br, which is br-eth0 br-int and br-ex. For br-eth0 or br-vxlan or br-vm0, this bridge response for process packet from or to the eth0 device or vxlan tunnel or vm0 etc. For br-int, this bridge response for internal connection with vms on this hypervisor, and vlan translate from and to br-ex. For br-ex, this bridge response for packet from or to extenal network. To connect these bridges, ovs use pair type port, which is VETH. The neutron/agent/common/ovs_lib.py do these works.

TAP/TUN device, TAP device is mirror device, TUN device is tunnel device, but how to use it??? And where is the file???

For FaaS use, iptables is needed, also iptables is used for SNAT nad DNAT. The iptables_firewall.py do these works.

For network resource devide, namespace is needed. The namespaces.py and namespaces_manager.py is response for these.


### Topology of neutron network in compute node and network node?

See <1>P8, compute node responses for connection between vm and network. For GRE mode, security layer is qbr-XXX with access control flow and br-int to connect. For VLAN mode, integration layer is br-tun with vlan mapping flows.

Network node responses for network like L3 routing for subnet and outside, DHCP for vm, SNAT/DNAT etc. After use DVR, vm with floating IP will send packet to public IP directly through compute node.


### DHCP?

Openstack use "dnsmasq" to DHCP. "dnsmasq" bind its interface to br-int in network node.


### Routing Agent of network node and DVR?

For linux kernel, a flow want routing will search local route table first, if miss, the flow will use default route and its interface. For compute node without DVR, the default route is network node. For compute node with DVR, if the vm has floating IP, compute node's route table will has a default route entry with this floating IP as src IP and an output interface associate. How to construct this is unknown??? But I think it's done in neutron floating IP module, it could get the floating IP from configures, and then add this default route into compute node. <1>P62 said route is added not into router module of kernel, but iptables, both on compute node and network node, it's mainly because there are few route, so route table size and its search speed is not problem.

For network node without DVR, route module is response for flow between north and sourth. Also DNAT nad SNAT is here. For flow goes into vm, flow will be DNAT first in INPUT chain of iptables in network node, and then route in FORWARD chain of iptables, or route in vrouter if have. For flow comes from vm, flow will be route first, and then SNAT in OUTPUT chain.

Neutron l3 code is located in neutron/agent/l3/, and its init is located in neutron/agent/l3_agent.py . In neutron/agent/l3/, there are agent.py response for route work; config.py response for configure; dvr_XXX.py response for dvr related; ha_XXX.py and keepalive.py response for HA; namespace_XXX.py response for namespace???; lagency_router.py response for add_floating_ip; route_info.py is response for routing information; router_process_queue may be response for queue the route request, these request may be comes from CMS and to configure the route informations in network node; others known???.

All command at last is excute by "ip_wrapper.netns.execute(cmd, check_exit_code=False)" in router_info.py, which is same as <1>P63 "sudo ip net exec ...", just like WebOVS does.


### Routing Service?

Routing Service is to supply routing feature, located in neutron/service/l3_router/XXX, compute route through configure is here.


### Floating IP and DVR in compute node?

As said above, when enable DVR and floating IP, compute node with IR will process traffic between vm in different subnet, and traffic from vm with floating IP to outside. And network node just need to process traffic of vm without floating IP in different subnet.

So after using DVR in vm, traffic between vm in same hypervisor is bridged by local ovs bridge, traffic between vm in different hypervisor is route by IR, which is local vrouter; traffic between vm and outside is route bu IR.

These is done by neutron/agent/l3/dvr_router.py .


### L2 pop and multi-table programing?

L2 pop is in /neutron/plugins/ml2/, is focus on contruct L2 network using tunnel or vlan, supporting ovs, linux bridge, or Hyper-V L2.


### BR-TUN flow table in non-DVR mode?

For ovs, <4> shows l2pop and multi-table, neutron as controller devide tables in BR-TUN as bellow. Notice, BR-TUN act as tunnel gateway, so do not need to process L2 local flow.
<ul>
    <li>table 0: entry table of flow, used for devide flow by in_port, which devide flow by source, as flow comes from VM or network node. Controller have all config of network map, so it knows what's network node port.</li>
    <li>table 1, DVR_PROCESS: enable and used for DVR mode.</li>
    <li>table 2, PATCH_LV_TO_TUN: </li>
    <li>table 3, GRE_TUN_TO_LV: </li>
    <li>table 4, VXLAN_TUN_TO_LV: </li>
    <li>table 5, DVR_NOT_LEARN: </li>
    <li>table 10, LEARN_FROM_TUN: learn table for local VM's mac address of this vxlan network. Its flow is: "table=10,priority=1 actions=learn(table=20,hard_timeout=300,priority=1,NXM_OF_VLAN_TCI[0..11],NXM_OF_ETH_DST[]=NXM_OF_ETH_SRC[],load:0->NXM_OF_VLAN_TCI[],load:NXM_NX_TUN_ID[]->NXM_NX_TUN_ID[],output:NXM_OF_IN_PORT[]),output:1"</li>
    <li>table 20, UCAST_TO_TUN: this is learned flow from table 10, notice here is GRE mode. Its flow is: "table=20, priority=2,dl_vlan=1,dl_dst=fa:16:3e:7e:ab:cc actions=strip_vlan,set_tunnel:0x3e9,output:5", that is if vlan==1 and dst_mac==fa:16:3e:7e:ab:cc, then set tunnel id and output to 5, it's same as vxlan network, network port will learn remote mac address of remote VM.</li>
    <li>table 21, ARP_RESPONDER: </li>
    <li>table 22, FLOOD_TO_TUN: used to proccess BUM flow.</li>
</ul>


### ARP Responser?

ARP Responser located in BR-TUN, which is ARP_RESPONSER table.
