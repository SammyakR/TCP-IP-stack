
#include "layer2.h"
#include <arpa/inet.h>
#include <stdio.h>
#include "../graph.h"
#include "../tcpconst.h"


void
layer2_frame_recv(node_t *node, interface_t *interface,
                    char *pkt, unsigned int pkt_size){

}

void
init_arp_table(arp_table_t **arp_table){
    *arp_table = (arp_table_t*)calloc(1, sizeof(arp_table_t));
    init_glthread(&((*arp_table)->arp_entries));
}


arp_entry_t *
arp_table_lookup(arp_table_t *arp_table, char *ip_addr){

    glthread_t *curr;
    arp_entry_t *arp_entry;
    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){
        arp_entry = arp_glue_to_arp_entry(curr);
        if(strncmp(arp_entry->ip_addr.ip_addr, ip_addr, 16) == 0){
            return arp_entry;
        }
    }ITERATE_GLTHREAD_END(&arp_table->arp->entries, curr)
    return NULL;
}

bool_t
arp_table_entry_add(arp_table_t *arp_table, arp_entry_t *arp_entry){
    arp_entry_t *arp_entry_old = arp_table_lookup(arp_table, 
                                    arp_entry->ip_addr.ip_addr);
    if(arp_entry_old && memcpy(arp_entry_old, arp_entry, 
            sizeof(arp_entry_t) == 0))
            return FALSE;
    
    if(arp_entry_old){
        delete_arp_table_entry(arp_table, arp_entry->ip_addr.ip_addr);
    }
    init_glthread(&arp_entry->arp_glue);
    glthread_add_next(&arp_table->arp_entries, &arp_entry->arp_glue);

    return TRUE;
}

void
arp_table_update_from_arp_reply(arp_table_t *arp_table, 
                    arp_hdr_t *arp_hdr, interface_t *iif){
    
    unsigned int src_ip;
    assert(arp_hdr->op_code == ARP_REPLY);
    arp_entry_t *arp_entry = calloc(1, sizeof(arp_entry_t));
    src_ip = htonl(arp_hdr->src_ip);
    inet_ntop(AF_INET, &src_ip, &arp_entry->ip_addr.ip_addr, 16);
    arp_entry->ip_addr.ip_addr[15]='\0';
    memcpy(arp_entry->mac_addr.mac, arp_hdr->src_mac.mac,
            sizeof(mac_add_t));
    strncpy(arp_entry->oif_name, iif->if_name, IF_NAME_SIZE);
    bool_t rc = arp_table_entry_add(arp_table, arp_entry);
    
    if(rc==FALSE){
        free(arp_entry);
    }

}

void
dump_arp_table(arp_table_t *arp_table){

    glthread_t *curr;
    arp_entry_t *arp_entry;
    
    ITERATE_GLTHREAD_BEGIN(&arp_table->arp_entries, curr){
        arp_entry=arp_glue_to_arp_entry(curr);
        printf("IP = %s, MAC = %u:%u:%u:%u:%u:%u, OIF = %s\n",
            arp_entry->ip_addr.ip_addr,
            arp_entry->mac_addr.mac[0],
            arp_entry->mac_addr.mac[1],
            arp_entry->mac_addr.mac[2],
            arp_entry->mac_addr.mac[3],
            arp_entry->mac_addr.mac[4],
            arp_entry->mac_addr.mac[5],
            arp_entry->oif_name);
    }ITERATE_GLTHREAD_END(&arp_table->arp_entries, curr);
}





void
delete_arp_table_entry(arp_table_t *arp_table, char *ip_addr){
    
}

static void
send_arp_reply_msg(ethernet_hdr_t *ethernet_hdr_in, interface_t* oif){
    arp_hdr_t *arp_hdr_in = (arp_hdr_t*)(ethernet_hdr_in->payload);
    ethernet_hdr_t *ethernet_hdr_reply = (ethernet_hdr_t*)calloc(1, sizeof(ethernet_hdr_t) + sizeof(arp_hdr_t));

    memcpy(ethernet_hdr_reply->dst_mac.mac, arp_hdr_in->src_mac.mac, sizeof(mac_add_t));
    memcpy(ethernet_hdr_reply->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));
    
    ethernet_hdr_reply->type = ARP_MSG;
    
    arp_hdr_t *arp_hdr_reply = (arp_hdr_t *)(ethernet_hdr_reply->payload);
    
    arp_hdr_reply->hw_type = 1;
    arp_hdr_reply->proto_type = 0x0800;
    arp_hdr_reply->hw_addr_len = sizeof(mac_add_t);
    arp_hdr_reply->proto_addr_len = 4;
    
    arp_hdr_reply->op_code = ARP_REPLY;
    memcpy(arp_hdr_reply->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));

    inet_pton(AF_INET, IF_IP(oif), &arp_hdr_reply->src_ip);
    arp_hdr_reply->src_ip =  htonl(arp_hdr_reply->src_ip);

    memcpy(arp_hdr_reply->dst_mac.mac, arp_hdr_in->src_mac.mac, sizeof(mac_add_t));
    arp_hdr_reply->dst_ip = arp_hdr_in->src_ip;
  
    send_pkt_out((char *)ethernet_hdr_reply, sizeof(ethernet_hdr_t) + sizeof(arp_hdr_t),
                    oif);

    ethernet_hdr_reply->FCS = 0; /*Not used*/
    free(ethernet_hdr_reply);

}


static void
process_arp_reply_msg(node_t *node, interface_t *iif,
                        ethernet_hdr_t *ethernet_hdr){

    printf("%s : ARP reply msg recvd on interface %s of node %s\n",
            __FUNCTION__, iif->if_name, iif->att_node->node_name);
    
    arp_table_update_from_arp_reply(NODE_ARP_TABLE(node), 
                (arp_hdr_t *)(ethernet_hdr->payload), iif);
    
}

void
send_arp_broadcast_request(node_t *node, 
        interface_t *oif, char *ip_addr){
    
    unsigned int payload_size = sizeof(arp_hdr_t);
    ethernet_hdr_t *ethernet_hdr = (ethernet_hdr_t *) calloc(1, 
                    ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size);

    if(!oif){
        oif=node_get_matching_subnet_interface(node, ip_addr);
        if(!oif){
            printf("Error: %s : No eligible subnet for ARP resolution Ip: %s",
                        node->node_name, ip_addr);
            return;
        }
    }
    /*STEP 1 : Prepare eth hdr */
    layer2_fill_with_broadcast_mac(ethernet_hdr->dst_mac.mac);
    memcpy(ethernet_hdr->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));
    ethernet_hdr->type=ARP_MSG;

    /* STEP 2 : */
    arp_hdr_t *arp_hdr = (arp_hdr_t *)ethernet_hdr->payload;
    arp_hdr->hw_type=1;
    arp_hdr->proto_type = 0x0800;
    arp_hdr->hw_addr_len = sizeof(mac_add_t);
    arp_hdr->proto_addr_len = 4;

    arp_hdr->op_code = ARP_BROAD_REQ;

    memcpy(arp_hdr->src_mac.mac, IF_MAC(oif), sizeof(mac_add_t));
    inet_pton(AF_INET, IF_IP(oif), &arp_hdr->src_ip);
    arp_hdr->src_ip = htonl(arp_hdr->src_ip);

    memset(arp_hdr->dst_mac.mac, 0, sizeof(mac_add_t));
    /* Never use ethernet->FCS =0 */
    ETH_FCS(ethernet_hdr, sizeof(arp_hdr_t)) = 0;

    /* STEP 3 : Dispatch the ARP Broadcast request */
    send_pkt_out((char *)ethernet_hdr, 
                    ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size, oif);
    
    free(ethernet_hdr);

}



static void
process_arp_broadcast_request(node_t *node, interface_t *iif,
                                ethernet_hdr_t *ethernet_hdr){

    printf("%s : ARP Broadcast msg recvd interface %s of node %s \n",
            __FUNCTION__, iif->if_name, iif->att_node->node_name);
    
    char ip_addr[16];
    arp_hdr_t *arp_hdr = (arp_hdr_t *)(ethernet_hdr->payload);

    unsigned int arp_dst_ip = htonl(arp_hdr->dst_ip);
    inet_ntop(AF_INET, &arp_dst_ip, ip_addr, 16);
    ip_addr[15]='\0';

    if(strncmp(IF_IP(iif), ip_addr, 16)){
        printf("%s : ARP BroadcasLayer2/layer2.o:t rq msg, Dst ip does not match %s did not match interface ip : %s\n",
        node->node_name, ip_addr, IF_IP(iif));
        return;
    }

    send_arp_reply_msg(ethernet_hdr, iif);

}

