// Implemented by MeteorVanish

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    #define min(x, y) (((x) < (y)) ? (x) : (y))
    /* 
     * misses: 256(+3) 
     * BlockSize: 8 * 8
     */
    if (M == 32 && N == 32) {
        int iBlock, jBlock, i, j, reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7;
        for (iBlock = 0; iBlock < N; iBlock += 8) {
            for (jBlock = 0; jBlock < M; jBlock += 8) {
                // copy
                for (i = 0; i < 8; ++i) {
                    reg0 = A[iBlock + i][jBlock + 0];
                    reg1 = A[iBlock + i][jBlock + 1];
                    reg2 = A[iBlock + i][jBlock + 2];
                    reg3 = A[iBlock + i][jBlock + 3];
                    reg4 = A[iBlock + i][jBlock + 4];
                    reg5 = A[iBlock + i][jBlock + 5];
                    reg6 = A[iBlock + i][jBlock + 6];
                    reg7 = A[iBlock + i][jBlock + 7];
                    B[jBlock + i][iBlock + 0] = reg0;
                    B[jBlock + i][iBlock + 1] = reg1;
                    B[jBlock + i][iBlock + 2] = reg2;
                    B[jBlock + i][iBlock + 3] = reg3;
                    B[jBlock + i][iBlock + 4] = reg4;
                    B[jBlock + i][iBlock + 5] = reg5;
                    B[jBlock + i][iBlock + 6] = reg6;
                    B[jBlock + i][iBlock + 7] = reg7;
                }
                // transpose
                for (i = 0; i < 8; ++i) {
                    for (j = i + 1; j < 8; ++j) {
                        reg0 = B[jBlock + i][iBlock + j];
                        B[jBlock + i][iBlock + j] = B[jBlock + j][iBlock + i];
                        B[jBlock + j][iBlock + i] = reg0;
                    }
                }
            }
        }
    }
    /* 
     * misses: 1184(+3)
     * BlockSize: 8 * 8
     */
    else if (M == 64 && N == 64) { 
        int iBlock, jBlock, i, j, reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7;
        for (iBlock = 0; iBlock < N; iBlock += 8) {
            for (jBlock = 0; jBlock < M; jBlock += 8) {
                // copy upper 4 lines
                for (i = 0; i < 4; ++i) {
                    reg0 = A[iBlock + i][jBlock + 0];
                    reg1 = A[iBlock + i][jBlock + 1];
                    reg2 = A[iBlock + i][jBlock + 2];
                    reg3 = A[iBlock + i][jBlock + 3];
                    reg4 = A[iBlock + i][jBlock + 4];
                    reg5 = A[iBlock + i][jBlock + 5];
                    reg6 = A[iBlock + i][jBlock + 6];
                    reg7 = A[iBlock + i][jBlock + 7];
                    B[jBlock + i][iBlock + 0] = reg0;
                    B[jBlock + i][iBlock + 1] = reg1;
                    B[jBlock + i][iBlock + 2] = reg2;
                    B[jBlock + i][iBlock + 3] = reg3;
                    B[jBlock + i][iBlock + 4] = reg4;
                    B[jBlock + i][iBlock + 5] = reg5;
                    B[jBlock + i][iBlock + 6] = reg6;
                    B[jBlock + i][iBlock + 7] = reg7;
                }
                // transpose upper 4 lines
                for (i = 0; i < 4; ++i) {
                    for (j = i + 1; j < 4; ++j) {
                        reg0 = B[jBlock + i][iBlock + j];
                        reg1 = B[jBlock + i][iBlock + j + 4];
                        B[jBlock + i][iBlock + j] = B[jBlock + j][iBlock + i];
                        B[jBlock + i][iBlock + j + 4] = B[jBlock + j][iBlock + i + 4];
                        B[jBlock + j][iBlock + i] = reg0;
                        B[jBlock + j][iBlock + i + 4] = reg1;
                    }
                }
                // swap lower-left and upper-right, copy and transpose lower-right
                for (i = 0; i < 4; ++i) {
                    reg0 = B[jBlock + i][iBlock + 4];
                    reg1 = B[jBlock + i][iBlock + 5];
                    reg2 = B[jBlock + i][iBlock + 6];
                    reg3 = B[jBlock + i][iBlock + 7];
                    B[jBlock + i][iBlock + 4] = A[iBlock + 4][jBlock + i];
                    reg4 = A[iBlock + 4][jBlock + i + 4];
                    B[jBlock + i][iBlock + 5] = A[iBlock + 5][jBlock + i];
                    reg5 = A[iBlock + 5][jBlock + i + 4];
                    B[jBlock + i][iBlock + 6] = A[iBlock + 6][jBlock + i];
                    reg6 = A[iBlock + 6][jBlock + i + 4];
                    B[jBlock + i][iBlock + 7] = A[iBlock + 7][jBlock + i];
                    reg7 = A[iBlock + 7][jBlock + i + 4];
                    B[jBlock + i + 4][iBlock + 0] = reg0;
                    B[jBlock + i + 4][iBlock + 1] = reg1;
                    B[jBlock + i + 4][iBlock + 2] = reg2;
                    B[jBlock + i + 4][iBlock + 3] = reg3;
                    B[jBlock + i + 4][iBlock + 4] = reg4;
                    B[jBlock + i + 4][iBlock + 5] = reg5;
                    B[jBlock + i + 4][iBlock + 6] = reg6;
                    B[jBlock + i + 4][iBlock + 7] = reg7;
                }
            }
        }
    }
    /* 
     * misses: 1790(+3)
     * BlockSize: 8 * 20
     */
    else if (M == 61 && N == 67) {
        int iBlock, jBlock, i, j, reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7;
        for (iBlock = 0; iBlock < N; iBlock += 8) {
            for (jBlock = 0; jBlock < M; jBlock += 20) {
                if (iBlock + 8 <= N && jBlock + 20 <= M) {
                    for (j = 0; j <= 20; ++j) {
                        reg0 = A[iBlock + 0][jBlock + j];
                        reg1 = A[iBlock + 1][jBlock + j];
                        reg2 = A[iBlock + 2][jBlock + j];
                        reg3 = A[iBlock + 3][jBlock + j];
                        reg4 = A[iBlock + 4][jBlock + j];
                        reg5 = A[iBlock + 5][jBlock + j];
                        reg6 = A[iBlock + 6][jBlock + j];
                        reg7 = A[iBlock + 7][jBlock + j];
                        B[jBlock + j][iBlock + 0] = reg0;
                        B[jBlock + j][iBlock + 1] = reg1;
                        B[jBlock + j][iBlock + 2] = reg2;
                        B[jBlock + j][iBlock + 3] = reg3;
                        B[jBlock + j][iBlock + 4] = reg4;
                        B[jBlock + j][iBlock + 5] = reg5;
                        B[jBlock + j][iBlock + 6] = reg6;
                        B[jBlock + j][iBlock + 7] = reg7;
                    }
                }
                else {
                    for (i = iBlock; i < min(iBlock + 8, N); ++i)
                        for (j = jBlock; j < min(jBlock + 20, M); ++j)
                            B[j][i] = A[i][j];
                }
            }
        }
    }
    // BlockSize: 8 * 8
    else {
        int iBlock, jBlock, i, j, reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7;
        for (iBlock = 0; iBlock < N; iBlock += 8) {
            for (jBlock = 0; jBlock < M; jBlock += 8) {
                if (iBlock + 8 <= N && jBlock + 8 <= M) {
                    for (j = 0; j <= 8; ++j) {
                        reg0 = A[iBlock + 0][jBlock + j];
                        reg1 = A[iBlock + 1][jBlock + j];
                        reg2 = A[iBlock + 2][jBlock + j];
                        reg3 = A[iBlock + 3][jBlock + j];
                        reg4 = A[iBlock + 4][jBlock + j];
                        reg5 = A[iBlock + 5][jBlock + j];
                        reg6 = A[iBlock + 6][jBlock + j];
                        reg7 = A[iBlock + 7][jBlock + j];
                        B[jBlock + j][iBlock + 0] = reg0;
                        B[jBlock + j][iBlock + 1] = reg1;
                        B[jBlock + j][iBlock + 2] = reg2;
                        B[jBlock + j][iBlock + 3] = reg3;
                        B[jBlock + j][iBlock + 4] = reg4;
                        B[jBlock + j][iBlock + 5] = reg5;
                        B[jBlock + j][iBlock + 6] = reg6;
                        B[jBlock + j][iBlock + 7] = reg7;
                    }
                }
                else {
                    for (i = iBlock; i < min(iBlock + 8, N); ++i)
                        for (j = jBlock; j < min(jBlock + 8 , M); ++j)
                            B[j][i] = A[i][j];
                }
            }
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 
}