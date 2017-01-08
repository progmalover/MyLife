#ifndef XXXX_EXT_H
#define XXXX_EXT_H

#include "openflow/nicira-ext.h"

#define XXX_VENDOR_ID_LENGTH 4
#define PXM_HEADER(VENDOR, FIELD, LENGTH) \
    NXM_HEADER__(VENDOR, FIELD, 0, (LENGTH))
#define PXM_HEADER_W(VENDOR, FIELD, LENGTH) \
    NXM_HEADER__(VENDOR, FIELD, 1, (LENGTH)*2)

enum xx_action_subtype {
    XXXX_PUSH_L2MPLS,
    XXXX_POP_L2MPLS,
};

// define STRUCT used in ofp-util.def
struct xx_action_push_l2mpls {
    ovs_be16 type;                  /* OFPAT11_EXPERIMENTER */
    ovs_be16 len;                   /* Length is 16. */
    ovs_be32 vendor;                /* PICA8_VENDOR_ID. */
    ovs_be16 subtype;               /* PXAST_PUSH_L2MPLS. */
    ovs_be16 ethertype;             /* Ethertype. */
    uint8_t pad[4];
};
OFP_ASSERT(sizeof(struct xx_action_push_l2mpls) == 16);

struct xx_action_pop_l2mpls {
    ovs_be16 type;                  /* OFPAT11_EXPERIMENTER */
    ovs_be16 len;                   /* Length is 16. */
    ovs_be32 vendor;                /* PICA8_VENDOR_ID. */
    ovs_be16 subtype;               /* PXAST_PUSH_L2MPLS. */
    uint8_t pad[6];
};
OFP_ASSERT(sizeof(struct xx_action_pop_l2mpls) == 16);


#endif
