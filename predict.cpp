#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include <string.h>
#include<stdlib.h>
#include<math.h>

#define USER 285//用户总数
#define ITEM 1682//物品总数
#define BEST_SIM_NUM 20//选取相似度最高（权重最大）的用户数

//#define USER 3//用户总数
//#define ITEM 10//物品总数
//#define BEST_SIM_NUM 1//选取相似度最高（权重最大）的用户数

//每个用户用一个链表来表示
typedef struct NODE
{
	int num;//用户编号1-285
	int rate[ITEM];//物品评分数组
	int result[ITEM];//最终结果数组
	struct NODE *next;
}NODE;

//打印整条链表
void Print_list(NODE *p)

{
     int r;
     NODE *s;
     s = p;
     while(s != NULL)
     {
		for(r = 0; r < ITEM; r++)
		{
			printf("%d %d %d\n", s->num, r+1, s->rate[r]);
		}
        s = s->next;             
     }
	 printf("\n");
}

//向text文件输出整条链表
void Fprint_list(NODE *p, FILE *fpw)

{
     int r;
     NODE *s;
     s = p;
     while(s != NULL)
     {
		for(r = 0; r < ITEM; r++)
		{
			fprintf(fpw, "%d %d %d\n", s->num, r+1, s->result[r]);
		}
		fprintf(fpw, "\n");
        s = s->next;             
     }
}

//求平均值，排除值为0的数
double Mean(int arr[], int num)
{
	int i, sum;
	double mean;
	int no_zero_num = 0;

	for(i = 0, sum = 0; i < num; i++)
	{
		if(arr[i])
		{
			sum = sum + arr[i];
			no_zero_num++;
		}
	}
	mean = sum*1.0/no_zero_num;

	return mean;
}

//求和
double Sum(double arr[], int num)
{
	int i;
	double sum;
	int no_zero_num = 0;

	for(i = 0, sum = 0; i < num; i++)
		sum = sum + arr[i];

	return sum;
}

//求权重，及编号为a的目标用户与其他用户的相似程度，注意a-1为数组下标，a_node为指向编号为a的节点的指针
void Weight(NODE *head, int a, NODE *a_node, double mean[], double w[])
{
	int i, j, count;
	double numerator;
	double denominator_a;
	double denominator_other;
	NODE *p;

	p = head;

	denominator_a = 0;
	for(j = 0; j < ITEM; j++)
	{
		if(a_node->rate[j])
		{
			denominator_a = denominator_a + pow(a_node->rate[j]-mean[a-1], 2);
//			printf("%.2f\n", denominator_a);
		}
	}
//	printf("\n");

	for(i = 0; i < USER; i++)
	{		
		if(i != a-1){
			numerator = 0;
			denominator_other = 0;
			count = 0;

			for(j = 0; j < ITEM; j++)
			{
				if((a_node->rate[j])&&(p->rate[j]))
				{
					count++;
					numerator = numerator + (a_node->rate[j]-mean[a-1])*(p->rate[j]-mean[i]);
					denominator_other = denominator_other + pow(p->rate[j]-mean[i], 2);
				}
			}
			if(count)
				w[i] = numerator/sqrt(denominator_a*denominator_other);
			else w[i] = 0;//若用户与a用户没有评价相同的物品，会使分母为0，因此单独赋值。此处赋值为0，使其对后续求和不造成影响
		}
		else w[i] = 0;//a用户本身不参与计算权重	
		
		p = p->next;
	}

}

//交换两数
void swap(int *a,int *b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

//对矩阵第一行进行快速排序，第二行改变相应位置
void Qksort(double r[][USER],  int  low,  int high)
{
	double x, y, z;
	int i, j;
	
	x = r[0][low]; 
	y = r[1][low];
	z = r[2][low];
	i = low; 
	j = high;

	if(low == high) return;

	while(i < j)
	{
		while(i<j && r[0][j]<=x) j--;
		r[0][i] = r[0][j];
		r[1][i] = r[1][j];
		r[2][i] = r[2][j];
		while(i<j && r[0][i]>=x) i++;
		r[0][j] = r[0][i];
		r[1][j] = r[1][i];
		r[2][j] = r[2][i];
	}
	r[0][i] = x;
	r[1][i] = y;
	r[2][i] = z;

	
	if(i != low)
	{
		Qksort(r, low, i-1);
	}

	if(i != high)
	{
		Qksort(r, i+1, high);
	}


}

//主函数
void main()
{
	int num;
	int rate[ITEM];
	int node_size = sizeof(NODE);
	NODE *head, *p;
	NODE *(con[USER]);
	int item;//物品标号

	FILE *fp, *fpw;

	int i, j, k, l, temp_num, temp_rate;
	int flag = 0;

	double mean[USER];//均值
	double w[USER];//权重
	double temp[3][USER];//第0行存放所有用户的权重，第1行存放用户i对物品j的评价，第2行存放用户i的评价平均值
	double kappa;//归一化指数
	double temp_p, predict;

	printf("Please wait...\n");

	//打开文件
	if((fp = fopen("train_sub_txt.txt", "r")) == NULL)
//	if((fp = fopen("test.txt", "r")) == NULL)
	{
		printf("File open error!\n");
		exit(0);
	}

	for(i = 0; i < USER; i++)
	{
		con[i] = (NODE *)malloc(sizeof(NODE));
		con[i]->next = NULL;
		con[i]->num = i+1;
		if(flag == 0)
		{
			for(j = 0; (j < ITEM)&&(!feof(fp)); j++) 
			{			
				fscanf(fp, "%d%d%d", &temp_num, &item, &temp_rate);
				if(temp_num != i+1)
					break;
				
				con[i]->rate[item-1] = temp_rate;

				if(item != j + 1)
				{
					for(k = j; k < item-1; k++) 
						con[i]->rate[k] = 0;//未评价的物品初始评价设为0
					j = item - 1;
				}
			}
			if(j != ITEM){
				for(k = j; k < ITEM; k++)
					con[i]->rate[k] = 0;//剩余的未评价的物品初始评价设为0
				flag = 1;//表示用户没有评价最后一项物品，使得文件指针已经指向下一用户所评价的第二项物品
			}
			else flag = 0;//表示用户评价最后一项物品，使得文件指针已经指向下一用户所评价的第一项物品
		}
		else
		{
			con[i]->rate[item-1] = temp_rate;
			for(j = 0; j < item-1; j++) 
				con[i]->rate[j] = 0;//开头未评价的物品初始评价设为0

			for(j = item; (j < ITEM)&&(!feof(fp)); j++) 
			{			
				fscanf(fp, "%d%d%d", &temp_num, &item, &temp_rate);
				if(temp_num != i+1)
					break;
				
				con[i]->rate[item-1] = temp_rate;
				if(item != j + 1)
				{
					for(k = j; k < item-1; k++) 
						con[i]->rate[k] = 0;//未评价的物品初始评价设为0
					j = item - 1;
				}
			}
			if(j != ITEM){
				for(k = j; k < ITEM; k++)
					con[i]->rate[k] = 0;//剩余的未评价的物品初始评价设为0
				flag = 1;//表示用户没有评价最后一项物品，使得文件指针已经指向下一用户所评价的第二项物品
			}
			else flag = 0;//表示用户评价最后一项物品，使得文件指针已经指向下一用户所评价的第一项物品
		}
	}

	//创建列表
	for(i = 0; i < USER-1; i++) con[i]->next=con[i+1];
	con[USER-1]->next=NULL;	
	head=con[0];

//	Print_list(head);
	
	//计算各个用户的评价均值
	for(i = 0; i < USER; i++)
	{
		mean[i] = Mean(con[i]->rate, ITEM);
//		printf("%.2f\n", mean[i]);
	}

	//数据处理，预测，将结果保存在result[]中
	for(i = 0; i < USER; i++)
	{
		Weight(head, i+1, con[i], mean, w);
		kappa = Sum(w, USER);//求所有权重的和，即归一化系数

		//将相关数据存于temp[][]中
		for(j = 0; j < USER; j++) 
		{
			temp[0][j] = w[j];
			temp[2][j] = mean[j];
		}
//			printf("%.2f\n", w[j]);
//		printf("\n");

		for(j = 0; j < ITEM; j++)
		{
			
			if(!(con[i]->rate[j]))//如果rate=0，则说明该物品还未被评价
			{
			//////////////////////////////////////////////////////////
//				con[i]->result[j] = con[i]->rate[j];//////////////////需要修改成预测算法
			///////////////////////////////////////////////////////////	

				//将相关数据存于temp[][]中
				for(k = 0; k < USER; k++)
				{
//					temp[0][k] = w[k];
					temp[1][k] = con[k]->rate[j];
//					printf("%.2f %.0f\n", temp[0][k], temp[1][k]);
				}
//				printf("\n");
				
				//对权重进行排序，temp对应的第1、2行数据也调整位置
				Qksort(temp, 0, USER-1);
				for(k = 0; k < 2; k++)
				{
					/*for(l = 0; l < USER; l++)
					{
						printf("%.2f ", temp[k][l]);
					}*/	
				
				}

				//取前BEST_SIM_NUM个符合要求的数，进行预测计算
				for(k = 0, l = 0, temp_p = 0; (l<BEST_SIM_NUM) && (k<USER); k++, l++)
				{
					if(!temp[0][k]) l--;
					else 
						temp_p = temp_p + temp[0][k]*(temp[1][k] - temp[2][k]);
				}
				predict = mean[j] + temp_p/kappa;
				if(predict < 1) con[i]->result[j] = 1;
				else if(predict > 5) con[i]->result[j] = 5;
				else con[i]->result[j] = predict;


//				printf("\n");

			}
			else con[i]->result[j] = con[i]->rate[j];
			
		}
	}

	//打开文件
	if((fpw = fopen("result.txt", "w")) == NULL)
//	if((fpw = fopen("test_result.txt", "w")) == NULL)
	{
		printf("File open error!\n");
		exit(0);
	}

	//向文件输出结果
	Fprint_list(head, fpw);

	//关闭文件
	if(fclose(fp))
	{
		printf("Can't close!");
		exit(0);
	}

	if(fclose(fpw))
	{
		printf("Can't close!");
		exit(0);
	}
	
	//删除链表，释放内存
	for(i = USER-2; i >= 1; i--)
		free(con[i]->next);
	free(head);

	printf("Prediction finished.\n");
}