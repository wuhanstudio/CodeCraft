#include "route.h"
#include "lib_io.h"
#include "lib_record.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <malloc.h>

#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>


/****分枝定界*****/
int **a;                           // 边的矩阵
int bestpow=-1;                    // 最好路径的权重
int bestnum;                       // 最好的路径的边数
int edgenum;                       // 边的条数
int num_must;                      // 必经点点数
int must_arr[50];                  // 必经点集合
int start_node;                    // 起始点
int end_node;                      // 终点
int num_node;                      // 点的总数
double x1=500.0,x2=1.0;
int bestpath[600];                 // 存储最好的路径

const int compare_num=30;          // 每个点路径信息最大存储数，用于比较
int use_compare_num=compare_num;
double rate=0.8;

typedef struct str
{
	struct str *next;
	int point;                     //点序号
	int road[600];                 //已经经历点的路径顺序
	int passnum;                   //已经经历的点的数目
	int pow;                       //路径加总权重
	int mustnum;                   //路径经历的必经点点数
}node;

typedef struct infostr
{
	int must_num[compare_num];     //路径经历的必经点点数
	int sumpow[compare_num];       //路径加总权重
}info_node;

info_node *node_info[600];

int split(char dst[][15], char* str, const char* spl)
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
	int num_must;
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

// 找到第i个节点的可行子节点
// A是二维路径权重矩阵
// i是父节点序号
// arr[0][10]存储子节点的序号
// arr[1][10]存储对应子节点的权重
// k存储可行子节点数目
// path是已走过的路径
// num是已走过路径的点数
int feasible_childnode(int **&A,int j,int arr[3][10],int num,int path[])
{
    //可行子节点
	int k=0;
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

int calculate_score(node *&A, info_node *&B)
{
	double C[use_compare_num];
	double D = (x1*A->mustnum) - (x2*A->pow);
	double low=x1*50.0, high=-50000;
	double E;
	int l=0;
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
       	if(num_node<=20)   
        {
                bonouns = 1;
                add = 0;
        }
        else if(num_node<=100) 
        {
                bonouns = 7;
                add = -0.1;                
        }
        else if(num_node<=150)
        {
                bonouns = 10;
                add = -0.5;
        }
        else if(num_node<=200)
        {
                bonouns = 4;
                add = 0.0;
        }
        else if(num_node<=250) 
        {
                bonouns = 3;
                add = 0.0;
        }
        else if(num_node<=300)
        {
                bonouns = 2;
                add = -0.5;
        }
        else if (num_node<=550) 
        {
            if(num_must>30)  
            {
                bonouns = 1;
                add = -0.1;
            }
            else            
            {
                bonouns = 2;
                add = 0;
            }
        }
        else                   
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

//pointnum当前点的点序号，num已经经历了的点的数目，path已经经历过的点 path[0]==startpoint;
void create()
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
							// //printf("bestpow:%d path:",bestpow);
							// for(int o=0;o<bestnum;o++)
							// {
							// 	//printf("%d,",bestpath[o]);
							// }
							// //printf("\nmustnode:");
							// for(int o=0;o<c->mustnum;o++)
							// {
							// 	//printf("%d|",c->mustnode[o]);
							// }
							// //printf("\n");
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


/****蚂蚁算法****/
#define MAXNODE 600
#define ThresholdTime_ms 9500
#define ThresholdNode 10

const double ALPHA = 1.0;           // 启发因子:信息素的重要程度
const double BETA = 2;              // 期望因子:城市间距离的重要程度
const double ROU = 0.89;            // 信息素残留参数
const int ANT_COUNT = 20;           // 蚂蚁数量
const int IT_COUNT = 20000;         // 迭代的次数
const double DBQ = 50.0;            // 信息素总量
const double DB_MAX = 10e9;         // 10的9次方,这真的只是个标记而已

int CITY_COUNT;                     // 城市的数量
int start, end;                     // 起点和终点

struct edgenode                     // 节点
{
	int no;
	int edg_no;                     // 父结点编号
	int cost;                       // 父结点权值
};


struct path
{
	double cost;
	std::vector<int> node_name;
	std::vector<int> edg_name;
	friend bool operator< (path p1, path p2)
	{
		return p1.cost > p2.cost;
	}
};

typedef std::list<edgenode> adjlist;


int rnd(int nLow, int nUpper)
{
	return nLow + (nUpper - nLow)*rand() / (RAND_MAX + 1);
}

double rnd(double dbLow, double dbUpper)
{
	double dbTemp = rand() / ((double)RAND_MAX + 1.0);
	return dbLow + dbTemp*(dbUpper - dbLow);
}

double ROUND(double dbA)
{
	return (double)((int)(dbA + 0.5));
}

void timer(int &out_ms)
{
	struct timeb rawtime;
	ftime(&rawtime);
	static int ms = rawtime.millitm;
	out_ms = rawtime.millitm - ms;
	ms = rawtime.millitm;
	if (out_ms < 0) out_ms += 1000;
}


class CAnt
{
    public:
        CAnt(void);
        ~CAnt(void);

    public:
        double m_dbPathLength;           // 走过路径长度
        int *m_nDemanCity;               // 必须去的城市
        int *m_nPath;                    // 蚂蚁走的路径	
        int *m_nAllowedCity;             // 没去过的城市
        int m_nCurCityNo;                // 当前城市编号
        int m_nMovedCityCount;           // 去过城市数量

    public:
        //初始化
        void Init(std::vector<int> deman_vec, std::vector<adjlist> adj_vec); 
        //选择下一个城市
        int ChooseNextCity(double **g_Distance, double **g_Trial, int deman_count, int deman_node_count); 
        //搜索路径
        void Search(std::vector<int> deman_vec, std::vector<adjlist> adj_vec, double **g_Distance, double **g_Trial); 
        //蚂蚁在城市间移动
        int Move(double **g_Distance, double **g_Trial, int deman_count, int deman_node_count); 
        //计算蚂蚁走过的路径长度
	    void CalPathLength(double **g_Distance); 
};

CAnt::CAnt()
{
}

CAnt::~CAnt(void)
{
}

//初始化函数
void CAnt::Init(std::vector<int> deman_vec, std::vector<adjlist> adj_vec)
{
	m_nPath = (int*)malloc(sizeof(int)*CITY_COUNT);
	m_nAllowedCity = (int*)malloc(sizeof(int)*CITY_COUNT);
	m_nDemanCity = (int*)malloc(sizeof(int)*CITY_COUNT);

	for (int i = 0; i<CITY_COUNT; i++)
	{
		m_nAllowedCity[i] = 1;                    // 所有城市没有去过
		m_nPath[i] = 0;                           // 路径全部设置为0
	}

	for (int i = 0; i < CITY_COUNT; i++)
	{
		if((adj_vec[i].size()==0)&&(i!=end))
			m_nAllowedCity[i] = 0;                // 陷阱已经去过
	}

	for (int i = 0; i<CITY_COUNT; i++)
	{
		m_nDemanCity[i] = 0;                      // 全部城市为非必经
	}
	for (unsigned int i = 0; i < deman_vec.size(); i++)
	{
		m_nDemanCity[deman_vec[i]] = 1;           // 设置必经城市
	}

	// 走过的路径长度为0
	m_dbPathLength = 0.0;

	// 随机选择一个起点
	m_nCurCityNo = start;

	// 出发城市存入路径数组
	m_nPath[0] = m_nCurCityNo;

	// 出发城市已经去过
	m_nAllowedCity[m_nCurCityNo] = 0;

	// 已去过的城市数量为1
	m_nMovedCityCount = 1;
}

int CAnt::ChooseNextCity(double **g_Distance, double **g_Trial, int deman_count, int deman_node_count)
{

	int nSelectedCity = -1; 

	// 计算当前城市和没去过的城市之间的信息素总和
	double dbTotal = 0.0;
	double *prob = (double*)malloc(sizeof(double)*CITY_COUNT);

	double Total = 0.0;

	for (int i = 0; i < CITY_COUNT; i++)
	{
        // 可去的并且没有去过的城市
		if ((m_nAllowedCity[i] == 1) && (g_Distance[m_nCurCityNo][i] != DB_MAX)) 
		{
			Total += pow(g_Trial[m_nCurCityNo][i], ALPHA)*pow(1.0 / g_Distance[m_nCurCityNo][i], BETA);
		}
	}

	for (int i = 0; i<CITY_COUNT; i++)
	{
        // 可去的并且没有去过的城市
		if ((m_nAllowedCity[i] == 1) && (g_Distance[m_nCurCityNo][i] != DB_MAX)) 
		{
            // 若为必经城市，则增大选择该城市的概率
			if (m_nDemanCity[i] == 1)
				prob[i] = 0.8;
			else
				prob[i] = pow(g_Trial[m_nCurCityNo][i], ALPHA)*pow(1.0 / g_Distance[m_nCurCityNo][i], BETA); 
            // 若需要经过的点还没走完且选择的新节点是终点，则该点不走
			if ((deman_count != deman_node_count) && (i == end))
				prob[i] = 0.0;
            // 累加信息素，得到总和
			dbTotal = dbTotal + prob[i]; 
		}
		else  // 如果城市去过了，则其被选中的概率值为0
			prob[i] = 0.0;
	}
	double dbTemp = 0.0;
	if (dbTotal > 0.0)   
	{
		dbTemp = rnd(0.0, dbTotal); 

		for (int i = 0; i<CITY_COUNT; i++)
		{
            //可去的并且没有去过的城市
			if ((m_nAllowedCity[i] == 1) && (g_Distance[m_nCurCityNo][i] != DB_MAX)) 
			{
				dbTemp = dbTemp - prob[i];
				if (dbTemp < 0.0)
				{
					nSelectedCity = i;
					break;
				}
			}
		}
	}
	free(prob);
	if (nSelectedCity == -1)
	{
		for (int i = 0; i<CITY_COUNT; i++)
		{
            // 可去的并且没有去过的城市
			if ((m_nAllowedCity[i] == 1) && (g_Distance[m_nCurCityNo][i] != DB_MAX)) 
			{
				nSelectedCity = i;
				break;
			}
		}
	}
	return nSelectedCity;
}

int CAnt::Move(double **g_Distance, double **g_Trial, int deman_count, int deman_node_count)
{
	int nCityNo = ChooseNextCity(g_Distance, g_Trial, deman_count, deman_node_count); //选择下一个城市

	if (nCityNo == -1)
	{
		return 0;
	}
	m_nPath[m_nMovedCityCount] = nCityNo;     //保存蚂蚁走的路径
	m_nAllowedCity[nCityNo] = 0;              //把这个城市设置成已经去过了
	m_nCurCityNo = nCityNo;                   //改变当前所在城市为选择的城市
	m_nMovedCityCount++;                      //已经去过的城市数量加1
	return 1;
}

void CAnt::Search(std::vector<int> deman_vec, std::vector<adjlist> adj_vec, double **g_Distance, double **g_Trial)
{
	Init(deman_vec, adj_vec); 

	//如果未经过所有必经城市，且当前不是最终城市，继续
	int deman_node_count = deman_vec.size();
	int deman_count = 0;
	while ((m_nCurCityNo != end) || (deman_count != deman_node_count))
	{
		int rt = Move(g_Distance, g_Trial, deman_count, deman_node_count);
		if (rt == 0)
		{
			std::vector<int>::iterator pre_it = find(deman_vec.begin(), deman_vec.end(), m_nCurCityNo);
			if (pre_it != deman_vec.end())
			{
				m_dbPathLength = DB_MAX;

				return;
			}
			m_nCurCityNo = m_nPath[m_nMovedCityCount - 2];
			m_nPath[m_nMovedCityCount-1] = 0;
			m_nMovedCityCount--;
			std::vector<int>::iterator it = find(deman_vec.begin(), deman_vec.end(), m_nCurCityNo);
			if (it != deman_vec.end())
			{
				deman_count--;
			}
			continue;
		}

		std::vector<int>::iterator it = find(deman_vec.begin(), deman_vec.end(), m_nCurCityNo);
		if (it != deman_vec.end())
			deman_count++;
	}
	//完成搜索后计算走过的路径长度
	CalPathLength(g_Distance);
}


//计算蚂蚁走过的路径长度
void CAnt::CalPathLength(double **g_Distance)
{
	m_dbPathLength = 0.0; //先把路径长度置0
	int m = 0;
	int n = 0;

	for (int i = 1; i<m_nMovedCityCount; i++)
	{
		m = m_nPath[i];
		n = m_nPath[i - 1];
		m_dbPathLength = m_dbPathLength + g_Distance[n][m];
	}
}


class CTsp
{
public:
	CTsp(void);
	~CTsp(void);

public:
	CAnt m_cAntAry[ANT_COUNT]; //蚂蚁数组
	CAnt m_cBestAnt;           //定义一个蚂蚁变量，用来保存搜索过程中的最优结果

public:
	//初始化数据
	void InitData(int num_node, std::vector<adjlist> adj_vec, double **g_Distance, double **g_Trial);

	//开始搜索
	void Search(std::vector<int> deman_vec, std::vector<adjlist> adj_vec, double **g_Distance, double **g_Trial);

	//更新环境信息素
	void UpdateTrial(double **g_Trial);

};

CTsp::CTsp(void)
{
}

CTsp::~CTsp(void)
{
}

void CTsp::InitData(int num_node, std::vector<adjlist> adj_vec, double **g_Distance, double **g_Trial)
{
	//先把最优蚂蚁的路径长度设置成一个很大的值
	m_cBestAnt.m_dbPathLength = DB_MAX;
	CITY_COUNT = num_node;
	//计算两两城市间距离
	// double dbTemp = 0.0;
	for (int i = 0; i<CITY_COUNT; i++)
	{
		for (int j = 0; j<CITY_COUNT; j++)
		{
			g_Distance[i][j] = DB_MAX;
		}
	}
	adjlist::iterator adj_it;
	for (int i = 0; i < num_node; i++)
	{	
		for (adj_it = adj_vec[i].begin(); adj_it != adj_vec[i].end(); adj_it++)
		{
			g_Distance[i][adj_it->no] = adj_it->cost;
		}
	}
    
	for (int i = 0; i<CITY_COUNT; i++)
	{
		for (int j = 0; j<CITY_COUNT; j++)
		{
			g_Trial[i][j] = 1.0;
		}
	}

}

void CTsp::UpdateTrial(double **g_Trial)
{
	double **dbTempAry = (double**)malloc(sizeof(double*)*CITY_COUNT);
	for (int i = 0; i<CITY_COUNT; i++)
		dbTempAry[i] = (double *)malloc(sizeof(double) * CITY_COUNT);
	for (int i = 0; i < CITY_COUNT; i++)
		for (int j = 0; j < CITY_COUNT; j++)
			dbTempAry[i][j] = 0;
	
	int m = 0;
	int n = 0;
	for (int i = 0; i<ANT_COUNT; i++)                                            //计算每只蚂蚁留下的信息素
	{
		for (int j = 1; j<m_cAntAry[i].m_nMovedCityCount; j++)
		{
			m = m_cAntAry[i].m_nPath[j];
			n = m_cAntAry[i].m_nPath[j - 1];
			dbTempAry[n][m] = dbTempAry[n][m] + DBQ / m_cAntAry[i].m_dbPathLength;//城市n->城市m的信息素
		}
	}
	for (int i = 0; i<CITY_COUNT; i++)
	{
		for (int j = 0; j<CITY_COUNT; j++)
		{
			g_Trial[i][j] = g_Trial[i][j] * ROU + dbTempAry[i][j]; //最新的环境信息素 = 留存的信息素 + 新留下的信息素
		}
	}
	for (int i = 0; i < CITY_COUNT; i++)
		free(dbTempAry[i]);
	free(dbTempAry);

}

void CTsp::Search(std::vector<int> deman_vec, std::vector<adjlist> adj_vec, double **g_Distance, double **g_Trial)
{

	// char cBuf[256]; 

	int out_ms, total_ms = 0;
	timer(out_ms);

	
	for (int i = 0; i<IT_COUNT; i++)
	{
        //死掉的蚂蚁
		int dead_ants = 0;   
		//每只蚂蚁搜索一遍
		for (int j = 0; j<ANT_COUNT; j++)
		{
			m_cAntAry[j].Search(deman_vec, adj_vec, g_Distance, g_Trial);
			if (m_cAntAry[j].m_dbPathLength == DB_MAX)
				dead_ants++;
		}

		//保存最佳结果
		for (int j = 0; j<ANT_COUNT; j++)
		{
			if (m_cAntAry[j].m_dbPathLength < m_cBestAnt.m_dbPathLength)
			{
				m_cBestAnt = m_cAntAry[j];
			}
		}
		timer(out_ms);
		total_ms += out_ms;
		if (total_ms > ThresholdTime_ms)
			return;

		//更新环境信息素
		UpdateTrial(g_Trial);

	    //sprintf(cBuf, "\n[%d] %.0f", i + 1, m_cBestAnt.m_dbPathLength);
		//printf(cBuf);
		//printf("\nThe number of ants is %d, The number of dead ants is %d\n ", N_ANT_COUNT, dead_ants);
	}

}


path NodenoIndexPath(std::vector<adjlist> adj_vec, CAnt m_cBestAnt)
{
	path best_path;
	for (int i = 1; i < m_cBestAnt.m_nMovedCityCount; i++)
	{
		adjlist::iterator adj_it;
		int m = m_cBestAnt.m_nPath[i];
		int n = m_cBestAnt.m_nPath[i - 1];
		best_path.cost = m_cBestAnt.m_dbPathLength;
		for (adj_it = adj_vec[n].begin(); adj_it != adj_vec[n].end(); adj_it++)
		{
			if (m == adj_it->no)
			{
				best_path.node_name.push_back(m);
				best_path.edg_name.push_back(adj_it->edg_no);
			}
		}
	}
	return best_path;
}

//建立邻接链表，返回邻接链表数组(图的表示)
std::vector<adjlist> build_adjlist(char *topo[], int edge_num, int* num_node)
{
	std::vector<adjlist> adj_vec(MAXNODE);
	for (int i = 0; i < edge_num; i++)
	{
		struct edgenode pedgnode;
		char* str = strtok(topo[i], ",");
		int node_info[4];
		int count = -1;
		while (str)
		{
			node_info[++count] = atoi(str);
			str = strtok(NULL, ",");
		}

		pedgnode.edg_no = node_info[0];
		pedgnode.no = node_info[2];
		pedgnode.cost = node_info[3];

		adjlist::iterator adj_it,ad_end_it;
		for (adj_it = adj_vec[node_info[1]].begin(); adj_it != adj_vec[node_info[1]].end(); adj_it++)
		{
			if (adj_it->no == pedgnode.no)
			{
				if (adj_it->cost > pedgnode.cost)
				{
					adj_vec[node_info[1]].erase(adj_it);
					break;
				}	
				else
					goto un_push;
			}
			if (adj_it->cost > pedgnode.cost)
			{
				adj_vec[node_info[1]].insert(adj_it, pedgnode);
				goto un_push;
			}
		}

		adj_vec[node_info[1]].push_back(pedgnode);
un_push:
		if (node_info[2] > *num_node)
			*num_node = node_info[2];
	}
	(*num_node)++;
	return adj_vec;
}

std::vector<int> analysis_demand(char *demand,int *st,int *en)
{
	char*deman_str[3];
	char* str = strtok(demand, ",");
	int count = -1;
	while (str)
	{
		deman_str[++count] = str;
		str = strtok(NULL, ",");
	}

	*st = atoi(deman_str[0]);
	*en = atoi(deman_str[1]);

	char* node_str = strtok(deman_str[2], "|");
	std::vector<int> deman_vec;
	while (node_str)
	{
		deman_vec.push_back(atoi(node_str));
		node_str = strtok(NULL, "|");
	}
	return deman_vec;
}

//判断是否满足约束条件
bool DmanSentence(path path_temp ,std::vector<int> &deman_vec)
{
	for (unsigned int j = 0; j < deman_vec.size(); j++)
	{
		std::vector<int>::iterator it = find( path_temp.node_name.begin(),  path_temp.node_name.end(), deman_vec[j]);
		if (it ==  path_temp.node_name.end())
		{
			return false;
		}
	}
	return true;

}

path find_path(int st, int en, std::vector<int> deman_vec, std::vector<adjlist> adj_vec)
{
	int out_ms,total_ms=0;
	timer(out_ms);
	int demand_count = deman_vec.size();
	path best_path;
	best_path.cost = DB_MAX;

	int *visit, *node_stack, *cost_stack, *edg_stack, top, v, head = 1;
	adjlist::iterator adj_it;
	visit = (int *)malloc(MAXNODE*sizeof(int));
	node_stack = (int *)malloc((MAXNODE + 1)*sizeof(int));
	cost_stack = (int *)malloc(MAXNODE*sizeof(int));
	edg_stack = (int *)malloc(MAXNODE*sizeof(int));
	for (int i = 0; i < MAXNODE; i++)
		visit[i] = 0;
	v = st;                             // 初始化为起点
	visit[st] = 1;
	top = 1;
	node_stack[top] = v;                // 起点入栈
	do{
		if (head == 1) {
			adj_it = adj_vec[v].begin();
			head = 0;
		}
		else ++adj_it;
		if (adj_it != adj_vec[v].end())
		{
			if (visit[adj_it->no] == 0)
			{
				visit[adj_it->no] = 1;
				top++;
				node_stack[top] = adj_it->no;
				edg_stack[top - 1] = adj_it->edg_no;
				cost_stack[top - 1] = adj_it->cost;
				if (adj_it->no == en)
				{
					if ((demand_count + 1) < top)
					{
						int cost = 0;
						path path_temp;
						for (int i = 1; i < top; i++)
						{
							path_temp.edg_name.push_back(edg_stack[i]);
							path_temp.node_name.push_back(node_stack[i]);
							cost += cost_stack[i];
						}
						path_temp.node_name.push_back(node_stack[top]);
						path_temp.cost = cost;
						if(DmanSentence(path_temp ,deman_vec))
						{
							if (best_path.cost > path_temp.cost)
							{
								best_path.cost = path_temp.cost;
								best_path.edg_name.swap(path_temp.edg_name);
								best_path.node_name.swap(path_temp.node_name);
							}
						}
					}

					visit[en] = 0;
					top--;
					v = node_stack[top];
					head = 0;
				}
				else {
					v = node_stack[top];
					head = 1;
				}
				timer(out_ms);
				total_ms += out_ms;
				if (total_ms > ThresholdTime_ms)
					return best_path;
			} 
		}
		else {
			visit[node_stack[top--]] = 0; //出栈
			if (top)
			{
				adj_it = adj_vec[node_stack[top]].begin();
				while (adj_it->no != v) ++adj_it;
				v = node_stack[top];
				head = 0;
			}
		}
	} while (top);

	return best_path;
}

void demanNoTuba(std::vector<adjlist> adj_vec, std::vector<int> deman_vec, std::vector<int> &TubaVec, int num_node)
{
	while (1)
	{
		int count = 0;
		for (unsigned int i = 0; i < deman_vec.size(); i++)
		{
			if (adj_vec[deman_vec[i]].size() == 1)
			{
				count++;
				adjlist::iterator adj_it = adj_vec[deman_vec[i]].begin();
				int NodeNo = adj_it->no;
				TubaVec.push_back(deman_vec[i]);
				TubaVec.push_back(NodeNo);
				deman_vec[i] = NodeNo;
			}
		}
		if (count == 0)
			return;
	}
}

void pre_process(std::vector<adjlist> *adj_vec,int num_node, std::vector<int> TubaVec)
{
	for (unsigned int i = 0; i < TubaVec.size(); i += 2)
	{
		int OutNo = TubaVec[i];//必经节点
		int InNo = TubaVec[i + 1];

		for (int j = 0; j < num_node; j++)
		{
			if (j == OutNo)
				continue;

			adjlist::iterator adj_it = (*adj_vec)[j].begin();
			while (adj_it != (*adj_vec)[j].end())
			{
				if (adj_it->no == InNo)
				{
					adj_it=(*adj_vec)[j].erase(adj_it);
				}
				else
				{
					adj_it++;
				}
			}
		}
	}
}

//你要完成的功能总入口
void search_route(char *graph[5000],int edge_num, char *condition,char *topo_file)
{
     edgenum=edge_num;
     a = (int **)malloc(sizeof(int *) * 5000);
     for (int i = edge_num-1;i>=0;i--)
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
     // 每根据边数选择合适的算法，每50边递增
     if ( (num_node>100&&num_node<=150) || (num_node>250&&num_node<=300) )
     {
        char *topo[5000];
        edge_num = read_file(topo, 5000, topo_file);
        if (num_node>250&&num_node<=300) 
            re_search_route(topo,edge_num,condition,30);
        else
            re_search_route(topo,edge_num,condition,10);
        // release_buff(topo, edge_num);
     }
     else if(num_node<=550)
     {
        num_must = read_demand(condition,must_arr,start_node,end_node);    
        if(num_node<=20)   
        {
            use_compare_num=15;
            rate=0.8;
            x2=4;
        }
        else if(num_node<=100) 
        {
            use_compare_num=25;
            rate=0.8;
            x2=4;
        }
        else if(num_node<=150) 
        {
             use_compare_num=100;
             rate=0.8;
             x2=4;
        }
        else if(num_node<=200) 
        {
            use_compare_num=20;
            rate=0.8;
            x2=4;
        }
        else if(num_node<=250) 
        {
            use_compare_num=23;
            rate=0.8;
            x2=20;
        }
        else if(num_node<=300)
        {
            use_compare_num=5;
            rate=0.8;
            x2=4;
        }
        else if (num_node<=550) 
        {
            if(num_must>30)  
            {
                use_compare_num=10;
                rate=0.8;
                x2=4;
            }
            else           
            {
                use_compare_num=25;
                rate=0.8;
                x2=30;
            }
        }

        for(int i=num_node-1; i>=0; i--)
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
            for (int i = 0; i < bestnum-1; i++)
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
        
        for(int  i=num_node-1; i>=0; i--)
        {
            free(node_info[i]);
        }
        for (int i = edge_num-1;i>=0;i--)
        {
            free(a[i]);
        }
        free(a);
    }
    else
    {
        char *topo[5000];
        edge_num = read_file(topo, 5000, topo_file);
        num_node = 0;
        std::vector<adjlist> adj_vec = build_adjlist(topo, edge_num, &num_node);

        std::vector<int> deman_vec = analysis_demand(condition,&start, &end);

        std::vector<int> TubaVec;

        demanNoTuba(adj_vec, deman_vec, TubaVec, num_node);
        pre_process(&adj_vec, num_node, TubaVec);
        for (int i = 0; i < num_node; i++)
        {
            if (adj_vec[i].size() == 0)
            {
                std::vector<int>::iterator it = find(deman_vec.begin(), deman_vec.end(), i);
                if (it != deman_vec.end())
                    return;
            }
        }
        path best_path;
        if (num_node < ThresholdNode)
        {
            best_path = find_path(start, end, deman_vec, adj_vec);
            if (best_path.cost == DB_MAX)
                return;
        }
        else
        {
            double **g_Distance; 
            g_Distance = (double **)malloc(sizeof(double *)*num_node);
            for (int i = 0; i<num_node; i++)
                g_Distance[i] = (double *)malloc(sizeof(double) * num_node);
            
            double **g_Trial;
            g_Trial = (double **)malloc(sizeof(double *)*num_node);
            for (int i = 0; i<num_node; i++)
                g_Trial[i] = (double *)malloc(sizeof(double) * num_node);

            CTsp tsp;
            tsp.InitData(num_node, adj_vec, g_Distance, g_Trial); 
            tsp.Search(deman_vec, adj_vec, g_Distance, g_Trial); 

            if (tsp.m_cBestAnt.m_dbPathLength == DB_MAX)
                return;
            for (int i = 0; i<tsp.m_cBestAnt.m_nMovedCityCount - 1; i++)
            best_path = NodenoIndexPath(adj_vec, tsp.m_cBestAnt);

        }

        for (unsigned int i = 0; i < best_path.edg_name.size(); i++)
            record_result(best_path.edg_name[i]);    	
        release_buff(topo, edge_num);
        
    }
       
}



