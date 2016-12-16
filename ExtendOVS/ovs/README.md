### Reference?

1. http://vinllen.com/ovs-facethe-subfacetde-guan-xi/
2. http://www.openvswitch.com/support/dist-docs/ovs-ofctl.8.html


### Pipeline in OVS view?

OVS is openflow switch!!! OVS support openflow feature, such as:
<ul>
    <li>Multi-table: neutron ML2 plugin use mutil-table program method to support dynamic L2 feature by tunnel.</li>
    <li>Flow: Flow with match and actions, refer <2> for flow.</li>
    <li>Group: group is used as actions in flow.</li>
    <li>Meter: Meter is used as actions in flow.</li>
    <li>Bridge/Port: refer spec.</li>
</ul>

Every Bridge is an openflow switch with all feature above. Bridge contains multi-table in ofproto layer holes flows. There are: flow match parse, action parse, flow save("What's ofproto struct to mantain flow?" section) in ofproto layer; ofproto flow to dpif flow dispacth, resubmit("resubmit action support?" section) support in dpif layer.

Openflow is for SDN. In neutron, openflow is only used for ML2 to construct L2 with tunnle, ODL has lots of openflow app.

TTP is not openflow, just a draft. It's for mutil-table programing in switch perspective, different from multi-table program in neutron ML2 plugin.

OVS is for NFV. In neutron, OVS is used in compute node and network node. Except OVS, there are IPTABLE, ROUTER, FIREWALL, DHCP etc in neutron network.


### OVS porting?

netedvXXXX
lcmgrXXXX
XXXX-dpif.X

As ../README.md said, there are few component for this. *lcmgr* is for use sdk code in both static and rpc. netdev-xxxx.x response for pysical port.

There are two type of physical port, one is physical port, the other is lag port. Virtual port is like OVS' netdev-vport.x does.

OVS has two layer flow in user space, first is ofproto, with multi-table, origin flow format, 1:1 mapping of user added flow. The other is dpif, no multi-table, merged flow format, 1:1 mapping of kernel flows. In linux implementation, the dpif layer flow is fetched by netlink command from kernel. Refer to <1> for packet_in process.


### Action extend for L2 MPLS?

lib/ofp-util.c
lib/ofp-util.def
lib/xxxx-ext.h 

These files show how to extend actions in flow mod.


### resubmit action support?

ofproto/ofproto-dpif-xlate.c

resubmit([port],[table]) format. Mutil-table support starts in OVS-2.3, it's done in "xlate_actions", "xlate_resubmit_action", "xlate_table_action", etc. 


### What's normal mode and flow mode?

To setting it, use `ovs-vsctl add-br -- set type=normal` is OK. Differents between normal mode and flow mode, it's how to deal with BUM flow, for normal mode is flood and learn, for flow mode is drop.


### What's ofproto struct to mantain flow?

???


### PATCH to OVS2.3.0?

I have commit a PATCH to OVS2.3.0, titled with "ofp-util: Avoid use-after-free error in ofputil_append_meter_config().", 34965317@qq.com.

As ofpbuf_put_uninit will realloc memory when _alloc_memory is not enough, so return pointer will be change.
