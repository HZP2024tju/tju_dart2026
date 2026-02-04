#ifndef _MITRX_H__
#define _MITRX_H__

#include <stdio.h>
#include <stdlib.h>
     //n行m列
float abS(float num);

void mitrx_clear(float* A,int n,int m);

void mitrx_add(float* A, float *B, float *result,int n, int m);

void mitrx_less(float* A, float* B, float* result, int n, int m);


void mitrx_x(float *A, float *B, float* result,int n1,int t1,int m1);

void mitrx_tran(float *A,int n,int m);


void mitrx_back(float *arr,float *result,int N);

#endif
