
#include "graph.h"
#include "CommandParser/libcli.h"

extern graph_t *build_first_topo();
extern graph_t *build_simple_l2_switch_topo();
graph_t *topo = NULL;


int main(int argc, char **argv){
    
    nw_init_cli();
    //topo = build_simple_l2_switch_topo();
    topo = build_first_topo();
    start_shell();
    return 0;
}

