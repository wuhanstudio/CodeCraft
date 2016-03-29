#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h> 
#include "time.h"

int split(char dst[][20], char* str, const char* spl);
int read_demand(char *demand,int must[],int &startnode,int &endnode);//demand是输入的condition，must[50]是必经节点数组，startnode返回起点序号，endnode返回终点的序号，函数返回必经点的点数
int feasible_childnode(int **&A,int j,int arr[2][10],int num,int path[]);//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[0][10]存储子节点的序号,arr[1][10]存储对应子节点的权重，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
int judge(int nummust,int mustarr[50],int test);//输入必经点点数，必经点数组，待测点的序号,待测点是必经点，为true否则为false
int sec(time_t &G);//返回当前的秒数
int time_used(time_t &H);//返回以用的时间
double similar(int num1,int *arr1,int num2,int *arr2);//计算两数组间的相似度，num1待测数组点数,arr1待测数组,num2比较数组点数,arr2比较数组

int countresult = 0;
int **a;//边的矩阵
int edgenum;//边的条数
int num_must;//必经点点数
int must_arr[50];//必经点集合
int start_node;//起始点
int end_node;//终点
int num_node;//点的总数
int bestpath[600];//存储最好的路径
int bestpow=-1;//最好路径的权重
int bestnum;//最好的路径的点数
int start_time;//开始时间
time_t T;//计时用的结构体

const int compare_num=15;//每个点路径信息最大存储数，用于比较
double rate=0.7;

double x1=500,x2=2;//x1必经点数量权重，x2路径权值和的权重

typedef struct str
{
	struct str *next;
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
	//int must_node[compare_num][50];//已经经历的必经点
	int sumpow[compare_num];//路径加总权重
}info_node;


info_node *node_info[600];

/*
typedef struct beststr
{
	int num;//已经存下的路径
	int must_num[20];//路径经历的必经点点数
	int must_node[20][50];//已经经历的必经点
	int sumpow[20];//路径加总权重
}best_node;
*/

//info_node *memory[600];


int calculate_score(node *&A, info_node *&B)
{
	double C[compare_num];
	double D = x1*A->mustnum - x2*A->pow,low=x1*50.0,high=-1,E;
	int l;
	for(int i=0;i < compare_num; i++)
	{
		C[i] = x1*B->must_num[i] - x2*B->sumpow[i];
		if(low > C[i])
		{
			low=C[i];
			l=i;
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
		B->must_num[l]=A->mustnum;
		B->sumpow[l]=A->pow;
		return 1;
	}
}

void creat(int pointnum,int num,int path[])//pointnum当前点的点序号，num已经经历了的点的数目，path已经经历过的点 path[0]==startpoint;
{
	int k;
	int arr[3][10];
	int i;
	printf("num_node:%d\n",num_node);
	int shortest[600];
	for(i=0;i<num_node;i++)
	{
		shortest[i]=12000;
	}
	node *c= (node *)malloc(sizeof(node)),*h;
	h=(node *)malloc(sizeof(node));
	node *l=(node *)malloc(sizeof(node));
	c->mustnum=0;
	c->pow=0;
	c->next=NULL;
	c->passnum=num;
	c->point=pointnum;
	memcpy(c->road, path ,c->passnum * sizeof(int));
	node *r,*m,*q;
	for(i=0;i<num_node;i++)
	{
		if(i==0)
		{
			l->next=c;
		}
		q=l;
		l=h;
		h=q;

		r = h->next;
		m = l;
		
		if(time_used(T)>=1000)
		{
			printf("超时!最大深度%d:",r->passnum);
			break;
		}
		while(r)
		{
			q = r;
			k = feasible_childnode(a,r->point,arr,r->passnum,r->road);

			for(int j=0; j < k; j++)
			{
				if(arr[0][j]==end_node)
				{

                    char name[50];
					//r = (node *)malloc(sizeof(node));
					c = (node *)malloc(sizeof(node));
					c->point=arr[0][j];
					c->passnum=r->passnum+1;
					c->pow=r->pow+arr[1][j];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[0][j];

					int estimate= (r->mustnum==num_must) ? 1 : 0;//判断路径是否符合条件		
					//estimate=judge(num_must,must_arr,r->passnum,r->road);
					if(estimate==1)
					{
						//符合条件的要存下来
						//计算路径总权重
						//int pathpow=calculate_pow(c->passnum, c->road);

						if((bestpow==-1)||(bestpow > (r->pow)))
						{
							bestnum = c->passnum;
							bestpow = c->pow;
							memcpy(bestpath, c->road ,bestnum * sizeof(int));
							printf("bestpow:%d path:",bestpow);
							sprintf(name,"result_%d.txt",countresult);

							FILE *fid=fopen(name,"w");
					
							for(int o=0;o<bestnum;o++)
							{
								if(bestpath[o+1])
								{
									fprintf(fid,"%d,%d\n",bestpath[o],bestpath[o+1]);
								}

								printf("%d,",bestpath[o]);
							}	
							fclose(fid);
                            printf("\n");
                            countresult++;

						}
					}
					free(c);
				}
				else if( ( ( r->pow + arr[1][j] ) < shortest[arr[0][j]] ) )
				{
					shortest[arr[0][j]] = r->pow + arr[1][j];
					c = (node *)malloc(sizeof(node));
					if(arr[2][j]==1)
					{
						c->mustnum = r->mustnum + 1;
						memcpy(c->mustnode, r->mustnode, r->mustnum * sizeof(int));
						c->mustnode[r->mustnum]=arr[0][j];
					}
					else
					{
						c->mustnum = r->mustnum;
						memcpy(c->mustnode, r->mustnode, r->mustnum * sizeof(int));
					}
					c->point=arr[0][j];
					c->passnum=r->passnum+1;
					c->pow=r->pow+arr[1][j];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[0][j];
					m->next=c;
					m=c;
				}
				else
				{

					c = (node *)malloc(sizeof(node));
					if(arr[2][j]==1)
					{
						c->mustnum = r->mustnum + 1;
						memcpy(c->mustnode, r->mustnode, r->mustnum * sizeof(int));
						c->mustnode[r->mustnum]=arr[0][j];
					}
					else
					{
						c->mustnum = r->mustnum;
						memcpy(c->mustnode, r->mustnode, r->mustnum * sizeof(int));
					}
					c->point=arr[0][j];
					c->passnum=r->passnum+1;
					c->pow=r->pow+arr[1][j];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[0][j];
					if(calculate_score(c, node_info[arr[0][j]])==1)
					{
						m->next=c;
						m=c;				
					}
					else
					{
						free(c);
					}

				}
			}
			r = r->next;
			free(q);
		}
		m->next = NULL;
	}
}


//你要完成的功能总入口
void search_route(char *graph[5000], int edge_num, char *condition)
{
	start_time=sec(T);
	edgenum=edge_num;
	a = (int **)malloc(sizeof(int *) * 5000);
	//info_node *memory[num_node];
	int i=0;
	while (graph[i])
	{
		a[i] = (int *)malloc(sizeof(int) * 3);
		char dst[5][20];
		split(dst, graph[i], ",");
		a[i][0] = atoi(dst[1]);
		a[i][1] = atoi(dst[2]);
		a[i][2] = atoi(dst[3]);
		i++;
	}
	num_node=a[edge_num-1][0]+1;
	num_must = read_demand(condition,must_arr,start_node,end_node);
	
	for(i=0; i<num_node; i++)
	{
		node_info[i] = (info_node *)malloc(sizeof(info_node));
		//node_info[i]->num = 0;
		for(int j=0;j<compare_num;j++)
		{
			node_info[i]->must_num[j]=0;
			node_info[i]->sumpow[j]=12000;
		}
	}
	
	int PATH[num_node];
	PATH[0]=start_node;
	creat(start_node,1,PATH);
	
	if(bestpow==-1)
	{
		printf("无解\n");
	}

	unsigned short result[] = { 2, 6, 5 };//示例中的一个解
	for (int i = 0; i < 3; i++)
		record_result(result[i]);
}

int split(char dst[][20], char* str, const char* spl)//分解字符串函数
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
	int num_must;//必经点点数
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

int feasible_childnode(int **&A,int j,int arr[3][10],int num,int path[])//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[0][10]存储子节点的序号,arr[1][10]存储对应子节点的权重，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
{
	int k=0;//可行子节点
	int m;
	for(int i=0;i<edgenum;i++)
	{
		if(j==A[i][0])
		{
			m = 0;
			for(int n=0;n<num;n++)
			{
				if(A[i][1] != path[n])
				{
					m++;
				}
				if(m == num)
				{
					arr[0][k]=A[i][1];
					arr[1][k]=A[i][2];
					arr[2][k]=judge(num_must,must_arr,arr[0][k]);
					k++;
				}
			}
		}
		if(((i + 1) >= edgenum)||(((i == A[i][0]) && (i != A[i+1][0]))))
		{
			break;
		}
	}
	return k;
}

int judge(int nummust,int mustarr[50],int test)//输入必经点点数，必经点数组，待测点的序号,待测点是必经点，为true否则为false
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

int sec(time_t &G)
{
	time(&G);
	struct tm *TT;
	TT=localtime(&G);
	return TT->tm_sec+60*TT->tm_min+3600*TT->tm_hour;
}

int time_used(time_t &H)
{
	return (sec(H)-start_time);
}

double similar(int num1,int *arr1,int num2,int *arr2)//num1待测数组点数,arr1待测数组,num2比较数组点数,arr2比较数组
{
	int k=0;
	for(int i=0; i<num1; i++)
	{
		for(int j=0; j<num2; j++)
		{
			if(arr1[i]==arr2[j])
			{
				k++;
				break;
			}
		}
	}
	double f = k / (1.0 * num1);
	return f;
}