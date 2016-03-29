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

int **a;//记录边信息的矩阵
int edgenum;//边的条数
int num_must;//必经点点数
int must_arr[50];//记录必经点的集合
int start_node;//起始点
int end_node;//终点
int num_node;//点的总数
int bestpath[600];//存储最好的路径
int bestpow=-1;//存储最好路径的权重
int bestnum;//最好的路径的点数
int start_time;//开始时间
time_t T;//计时用的结构体//时间模块可以不必太在意
const int compare_num=10;//记忆库的容量
const double rate =0.7;//容错的比率
double x1=500,x2=1;//x1为必经点数量权重，x2路径权值和的权重 score=x1*mustnum-x2*sumpow  score分数越高代表该路径越好


typedef struct str
{
	struct str *next;
	int point;//点序号
	int road[600];//已经经历点的路径顺序
	int passnum;//已经经历的点的数目
	int pow;//路径加总权重
	int mustnum;//路径经历的必经点点数
	int mustnode[50];//已经经历的必经点
}node;//路径生成树里要生成的链结，里面存储当前点的点序号point，该路径目前经历过的点的顺序road[600],已经经历过的点的数目passnum，经历过的路径总权重pow，该条路径包含的必经点点数mustnum以及必经点点的集合数组mustnode[50]


typedef struct infostr
{
	int must_num[compare_num];//路径经历的必经点点数
	int sumpow[compare_num];//路径加总权重
}info_node;//score=x1*must_num-x2*sumpow  score分数越高代表该路径越好
//记忆库结构体，compare_num为记忆库的容量，前面定义为10，意思是对于点i来说，经历i点的前十个最佳路径会被存储在info_node[i]里,里面包含的信息包括到达i点路径的必经点点数must_num，到达i点路径的权重，这样的路径会保存10个，用于之后的比较，如果出现比10个当中最差路径更好的，就会把该记忆库最差的路径信息给替换掉

info_node *node_info[600];//由于最大有600个点，所以先生命600个指针

int calculate_score(node *&A, info_node *&B)//待测的链结A，对应节点的记忆库B，比如待测的链结当前节点为1，就调出1的记忆库
{
	double C[compare_num];
	double D = x1*A->mustnum - x2*A->pow;//对当前待测的链结A进行打分
	double low=x1*50.0,high=-50000;//low是为了记下记忆库最低分，所以一开始给一个最大值，high是为了记下记忆库的最高分，所以一开始给一个最小值
	int l;//是为了记下最低分对应的记忆库的位置
	double E;//是一个判断系数
	
	for(int i=0;i < compare_num; i++)
	{
		C[i] = x1*B->must_num[i] - x2*B->sumpow[i];//给记忆库的每个记忆打分
		if(low > C[i])
		{
			low=C[i];//记下最低分
			l=i;//记下最低分对应记忆的位置，方便之后把最低分记忆给换掉
		}
		if(high < C[i])
		{
			high = C[i];//记下最高分
		}
		
	}
	if(D<low)//如果待测链接的得分比记忆库里的最低分还要低，就有一定的概率该链结会被剪枝
	{
		E = (low-D)/(high-D);//这里是一个比例系数，所得值范围为（0,1），开环区间，E越是接近0，即得分与low的分数越是接近，被保留下来的概率越大，否则越是接近1，得分远远小于low，被保留下的概率就越小
		if(((1-E)*rand()/(RAND_MAX+1.0))>rate)//rand()/(RAND_MAX+1.0)是0到1的随机数，比如0.3456。（1-E）的情况与上面所述正好相反，这里rate是一个容错概率，假如（1-E）==1，rate==0.8,那么该链结就有0.2的概率被保留下来。rate越大，留下来越困难
		{
			return 1;//允许让待测子节点继续生成
		}
		else
		{
			return 0;//不允许让待测子节点继续生成
		}
	}
	else
	{
		B->must_num[l]=A->mustnum;//如果待测链接的得分比最低分的记忆高就把最低分记忆给替换掉
		B->sumpow[l]=A->pow;
		return 1;//允许让待测子节点继续生成
	}
}

/*                                                              			i
                      			startnode 比如为5                      	 	0
	子树1：    		1 				4			1
	子树2：               2    3                           2   3                     2
	子树3：             5  8  8 7                        4  3  1 4                   3
	....
	....
	....
       
       i=0:	
       l->5->NULL
       h->
       经历了q=l;l=h;h=q;后
       h->5->NULL
       l->
       在h后面的信息使用完之后：
       h->
       l->2->3->2->3->NULL
       
       i=1:
       经历了q=l;l=h;h=q;后
       h->2->3->2->3
       l->
       在h后面的信息使用完之后：
       h->
       l->5->8->8->7->4->3->1->4->NULL
	
       i=2:
       经历了q=l;l=h;h=q;后
       h->5->8->8->7->4->3->1->4->NULL
       l->
	....
	....


*/
void creat(int pointnum,int num,int path[])//pointnum当前点的点序号，num已经经历了的点的数目，path已经经历过的点 path[0]==startpoint;
{
	int k;
	int arr[2][10];//一个临时2维数组，用于记录生成的可行点，已经生成的可行点的权重
	int i;
	//printf("num_node:%d\n",num_node);
	int shortest[600];//这里可以参考dijkstra算法，存下当前到达点i的路径的最短权重shortest[i]
	for(i=0;i<num_node;i++)
	{
		shortest[i]=12000;
	}//一开始赋值时，shortest很大很糟糕，方便后面的替换
	node *c= (node *)malloc(sizeof(node));//申请根链结，也就是记录起点的信息
	c->mustnum=0;//在只包含起点的路径里，必经点数量一定为0
	c->pow=0;//在只包含起点的路径里，路径权重一定为0
	c->next=NULL;//这里是单向链表，把该链结的尾巴指向NULL，因为后面有个while(r),NULL可以作为循环的终止条件
	c->passnum=num;//在只包含起点的路径里，路径经过的点数一定为1
	c->point=pointnum;//在只包含起点的路径里，当前点c->point一定为起点
	memcpy(c->road, path ,c->passnum * sizeof(int));//把路径点的顺序path复制到c->road里
	
	node *h=(node *)malloc(sizeof(node));//h是一个链表的表头，h后面的链结串是要生成子树的，每判断完一个链结后，就会把该链结给删掉判断下一个链结
	node *l=(node *)malloc(sizeof(node));//l也是一个链表的表头，l后面一开始是空的，但是随着h后面链结子树的生，l作为表头，开始向后记录h子树的信息
	node *r,*m,*q;//r,m,q,为中间变量，r主要作用为read方便读取h链结的信息，读完一个链结后r=r->next,r指向下一个，再读下一个。q的作用主要是为了删除已经用完的链结，q=r,在r用完当前链接的信息指向了下一个链结，即r=r->next后，free(q),把上一个链结给删除了。m的作用是连接生成的的子树链结，把他们连在l后面，m一开始是m=l,然后生成了一个子链结c后,m->next=c，也就是l->next=c ,l的尾巴挂上了c,然后m=c,重新申请一个空间地址c，再m->next=c，m=c;直到没有子树生成m->next=NULL;
	for(i=0;i<num_node;i++)
	{
		if(i==0)
		{
			l->next=c;//一开始链表的表头l后面只有一个链结，也就是起点链结c，表头h后面没有链结，后面把h和l的地址进行交换，让h后面有链结信息，l后面为空
		}
		
		q=l;
		l=h;
		h=q;//这里q仅仅是作为中间变了，方便h和l的信息交换

		r = h->next;//r指向表头后第一个链结,这里r的作用是为了读取链结信息，读完后再指向下一个
		m = l;//m指向表头l，m是为了连接生成的子树的,把他们都连在l后面，最后结果就像图解上面的情况
		
		if(time_used(T)>=100)
		{
			printf("超时!最大深度:%d\n",r->passnum);
			break;
		}//这里不用太在意，为了测试算法性能的
		while(r)//读取h后面的链结的，r为NULL时终止循环
		{
			q = r;//这里q是为了释放已经用完的节点空间
			k = feasible_childnode(a,r->point,arr,r->passnum,r->road);//这里是可行子树的生成，r->passnum,r->road：这是父节点的路径信息包括路径经过的点数，以及路径数组，arr[2][10]里面存下可以用的生成点，如果父节点里面r->road[600]已经包含了即将要生成的子节点,那么就会把该子节点删去，k是要生成的可用子节点点数，这里是保证生成的路径不会有重复点
			//printf("bbb\n");
			for(int j=0; j < k; j++)//对每个可用的子节点进行判断
			{
				if(arr[0][j]==end_node)//如果该子节点为终点，那么判断该路径是否满足条件，满足条件就输出，作为我们的可行解之一
				{
					r->point=arr[0][j];
					r->road[r->passnum]=arr[0][j];
					r->passnum=r->passnum+1;
					r->pow=r->pow+arr[1][j];
                    
                    int tt=0;
                    while(r->road[tt])
                    {
                        //printf("%d->",r->road[tt]);
                        tt++;
                    }
                    //printf("\n");
                    int nn=0;
                    int pp =0;
                    int match=0;
                    while(r->road[nn])
                    {
                        pp=0;
                        while(must_arr[pp])
                        {
                            if(r->road[nn]==must_arr[pp])
                            {
                               //printf("%d & %d匹配\n",r->road[nn],must_arr[pp]);                                                            
                               match++;;  
                            }
                            //else
                                //printf("%d & %d不匹配\n",r->mustnode[nn],must_arr[pp]);                            
                            pp++;                    
                        }
                        nn++;
                    }
                    //getchar();
					int estimate = (match>=num_must) ? 1 : 0;//判断路径是否符合条件	
                    //printf("判断结果:%d,%d-->%d\n",match,num_must,estimate);
                    	
					if(estimate==1)
					{
						//符合条件的要存下来
						//计算路径总权重
						//int pathpow=calculate_pow(c->passnum, c->road);
						int countresult = 0;
						char name[50];
						if((bestpow==-1)||(bestpow > (r->pow)))
						{
							bestnum = r->passnum;
							bestpow = r->pow;
							memcpy(bestpath, r->road ,bestnum * sizeof(int));
							printf("bestpow:%d\nbestpath:\n",bestpow);
							sprintf(name,"result_%d.txt",countresult);
							FILE *fid=fopen(name,"w");
					        int o;
							for(o=0;o<bestnum-1;o++)
							{
								//printf("%d,",bestpath[o]);                                
								//if(bestpath[o+1])
								//{
                                    //printf("%d:%d,%d\n",o,bestpath[o],bestpath[o+1]);
                                    printf("%d->",bestpath[o]);
									fprintf(fid,"%d,%d\n",bestpath[o],bestpath[o+1]);
								//}
                            };
                            printf("%d\n",bestpath[o]);
                            o=0;
                            //while(r->mustnode[o])
							//{
                            //       printf("结果经历的必经点%d:\n",r->mustnode[o]);
                            //        o++;
							//}	
							fclose(fid);
							countresult++;							
							printf("\n");
						}
					}
				}
				else if( ( ( r->pow + arr[1][j] ) < shortest[arr[0][j]] ) )//这里用dijkstra的一个公式，找到到达节点j的最短路径，如果新生成的路径权值大于这个最短路径就把他的子树给减去，不让他再往下长
				{
					shortest[arr[0][j]] = r->pow + arr[1][j];
					c = (node *)malloc(sizeof(node));
					if(judge(num_must,must_arr,arr[0][j])==1)
					{
						c->mustnum = r->mustnum + 1;
                        //printf("找到第%d个必经点:%d\n",c->mustnum,arr[0][j]);
                        for(int i=0;i<r->mustnum;i++)
                        {
                            c->mustnode[i]=r->mustnode[i];
                        }
                        //printf("已找到的必经点:\n");
							//for(int o=0;o<r->mustnum;o++)
							//{
							//	printf("%d\n",c->mustnode[o]);
							//}	
						//memcpy(c->mustnode, r->mustnode, r->mustnum * sizeof(int));
						c->mustnode[r->mustnum]=arr[0][j];
					}
					else
					{
						c->mustnum = r->mustnum;
                        for(int i=0;i<r->mustnum;i++)
                        {
                            c->mustnode[i]=r->mustnode[i];
                        }
                        	//printf("mustNUM:\n");
							//for(int o=0;o<r->mustnum;o++)
							//{
							//	printf("%d\n,",r->mustnode[o]);
							//}	
						//memcpy(c->mustnode, r->mustnode, r->mustnum * sizeof(int));
					}
					c->point=arr[0][j];
					c->passnum=r->passnum+1;
					c->pow=r->pow+arr[1][j];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[0][j];
					m->next=c;
					m=c;
				}
				else//在前两个条件都不满足的情况下，即子树的节点即不是终点又不是对应该点的最短路径，我们要有一个合理的判断，是否该把这条枝干给剪去或者留下来
				{
					//申请一个待测链结c，里面记下待测节点的情况
					c = (node *)malloc(sizeof(node));
					c->pow=r->pow+arr[1][j];
					memcpy(c->road, r->road ,r->passnum * sizeof(int));
					c->road[r->passnum]=arr[0][j];
					c->passnum=r->passnum+1;
					c->point = arr[0][j];
					if( judge( num_must, must_arr, arr[0][j] ) == 1 )//如果该子节点是必经点，那么就更新一下路径节点，加上这个子节点
					{
                        for(int i=0;i<r->mustnum;i++)
                        {
                            c->mustnode[i]=r->mustnode[i];
                        }
						//memcpy(c->mustnode, r->mustnode, r->mustnum * sizeof(int));
						c->mustnode[r->mustnum]=arr[0][j];
						c->mustnum = r->mustnum + 1;
					}
					if(calculate_score(c, node_info[arr[0][j]])==1)//判断该待测节点是否满足条件继续往下生成，
					{
						//c = (node *)malloc(sizeof(node));
						m->next=c;//允许该待测节点继续往下生成
						m=c;				
					}
					else
					{
						free(c);//这里只是为了释放c的空间，不允许该子节点继续往下生成路径时，就把申请的待测链结c给释放掉，不必太在意
					}
				}
			}
			r = r->next;
			free(q);
		}
		m->next = NULL;//给表头为l的链表尾巴上加一个NULL，方便再h和l交换后，用while（r）读取h链表时，有一个终止条件
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
	}//记录边的信息，a[i][0]记下第i条边的起点，a[i][1]记下第i条边的终点,a[i][2]记下第i条边的权重
	num_node=a[edge_num-1][0]+1;//num_node点的总数
	num_must = read_demand(condition,must_arr,start_node,end_node);//num_must：demand.csv文件里的必经点点数。must_arr：必经点数组。start_node,end_node：起始点，终点
	//for(int j=0;j<num_must;j++)
    //{
    //   printf("must_arr[%d]:%d\n",j,must_arr[j]);
    //}
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
	//声明记忆库，有num_node数量的点数，就申请num_node个空间，并且记忆库的初始记忆要赋值，为compare_num数量的记忆容量赋值，node_info[i]->must_num[j]赋值为0;node_info[i]->sumpow[j]赋值为12000;表示一开始到达i节点的第j个记忆的必经点为0，一开始到达i节点的第j个记忆的权重为12000，score=x1*must_num-x2*sumpow，这表明初始记忆分数很低，记下的东西很糟糕，方便之后出现好的记忆后进行替换
	
	int PATH[num_node];
	PATH[0]=start_node;//这里不用太在意，这里是为了方便生成第一个初始点的，也就是根的链结，
	creat(start_node,1,PATH);//输入起点start_node,因为一开始只有起点一个点，所以输入1，PATH也只有一个点，所以PATH[0]=start_node;
	
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
	char sel[50][20];
	int num_must;                       //必经点点数
    //printf("--------------------\n");     
    //printf("%s",demand);
    //printf("--------------------\n");     	
    split(list, demand, ",");
    
	startnode=atoi(list[0]);            // 起点
	endnode=atoi(list[1]);              // 终点  
    
	num_must=split(sel,list[2],"|"); 
    //printf("%d",num_must);   
	for(int k=num_must-1;k>=0;k--)
	{
		must[k]=atoi(sel[k]);
        //printf("must %d: %d\n",k,must[k]);
        
	}
	return num_must; 
}

int feasible_childnode(int **&A,int j,int arr[2][10],int num,int path[])//找到第i个节点的可行子节点 ,A是二维路径权重矩阵，i是父节点序号，arr[0][10]存储子节点的序号,arr[1][10]存储对应子节点的权重，k存储可行子节点数目，path是已走过的路径，num是已走过路径的点数
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
			}
			if(m == num)
			{
				arr[0][k]=A[i][1];
				arr[1][k]=A[i][2];
				k++;
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