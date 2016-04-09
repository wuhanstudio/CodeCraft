#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h> 
#include<vector>
#include<list>
#include <queue>
#include <algorithm>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
#include "ant_search.h"

int split(char dst[][15], char* str, const char* spl);
int read_demand(char *demand,int must[],int &startnode,int &endnode);//demand是输入的condition，must[50]是必经节点数组，startnode返回起点序号，endnode返回终点的序号，函数返回必经点的点数
int feasible_childnode(int **&A,int j,int arr[2][10],int num,int path[]);//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[0][10]存储子节点的序号,arr[1][10]存储对应子节点的权重，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
int judge(int nummust,int mustarr[50],int test);//输入必经点点数，必经点数组，待测点的序号,待测点是必经点，为true否则为false
int sec(time_t &G);//返回当前的秒数
int time_used(time_t &H);//返回以用的时间
void create();
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

const int compare_num=30;//每个点路径信息最大存储数，用于比较
 int use_compare_num=compare_num;
double rate=0.8;
double x1=500,x2=4;//x1必经点数量权重，x2路径权值和的权重

typedef struct str
{
	struct str *next;
	int point;//点序号
	int road[600];//已经经历点的路径顺序
	int passnum;//已经经历的点的数目
	int pow;//路径加总权重
	int mustnum;//路径经历的必经点点数
}node;


typedef struct infostr
{
	int must_num[compare_num];//路径经历的必经点点数
	int sumpow[compare_num];//路径加总权重
}info_node;


info_node *node_info[600];

//你要完成的功能总入口
void search_route(char *graph[5000], int edge_num, char *condition)
{
    if(edge_num>550)
    {
        //printf("蚂蚁算法\n");
		/*----------建立邻接链表------*/
        int num_node = 0;
        std::vector<adjlist> adj_vec = build_adjlist(graph, edge_num, &num_node);

        /*----------分析需求点集------*/
        std::vector<int> deman_vec = analysis_demand(condition,&st, &en);

        /*--------------预处理---------*/
        std::vector<int> TubaVec;
        demanNoTuba(adj_vec, deman_vec, TubaVec, num_node);
        pre_process(&adj_vec, num_node, TubaVec);

        for (int i = 0; i < num_node; i++)
        {
            if (adj_vec[i].size() == 0)
            {
                std::vector<int>::iterator it = find(deman_vec.begin(), deman_vec.end(), i);
                if (it != deman_vec.end())
                {
                    //printf("NA\n");
                    return;
                }
            }
        }
        path best_path;
        if (num_node < ThresholdNode)
        {
            /*方式一-----------回溯寻路---------*/
            best_path = find_path(st, en, deman_vec, adj_vec);
            if (best_path.cost == DB_MAX)
            {
                //printf("NA\n");
                return;
            }
        }
        else
        {
            /*方式二-----------蚁群寻路---------*/
            double **g_Trial; //两两城市间信息素，就是环境信息素
            g_Trial = (double **)malloc(sizeof(double *)*num_node);
            for (int i = 0; i<num_node; i++)
                g_Trial[i] = (double *)malloc(sizeof(double) * num_node);

            double **g_Distance; //两两城市间距离
            g_Distance = (double **)malloc(sizeof(double *)*num_node);
            for (int i = 0; i<num_node; i++)
                g_Distance[i] = (double *)malloc(sizeof(double) * num_node);


            CTsp tsp;
            tsp.InitData(num_node, adj_vec, g_Distance, g_Trial); //初始化
            tsp.Search(deman_vec, adj_vec, g_Distance, g_Trial); //开始搜索
            //输出路径结果，以顶点名表示
            //printf("\nThe start point is: %d , The end point is: %d\n\n", st, en);
            if (tsp.m_cBestAnt.m_dbPathLength == DB_MAX)
            {
                //printf("NA\n");
                return;
            }

            //printf("Vertex represents the best path:  ");
            //for (int i = 0; i<tsp.m_cBestAnt.m_nMovedCityCount - 1; i++)
                //printf("%d-> ", tsp.m_cBestAnt.m_nPath[i]);
            //printf("%d ", tsp.m_cBestAnt.m_nPath[tsp.m_cBestAnt.m_nMovedCityCount - 1]);

            best_path = NodenoIndexPath(adj_vec, tsp.m_cBestAnt);
                    /*----------将结果写入文件-------------*/
            for (int i = 0; i < best_path.edg_name.size(); i++)
                record_result(best_path.edg_name[i]);

            /*----------输出路径结果，以边名表示-------------*/
            //printf("\nEdge represents the best path:  ");
            //for (int j = 0; j < best_path.edg_name.size(); j++)
            //{
                //printf("%d|", best_path.edg_name[j]);
            //}
            //printf("\nCost=%lf,\n\n", best_path.cost);

        }
    }
    else
    {
        edgenum=edge_num;
        a = (int **)malloc(sizeof(int *) * 5000);
        int i;
        num_node = 0;
        for (i = edge_num-1;i>=0;i--)
        {
            a[i] = (int *)malloc(sizeof(int) * 4);
            char dst[5][15];
            split(dst, graph[i], ",");
            a[i][0] = atoi(dst[1]);
            a[i][1] = atoi(dst[2]);
            a[i][2] = atoi(dst[3]);
            a[i][3] = atoi(dst[0]);
            num_node = a[i][1]> num_node?a[i][1]:num_node;
        }
        num_node=num_node+1;
        num_must = read_demand(condition,must_arr,start_node,end_node);
        // use_compare_num=20;//每个点路径信息最大存储数，用于比较
        // rate=0.8;
        // x2=4;//x1必经点数量权重，x2路径权值和的权重	
        
        if(num_node<=20)   //1-5
        {
            use_compare_num=25;//每个点路径信息最大存储数，用于比较
            rate=0.8;
            x2=4;//x1必经点数量权重，x2路径权值和的权重
        }
        else if(num_node<=100) //6
        {
            use_compare_num=25;//每个点路径信息最大存储数，用于比较
            rate=0.8;
            x2=4;//x1必经点数量权重，x2路径权值和的权重
        }
        else if(num_node<=150) //7
        {
            use_compare_num=15;//每个点路径信息最大存储数，用于比较
            rate=0.8;
            x2=4;//x1必经点数量权重，x2路径权值和的权重
            x1=300;
        }
        else if(num_node<=200) //8
        {
            use_compare_num=20;//每个点路径信息最大存储数，用于比较
            rate=0.8;
            x2=4;//x1必经点数量权重，x2路径权值和的权重
        }
        else if(num_node<=250) //9
        {
            use_compare_num=23;//每个点路径信息最大存储数，用于比较
            rate=0.8;
            x2=20;//x1必经点数量权重，x2路径权值和的权重
        }
        else if(num_node<=300)//10
        {
            use_compare_num=5;//每个点路径信息最大存储数，用于比较
            rate=0.8;
            x2=4;//x1必经点数量权重，x2路径权值和的权重
        }

        // INCREDIBLE
        else if (num_node<=550) 
        {
            if(num_must>30)  //11
            {
                use_compare_num=10;//每个点路径信息最大存储数，用于比较
                rate=0.8;
                x2=4;//x1必经点数量权重，x2路径权值和的权重
            }
            else             // 12-13
            {
                use_compare_num=25;//每个点路径信息最大存储数，用于比较
                rate=0.8;
                x2=30;//x1必经点数量权重，x2路径权值和的权重
            }
        }

        for(i=num_node-1; i>=0; i--)
        {
            node_info[i] = (info_node *)malloc(sizeof(info_node));
            for(int j=compare_num-1;j>=0;j--)
            {
                node_info[i]->must_num[j]=0;
                node_info[i]->sumpow[j]=12000;
            }
        }	
        create();
        if(bestpow>-1)
        {
            for (i = 0; i < bestnum-1; i++)
            {
                for(int j=0;j<edge_num;j++)
                {
                    if(a[j][0]==bestpath[i]&&a[j][1]==bestpath[i+1])
                    {
                        record_result(a[j][3]);
                    }
                }
            }
        }
        
        for(i=num_node-1; i>=0; i--)
        {
            free(node_info[i]);
        }
        for (i = edge_num-1;i>=0;i--)
        {
            free(a[i]);
        }
        free(a);	
    }
       
}

int calculate_score(node *&A, info_node *&B)
{
	double C[use_compare_num];
	double D = (x1*A->mustnum) - (x2*A->pow);
	double low=x1*50.0, high=-50000;
	double E;
	int l;
	for(int i=use_compare_num-1;i >=0; i--)
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
        int bonouns=1;
        double add = 0.0;
       	if(num_node<=20)   //1-5
        {
                bonouns = 1;
                add = 0;
        }
        else if(num_node<=100) //6
        {
                bonouns = 7;
                add = -0.1;                
        }
        else if(num_node<=150) //7
        {
                bonouns = 10;
                add = -0.5;
        }
        else if(num_node<=200) //8
        {
                bonouns = 4;
                add = 0.0;
        }
        else if(num_node<=250) //9
        {
                bonouns = 3;
                add = 0.0;
        }
        else if(num_node<=300)//10
        {
                bonouns = 2;
                add = -0.5;
        }

        // INCREDIBLE
        else if (num_node<=550) 
        {
            if(num_must>30)  //11
            {
                bonouns = 1;
                add = -0.1;
            }
            else             // 12-13
            {
                bonouns = 2;
                add = 0;
            }
        }
        else                   // 14-15
        {
            bonouns = 5;
            x1=1;            
            add = 0.0;
        }
        double score = (1-E)*(bonouns+add)*rand()/(RAND_MAX+1.0);
		if(score>rate)
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

void create()//pointnum当前点的点序号，num已经经历了的点的数目，path已经经历过的点 path[0]==startpoint;
{
    bool happy = false;
	int k;
	int arr[3][10];
	int i;
	int shortest[600];
	for(i=num_node-1;i>=0;i--)
	{
		shortest[i]=12000;
	}
	node *c= (node *)malloc(sizeof(node));
	node *h= (node *)malloc(sizeof(node));
	node *l= (node *)malloc(sizeof(node));
	c->mustnum=0;
	c->pow=0;
	c->next=NULL;
	c->passnum=1;
	c->point=start_node;
	c->road[0]=start_node;
	node *r,*m,*q;
	for(i=0;i<num_node;i++)
	{
        if(happy) break;
		if(i==0)
		{
			l->next=c;
		}
		q=l;
		l=h;
		h=q;

		r = h->next;
		m = l;
		while(r)
		{
            if(happy) break;            
			q = r;
			k = feasible_childnode(a,r->point,arr,r->passnum,r->road);

			for(int j=0; j < k; j++)
			{
                if(happy) break;                
				if(arr[0][j]==end_node)
				{
					c = (node *)malloc(sizeof(node));
					c->point=arr[0][j];
					c->passnum=r->passnum+1;
					c->pow=r->pow+arr[1][j];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[0][j];
                    c->mustnum = r->mustnum;
					int estimate= (c->mustnum==num_must) ? 1 : 0;//判断路径是否符合条件
					if(estimate==1)
					{

						if((bestpow==-1)||(bestpow > (c->pow)))
						{
							bestnum = c->passnum;
							bestpow = c->pow;
							memcpy(bestpath, c->road ,bestnum * sizeof(int));
                            if(num_node>550)
                            {
                                for (i = 0; i < bestnum-1; i++)
                                {
                                    for(int j=0;j<edgenum;j++)
                                    {
                                        if(a[j][0]==bestpath[i]&&a[j][1]==bestpath[i+1])
                                        {
                                            record_result(a[j][3]);
                                        }
                                    }
                                }
                                happy = true;
                                break;
                            }
                            
							// printf("bestpow:%d path:",bestpow);
							// for(int o=0;o<bestnum;o++)
							// {
							// 	printf("%d,",bestpath[o]);
							// }
							// printf("\nmustnode:");
							// for(int o=0;o<c->mustnum;o++)
							// {
							// 	printf("%d|",c->mustnode[o]);
							// }
							// printf("\n");
                            
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
					}
					else
					{
						c->mustnum = r->mustnum;
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
					}
					else
					{
						c->mustnum = r->mustnum;
					}
					c->point=arr[0][j];
					c->passnum=r->passnum+1;
					c->pow=r->pow+arr[1][j];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[0][j];
					if(arr[2][j]==1)
                    {
                        m->next=c;
						m=c;	
                    }                    
					else if(calculate_score(c, node_info[arr[0][j]])==1)
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
	free(h);
	free(l);
}

int split(char dst[][15], char* str, const char* spl)//分解字符串函数
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
	char list[50][15];
	char sel[3][15];
	int num_must;//必经点点数
	num_must=split(list, demand, "|");
	split(sel,list[0],",");
	startnode=atoi(sel[0]);
	endnode=atoi(sel[1]);
	if(sel[2] != NULL)
	{
		must[0]=atoi(sel[2]);
	}
	for(int k=num_must-1;k>=1;k--)
	{
		must[k]=atoi(list[k]);
	}
	return num_must;
}

int feasible_childnode(int **&A,int j,int arr[3][10],int num,int path[])//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[0][10]存储子节点的序号,arr[1][10]存储对应子节点的权重，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
{
	int k=0;//可行子节点
	for(int i=0;i<edgenum;i++)
	{
		if(j==A[i][0])
		{
			bool passed=false;
			for(int n=num-1;n>=0;n--)
			{
				if(A[i][1] == path[n])
				    passed = true;
			}
			if(!passed)
			{
				arr[0][k]=A[i][1];
				arr[1][k]=A[i][2];
				arr[2][k]=judge(num_must,must_arr,arr[0][k]);
				k++;
			}
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



