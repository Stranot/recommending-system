#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include <string.h>
#include<stdlib.h>
#include<math.h>

#define USER 285//�û�����
#define ITEM 1682//��Ʒ����
#define BEST_SIM_NUM 20//ѡȡ���ƶ���ߣ�Ȩ����󣩵��û���

//#define USER 3//�û�����
//#define ITEM 10//��Ʒ����
//#define BEST_SIM_NUM 1//ѡȡ���ƶ���ߣ�Ȩ����󣩵��û���

//ÿ���û���һ����������ʾ
typedef struct NODE
{
	int num;//�û����1-285
	int rate[ITEM];//��Ʒ��������
	int result[ITEM];//���ս������
	struct NODE *next;
}NODE;

//��ӡ��������
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

//��text�ļ������������
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

//��ƽ��ֵ���ų�ֵΪ0����
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

//���
double Sum(double arr[], int num)
{
	int i;
	double sum;
	int no_zero_num = 0;

	for(i = 0, sum = 0; i < num; i++)
		sum = sum + arr[i];

	return sum;
}

//��Ȩ�أ������Ϊa��Ŀ���û��������û������Ƴ̶ȣ�ע��a-1Ϊ�����±꣬a_nodeΪָ����Ϊa�Ľڵ��ָ��
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
			else w[i] = 0;//���û���a�û�û��������ͬ����Ʒ����ʹ��ĸΪ0����˵�����ֵ���˴���ֵΪ0��ʹ��Ժ�����Ͳ����Ӱ��
		}
		else w[i] = 0;//a�û������������Ȩ��	
		
		p = p->next;
	}

}

//��������
void swap(int *a,int *b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

//�Ծ����һ�н��п������򣬵ڶ��иı���Ӧλ��
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

//������
void main()
{
	int num;
	int rate[ITEM];
	int node_size = sizeof(NODE);
	NODE *head, *p;
	NODE *(con[USER]);
	int item;//��Ʒ���

	FILE *fp, *fpw;

	int i, j, k, l, temp_num, temp_rate;
	int flag = 0;

	double mean[USER];//��ֵ
	double w[USER];//Ȩ��
	double temp[3][USER];//��0�д�������û���Ȩ�أ���1�д���û�i����Ʒj�����ۣ���2�д���û�i������ƽ��ֵ
	double kappa;//��һ��ָ��
	double temp_p, predict;

	printf("Please wait...\n");

	//���ļ�
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
						con[i]->rate[k] = 0;//δ���۵���Ʒ��ʼ������Ϊ0
					j = item - 1;
				}
			}
			if(j != ITEM){
				for(k = j; k < ITEM; k++)
					con[i]->rate[k] = 0;//ʣ���δ���۵���Ʒ��ʼ������Ϊ0
				flag = 1;//��ʾ�û�û���������һ����Ʒ��ʹ���ļ�ָ���Ѿ�ָ����һ�û������۵ĵڶ�����Ʒ
			}
			else flag = 0;//��ʾ�û��������һ����Ʒ��ʹ���ļ�ָ���Ѿ�ָ����һ�û������۵ĵ�һ����Ʒ
		}
		else
		{
			con[i]->rate[item-1] = temp_rate;
			for(j = 0; j < item-1; j++) 
				con[i]->rate[j] = 0;//��ͷδ���۵���Ʒ��ʼ������Ϊ0

			for(j = item; (j < ITEM)&&(!feof(fp)); j++) 
			{			
				fscanf(fp, "%d%d%d", &temp_num, &item, &temp_rate);
				if(temp_num != i+1)
					break;
				
				con[i]->rate[item-1] = temp_rate;
				if(item != j + 1)
				{
					for(k = j; k < item-1; k++) 
						con[i]->rate[k] = 0;//δ���۵���Ʒ��ʼ������Ϊ0
					j = item - 1;
				}
			}
			if(j != ITEM){
				for(k = j; k < ITEM; k++)
					con[i]->rate[k] = 0;//ʣ���δ���۵���Ʒ��ʼ������Ϊ0
				flag = 1;//��ʾ�û�û���������һ����Ʒ��ʹ���ļ�ָ���Ѿ�ָ����һ�û������۵ĵڶ�����Ʒ
			}
			else flag = 0;//��ʾ�û��������һ����Ʒ��ʹ���ļ�ָ���Ѿ�ָ����һ�û������۵ĵ�һ����Ʒ
		}
	}

	//�����б�
	for(i = 0; i < USER-1; i++) con[i]->next=con[i+1];
	con[USER-1]->next=NULL;	
	head=con[0];

//	Print_list(head);
	
	//��������û������۾�ֵ
	for(i = 0; i < USER; i++)
	{
		mean[i] = Mean(con[i]->rate, ITEM);
//		printf("%.2f\n", mean[i]);
	}

	//���ݴ���Ԥ�⣬�����������result[]��
	for(i = 0; i < USER; i++)
	{
		Weight(head, i+1, con[i], mean, w);
		kappa = Sum(w, USER);//������Ȩ�صĺͣ�����һ��ϵ��

		//��������ݴ���temp[][]��
		for(j = 0; j < USER; j++) 
		{
			temp[0][j] = w[j];
			temp[2][j] = mean[j];
		}
//			printf("%.2f\n", w[j]);
//		printf("\n");

		for(j = 0; j < ITEM; j++)
		{
			
			if(!(con[i]->rate[j]))//���rate=0����˵������Ʒ��δ������
			{
			//////////////////////////////////////////////////////////
//				con[i]->result[j] = con[i]->rate[j];//////////////////��Ҫ�޸ĳ�Ԥ���㷨
			///////////////////////////////////////////////////////////	

				//��������ݴ���temp[][]��
				for(k = 0; k < USER; k++)
				{
//					temp[0][k] = w[k];
					temp[1][k] = con[k]->rate[j];
//					printf("%.2f %.0f\n", temp[0][k], temp[1][k]);
				}
//				printf("\n");
				
				//��Ȩ�ؽ�������temp��Ӧ�ĵ�1��2������Ҳ����λ��
				Qksort(temp, 0, USER-1);
				for(k = 0; k < 2; k++)
				{
					/*for(l = 0; l < USER; l++)
					{
						printf("%.2f ", temp[k][l]);
					}*/	
				
				}

				//ȡǰBEST_SIM_NUM������Ҫ�����������Ԥ�����
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

	//���ļ�
	if((fpw = fopen("result.txt", "w")) == NULL)
//	if((fpw = fopen("test_result.txt", "w")) == NULL)
	{
		printf("File open error!\n");
		exit(0);
	}

	//���ļ�������
	Fprint_list(head, fpw);

	//�ر��ļ�
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
	
	//ɾ�������ͷ��ڴ�
	for(i = USER-2; i >= 1; i--)
		free(con[i]->next);
	free(head);

	printf("Prediction finished.\n");
}