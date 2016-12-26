
#ifndef CTRL_EVENT_H
#define CTRL_EVENT_H 1

#include <config.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "ctrlr-commons.h"
#include "latch.h"
#include "ovs-thread.h"
#include "list.h"

typedef enum {
    E_IP_DOWN = 0,                   /* E1, the switch become down */
    E_IP_UP,                              /* E2, the switch become up */
    E_DB_FAIL,                          /* E3, failed to configure the ovs db */
    E_DB_OK,                             /* E4, ovs db has been configured success */
    E_DB_CHANGED,                   /* E5, ovs db changed */
    E_CTRL_CHANGED,                /* E6, ovs db controller column changed (omit tables) */
    E_SW_CHANGED,                  /* E7, ovs db swirtch clumn changed */
    E_TOPO_CHANGED,               /* E8, ovs db topo clumn changed */
    E_SDR_CHANGED,               /* E9, ovs db xorp clumn changed */

    E_AUTHORIZE_FAIL,              /*E10, authorize failed*/
    E_AUTHORIZE_OK,                 /*E11, authorize OK*/
    E_OF_STATIC_FAIL,               /*E12, set static flow fail*/
    E_OF_STATIC_OK,                 /*E13, set static flow OK*/
    E_OF_DYNAMIC_FAIL,           /*E14, dynamic failed*/
    E_OF_CONNECTION_FAIL,      /*E15, connection broken*/
    E_OF_CONFIG_CHANGED,      /*E16, template changed */

    E_FLOW_REMOVED,               /*E17, of protocol event*/
    E_FLOW_ADDED,                   /*E18, of protocol event,extended message*/
    E_PORT_DOWN,                     /*E19, of protocol event*/
    E_PORT_UP,                          /*E20, of protocol event*/
    E_METER_REMOVED,              /*E21, of protocol event*/
    E_GROUP_REMOVED,             /*E22, of protocol event*/
    E_PACKET_IN,                       /*E23, this event shouldn't be used.*/

    E_BRIDGE_CREATE,                /*E24, add bridge*/
    E_BRIDGE_DELETE,                /*E25, delete bridge*/
    E_BRIDGE_STOP,                   /* E26, bridge stopped */

    E_PROBE_ALL_SWITCHES,      /* E27, probe all swithes */
    E_ARP_RESET_BRIDGE_ITEM,  /* E28, reset bridge arp item */
    E_ARP_SET_ITEM_MAC,          /* E29, set arp item mac */
    E_ARP_ADD_ITEM,                  /* E30, add a arp item */
    E_ARP_DEL_ITEM,                  /* E31, delete a arp item */
    E_ARP_ITEM_ALIVE,              /* E32, remove arp bgp routes */
    E_ARP_REMOVE_ROUTE,       /* E33, remove arp bgp routes */
    E_ARP_UPDATE_BGP_ROUTE,       /* E34, update arp bgp routes */

    E_UPDATE_LLDP,                   /* E35, update lldp info for topology */

    E_MAX_EVENTS
} CTRLR_EVENTS;

#define SWITCH_EVENT_SIZE   ((E_MAX_EVENTS / 32) + 1)

typedef struct {
    CTRLR_EVENTS event_id;
    CTRLR_MODULE_ID module_id;
    ctrlr_bridge_id bridge_id;
    uint32_t  data_length;
    uint8_t*  data;
} ctrlr_event_data;

typedef struct {
    CTRLR_MODULE_ID  module_id; // module id in 'PICA_MODULE_ID'.
    // callback function of registered module.
    uint32_t  (*notify_event)(ctrlr_event_data *event);
    // event mask.
    unsigned long  event_mask[SWITCH_EVENT_SIZE];
} switch_event_data;

typedef struct {
    struct list        event_list; // store event into list.
    // an array(iter is module id) hold 'switch_event_data'
    switch_event_data switch_event_list[CTRLR_LAST_MODULE_ID];
    // one module register to 'event_cfg_data' will increase 'notify_list_size'
    // until 'CTRLR_LAST_MODULE_ID' max value.
    uint32_t          notify_list_size;
    // Race condition for all thread to access this struct.
    pthread_cond_t wake_cond;
    // Parent thread set exit_latch to notice child thread. Here parent thread
    // is main thread who call 'start_event_task', child thread is event thread.
    struct latch exit_latch;           /* Tells child threads to exit. */
    // Parent thread ID, for child thread use.
    pthread_t thread;                  /* Thread ID. */
    // Increase for new event comes.
    uint32_t  n_events;

} event_cfg_data;

typedef struct {
    struct list event_list;
    ctrlr_event_data event_data;
} event_elem_data;

bool init_event_cfg(void);
bool register_event_callback(switch_event_data* sw_list);
bool deregister_event_callback(CTRLR_MODULE_ID  module_id);
bool generate_event(ctrlr_event_data *event);
void garbage_event_data(event_elem_data *event_elem);
bool process_event(void *arg);
bool destroy_event_cfg(void);
bool start_event_task(void);
bool stop_event_task(void);

char* event_string(CTRLR_EVENTS event);

#endif /*CTRL_EVENT_H*/
