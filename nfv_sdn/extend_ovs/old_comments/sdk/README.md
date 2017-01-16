### Pipeline?

To code for net device of xxx switch chip, you have first sign a confidentiality agreement, then you have to know the pipeline of the chip, and then use its sdk.

Basic pipeline is as bellow, chip is more complicated then this, for example, L3 ECMP interface could only assign to L3 table not TCAM table, tunnel must process before vlan, and etc.

Ingress -- Ingress -- Vlan ---- Termination -- Unicast -- TCAM -- Action --- Egress
Port       Port       Table     Mac            Routing     |      Set     |  Port
  |     |- Table                Table          Table       |        |     |     |
  |     |   |                        |                     |        |     |     |
  |     |   |                        |                     |        |   Tunnel -|
Tunnel -|   |                        |-------- Multicast --|        |   Interface
Interface   |                        |         Routing     |        |
            |                        |         Table       |        |
            |                        |------|              |        |
            |                               |              |        |
            |--------------------------------- FDB --------|  Group Table Entry
                                               Table            L2 interface
                                                                L2 multicast
                                                                L2 flood
                                                                L3 interface
                                                                L3 multicast
                                                                L3 ECMP
                                                                L2 overlay
                                                                L2 overlay multicast
                                                                L2 overlay flood

After pipeline, we have to wrapper chipset's sdk for net device like router and switch, here we discuss OVS.


### how to code?

The xxx_sdk.x is basic sdk code of chipset, in these files we wrapper sdk code for some function for router usage. The ctype_api.x is C type wrapper for OVS usage.
