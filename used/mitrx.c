
#include <stdio.h>
#include <stdlib.h>
#include "mitrx.h"
     //n行m列
float abS(float num)
{
	if (num >= 0)
	{
		return num;
	}
	else
	{
		return -num;
	}
}
void mitrx_clear(float* A,int n,int m)
{
	for (int i = 0; i < n * m; i++)
	{
		A[i] = 0;
	}
}
void mitrx_add(float* A, float *B, float *result,int n, int m)
{
	mitrx_clear(result, n, m);
	for (int i1 = 0; i1 < n; i1++)
	{
		for (int i2 = 0; i2 < m; i2++)
		{
			result[i1*m + i2] = A[i1*m + i2] + B[i1*m + i2];
		}
	}
}

void mitrx_less(float* A, float* B, float* result, int n, int m)
{
	mitrx_clear(result, n, m);
	for (int i1 = 0; i1 < n; i1++)
	{
		for (int i2 = 0; i2 < m; i2++)
		{
			result[i1*m + i2] = A[i1*m + i2] - B[i1*m + i2];
		}
	}
}

void mitrx_x(float *A, float *B, float* result,int n1,int t1,int m1)
{
	mitrx_clear(result, n1, m1);
	int i = 0;
	int j = 0;
	int m = 0;

	for (i = 0; i < n1; i++)
	{
		for (j = 0; j < m1; j++)
		{
			float sum = 0;
			for (m = 0; m < t1; m++)
			{
				sum += A[i*t1 + m] * B[m*m1 + j]; //A[i][m]  B[m][j]
			}
			result[i*m1 + j] = sum;
		}
	}
}

void mitrx_tran(float *A,int n,int m)
{
	float* temp =(float*) malloc(m * n * sizeof(float));
	for (int i1 = 0; i1 < n; i1++)
	{
		for (int i2 = 0; i2 < m; i2++)
		{
			temp[i2 * n + i1] = A[i1 * m + i2];
		}
	}
	for (int i1 = 0; i1 < n*m; i1++)
	{
		A[i1] = temp[i1];
	}
	free(temp);
}

void mitrx_back(float *arr,float *result,int N)
{
	mitrx_clear(result, N,N);
	int i, j, k;
	float* W = (float*)malloc(N * 2 * N * sizeof(float)); //N*2N
	float tem_1, tem_2, tem_3;

	// 对矩阵右半部分进行扩增
	for (i = 0; i < N; i++) {
		for (j = 0; j < 2 * N; j++) {
			if (j < N) {
				W[i*2 *N + j] = (float)arr[i*N + j];
			}
			else {
				W[i*N * 2 + j] = (float)(j - N == i ? 1 : 0);
			}
		}
	}

	for (i = 0; i < N; i++)
	{
		// 判断矩阵第一行第一列的元素是否为0，若为0，继续判断第二行第一列元素，直到不为0，将其加到第一行
		if (((int)W[i * 2 * N + i]) == 0)
		{
			for (j = i + 1; j < N; j++)
			{
				if (((int)W[j * 2 * N + i]) != 0) break;
			}
			if (j == N)
			{
				printf("\ncan not back\n");
				break;
			}
			//将前面为0的行加上后面某一行
			for (k = 0; k < 2 * N; k++)
			{
				W[i * 2 * N + k] += W[j * 2 * N + k];
			}
		}

		//将前面行首位元素置1
		tem_1 = W[i * 2 * N + i];
		for (j = 0; j < 2 * N; j++)
		{
			W[i * 2 * N + j] = W[i * 2 * N + j] / tem_1;
		}

		//将后面所有行首位元素置为0
		for (j = i + 1; j < N; j++)
		{
			tem_2 = W[j * 2 * N + i];
			for (k = i; k < 2 * N; k++)
			{
				W[j * 2 * N + k] = W[j * 2 * N + k] - tem_2 * W[i * 2 * N + k];
			}
		}
	}

	// 将矩阵前半部分标准化
	for (i = N - 1; i >= 0; i--)
	{
		for (j = i - 1; j >= 0; j--)
		{
			tem_3 = W[j * 2 * N + i];
			for (k = i; k < 2 * N; k++)
			{
				W[j * 2 * N + k] = W[j * 2 * N + k] - tem_3 * W[i * 2 * N + k];
			}
		}
	}

	//得出逆矩阵
	for (i = 0; i < N; i++)
	{
		for (j = N; j < 2 * N; j++)
		{
			result[i*N + j - N] = W[i * 2 * N + j];
		}
	}
	free(W);
}

