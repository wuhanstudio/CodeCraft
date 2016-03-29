#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

void validateRoute();
void validateDemand();
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

int main()
{
   
    validateRoute();
    validateDemand();
}

void validateRoute()
{
    /* 测试路径是否有效*/    
    char line[50];
	char result_line[50];
    char split_temp[5][20];
	int line_count = 1;
    int route_count = 1;
    bool succeed = false;
    char name[50];
    int countresult=0;
    
    FILE* result_file;
    sprintf(name,"result_%d.txt",countresult);        
	result_file = fopen(name,"r");
    do
    {
       bool failed = false;          
       printf("\n");
       printf("--------------------------\n"); 
       printf("开始测试文件: %s\n",name);      
       printf("-------------------------\n");                   
       countresult++;    
	   while(!feof(result_file))
	   {
		    fgets(result_line,1024,result_file);
            //printf("%s",result_line);     
            split(split_temp,result_line,",");
            int result_start = atoi(split_temp[0]);
            int result_end =  atoi(split_temp[1]);
            if(result_start!=result_end)
            {
		        FILE* graph_file = fopen("topo.csv","r");        
		        while(!feof(graph_file))
		        {
		            fgets(line,1024,graph_file);
                    split(split_temp,line,",");
                    int graph_start = atoi(split_temp[1]);
                    int graph_end = atoi(split_temp[2]);
                    line_count++;
    
			        if( (result_start==graph_start) && (result_end==graph_end ) )
                    {
                        //printf("第%d条路径匹配成功%d->%d 和 %d->%d\n",route_count,result_start,result_end,graph_start,graph_end);
                        succeed=true;
                        break;
                    }
		            //else
				        //printf("第%d条路径匹配失败%d->%d 和 %d->%d\n",route_count,result_start,result_end,graph_start,graph_end);            
                }
                if(!succeed)
                {
			   	    printf("第%d条路径匹配失败%d->%d\n",route_count,result_start,result_end);          
                    failed = true;  
                    //break;            
                }
                succeed = false;
                route_count++;
		        line_count = 1;
            }
	    }
        if(failed)
        {
            printf("[x] 路径匹配失败\n");
        }
        else
        {
            printf("[*] 路径匹配成功\n");            
        }
        sprintf(name,"result_%d.txt",countresult);        
	    result_file = fopen(name,"r");
    }while(result_file!=NULL);   
    
}

void validateDemand()
{
    char demand[100];
    FILE* demand_file = fopen("demand.csv","r");
    fgets(demand,1024,demand_file);
    char list[50][20];
	char sel[50][20];
	int num_must;                       //必经点点数
    printf("\n");
    printf("---------------------------------\n");     
    printf("开始测试是否经过所有必经点\n");    
    printf("%s\n",demand);    
    printf("---------------------------------\n");     
    //getchar();
    split(list, demand, ",");
    int must[50];
	int startnode=atoi(list[0]);            // 起点
	int endnode=atoi(list[1]);              // 终点  
    
	num_must=split(sel,list[2],"|"); 
    //printf("%s,\n%d",sel,num_must);   
	for(int k=num_must-1;k>=0;k--)
	{
		must[k]=atoi(sel[k]);
        //printf("must %d: %d\n",k,must[k]);  
	}
    
 
    int countresult = 0;
    FILE* result_file;
    char name[50];
    sprintf(name,"result_%d.txt",countresult);        
	result_file = fopen(name,"r");
	char result_line[50];           
    do
    {  
        int nn=0;
        int pp =0;
        int match=0;
        fgets(result_line,1024,result_file);                
        while(!feof(result_file))
        {
            char split_temp[5][20];        
            
            split(split_temp,result_line,",");
            int result_start = atoi(split_temp[0]);
            pp=0;
            while(pp<num_must)
            {
                if(result_start==must[pp])
                {
                    match++;;                      
                    printf("[%d] %d 是必经点;\n",match,result_start);                                                            
                }
                //else
                //{
                //    printf("[x] %d 不是必经点 %d\n",result_start,pp);    
                //}                        
                pp++;                    
            }
            nn++;
            fgets(result_line,1024,result_file);                    
        };
        if(match==num_must)
        {
            printf("[*] 必经点总数:%d,经过必经点数量%d\n",num_must,match);
        }
        else
        {
            printf("[x] 必经点总数:%d,经过必经点数量%d\n",num_must,match);            
        }
        countresult++;
        sprintf(name,"result_%d.txt",countresult);        
	   result_file = fopen(name,"r");
    }while(result_file!=NULL);
}