#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h> 

int split(char dst[][20], char* str, const char* spl);
char* itoa(int num,char*str,int radix);
int childnode(int **&A,int i,int arr[10]);//找到第i个节点的子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[10]存储子节点的序号，k存储子节点数目
int read_demand(char *demand,int must[],int &startnode,int &endnode);//demand是输入的condition，must[50]是必经节点数组，startnode返回起点序号，endnode返回终点的序号，函数返回必经点的点数
void shift_left(int m,int array[10]);//数组在m处右边的数据全部左移，删掉m点，不够的在末尾补array[9]的数据
int judge(int nummust,int mustarr[50],int numpass,int route[600]);//输入必经点点数，必经点数组，待测路径走过的点数，待测路径点的序号,符合条件为true不符合为false
int calculate_pow(int num, int pathpass[600]);//计算路径总权重，输入该路径的点数，该路径点的顺序，输出该路径的权重
int feasible_childnode(int **&A,int i,int arr[10],int num,int path[600]);//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[10]存储子节点的序号，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
#define num_max 600

//定义有600个点
int **a;//路径矩阵
int start;//start记录点的数目

int num_must;//必经点点数
int must_arr[50];//必经点集合
int start_node;//起始点
int end_node;//终点
int bestpath[600];//存储最好的路径
int bestpow=-1;//最好路径的权重
int bestnum;//最好的路径的点数


typedef struct str
{
	struct str *next;
	int point;//点序号
	int road[600];//已经经历点的路径顺序
	int passnum;//已经经历的点的数目
}node;

/*
typedef struct Arraylist
{
	struct str *pre;
	struct str *next;
	int road[600];//已经经历点的路径顺序
	int pow;//路径权重
}
*/




/*
void 
{
	node *c, *r;
	r = h;
	printf("输入链表长度：\n");
	scanf("%d", &n);
	for (int i = 0; i<n; i++)
	{
		c = (node *)malloc(sizeof(node));//分配一个节点的空间
		printf("输入数值：");
		scanf("%d",&(c->num));//输入该节点的值
		r->next = c;//r节点的尾巴连上c
		c->pre = r;//c节点的头连上r
		r = c;//r向后移一位
	}
	r->next = h->next;
	h->next->pre = r;//双向循环链表，即链表的头尾相连
}*/




//startpoint,1,path[0]=startpoint;
void creat(int pointnum,int num,int path[600])//pointnum当前点的点序号，num已经经历了的点的数目，path已经经历过的点 path[0]==startpoint;
{
	//int arr[10];
	int k;
	int arr[10];
	node *c= (node *)malloc(sizeof(node)),*h;
	h=(node *)malloc(sizeof(node));
	node *l=(node *)malloc(sizeof(node));
	c->next=NULL;
	c->passnum=num;
	c->point=pointnum;
	memcpy(c->road, path ,c->passnum * sizeof(int));
	node *r,*m,*q;
	for(int i=0;i<start+1;i++)
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
			q = r;
			
			k = feasible_childnode(a,r->point,arr,r->passnum,r->road);
			//printf("k:%d\n",k);
			for(int j=0; j<k; j++)
			{
				if(arr[j]==end_node)
				{
					c = (node *)malloc(sizeof(node));
					c->point=arr[j];
					c->passnum=r->passnum+1;
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[j];
					/*
					for(int u=0;u<c->passnum;u++)
					{
						printf("%d->",c->road[u]);
					}
					printf("\n");*/
					int estimate;		
					estimate=judge(num_must,must_arr,c->passnum,c->road);//判断路径是否符合条件
					if(estimate==1)
					{
						//符合条件的要存下来
						//计算路径总权重
						int pathpow=calculate_pow(c->passnum, c->road);
						if((bestpow==-1)||(bestpow > pathpow))
						{
							bestnum=c->passnum;
							bestpow = pathpow;
							memcpy(bestpath, c->road ,bestnum * sizeof(int));
						}
					}
				}
				else
				{
					c = (node *)malloc(sizeof(node));
					c->point=arr[j];
					c->passnum=r->passnum+1;
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[j];
					m->next=c;
					m=c;
					//printf("i:%d\n",i);
				}
			}
			r = r->next;
			if(q != h)
			{
				free(q);
			}
		}
		
		if(r != h)
		{
			free(r);
		}
		m->next = NULL;
	}
	
	/*
	int k;//可行的子节点数目
	int pathpow;
	int array[10];//即将要生成的子节点
	k=childnode(a,pointnum,array);//a是路径矩阵
	for(int m=0;m<k;m++)//生成的子节点如果已被经历，就把它去掉
	{
		for(int n=0;n<num;n++)
		{
			if(array[m]==path[n])
			{
				shift_left(m,array);//array数组的m点被删，m往后的数据左移一位
				m--;
				k--;
			}
		}
	}
	node *c[10];
	if(pointnum == end_node)
	{
		int estimate;		
		estimate=judge(num_must,must_arr,num,path);//判断路径是否符合条件
		if(estimate==1)
		{
			//符合条件的要存下来
			//计算路径总权重
			pathpow=calculate_pow(num, path);
			if(bestpow==-1)
			{
				bestnum=num;
				bestpow = pathpow;
				memcpy(bestpath, path ,bestnum * sizeof(int));
				printf("OK\n");
			}
			else if(bestpow > pathpow)
			{
				bestnum=num;
				bestpow = pathpow;
				memcpy(bestpath, path ,bestnum * sizeof(int));
			}
			return ;
		}
		else
		{
			//不符合条件的删掉并返回
			return ;
		}
	}
	else if(k==0)
	{
		//不符合条件的删掉并返回
		return ;
	}
	else
	{
		for(int i=0;i<k;i++)
		{
			c[i]=(node *)malloc(sizeof(node));//分配一个节点的空间
			c[i]->point = array[i];
			c[i]->passnum = num+1;
			memcpy(c[i]->road, path ,num * sizeof(int));
			c[i]->road[num] = array[i];
			creat(c[i]->point,c[i]->passnum,c[i]->road);
		}
	}*/
	
}




//你要完成的功能总入口
void search_route(char *graph[5000], int edge_num, char *condition)
{
	a = (int **)malloc(sizeof(int *) * num_max);
	for (int i = 0; i<num_max; i++)
	{
		a[i] = (int *)malloc(sizeof(int) * num_max);//分配每个指针所指向的数组
	}
	
	for (int i=0; i < num_max;i++)
		for (int j = 0; j < num_max; j++)
			*(*(a + i) + j) = 1000;
	
	int i=0;
	while (graph[i])
	{
		char dst[5][20];
		split(dst, graph[i], ",");
		start = atoi(dst[1]);
		int end = atoi(dst[2]);
		int pow = atoi(dst[3]);
		if(*(*(a+start)+end) > pow)
			*(*(a+start)+end) = pow;
		i++;
	}
	num_must = read_demand(condition,must_arr,start_node,end_node);//
	int PATH[600];
	PATH[0]=start_node;
	creat(start_node,1,PATH);
	if(bestpow==-1)
	{
		printf("\n无解\n");
	}
	else
	{
		printf("\n");
		for(int u=0;u<bestnum;u++)
		{
			printf("%d->",bestpath[u]);
		}
		printf("\n");
		printf("bestpow:%d",bestpow);
		printf("\n");
	}
		
	//printf("bestpow:%d\n",bestpow);
	/*	
	node *cc;
	cc=(node *)malloc(sizeof(node));
	cc->point=start_node;
	cc->passnum=1;
	cc->road[0]=start_node;
	
	printf("success");
	creat(cc->point,cc->passnum,cc->road);
	printf("bestpow:%d\n",bestpow);
	
	for(int qq=0;qq<bestnum;qq++);
	{
		printf("%d->",bestpath[qq]);
	}
	
	int aa[600];
	int num_aa=3;
	aa[0]=1;
	aa[1]=5;
	aa[2]=15;
	printf("part:%d,%d\n",a[1][5],a[5][15]);
	calculate_pow(num_aa, aa);
	
	int aa[9]={1,2,3,4,5,6,7,8};
	int bb[5];
	memcpy(bb, aa, 4 * sizeof(int));
	for(int p=0;p<5;p++)
	{
		printf("%d\n",bb[p]);
	}
	printf("%s\n",condition);
	int mustpass[50];
	int sta,end;
	int num_list = read_demand(condition,mustpass,sta,end);
	printf("startnode,endnode:%d,%d\n",sta,end);
	for(int j=0;j<num_list;j++)
	{
		printf("%d:%d\n",j,mustpass[j]);
	}
	

	int b[10];
	int t;
	t = childnode(a,0,b);
	
	printf("\nb:\n");
	for(int y=0;y<t;y++)
	{
		printf("%d\n",b[y]);
	}*/
	//printf("%d\n",t);
	//printf("\n");
	//
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

char* itoa(int num,char*str,int radix)
{	/*索引表*/
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned unum;/*中间变量*/
	int i=0,j,k;
	/*确定unum的值*/
	if(radix==10 && num<0)/*十进制负数*/
	{
		unum=(unsigned)-num;
		str[i++]='-';
	}
	else unum=(unsigned)num;/*其他情况*/
	/*转换*/
	do{
		str[i++]=index[unum % (unsigned)radix];
		unum/=radix;
	}while(unum);
	str[i]='\0';
	/*逆序*/
	if(str[0]=='-') k=1;/*十进制负数*/
	else k=0;
	char temp;
	for(j=k;j<=(i-1)/2;j++)
	{
		temp=str[j];
		str[j]=str[i-1+k-j];
		str[i-1+k-j]=temp;
	}
	return str;
}

int childnode(int **&A,int i,int arr[10])//找到第i个节点的子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[10]存储子节点的序号，k存储子节点数目
{
	/*	
	for(int j=0;j<10;j++)
	{
		arr[j]=-1;
	}	*/
	int k=0;
	for(int j=0; j<start; j++)
	{
		if(A[i][j] != 1000)
		{
			arr[k]=j;
			k++;
		}
	}
	return k;
}

int read_demand(char *demand,int must[50],int &startnode,int &endnode)
{
	char list[50][20];
	char sel[3][20];
	//int must[50];//必经点序列
	//int startnode,endnode;//起点，终点
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
	/*
	printf("startnode,endnode:%d,%d\n",startnode,endnode);
	for(int j=0;j<num_must;j++)
	{
		printf("%d:%d\n",j,must[j]);
	}*/
	return num_must;
}

void shift_left(int m,int array[10])
{
	for(;m<9;m++)
	{
		array[m]=array[m+1];
	}
	/*
	for(int i=0;i<10;i++)
	{
		printf("%d\n",array[i]);
	}
	*/
}

int judge(int nummust,int mustarr[50],int numpass,int route[600])//输入必经点点数，必经点数组，待测路径走过的点数，待测路径点的序号,符合条件为true不符合为false
{
	int num = 0;
	if(nummust==0)
	{
		return 1;
	}
	else
	{
		for(int i=0;i<nummust;i++)
		{
			for(int j=0;j<numpass;j++)
			{
				if(mustarr[i]==route[j])
				{
					num++;
				}
			}
		}
	}
	if(num == nummust)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int calculate_pow(int num, int pathpass[600])//计算路径总权重，输入该路径的点数，该路径点的顺序，输出该路径的权重
{
	int sum_pow=0;
	for(int i=0;i<(num-1);i++)
	{
		sum_pow += a[pathpass[i]][pathpass[i+1]];
	}
	return sum_pow;
}

int feasible_childnode(int **&A,int i,int arr[10],int num,int path[600])//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[10]存储子节点的序号，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
{	
	int k=0;//可行的子节点
	int m;
	for(int j=0; j<start; j++)
	{
		m=0;
		if(A[i][j] != 1000)
		{
			for(int n=0;n<num;n++)
			{
				if(j != path[n])
				{
					m++;
				}
				if(m == num)
				{
					arr[k]=j;
					k++;
				}
			}
		}
	}		
	return k;
}