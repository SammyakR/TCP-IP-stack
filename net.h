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

typedef struct node_nw_props_
{

    /*L3 properties*/
    bool_t is_lb_addr_config;
    ip_add_t lb_addr;
}node_nw_props_t;

static inline void
init_node_nw_props(node_nw_props_t *node_nw_prop){
    node_nw_prop->is_lb_addr_config = FALSE;
    memset(node_nw_prop->lb_addr.ip_addr, 0, 16);
}

typedef struct intf_nw_props_
{
    mac_add_t mac_add;

    bool_t is_ipadd_config;
    ip_add_t ip_add;
    char mask;

} intf_nw_props_t;

static inline void
init_intf_nw_props(intf_nw_props_t *intf_nw_props){
    memset(intf_nw_props->mac_add.mac, 0, 8);
    intf_nw_props->is_ipadd_config = FALSE;
    memset(intf_nw_props->ip_add.ip_addr, 0, 16);
    intf_nw_props->mask = 0;
}

/*GET shorthand macros */
#define IF_MAC(intf_ptr)  ((intf_ptr)->intf_nw_props.mac_add.mac)
#define IF_IP(intf_ptr)   ((intf_ptr)->intf_nw_props.ip_add.ip_addr)
#define NODE_LO_ADDR(node_ptr)  (node_ptr->node_nw_props.lb_addr.ip_addr)

/*APIs to set nw node properties*/
bool_t node_set_loopback_address(node_t *node, char *ip_addr);
bool_t node_set_intf_ip_address(node_t *node, char *local_if, char *ip_addr, char mask);
bool_t node_unset_intf_ip_address(node_t *node, char *local_if);

/*Display Routines NW specific*/
void dump_nw_graph(graph_t *graph);
void dump_node_nw_props(node_t *node);
void dump_interface_nw_props(interface_t *interface);

#endif  /*__NET__*/


