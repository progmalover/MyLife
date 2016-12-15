### Basic datapath flow?

                   --------------
                   |excute      |
                   |PUT/DEL/EXEC|
                   |actions     |
                   |here        |
                   --------------
                        |
handle_upcalls ---- dpif_operate ---- dpif_linux_flow_put -----------------------
    |                   |                                                       |
run_fast                ------------- dpif_linux_flow_excute ------------       |
    |                                                                   |       |
dpif_recv ---------------------------------------                       |       |
                                                |                       |       |
User Space:                                     |                       |       |
================================================|=======================|=======|
Kernel Space:                                   |                       |       |
                                                |                       |       |
@SKB packet_in                                  |                       |       |
        |                                       |                       |       |
        |                                       |                       |       |
@FUNC ovs_vport_receive                 @FUNC genlmsg_unicast           |       |
        |                                       |                       |       |
        |                                       |                       |       |
@FUNC ovs_dp_process_receive_packet     @FUNC queue_userspace_packet    |       |
        |                                       |                       |       |
        |                               N       |                       |       |
@FUNC ovs_flow_tbl_lookup ---- match? ---- @FUNC ovs_dp_upcall          |       |
        |                       | Y                                     |       |
        |                       |                                       |       |
        |           @FUNC ovs_excute_actions <-- @FUNC ovs_packet_cmd_excute    |
        |                       |                                               |
        |                       |                                               |
        |                  -------------        @FUNC ovs_flow_cmd_new_or_set ---
        |                  |output     |                        |
        GET                |userspace  |-->@SKB packet_out      |
        |                  |vlan action|                        |
        |                  -------------                        |
       \ /                             NEW/SET                  |
@GLOBAL exact-flow-table <---------------------------------------

### How to get/send packet in datapath?

How to receive packet before linux stack? How to send packet to linux stack by local port?
