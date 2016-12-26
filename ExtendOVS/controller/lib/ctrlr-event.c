// Basic run flow is:
//
// MainThread   EventThread     Event
//      |       ^   |           |
//      |-------|   |           |
//      | create    |           |
//      |           |           |
//      |---------->|           |
//      | register  |           |
//      | callback  |           |
//      | handler   |           |
//      |           |           |
//      |<----------------------|
//  store event     |           |
//  into *event_cfg |           |
//  by              |           |
//  'generate_event'|           |
//      |---------->|           |
//      |           |           |
//      |       Process event   |
//      |       by              |
//      |       'process_event' |
//      |       use callback    |
//      |       handler         |
//      |           |---------->|
//      |           |           |

// This is basic implementation, advanced will be:
// `ThreadPool
// `Proact

#include "ctrlr-event.h"
#include "ctrlr-state.h"
#include "util.h"
#include "vlog.h"
#include "bitmap.h"

VLOG_DEFINE_THIS_MODULE(ctrl_event);

// this is event storage. For *event_cfg, there are kinds of mutex:
// 1. 'event_mutex' to protect access of '*event_cfg'.
// 2. 'wake_cond' for child thread who hold '*event_cfg' to notice other
// child thread to access '*event_cfg'.
// 3. 'exit_latch' for parent threaf to notice child thread to leave and
// exit.
static struct ovs_mutex event_mutex = OVS_MUTEX_INITIALIZER;
static event_cfg_data *event_cfg OVS_GUARDED_BY(event_mutex);

// register_event_callback:
// Register 'switch_event_data* sw_list' into 'event_cfg_data *event_cfg'
// @Param sw_list: indicate module_id, callback function to process event
//      and event_mask for event this module cared.
bool
register_event_callback(switch_event_data* sw_list)
{
    if (sw_list == NULL) {
        VLOG_ERR(" %s: registrar_parameter is NULL\n", __FUNCTION__);
        return false;
    }

    if (sw_list->module_id >= CTRLR_LAST_MODULE_ID
               || sw_list->module_id <= CTRLR_FIRST_MODULE_ID) {
        VLOG_ERR(" %s: module (%u) is outside valid range\n",
                            __FUNCTION__, sw_list->module_id);
        return false;
    }


    ovs_mutex_lock(&event_mutex);

    if (event_cfg == NULL) {
        VLOG_ERR(" %s: event module uninitialized\n", __FUNCTION__);
        ovs_mutex_unlock(&event_mutex);
        return false;
    }

    if (event_cfg->notify_list_size >= CTRLR_LAST_MODULE_ID) {
        VLOG_ERR(" %s: event_cfg is full\n", __FUNCTION__);
        ovs_mutex_unlock(&event_mutex);
        return false;
    }
    event_cfg->switch_event_list[event_cfg->notify_list_size].module_id
                                    = sw_list->module_id;
    event_cfg->switch_event_list[event_cfg->notify_list_size].notify_event
                                    = sw_list->notify_event;
    memcpy(event_cfg->switch_event_list[event_cfg->notify_list_size].event_mask,
                    sw_list->event_mask,sizeof(unsigned long)*SWITCH_EVENT_SIZE);
    event_cfg->notify_list_size++;

    ovs_mutex_unlock(&event_mutex);

    return true;

}

bool
deregister_event_callback(CTRLR_MODULE_ID  module_id)
{
    uint32_t i;
    bool found = false;

    if (module_id >= CTRLR_LAST_MODULE_ID
               || module_id <= CTRLR_FIRST_MODULE_ID) {
        VLOG_ERR(" %s: module (%u) is outside valid range\n",
                            __FUNCTION__, module_id);
        return false;
    }


    ovs_mutex_lock(&event_mutex);

    if (event_cfg == NULL) {
        VLOG_ERR(" %s: event module uninitialized\n", __FUNCTION__);
        ovs_mutex_unlock(&event_mutex);
        return false;
    }

    for (i = 0; i < event_cfg->notify_list_size; i++) {
        if (event_cfg->switch_event_list[i].module_id == module_id) {
            event_cfg->notify_list_size--;
            for (; i < event_cfg->notify_list_size; i++) {
                event_cfg->switch_event_list[i]
                   = event_cfg->switch_event_list[i + 1];
            }
            found = true;
        }
    }

    ovs_mutex_unlock(&event_mutex);
    if(!found) {
        VLOG_ERR(" %s: module (%u) didn't register event\n", __FUNCTION__, module_id);
    }
    return false;

}

// garbage_event_data is used in process_event.
void
garbage_event_data(event_elem_data *event_elem)
{
    if(event_elem->event_data.data_length && event_elem->event_data.data != NULL) {
        free(event_elem->event_data.data);
    }
    free(event_elem);
}

// generate_event:
// This is called by main thread or front thread to add event into *event_cfg.
bool
generate_event(ctrlr_event_data *event)
{
    event_elem_data* new_elem;

    if (event->module_id >= CTRLR_LAST_MODULE_ID
               || event->module_id <= CTRLR_FIRST_MODULE_ID) {
        VLOG_ERR(" %s: module (%u) is outside valid range\n",
                            __FUNCTION__, event->module_id);
        return false;
    }

    ovs_mutex_lock(&event_mutex);

    if (event_cfg == NULL) {
        VLOG_ERR(" %s: event module uninitialized\n", __FUNCTION__);
        ovs_mutex_unlock(&event_mutex);
        return false;
    }
    ovs_mutex_unlock(&event_mutex);

    new_elem = (event_elem_data*)xmalloc(sizeof(*new_elem));
    if(new_elem == NULL) {
        VLOG_ERR(" %s: malloc memory failed\n", __FUNCTION__);
        return false;
    }
    list_init(&new_elem->event_list);
    new_elem->event_data = *event;
    if(event->data_length) {
        new_elem->event_data.data = (uint8_t *)xmalloc(event->data_length);
        if(new_elem->event_data.data == NULL) {
            VLOG_ERR(" %s: alloc memory failed\n", __FUNCTION__);
            garbage_event_data(new_elem);
            return false;
        }
        memcpy(new_elem->event_data.data, event->data, event->data_length);
    }

    ovs_mutex_lock(&event_mutex);
    list_push_back(&event_cfg->event_list, &new_elem->event_list);
    event_cfg->n_events++;
    xpthread_cond_signal(&event_cfg->wake_cond);
    ovs_mutex_unlock(&event_mutex);

    return true;
}

bool
init_event_cfg()
{
    ovs_mutex_lock(&event_mutex);
    if(event_cfg != NULL) {
        VLOG_ERR(" %s: reload the event task", __FUNCTION__);
        ovs_mutex_unlock(&event_mutex);
        return false;
    }
    event_cfg = xmalloc(sizeof(*event_cfg));
    if(event_cfg == NULL) {
        VLOG_ERR(" %s: alloc memory failed", __FUNCTION__);
        ovs_mutex_unlock(&event_mutex);
        return false;
    }
    list_init(&event_cfg->event_list);

    event_cfg->notify_list_size = 0;
    bzero(event_cfg->switch_event_list, sizeof(switch_event_data)*CTRLR_LAST_MODULE_ID);

    xpthread_cond_init(&event_cfg->wake_cond, NULL);
    latch_init(&event_cfg->exit_latch);
    ovs_mutex_unlock(&event_mutex);

    return true;
}

bool
destroy_event_cfg()
{
    ovs_mutex_lock(&event_mutex);
    if(event_cfg == NULL) {
        VLOG_ERR(" %s: event config is NULL", __FUNCTION__);
        ovs_mutex_unlock(&event_mutex);
        return false;
    }
    while(list_is_empty(&event_cfg->event_list) == false) {
        event_elem_data* event_elem = (event_elem_data*) list_pop_front(&event_cfg->event_list);
        garbage_event_data(event_elem);
    }
    free(event_cfg);
    event_cfg = NULL;
    ovs_mutex_unlock(&event_mutex);
    return true;
}

// Run in event thread to dispatch event to its handler, and event thread will
// call callback function to process '&event_elem->event_data'.
// After this, call 'update_bridge_state' to update bridge state, this is
// uneccessary.
bool
process_event(void *arg)
{
    uint32_t i;
    event_cfg_data *handler = arg;

    /*mutex has been take by threads*/
    ovs_mutex_lock(&event_mutex);
    if (list_is_empty(&event_cfg->event_list) == false) {
        event_elem_data* event_elem;

        event_elem = (event_elem_data*) list_pop_front(&event_cfg->event_list);
        ovs_mutex_unlock(&event_mutex);

        for (i = 0; i < event_cfg->notify_list_size; i++) {
            if(bitmap_is_set(event_cfg->switch_event_list[i].event_mask,
                             event_elem->event_data.event_id)) {
                VLOG_DBG("%s:begin to handle event %s", __FUNCTION__,
                          event_string(event_elem->event_data.event_id));
                event_cfg->switch_event_list[i].notify_event(&event_elem->event_data);
                VLOG_DBG("%s:event %s is handled", __FUNCTION__,
                          event_string(event_elem->event_data.event_id));
            }
        }
        if (!update_bridge_state(&event_elem->event_data.bridge_id,
                                 event_elem->event_data.event_id)) {
            //VLOG_WARN("Update bridge state error.");
        }
        garbage_event_data(event_elem);
        handler->n_events--;
    } else {
        ovs_mutex_unlock(&event_mutex);
    }

    return true;
}

// Main thread to process event, call this thread 'event thread'.
static void *
ctrl_event_handler(void *arg)
{
    event_cfg_data *handler = arg;

    set_subprogram_name("event_handler");

    while (!latch_is_set(&handler->exit_latch)) {
        ovs_mutex_lock(&event_mutex);
        if (!handler->n_events) {
            ovs_mutex_cond_wait(&handler->wake_cond, &event_mutex);
        }
        ovs_mutex_unlock(&event_mutex);

        process_event(arg);
    }

    return NULL;
}

// This is main entry to start event thread.
bool
start_event_task(void)
{
    if (!init_event_cfg()) {
        VLOG_ERR("init event cfg failed\n");
        return false;
    }
    if (!init_state_cfg()) {
        VLOG_ERR("init state cfg failed\n");
        destroy_event_cfg();
        return false;
    }
    xpthread_create(&event_cfg->thread, NULL, ctrl_event_handler,
                     event_cfg);

    return true;
}

bool
stop_event_task(void)
{
    ovs_mutex_lock(&event_mutex);
    latch_set(&event_cfg->exit_latch);
    xpthread_cond_signal(&event_cfg->wake_cond);
    ovs_mutex_unlock(&event_mutex);

    xpthread_join(event_cfg->thread, NULL);
    destroy_event_cfg();
    destroy_state_cfg();
    return true;
}

char* event_string(CTRLR_EVENTS event)
{
    switch(event) {
    case E_IP_DOWN:
        return "E_IP_DOWN";

    case E_IP_UP:
        return "E_IP_UP";

    case E_DB_FAIL:
        return "E_DB_FAIL";

    case E_DB_OK:
        return "E_DB_OK";

    case E_DB_CHANGED:
        return "E_DB_CHANGED";

    case E_CTRL_CHANGED:
        return "E_CTRL_CHANGED";

    case E_SW_CHANGED:
        return "E_SW_CHANGED";

    case E_TOPO_CHANGED:
        return "E_TOPO_CHANGED";

    case E_SDR_CHANGED:
        return "E_SDR_CHANGED";

    case E_AUTHORIZE_FAIL:
        return "E_AUTHORIZE_FAIL";

    case E_AUTHORIZE_OK:
        return "E_AUTHORIZE_OK";

    case E_OF_STATIC_FAIL:
        return "E_OF_STATIC_FAIL";

    case E_OF_STATIC_OK:
        return "E_OF_STATIC_OK";

    case E_OF_DYNAMIC_FAIL:
        return "E_OF_DYNAMIC_FAIL";

    case E_OF_CONNECTION_FAIL:
        return "E_OF_CONNECTION_FAIL";

    case E_OF_CONFIG_CHANGED:
        return "E_OF_CONFIG_CHANGED";

    case E_FLOW_REMOVED:
        return "E_FLOW_REMOVED";
        break;

    case E_FLOW_ADDED:
        return "E_FLOW_ADDED";

    case E_PORT_DOWN:
        return "E_PORT_DOWN";
        break;

    case E_PORT_UP:
        return "E_PORT_UP";

    case E_METER_REMOVED:
        return "E_METER_REMOVED";

    case E_GROUP_REMOVED:
        return "E_GROUP_REMOVED";

    case E_PACKET_IN:
        return "E_PACKET_IN";

    case E_BRIDGE_CREATE:
        return "E_BRIDGE_CREATE";

    case E_BRIDGE_DELETE:
        return "E_BRIDGE_DELETE";

    case E_BRIDGE_STOP:
        return "E_BRIDGE_STOP";

    case E_PROBE_ALL_SWITCHES:
        return "E_PROBE_ALL_SWITCHES";

    case E_ARP_RESET_BRIDGE_ITEM:
        return "E_ARP_RESET_BRIDGE_ITEM";

    case E_ARP_SET_ITEM_MAC:
        return "E_ARP_SET_ITEM_MAC";

    case E_ARP_ADD_ITEM:
        return "E_ARP_ADD_ITEM";

    case E_ARP_DEL_ITEM:
        return "E_ARP_DEL_ITEM";

    case E_ARP_ITEM_ALIVE:
        return "E_ARP_ITEM_ALIVE";

    case E_ARP_REMOVE_ROUTE:
        return "E_ARP_REMOVE_ROUTE";

    case E_ARP_UPDATE_BGP_ROUTE:
        return "E_ARP_UPDATE_BGP_ROUTE";

    case E_UPDATE_LLDP:
        return "E_UPDATE_LLDP";


    case E_MAX_EVENTS:
    default:
        return "UNKNOWN EVENT";
    }
}
