#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h> 
#include "time.h"
#include "math.h"
extern int **a;//边的矩阵
extern int bestpow;//最好路径的权重
extern int bestnum;//最好的路径的边数
extern int edgenum;//边的条数
extern int num_must;//必经点点数
extern int must_arr[50];//必经点集合
extern int start_node;//起始点
extern int end_node;//终点
extern int num_node;//点的总数

extern double x1,x2;
 const int re_compare_num=100;//记忆库的容量
extern int bestpath[600];//存储最好的路径

typedef struct re_strc
{
	int father_edge;
	struct re_strc *next;
}feasible_father_edge;

typedef struct re_str
{
	int num;//可行边条数
	feasible_father_edge *head; //存储数据链的头
}e2s_edge;
e2s_edge *e2s[600];
typedef struct re_infostr
{
	int score[re_compare_num]; //路径经历的必经点点数
}re_info_node;
int feasible_fathernode(int **&A,int end,feasible_father_edge *head,int numc,int path[],int mustpass);
re_info_node *re_node_info[600];

typedef struct re_strcc
{
	struct re_strcc *next;
	int point;
	int road[599];//已经经历点的路径顺序
	int passnum;//已经经历的边的数目
	int pow;//路径加总权重
	int mustnum;//路径经历的必经点点数
	double score;//得分
}re_node;

int re_calculate_score(re_node *&A, re_info_node *&B)
{
	//double E;
	double low=x1*100.0,high=-1*x1*100.0;
	int l;
	
	for(int i=re_compare_num-1; i>=0 ; i--)
	{
		if(low > B->score[i])
		{
			low = B->score[i];
			l = i;
		}
		if(high < B->score[i])
		{
			high = B->score[i];
		}
	}
	if(A->score < low)
	{
		return 0;
		/*
		E = (A->score-low);
		if((pow(3,E/x1)*rate)>(rand()/RAND_MAX))
		{
			return 1;
		}
		else
		{
			return 0;
		}*/
	}
	else
	{
		B->score[l]=A->score;
		return 1;
	}
}

void re_create()
{
	int i;
	int k;
	feasible_father_edge *head;
	head=(feasible_father_edge *)malloc(sizeof(feasible_father_edge));
	int shortest[600];
	for(i=0;i<num_node;i++)
	{
		shortest[i]=12000;
	}
	re_node *c= (re_node *)malloc(sizeof(re_node));
	re_node *h= (re_node *)malloc(sizeof(re_node));
	re_node *l= (re_node *)malloc(sizeof(re_node));
	c->mustnum=0;
	c->pow=0;
	c->next=NULL;
	c->passnum=0;
	c->point=end_node;
	c->score=0;
	re_node *r,*m,*q,*p;
	feasible_father_edge *index;
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
		
		while(r)
		{
			p=r;
			//printf("i:%d\n",i);
			k =feasible_fathernode(a,r->point,head,r->passnum,r->road,r->mustnum);//可行点，权重，分数，
			index=head;
			//printf("k:%d\n",k);
			for(int j=0; j < k ; j++)
			{	
				index=index->next;
				c = (re_node *)malloc(sizeof(re_node));
				if(a[index->father_edge][4]==-1)
				{
					//printf("a1\n");
					c->point=a[index->father_edge][1];
					c->passnum=r->passnum+1;
					c->pow=r->pow+a[index->father_edge][3];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=index->father_edge;
					int estimate= (r->mustnum==num_must) ? 1 : 0;//判断路径是否符合条件		
					if(estimate==1)
					{
						if((bestpow==-1)||(bestpow > (c->pow)))
						{
							bestnum = c->passnum;
							bestpow = c->pow;
							memcpy(bestpath, c->road ,bestnum * sizeof(int));

							/*
							printf("bestpow:%d path:",bestpow);
							for(int o=bestnum-1;o>=0;o--)
							{
								printf("%d,",a[bestpath[o]][1]);
								printf("%d,",a[bestpath[o]][2]);
							}
							printf("\n");*/
						}
					}
					free(c);
					//printf("a2\n");
				}
				else if( ( ( r->pow + a[index->father_edge][3] ) < shortest[a[index->father_edge][1]] ) )
				{
					//printf("b1\n");
					shortest[a[index->father_edge][1]] = r->pow + a[index->father_edge][3];
					if(a[index->father_edge][4]==1)
					{
						c->mustnum = r->mustnum + 1;
					}
					else
					{
						c->mustnum = r->mustnum;
					}
					c->point=a[index->father_edge][1];
					c->passnum=r->passnum+1;
					c->pow=r->pow+a[index->father_edge][3];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=index->father_edge;
					c->score=r->score+x1*a[index->father_edge][4]-x2*a[index->father_edge][3];
					m->next=c;
					m=c;
					//printf("b2\n");
				}
				else
				{
					//printf("c1\n");
					if(a[index->father_edge][4]==1)
					{
						c->mustnum = r->mustnum + 1;
					}
					else
					{
						c->mustnum = r->mustnum;
					}
					c->point=a[index->father_edge][1];
					c->passnum=r->passnum+1;
					c->pow=r->pow+a[index->father_edge][3];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=index->father_edge;
					c->score=r->score+x1*a[index->father_edge][4]-x2*a[index->father_edge][3];
					//srand((rand_seed++)+(int)time(NULL));
					if(re_calculate_score(c, re_node_info[a[index->father_edge][1]])==1)
					{
						m->next=c;
						m=c;
					}
					else
					{
						free(c);
					}
					//printf("c2\n");
				}
			}
			index=head;
			for(int free_index=0;free_index<k;free_index++)
			{
				index=index->next;
				free(index);
			}
			r = r->next;
			free(p);
		}
		m->next = NULL;
	}	
}

//你要完成的功能总入口
void re_search_route(char *graph[5000], int edge_num, char *condition)
{
    x1=500.0,x2=1.0; 
    bestpow=600*20;//最好路径的权重   
	edgenum=edge_num;
	
	num_must = read_demand(condition,must_arr,start_node,end_node);
	a = (int **)malloc(sizeof(int *) * 5000);
	int i;
	for (i = edge_num-1;i>=0;i--)
	{
		a[i] = (int *)malloc(sizeof(int) * 5);
		char dst[5][15];
		split(dst, graph[i], ",");
		a[i][0] = atoi(dst[0]);//边序号
		a[i][1] = atoi(dst[1]);//起点
		a[i][2] = atoi(dst[2]);//终点
		a[i][3] = atoi(dst[3]);//权重
		a[i][4] = 0;//普通边，边信息
		if(a[i][1]==start_node)
		{
			a[i][4] = -1;//起点边
		}
		
		for(int j=0;j<num_must;j++)
		{
			if(a[i][1] == must_arr[j])
			{
				a[i][4] = 1;//必经点前馈边
			}
		}
		num_node=num_node>a[i][1]?num_node:a[i][1];
	}
	num_node++;

	for(i=num_node-1;i>=0;i--)
	{
		e2s[i]=(e2s_edge *)malloc(sizeof(e2s_edge));
		e2s[i]->num=0;
		e2s[i]->head=(feasible_father_edge *)malloc(sizeof(feasible_father_edge));
	}
	for (i = edge_num-1;i>=0;i--)
	{
		
		feasible_father_edge *c,*r;
		r=e2s[a[i][2]]->head;
		
		for(int j=0;j<e2s[a[i][2]]->num;j++)
		{
			r=r->next;
		}
		c=(feasible_father_edge *)malloc(sizeof(feasible_father_edge));
		c->father_edge=i;
		r->next=c;
		e2s[a[i][2]]->num++;
	}

	for(i=num_node-1; i>=0; i--)
	{
		re_node_info[i] = (re_info_node *)malloc(sizeof(re_info_node));
		for(int j=re_compare_num-1;j>=0;j--)
		{
			re_node_info[i]->score[j]=-12000;
		}
	}
	
	re_create();

	for(int o=bestnum-1;o>=0;o--)
	{
		record_result(a[bestpath[o]][0]);
	}
	
	for(i=num_node-1; i>=0; i--)
	{
		free(re_node_info[i]);
	}
	for(i=num_node-1; i>=0; i--)
	{
		free(e2s[i]->head);
		free(e2s[i]);
	}
	for (i = edge_num-1;i>=0;i--)
	{
		free(a[i]);
	}
	free(a);
	
/*
	
    unsigned short result[] = {2, 6, 3};//示例中的一个解

    for (int i = 0; i < 3; i++)
        record_result(result[i]);*/
}

int feasible_fathernode(int **&A,int end,feasible_father_edge *head,int numc,int path[],int mustpass)
{
	
	int k=0;//可行子节点
	int m;
	feasible_father_edge *r,*q,*c;
	r=e2s[end]->head;
	q=head;
	for(int i=0;i<e2s[end]->num;i++)
	{
		r=r->next;
		if(A[r->father_edge][4]==-1)//起点边
		{
			if(mustpass>=num_must)
			{
				c=(feasible_father_edge *)malloc(sizeof(feasible_father_edge));
				c->father_edge=r->father_edge;
				head->next=c;//
				k=1;
				return k;
			}
			else
			{
				continue;
			}	
		}
		m = 0;
		for(int n=0;n<numc;n++)
		{
			if(n==0)
			{
				if(A[r->father_edge][1] != A[path[n]][1])
				{
					m++;
				}
				if(A[r->father_edge][1] != A[path[n]][2])
				{
					m++;
				}
			}
			else
			{
				if(A[r->father_edge][1] != A[path[n]][1])
				{
					m++;
				}
			}
		}
		if(m == (numc+1))
		{
			c=(feasible_father_edge *)malloc(sizeof(feasible_father_edge));
			c->father_edge=r->father_edge;
			q->next=c;
			q=q->next;
			k++;
		}
		else if(numc==0)
		{
			c=(feasible_father_edge *)malloc(sizeof(feasible_father_edge));
			c->father_edge=r->father_edge;
			q->next=c;
			q=q->next;
			k++;
		}
	}
	return k;
}

