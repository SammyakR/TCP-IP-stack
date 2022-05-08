#ifndef __NET__
#define __NET__

#include "utils.h"
#include <memory.h>


typedef struct graph_ graph_t;
typedef struct interface_ interface_t;
typedef struct node_ node_t;

void
interface_assign_mac_address(interface_t *interface);


typedef struct ip_add_
{
    char ip_addr[16];
} ip_add_t;

typedef struct mac_add_ {
    char mac[8];
}mac_add_t;

typedef struct arp_table_ arp_table_t;
typedef struct mac_table_ mac_table_t;

typedef struct node_nw_props_
{
    unsigned int flags;
    /* L2 prop */
    arp_table_t *arp_table;
    mac_table_t *mac_table;

    /*L3 properties*/
    bool_t is_lb_addr_config;
    ip_add_t lb_addr;
} node_nw_props_t;

extern void init_arp_table(arp_table_t **arp_table);


static inline void
init_node_nw_props(node_nw_props_t *node_nw_prop){
    node_nw_prop->flags = 0;
    node_nw_prop->is_lb_addr_config = FALSE;
    memset(node_nw_prop->lb_addr.ip_addr, 0, 16);
    init_arp_table(&(node_nw_prop->arp_table));
}

typedef enum{
    ACCESS,
    TRUNK,
    L2_MODE_UNKNOWN
} intf_l2_mode_t;

static inline char *
intf_l2_mode_str(intf_l2_mode_t intf_l2_mode){

    switch(intf_l2_mode){
        case ACCESS:
            return "access";
        case TRUNK:
            return "trunk";
        default:
            return "L2_MODE_UNKNOWN";
    }
}

#define MAX_VLAN_MEMBERSHIP 10

typedef struct intf_nw_props_
{
    mac_add_t mac_add;
    intf_l2_mode_t intf_l2_mode;
    unsigned int vlans[MAX_VLAN_MEMBERSHIP];
    bool_t is_ipadd_config;
    ip_add_t ip_add;
    char mask;

} intf_nw_props_t;

static inline void
init_intf_nw_props(intf_nw_props_t *intf_nw_props){
    memset(intf_nw_props->mac_add.mac, 0, 
            sizeof(intf_nw_props->mac_add.mac));
    intf_nw_props->is_ipadd_config = FALSE;
    memset(intf_nw_props->ip_add.ip_addr, 0, 16);
    intf_nw_props->mask = 0;
}

/*GET shorthand macros */
#define IF_MAC(intf_ptr)  ((intf_ptr)->intf_nw_props.mac_add.mac)
#define IF_IP(intf_ptr)   ((intf_ptr)->intf_nw_props.ip_add.ip_addr)
#define NODE_LO_ADDR(node_ptr)  (node_ptr->node_nw_props.lb_addr.ip_addr)
#define NODE_ARP_TABLE(node_ptr) (node_ptr->node_nw_props.arp_table)
#define NODE_MAC_TABLE(node_ptr) (node_ptr->node_nw_props.mac_table)
#define IF_L2_MODE(intf_ptr)    (intf_ptr->intf_nw_props.intf_l2_mode)
#define IS_INTF_L3_MODE(intf_ptr)   \
        (intf_ptr->intf_nw_props.is_ipadd_config == TRUE)

/*APIs to set nw node properties*/
bool_t node_set_loopback_address(node_t *node, char *ip_addr);
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask);
bool_t node_unset_intf_ip_address(node_t *node, char *local_if);
interface_t *
node_get_matching_subnet_interface(node_t *node, char *ip_addr);

/*Display Routines NW specific*/
void dump_nw_graph(graph_t *graph);
void dump_node_nw_props(node_t *node);
void dump_interface_nw_props(interface_t *interface);


unsigned int
ip_addr_p_to_n(char *ip_addr);

void 
ip_addr_n_to_p(unsigned int ip_addr, char *ip_add_str);

char *
pkt_buffer_shift_right(char *pkt, unsigned int pkt_size, 
                       unsigned int total_buffer_size);


#endif  /*__NET__*/


