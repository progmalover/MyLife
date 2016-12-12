### OVS porting?

As ../README.md said, there are few component for this. *lcmgr* is for use sdk code in both static and rpc. netdev-xxxx.x response for pysical port.

There are two type of physical port, one is physical port, the other is lag port. Virtual port is like OVS' netdev-vport.x does.


### What's normal mode and flow mode?

To setting it, use `ovs-vsctl add-br -- set type=normal` is OK. Differents between normal mode and flow mode, it's how to deal with BUM flow, for normal mode is flood and learn, for flow mode is drop.
