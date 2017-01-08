// All c file have to include <config.h> first in linux.
#include <config.h>

// include lcmgr-provider for "struct lcmgr_class"
#include "lcmgr-provider.h"

// This is usefull when debug, you could switch debug log of this module
// like `ovs-appctl vlog-set lcmgr DEBUG`
VLOG_DEFINE_THIS_MODULE(lcmgr);

// Bellow is to code for apis in lcmgr.h, the lcmgr is register when startup.
static const struct lcmgr_class *lcmgr = NULL;

void
lcmgr_register_provider(const struct lcmgr_class *lc)
{
    // Can register only once.
    ovs_assert(lcmgr == NULL);

    lcmgr = lc;

    //ignore(vlog_set_levels_from_string("lcmgr:DBG"));
}

// This is demo, others are same.
bool
lcmgr_api_init(bool openflow, int32_t *slot_id)
{
    VLOG_DBG("%s", __FUNCTION__);

    if (!lcmgr->init) {
        return true;
    }
    if (!lcmgr->init(openflow, slot_id)) {
        VLOG_ERR("Fail to %s", __FUNCTION__);
        return false;
    }
    return true;
}