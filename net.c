#include "graph.h"
#include <stdio.h>
#include <memory.h>
#include "utils.h"
#include <sys/socket.h>
#include <netinet/in.h>


/*
void
interface_assign_mac_address(interface_t *interface){
    memset(IF_MAC(interface), 0, 8);
    strcpy(IF_MAC(interface), interface->att_node->node_name);
    strcat(IF_MAC(interface), interface->if_name);
}
*/

static unsigned int
hash_code(void *ptr, unsigned int size){
    unsigned int value =0, i =0;
    char *str = (char *)ptr;
    while(i<size){
        value += *str;
        value *= 97;
        str++;
        i++;
    }
    return value;
}

void
interface_assign_mac_address(interface_t *interface){
    node_t *node = interface->att_node;
    if(!node) return;

    unsigned int hash_code_val = 0;
    hash_code_val = hash_code(node->node_name, NODE_NAME_SIZE);
    hash_code_val *= hash_code(interface->if_name, IF_NAME_SIZE);
    memset(IF_MAC(interface), 0, sizeof(IF_MAC(interface)));
    memcpy(IF_MAC(interface), (char *)&hash_code_val, sizeof(unsigned int));

}



bool_t node_set_loopback_address(node_t *node, char *ip_addr){
    assert(ip_addr);

    node->node_nw_props.is_lb_addr_config = TRUE;
    strncpy(NODE_LO_ADDR(node), ip_addr, 16);
    NODE_LO_ADDR(node)[15] = '\0';

    return TRUE;
}


bool_t 
node_set_intf_ip_address(node_t *node, char *local_if,
                                char *ip_addr, char mask){

    interface_t *interface = get_node_if_by_name(node, local_if);
    if(!interface) assert(0);

    strncpy(IF_IP(interface), ip_addr, 16);
    IF_IP(interface)[15] = '\0';
    interface->intf_nw_props.mask = mask;
    interface->intf_nw_props.is_ipadd_config = TRUE;


    return TRUE;
}

char *
pkt_buffer_shift_right(char *pkt, unsigned int pkt_size, 
                       unsigned int total_buffer_size)
{
        char *temp = NULL;
        return temp;
}

interface_t *
node_get_matching_subnet_interface(node_t *node, char *ip_addr){
    char ip_subnet[32];
    char if_subnet[32];
    for(int i=0; i<MAX_INTF_PER_NODE; i++){
        char mask = node->intf[i]->intf_nw_props.mask;

        apply_mask(ip_addr, mask, ip_subnet);
        apply_mask(node->intf[i]->intf_nw_props.ip_add.ip_addr, 
                                    mask, if_subnet);
        
        if(strcmp(if_subnet, ip_subnet) == 0)
            return node -> intf[i];
    }

    return NULL;
}

/*
================================
Display Routines
================================
*/


void dump_node_nw_props(node_t *node){

    printf("\nNode Name = %s\n", node->node_name);
    // printf("\t node flags : %u", node->node_nw_props.flags);
    if(node->node_nw_props.is_lb_addr_config){
        printf("\t  lo addr : %s/32\n", NODE_LO_ADDR(node));
    }
}

void dump_intf_props(interface_t *interface){

    dump_interface(interface);

    if(interface->intf_nw_props.is_ipadd_config){
        printf("\t IP Addr = %s/%u", IF_IP(interface), 
                            interface->intf_nw_props.mask);
        
        printf("\t MAC : %u:%u:%u:%u:%u:%u\n", 
        IF_MAC(interface)[0], IF_MAC(interface)[1],
        IF_MAC(interface)[2], IF_MAC(interface)[3],
        IF_MAC(interface)[4], IF_MAC(interface)[5]);
    }
    else{
         printf("\t l2 mode = %s", intf_l2_mode_str(IF_L2_MODE(interface)));
         printf("\t vlan membership : ");
         int i = 0;
         for(; i < MAX_VLAN_MEMBERSHIP; i++){
            if(interface->intf_nw_props.vlans[i]){
                printf("%u  ", interface->intf_nw_props.vlans[i]);
            }
         }
         printf("\n");
    }

}

void dump_nw_graph(graph_t *graph){

    node_t *node;
    glthread_t *curr;
    interface_t *interface;
    unsigned int i;
    
    printf("Topology Name = %s\n", graph->topology_name);

    ITERATE_GLTHREAD_BEGIN(&graph->node_list, curr){

        node = graph_glue_to_node(curr);
        dump_node_nw_props(node);
        for( i = 0; i < MAX_INTF_PER_NODE; i++){
            interface = node->intf[i];
            if(!interface) break;
            dump_intf_props(interface);
        }
    } ITERATE_GLTHREAD_END(&graph->node_list, curr);

}


/*
==============================
    Address Conversion
==============================
*/
void 
ip_addr_n_to_p(unsigned int ip_addr, char *ip_add_str)
{
    ip_addr = htonl(ip_addr);
    inet_ntop(AF_INET, &ip_addr, ip_add_str, 16);
}

unsigned int
ip_addr_p_to_n(char *ip_addr)
{
    unsigned int decimal_rep;
    inet_pton(AF_INET, ip_addr, &decimal_rep);
    decimal_rep = htonl(decimal_rep);
    return decimal_rep;
}


/*
================
   VLAN 
================
*/

unsigned int
get_access_intf_operating_vlan_id(interface_t *interface){
    if(IF_L2_MODE(interface) != ACCESS){
        assert(0);
    }
    return interface->intf_nw_props.vlans[0];
}

bool_t
is_trunk_interface_vlan_enabled(interface_t *interface,
                                unsigned int vlan_id){
    
    if(IF_L2_MODE(interface) != TRUNK){
        assert(0);
    }

    unsigned int i = 0;
    for( ; i < MAX_VLAN_MEMBERSHIP; i++){
        if(interface->intf_nw_props.vlans[i] == vlan_id)
            return TRUE;
    }

    return FALSE;
}