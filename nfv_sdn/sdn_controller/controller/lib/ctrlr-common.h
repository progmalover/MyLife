
#ifndef CTRL_COMMONS_H
#define CTRL_COMMONS_H 1

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "util.h"
#include "list.h"
#include "smap.h"
#include "packets.h"

#define IPV4_LEN 4
#define IPV6_LEN 16
#define MAX_IP_LEN 16

#define MAC_ADDR_LEN  6
#define MAX_BRIDGE_NAME_SIZE 128

typedef enum {
    CTRLR_FIRST_MODULE_ID = 0,

    CTRLR_CONFIGURE_MODULE_ID,
    CTRLR_ARP_MODULE_ID,
    CTRLR_SWITCH_MODULE_ID,
    CTRLR_TOPO_MODULE_ID,
    CTRLR_SDR_MODULE_ID,
    CTRLR_OPENFLOW_MODULE_ID,
    CTRLR_EVENT_MODULE_ID,
    CTRLR_STATS_MODULE_ID,
    CTRLR_STP_MODULE_ID,
    CTRLR_LLDP_MODULE_ID,
    CTRLR_SNMP_MODULE_ID,
    CTRLR_BFD_MODULE_ID,
    CTRLR_LAST_MODULE_ID
} CTRLR_MODULE_ID;

typedef uint8_t ipv4_addr[IPV4_LEN];
typedef uint8_t ipv6_addr[IPV6_LEN];

enum {
    IPADDR_TYPE_V4 = 0,
    IPADDR_TYPE_V6,
};

typedef struct {
    int type;
    union {
        ovs_be32 ipv4;
        struct in6_addr ipv6;
    } addr;
} ctrlr_ip_type;

struct router_ip {
    ctrlr_ip_type ip;
    ctrlr_ip_type mask;
};

typedef struct {
    ctrlr_ip_type  ip;
    char br_name[MAX_BRIDGE_NAME_SIZE];
} ctrlr_bridge_id;

typedef enum {
    M_SW_STATE,
    M_SW_INFO,
    M_SW_ERR,
    M_BR_INFO,
    M_BR_ERR,
} MSG_TYPE;

struct nms_notify_info {
    MSG_TYPE type;
    char *name;  /* switch or bridge name */
    char *msg;
};

uint32_t bridge_hash(const ctrlr_bridge_id *bridge);

bool bridge_is_equal(const ctrlr_bridge_id *bridge1,
                         const ctrlr_bridge_id *bridge2);

//void bridge_ip_to_addr(const ctrlr_ip_type *ip, ip_addr_t *ip_addr);
//void addr_to_bridge_ip(const ip_addr_t *ip_addr, ctrlr_ip_type *ip);
char *vconn_name_to_ip(const char *vconn_name);
bool ip_addr_from_str(const char *str, ctrlr_ip_type *ip);
bool router_ip_from_str(const char * str, struct router_ip *r_ip);
char *format_ctrlr_ip_with_mask(const ctrlr_ip_type *ip,
                                                  const ctrlr_ip_type *mask);
ctrlr_ip_type ctrlr_ip_type_bit_nega(const ctrlr_ip_type *ip);
ctrlr_ip_type ctrlr_ip_type_bit_and(const ctrlr_ip_type *ip, const ctrlr_ip_type *mask);
ctrlr_ip_type ctrlr_ip_type_bit_or(const ctrlr_ip_type *ip, const ctrlr_ip_type *mask);
bool is_in_same_subnet(const struct router_ip *ip_src, const ctrlr_ip_type *ip);
char *remove_prefix(const char *pre_name);
char *add_prefix(const char *name, const char *prefix);
struct json *build_nms_msg(struct nms_notify_info *sni);
void print_buffer(const unsigned char *data, size_t size);
bool check_str_equal(const char *src, const char *dst);
bool check_smap_equal(const struct smap *s_src, const struct smap *s_dst);
bool is_any_ip(const ctrlr_ip_type *ip);
#endif
