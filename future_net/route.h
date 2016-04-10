#ifndef __ROUTE_H__
#define __ROUTE_H__

void search_route(char *topo[5000], char *graph[5000],int edge_num, char *condition);
int split(char dst[][15], char* str, const char* spl);
int read_demand(char *demand,int must[],int &startnode,int &endnode);//demand是输入的condition，must[50]是必经节点数组，startnode返回起点序号，endnode返回终点的序号，函数返回必经点的点数
void re_search_route(char *graph[5000], int edge_num, char *condition);




int feasible_childnode(int **&A,int j,int arr[2][10],int num,int path[]);//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[0][10]存储子节点的序号,arr[1][10]存储对应子节点的权重，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
int judge(int nummust,int mustarr[50],int test);//输入必经点点数，必经点数组，待测点的序号,待测点是必经点，为true否则为false
void create();
#endif
