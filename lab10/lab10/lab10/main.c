/*
 * lab10.c
 *
 * Created: 18/12/2020 4:26:23 μμ
 * Author : Δημήτρης
 */ 

#include <avr/io.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/cpufunc.h>

#define N 3

volatile int int_matrix_A[3][3] = { { 1, 1, 1 },
						            { 2, 2, 2 },
						            { 3, 3, 3 } };
volatile int int_matrix_B[3][3] = { { 1, 1, 1 },
						            { 2, 2, 2 },
						            { 3, 3, 3 } };
volatile int int_matrix_C[3][3];

volatile float float_matrix_A[3][3] = { { 1.0, 1.0, 1.0 },
							            { 2.0, 2.0, 2.0 },
							            { 3.0, 3.0, 3.0 } };
volatile float float_matrix_B[3][3]= { { 1.0, 1.0, 1.0 },
									   { 2.0, 2.0, 2.0 },
									   { 3.0, 3.0, 3.0 } };
volatile float float_matrix_C[3][3];

void int_multiply(volatile int A[][N], volatile int B[][N], volatile int C[][N]){
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			C[i][j] = 0;
			for (int k = 0; k < N; k++){
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	return;
}

void float_multiply(volatile float A[][N], volatile float B[][N], volatile float C[][N]){
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			C[i][j] = 0.0;
			for (int k = 0; k < N; k++){
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	return;
}

int main(void){
	
	int_multiply(int_matrix_A, int_matrix_B, int_matrix_C);
	
	float_multiply(float_matrix_A,float_matrix_B, float_matrix_C);
	
	/*for (int i=0; i<N; i++){
		for (int j=0; j<N; j++){
			printf("%f", float_matrix_C[i][j]);
		}
	}*/
	while(1){
		
	}
}

