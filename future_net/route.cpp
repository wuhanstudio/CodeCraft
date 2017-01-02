#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h> 
#include <omp.h>
#include "route.h"
#include "lib_record.h"

int split(char dst[][20], char* str, const char* spl);
int read_demand(char *demand,int must[],int &startnode,int &endnode);
int feasible_childnode(int **&A,int j,int arr[2][10],int num,int path[]);
int judge(int nummust,int mustarr[50],int test);

int **a;//边的矩阵
int num_edge;//边的条数
int num_node;//点的总数
int num_must;//必经点点数
int must_arr[50];//必经点集合

int start_node;//起始点
int end_node;//终点

int bestpath[600];//存储最好的路径
int bestpow=-1;//最好路径的权重
int bestnum;//最好的路径的点数

const int compare_num=15;//每个点路径信息最大存储数，用于比较
double rate=0.7;
double x1=500,x2=2;//x1必经点数量权重，x2路径权值和的权重

typedef struct Node
{
	struct Node *next;
	int point;//点序号
	int road[600];//已经经历点的路径顺序
	int passnum;//已经经历的点的数目
	int pow;//路径加总权重
	int mustnum;//路径经历的必经点点数
	int mustnode[50];//已经经历的必经点
}node;

typedef struct infostr
{
	int must_num[compare_num];//路径经历的必经点点数
	int sumpow[compare_num];//路径加总权重
}info_node;
info_node *node_info[600];

int calculate_score(node *&A, info_node *&B)
{
	double C[compare_num];
	double D = x1*A->mustnum - x2*A->pow,low=x1*50.0,high=-1,E;
	int lowest;
	for(int i=0;i < compare_num; i++)
	{
		C[i] = x1*B->must_num[i] - x2*B->sumpow[i];
		if(low > C[i])
		{
			low=C[i];
			lowest=i;
		}
		if(high < C[i])
		{
			high = C[i];
		}	
	}
	if(D<low)
	{
		E = (low-D)/(high-D);
		if(((1-E)*rand()/(RAND_MAX+1.0))>rate)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		B->must_num[lowest]=A->mustnum;
		B->sumpow[lowest]=A->pow;
		return 1;
	}
}

// pointnum: 当前点的点序号
// num     : 已经经历了的点的数目
// path    : 已经经历过的点 
void create(int pointnum,int num,int path[])
{
	const int g_ncore = omp_get_num_procs(); //获取执行核的数量
	printf("Use Core : %d\n",g_ncore );
	printf("Total Nodes Number:%d\n",num_node);

	int shortest[600];
	for(int i=0;i<num_node;i++)
	{
		shortest[i]=12000;
	}

	node *h;
	node *l = (node *)malloc(sizeof(node));

	node *start = (node *)malloc(sizeof(node));
	start -> mustnum = 0;
	start -> pow = 0;
	start -> next = NULL;
	start -> passnum = num;
	start -> point = pointnum;
	memcpy(start->road, path ,start->passnum * sizeof(int));
	l->next=start;
	start = NULL;
	free(start);

	node *r;
	int max_loop = 1;
	for(int i=1;i<num_node;i++)
	{
		printf("Depth:%d#%d\n",i,max_loop );
		h=l;
		r = h->next;
		//l = h;
		// Record Every Loop
		int next_loop = 0;
		if(max_loop>0)
		{
			node**   end_loop = (node**)malloc(sizeof(node*)*g_ncore);
			node** start_loop = (node**)malloc(sizeof(node*)*g_ncore);
			for (int o = 0; o < g_ncore; ++o)
			{
				start_loop[o] = (node*)malloc(sizeof(node));
				start_loop[o]->next = NULL;
			}
			node** r_record = (node**)malloc(sizeof(node*)*max_loop);
			for (int o = 0; o < max_loop; ++o)
			{
				r_record[o] = r;
				r = r->next;
			}
			#pragma omp parallel for num_threads(g_ncore)
			for(int o=0;o<max_loop;o++)
			{
				int arr[3][10];
				node *temp;
				int k = feasible_childnode(a,r_record[o]->point,arr,r_record[o]->passnum,r_record[o]->road);
				for(int j=0; j < k; j++)
				{
					if(arr[0][j]==end_node)
					{
						temp = (node *)malloc(sizeof(node));
						temp -> point   = arr[0][j];
						temp -> passnum = r_record[o]->passnum+1;
						temp -> pow     = r_record[o]->pow+arr[1][j];
						memcpy(temp->road, r_record[o]->road ,r_record[o]->passnum * sizeof(int));
						temp -> road[r_record[o]->passnum]=arr[0][j];
	                    temp -> mustnum = r_record[o]->mustnum;
	                    //判断路径是否符合条件
						int estimate = (temp->mustnum==num_must) ? 1 : 0;		
						if(estimate)
						{
							if((bestpow==-1)||(bestpow > (temp->pow)))
							{
								#pragma omp critical
								{
								bestnum = temp->passnum;
								bestpow = temp->pow;
								memcpy(bestpath, temp->road ,bestnum * sizeof(int));
								//printf("bestpow:%d\n",bestpow);
								}
							}
						}
						free(temp);
					}
					else if( ( ( r_record[o]->pow + arr[1][j] ) < shortest[arr[0][j]] ) )
					{
						shortest[arr[0][j]] = r_record[o]->pow + arr[1][j];
						temp = (node *)malloc(sizeof(node));
						if(arr[2][j]==1)
						{
							temp->mustnum = r_record[o]->mustnum + 1;
							memcpy(temp->mustnode, r_record[o]->mustnode, r_record[o]->mustnum * sizeof(int));
							temp->mustnode[r_record[o]->mustnum]=arr[0][j];
						}
						else
						{
							temp->mustnum = r_record[o]->mustnum;
							memcpy(temp->mustnode, r_record[o]->mustnode, r_record[o]->mustnum * sizeof(int));
						}
						temp->point=arr[0][j];
						temp->passnum=r_record[o]->passnum+1;
						temp->pow=r_record[o]->pow+arr[1][j];
						memcpy(temp->road, r_record[o]->road ,r_record[o]->passnum * sizeof(int));
						temp->road[r_record[o]->passnum]=arr[0][j];
						
						if(start_loop[omp_get_thread_num()]->next==NULL)
						{
							start_loop[omp_get_thread_num()]->next = temp;
						}
						else
						{
							end_loop[omp_get_thread_num()]->next = temp;
						}
						end_loop[omp_get_thread_num()] = temp;
						printf("%d ",end_loop[omp_get_thread_num()]->point );
						next_loop++;

					}
					else
					{
						temp = (node *)malloc(sizeof(node));
						if(arr[2][j]==1)
						{
							temp->mustnum = r_record[o]->mustnum + 1;
							memcpy(temp->mustnode, r_record[o]->mustnode, r_record[o]->mustnum * sizeof(int));
							temp->mustnode[r_record[o]->mustnum]=arr[0][j];
						}
						else
						{
							temp->mustnum = r_record[o]->mustnum;
							memcpy(temp->mustnode, r_record[o]->mustnode, r_record[o]->mustnum * sizeof(int));
						}
						temp -> point   = arr[0][j];
						temp -> passnum = r_record[o]->passnum+1;
						temp -> pow     = r_record[o]->pow+arr[1][j];
						memcpy(temp->road, r_record[o]->road ,r_record[o]->passnum * sizeof(int));
						temp -> road[r_record[o]->passnum] = arr[0][j];
						if(calculate_score(temp, node_info[arr[0][j]])==1)
						{
							if(start_loop[omp_get_thread_num()]->next==NULL)
							{
								start_loop[omp_get_thread_num()]->next = temp;
							}
							else
							{
								end_loop[omp_get_thread_num()]->next = temp;
							}
							end_loop[omp_get_thread_num()] = temp;
							printf("%d ",end_loop[omp_get_thread_num()]->point );
							next_loop++;
						}
						else
						{
							free(temp);
						}
					}
				}
			}
			node* m = l;
			printf("\n");
			for (int o = 0; o < g_ncore; ++o)
			{
				if(start_loop[o]->next!=NULL)
				{
					m->next = start_loop[o];
					m = m->next;
					while(m->next!=NULL)
					{
						printf("%d-",m->next->point );
						m = m->next;
					}
				}
				//	start_loop[o]->next = NULL;
			}
			free(r_record);	
			free(end_loop);
			printf("\n");
		}

		max_loop = next_loop;
	}
	free(l);
}

void search_route(char *graph[5000], int edge, char *condition)
{
	num_edge=edge;
	a = (int **)malloc(sizeof(int *) * 5000);
	for (int i = 0; i < edge; ++i)
	{
		a[i] = (int *)malloc(sizeof(int) * 3);
		char dst[5][20];
		split(dst, graph[i], ",");
		// Start
		a[i][0] = atoi(dst[1]);
		// End
		a[i][1] = atoi(dst[2]);
		// Power
		a[i][2] = atoi(dst[3]);
	}

	// Total Nodes
	num_node=a[edge-1][0]+1;
	num_must = read_demand(condition,must_arr,start_node,end_node);
	for(int i=0; i<num_node; i++)
	{
		node_info[i] = (info_node *)malloc(sizeof(info_node));
		for(int j=0;j<compare_num;j++)
		{	
			node_info[i]->must_num[j]=0;
			node_info[i]->sumpow[j]=12000;
		}
	}
	
	int PATH[num_node];
	PATH[0]=start_node;
	printf("Size of Node   : %ld\n",sizeof(node) );
	printf("Start Node : %d\n",start_node );
	
	create(start_node,1,PATH);

	if(bestpow==-1)
	{
		printf("NA\n");
	}

	for (int i = 0; i < bestnum; i++)
		record_result(bestpath[i]);

	for (int i = 0; i < edge; ++i)
	{
		free(a[i]);
	}
	free(a);
	for(int i=0; i<num_node; i++)
	{
		free(node_info[i]);
	}
}

//分解字符串函数
int split(char dst[][20], char* str, const char* spl)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while( result != NULL )
    {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}


int read_demand(char *demand,int must[50],int &startnode,int &endnode)
{
	char list[50][20];
	char sel[3][20];
	int num_must; //必经点点数
	num_must=split(list, demand, "|");
	split(sel,list[0],",");
	startnode=atoi(sel[0]);
	endnode=atoi(sel[1]);
	if(sel[2] != NULL)
	{
		must[0]=atoi(sel[2]);
	}
	for(int k=1;k<num_must;k++)
	{
		must[k]=atoi(list[k]);
	}
	return num_must;
}

//找到第j个节点的可行子节点 ,
//A是二维路径权重矩阵，
//j是父节点序号，
//arr[0][10]存储子节点的序号,arr[1][10]存储对应子节点的权重，
//k存储可行子节点数目，
//path是已走过的路径，
//num是已走过路径的点数
int feasible_childnode(int **&A,int j,int arr[3][10],int num,int path[])
{
	int k=0;//可行子节点
	for(int i=0;i<num_edge;i++)
	{
		if(j==A[i][0])
		{
			int	index = 0;
			for(int n=0;n<num;n++)
			{
				if(A[i][1] != path[n])
				{
					index++;
				}
				if(index == num)
				{
					arr[0][k]=A[i][1];
					arr[1][k]=A[i][2];
					arr[2][k]=judge(num_must,must_arr,arr[0][k]);
					k++;
				}
			}
		}
	}
	return k;
}

//输入必经点点数，必经点数组，待测点的序号,待测点是必经点，为true否则为false
int judge(int nummust,int mustarr[50],int test)
{
	for(int i=0;i < nummust;i++)
	{
		if(test==mustarr[i])
		{
			return 1;
		}
	}
	return 0;
}
