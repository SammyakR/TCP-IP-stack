
#include "graph.h"
#include "CommandParser/libcli.h"

extern graph_t *build_first_topo();
graph_t *topo = NULL;


int main(int argc, char **argv){
    
    nw_init_cli();
    topo = build_first_topo();
    //dump_nw_graph(topo);

    sleep(2);
    node_t *snode = get_node_by_node_name(topo, "R0_re");
    interface_t *oif = get_node_if_by_name(snode, "eth0/0");

    char msg[] = "Hello, how are you\0";
    send_pkt_out(msg, strlen(msg), oif);

    sleep(2);
    node_t *nsnode = get_node_by_node_name(topo, "R2_re");
    interface_t *noif = get_node_if_by_name(snode, "eth0/0");
    send_pkt_flood(nsnode, noif, msg, strlen(msg));

    start_shell();
    return 0;
}

