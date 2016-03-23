#include "stdio.h"
#include "route.h"
#include "lib_io.h"
#include "lib_time.h"

int main(int argc, char *argv[])
{
    print_time("Begin");
    char *graph[5000];
    int edge_num;
    char *condition;
    int condition_num;

    char *graph_file = argv[1];
    edge_num = read_file(graph, 5000, graph_file);
    char *condition_file = argv[2];
    condition_num = read_file(&condition, 1, condition_file);
    
    /*
    printf("------------\n"); 
    int i=0;
    while(graph[i])
    {
        printf("Graph %d: %s\n",i,graph[i]);
	i++;
    }
    printf("------------\n");
    printf("Edge:%d\n",edge_num);
    printf("------------\n");
    printf("Condition:%s\n",condition);
    */
    search_route(graph, edge_num, condition);

    char *result_file = argv[3];
    write_result(result_file);
    release_buff(graph, edge_num);
    release_buff(&condition, condition_num);

    print_time("End");

	return 0;
}

