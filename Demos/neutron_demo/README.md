# Refer to

1. openstack_understand_neutron


### What's neutron?

Neutron is focus on "NAAS", which means network as a service, which means network could be shaped and supply service lie QoS as customer needed like through openstack.

For SDN use case, there are CMS and CONTROLLER both, CMS response for setting database like create a bridge, CONTROLLER response for packet/flow/group/meter/etc processing. Basic use of neutron is act as CMS.

For supporting neutron in linux, some network skills are needed in transport layer as bellow. First is "linux bridge", it's a L2 kernel module, which could connect netdev(like eth0 or vm0). it's just like a hub, No need to develop linux bridge.

Second is "openvswitch" as ovs, for a hypervisor, there are commonly three br, which is br-eth0 br-int and br-ex. For br-eth0 or br-vxlan or br-vm0, this bridge response for process packet from or to the eth0 device or vxlan tunnel or vm0 etc. For br-int, this bridge response for internal connection with vms on this hypervisor, and vlan translate from and to br-ex. For br-ex, this bridge response for packet from or to extenal network. To connect these bridges, ovs use pair type port, which is VETH. The neutron/agent/common/ovs_lib.py do these works.

TAP/TUN device, TAP device is mirror device, TUN device is tunnel device, but how to use it??? And where is the file???

For FaaS use, iptables is needed, also iptables is used for SNAT nad DNAT. The iptables_firewall.py do these works.

For network resource devide, namespace is needed. The namespaces.py and namespaces_manager.py is response for these.



