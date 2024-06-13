#ifndef UNTITLED1_FUNC_H
#define UNTITLED1_FUNC_H

#include "structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <threads.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

typedef struct {
    index_record * buf;
    int block_size;
    int thread_num;
} thread_args;

typedef struct {
    int block_size;
    int threads;
    char* file_name;
} file_sort_args;

extern int size;
extern int blocks;
extern int threads;

extern pthread_barrier_t barrier;
extern pthread_mutex_t mutex;
extern index_record* cur;

int compare(const void *a, const void *b);

void *sort_in_memory(void *thread);

void *sort_file_in_memory(void *data);

void init_barrier_mutex();

void destroy_barrier_mutex();

void merge_blocks(index_record *current, int mergeStep, int block_size, int bufSize);

void sort_block(index_record *current, int block_size, int (*compare)(const void *, const void *));

void open_file_or_exit(const char *file_name, FILE **file, char *mode);

#endif //UNTITLED1_FUNC_H
