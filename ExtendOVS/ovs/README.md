### OVS porting?

As ../README.md said, there are few component for this. *lcmgr* is for use sdk code in both static and rpc. netdev-xxxx.x response for pysical port.

There are two type of physical port, one is physical port, the other is lag port. Virtual port is like OVS' netdev-vport.x does, so omit it.