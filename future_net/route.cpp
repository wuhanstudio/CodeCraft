#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "route.h"
#include "lib_record.h"
typedef struct Point
{
	int root;
	int leaf;
	int pow;
	bool required;
} Point;

void split(char **arr,char *str,const char *del) {
	char *s = strtok(str, del);
	while (s != NULL) {
		*arr++ = s;
		s = strtok(NULL, del);
	}
}

void search_route(char *graph[5000], int edge_num, char *condition)
{
	unsigned short result[] = { 2, 6, 5 };
	char *dst[4];
	int max = 0;
	
	int *matrix[600];

	for (int i = 0; i < 600; i++)
	{
		matrix[i] = (int *)malloc(sizeof(int) * 600);
		if (matrix[i] == NULL)
		{
			return;
		}
	}

	for (int i = 0;i < 600;i++)
	{
		for (int j = 0;j < 600;j++)
			matrix[i][j] = 0;
	}

	for (int i = 0;i < edge_num;i++)
	{
		split(dst, (char*)graph[i], ",");

		int start = atoi(dst[1]);
		int end = atoi(dst[2]);

		matrix[start][end] = atoi(dst[3]);
		max = atoi(dst[1])>max ? atoi(dst[1]) : max;
		max = atoi(dst[2])>max ? atoi(dst[2]) : max;
	}

	/*
	printf("%s\n",condition );
	printf("-----%d x %d-----\n",max,max);
	for (int i = 0;i < max;i++)
	{
		for (int j = 0;j < max;j++)
			printf("%d,", matrix[i][j]);
		printf("\n");
	}
	*/

	int i=0;
	char *dem[4];
	char *demand_temp[60];

	split(dem,condition,",");
	int start = atoi(dem[0]);
	int end = atoi(dem[1]);

	split(demand_temp,dem[2],"|");
	int demand[60];
	while(demand_temp[i])
	{
		demand[i] = atoi(demand_temp[i]);
		i++;
	}

	// for(i=0;i<max;i++)
	// {
	// 	if(matrix[start][i]!=0)
	// 		printf("matrix[%d][%d]=%d\n",start,i,matrix[start][i] );
	// }

	for (int i = 0; i < 3; i++)
		record_result(result[i]);
}

