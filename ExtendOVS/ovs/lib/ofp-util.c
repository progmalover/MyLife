// This is to extend new action, it's like TTP_ROUTE_TABLE_ENTRY_INC etc.
//
// First "names" contains all ENUM-STRUCT-NAME, just like
// "static uint32_t cur_l3_ipv6_route_table_entries = 0".
//
// Then supply two funtion, "void ofputil_init_##ENUM(struct STRUCT)"
// and "struct STRUCT *ofputil_put_##ENUM(struct ofpbuf*)" for use.
//
// Last it's used in ofp-actions.c .

#ifdef XXXX
// include this header file for STRUCT of XXXX_ACTION.
#include "xxxx-ext.h"
#endif

/* ofp-util.def lists the mapping from names to action. */
// This is all actions used. After macro extend, this array is:
// static const char *const names[1] = { // whether names and its value could NOT change.
//     NULL, // names[0] == NULL
//     ...,
//     0xascbsdsad, // names[n] = point to "push_l2_mpls"
//     ....
// };
// Here just contain its name, real operator is located in
// ofputil_xxxx_push_l2_mpls etc funstion.
static const char* const names[OFPUTIL_N_ACTIONS] = {
    NULL,
    #define OFPAT10_ACTION(ENUM, STRUCT, NAME)             NAME,
    #define OFPAT11_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME) NAME,
    #define OFPAT13_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME) NAME,
    #define NXAST_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME)   NAME,
#ifdef XXXX
    #define XXXX_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME)   NAME,
#endif
    #include "ofp-util.def"
};

// To use names, we have to define some funtion:
// Give action name by *name, get action code like XXXX_PUSH_L2MPLS
int
ofputil_action_code_from_name(const char *name);

const char *
ofputil_action_name_from_code(enum ofputil_action_code code);

enum ofputil_action_code
ofputil_action_code_from_ofp13_action(enum ofp13_action_type type);

// This is main entry function to use names, this function decode *buf by code
// and then return a pointer to "struct xx_action_push_l2mpls".
//
// So user call this function like this:
// struct xx_action_push_l2mpls *s = ofputil_put_action(PXAST_PUSH_L2MPLS, *buf);
void *
ofputil_put_action(enum ofputil_action_code code, struct ofpbuf *buf){
    switch(code) {
        ...
#ifdef XXXX
// we use XXXX_ACTION here.
#define XXXX_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME) \
        case OFPUTIL_##ENUM: return ofputil_put_##ENUM(buf);
#endif
    }
}

#ifdef XXXX
// This is to define XXXX_ACTION macro into ofputil_init_ENUM function.
// After extend, it's:
// void
// ofputil_init_XXXX_PUSH_L2MPLS(struct xx_action_push_l2mpls *s) {
//     fill content by *s, "struct xx_action_push_l2mpls" is defined in header
//     file, basically it's memory with ovsbe32 and ovsbe16 content.
// }
//
// ofputil_put_XXXX_PUSH_L2MPLS(struct ofpbuf *buf) { \
//     fill "struct xx_action_push_l2mpls" by buf.
// }
#define XXXX_ACTION(ENUM, STRUCT, EXTENSIBLE, NAME) \
    void
    ofputil_init_##ENUM(struct STRUCT *s) \
    { \
        memset(s, 0, sizeof *s); \
        s->type = htons(OFPAT11_EXPERIMENTER); \
        s->len = htons(sizeof *s); \
        s->vendor = htonl(XXXX_VENDOR_ID); \
        s->subtype = htons(ENUM); \
    }

    struct STRUCT * \
    ofputil_put_##ENUM(struct ofpbuf *buf) \
    { \
        struct STRUCT *s = ofpbuf_put_uninit(buf, sizeof *s); \
        ofputil_init_##ENUM(s); \
        return s; \
    }
#endif
