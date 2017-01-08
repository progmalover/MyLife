### Normal OVS?

Normal OVS: init chain and thread, configure and tables and related API, packet flow. Refer to ARCHITECTURE.md .


### Introduction?

<ul>
    <li>OVS Extend:
        <ul>
            <li>Porting by switch chipset.</li>
            <li>RPC.</li>
            <li>Extend MPLS action.</li>
            <li>Resubmit implementation by OVS and by chipset.</li>
            <li>Local stack port and CPU port implementation.</li>
            <li>QoS, tunnel port implementation.</li>
            <li>Direct flow put.</li>
            <li>Vlan.</li>
        </ul>
    </li>
    <li>Controller Extend:
    </li>
    <li>Neutron Extend:
    </li>
</ul>


### Porting by switch chipset?

OVS is portable into switch chips, to do this, we have to first prepare SDK interface for OVS as ./sdk folder said; then we have to code netdev-XXX and dpif-XXX layer as ./ovs folder said.


### RPC?

In configure layer, we use static and RPC method, refer to ./ovs/lcmgr-shared.c
