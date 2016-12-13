#ifndef XXXX_EXT_H
#define XXXX_EXT_H

#include "openflow/nicira-ext.h"

#define PXM_VENDOR_ID_LENGTH 4
#define PXM_HEADER(VENDOR, FIELD, LENGTH) \
    NXM_HEADER__(VENDOR, FIELD, 0, (LENGTH))
#define PXM_HEADER_W(VENDOR, FIELD, LENGTH) \
    NXM_HEADER__(VENDOR, FIELD, 1, (LENGTH)*2)



#endif
