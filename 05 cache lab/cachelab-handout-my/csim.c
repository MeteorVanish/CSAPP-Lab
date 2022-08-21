// Cache Simulator
// Implemented by MeteorVanish

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "cachelab.h"

#define TRUE 1
#define FALSE 0

int hit_cnt = 0, miss_cnt = 0, eviction_cnt = 0;

struct INPUT {
    char op;
    long long addr;
    int size;
} Input[1000000];

int main(int argc, char *argv[])
{
    // ===== Init =====
    int verboseFlag = FALSE, S = 0, E = 0, b = 0;
    char *Filename = NULL;
    int opt, validFlag = 0; 
    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
        switch (opt) {
            case 'v':
                verboseFlag = TRUE;
                break;
            case 's':
                if (isdigit(optarg[0])) ++validFlag;
                S = 1 << atoi(optarg);
                break;
            case 'E':
                if (isdigit(optarg[0])) ++validFlag;
                E = atoi(optarg);
                break;
            case 'b':
                if (isdigit(optarg[0])) ++validFlag;
                b = atoi(optarg);
                break;
            case 't':
                ++validFlag;
                Filename = optarg;
                break;
            default:
                abort();
        }
    }
    if (validFlag != 4) {
        fprintf(stderr, "Input Error!\nUsage: ./csim [-v] -s <num> -E <num> -b <num> -t <file>\n");
        return 1;
    }
    long long **cache = (long long **)malloc(S * sizeof(long long *));
    int **timestamp = (int **)malloc(S * sizeof(int *));
    for (int i = 0; i < S; ++i) {
        cache[i] = (long long *)malloc(E * sizeof(long long));
        timestamp[i] = (int *)malloc(E * sizeof(int));
        memset(cache[i], 0xff, E * sizeof(long long));
        memset(timestamp[i], 0xff, E * sizeof(int));
    }
    FILE *fp = fopen(Filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Cann't open file <%s>!\n", Filename);
        return 1;
    }
    int cnt = 0;
    while (fscanf(fp, " %c %llx,%x", &Input[cnt].op, &Input[cnt].addr, &Input[cnt].size) != EOF) ++cnt;
    
    // === Simulate ===
    int set, line, stampMin, existFlag = FALSE;
    long long addr;
    for (int i = 0; i < cnt; ++i) {
        if (Input[i].op == 'I') continue;
        if (verboseFlag) printf("%c %llx,%x ", Input[i].op, Input[i].addr, Input[i].size);
        set = (Input[i].addr >> b) & (S - 1);    
        addr = Input[i].addr - Input[i].addr % (1 << b);
        stampMin = 0x7fffffff;
        for (int j = 0; j < E; ++j) {
            if (cache[set][j] == addr) {
                existFlag = TRUE;
                line = j;
                break;
            }
            if (timestamp[set][j] < stampMin) {
                line = j;
                stampMin = timestamp[set][j];
            }
        }
        if (existFlag) {
            existFlag = FALSE;
            timestamp[set][line] = i;
            if (Input[i].op == 'M') {
                hit_cnt += 2;
                if (verboseFlag) printf("hit hit \n");
            }
            else { // L or S
                ++hit_cnt;
                if (verboseFlag) printf("hit \n");
            }
        }
        else {
            cache[set][line] = addr;
            timestamp[set][line] = i;
            if (Input[i].op == 'M') {
                ++hit_cnt, ++miss_cnt;
                if (stampMin != -1) ++eviction_cnt;
                if (verboseFlag) {
                    if (stampMin != -1) printf("miss eviction hit \n");
                    else printf("miss hit \n");
                }
            }
            else { // L or S
                ++miss_cnt;
                if (stampMin != -1) ++eviction_cnt;
                if (verboseFlag) {
                    if (stampMin != -1) printf("miss eviction \n");
                    else printf("miss \n");
                }
            }
        }
    }

    // === Ending ===
    for (int i = 0; i < S; ++i) {
        free(cache[i]);
        free(timestamp[i]);
    }
    free(cache);
    free(timestamp);
    fclose(fp);
    printSummary(hit_cnt, miss_cnt, eviction_cnt);
    return 0;
}
