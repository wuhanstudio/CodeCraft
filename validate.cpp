#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

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
       printf("开始测试文件: %s\n",name);                 
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
                        printf("第%d条路径匹配成功%d->%d 和 %d->%d\n",route_count,result_start,result_end,graph_start,graph_end);
                        succeed=true;
                        break;
                    }
		            //else
				        //printf("第%d条路径匹配失败%d->%d 和 %d->%d\n",route_count,result_start,result_end,graph_start,graph_end);            
                }
                if(!succeed)
                {
			   	    printf("第%d条路径匹配失败%d->%d\n",route_count,result_start,result_end);            
                    //break;            
                }
                succeed = false;
                route_count++;
		        line_count = 1;
            }
	    }
        sprintf(name,"result_%d.txt",countresult);        
	    result_file = fopen(name,"r");
    }while(result_file!=NULL);    
}