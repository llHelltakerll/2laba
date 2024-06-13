#define _POSIX_C_SOURCE 200112L

#include "func.h"

void open_file_or_exit(const char *file_name, FILE **file, char *mode) {
    *file = fopen(file_name, mode);
    if (*file == NULL) {
        printf("Failed to open file.\n");
        exit(EXIT_FAILURE);
    }
}

size_t get_file_size(FILE *file) {
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return file_size;
}

void init_barrier_mutex() {
    if (pthread_barrier_init(&barrier, NULL, threads)) {
        printf("Failed to open barrier.\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Failed to open mutex.\n");
        exit(EXIT_FAILURE);
    }
}

void destroy_barrier_mutex() {
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
}

int compare(const void *a, const void *b) {
    if (((index_record *) a)->time_mark < ((index_record *) b)->time_mark)
        return -1;
    else if (((index_record *) a)->time_mark > ((index_record *) b)->time_mark)
        return 1;
    else
        return 0;
}

void sort_block(index_record *current, int block_size, int (*compare)(const void *, const void *)) {
    qsort(current, block_size, sizeof(index_record), compare);
}

void merge_blocks(index_record *current, int mergeStep, int block_size, int bufSize) {
    index_record *left = (index_record *) malloc(bufSize * sizeof(index_record));
    memcpy(left, current, (mergeStep / 2) * block_size * sizeof(index_record));

    index_record *right = (index_record *) malloc(bufSize * sizeof(index_record));
    memcpy(right, current + (mergeStep / 2) * block_size, (mergeStep / 2) * block_size * sizeof(index_record));

    int i = 0, j = 0;
    while (i < bufSize && j < bufSize) {
        if (left[i].time_mark < right[j].time_mark) {
            current[i + j].time_mark = left[i].time_mark;
            current[i + j].recno = left[i].recno;
            i++;
        } else {
            current[i + j].time_mark = right[j].time_mark;
            current[i + j].recno = right[j].recno;
            j++;
        }
    }

    if (i == bufSize) {
        while (j < bufSize) {
            current[i + j].time_mark = right[j].time_mark;
            current[i + j].recno = right[j].recno;
            j++;
        }
    }

    if (j == bufSize) {
        while (i < bufSize) {
            current[i + j].time_mark = left[i].time_mark;
            current[i + j].recno = left[i].recno;
            i++;
        }
    }

    free(left);
    free(right);
}

void *sort_in_memory(void *thread) {
    thread_args *args = (thread_args *) thread;
    pthread_barrier_wait(&barrier);

    printf("Sort in %d thread.\n", args->thread_num);
    while (cur < args->buf + size) {
        pthread_mutex_lock(&mutex);

        if (cur < args->buf + size) {
            index_record *temp = cur;
            cur += args->block_size;

            pthread_mutex_unlock(&mutex);
            sort_block(temp, args->block_size, compare);
        } else {
            pthread_mutex_unlock(&mutex);
            pthread_barrier_wait(&barrier);
            break;
        }
    }

    printf("Merging in %d thread.\n", args->thread_num);
    int mergeStep = 2;

    while (mergeStep <= blocks) {
        pthread_barrier_wait(&barrier);
        cur = args->buf;

        while (cur < args->buf + size) {
            pthread_mutex_lock(&mutex);
            if (cur < args->buf + size) {
                index_record *temp = cur;
                cur += mergeStep * args->block_size;
                pthread_mutex_unlock(&mutex);
                int bufSize = (mergeStep / 2) * args->block_size;
                merge_blocks(temp, mergeStep, args->block_size, bufSize);
            } else {
                pthread_mutex_unlock(&mutex);
                break;
            }
        }

        mergeStep *= 2;
    }

    pthread_mutex_unlock(&mutex);
    pthread_barrier_wait(&barrier);
    return NULL;
}

void *sort_file_in_memory(void *data) {
    file_sort_args *fs_args = (file_sort_args *) data;

    FILE *file;
    open_file_or_exit(fs_args->file_name, &file, "rb+");

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    int fd = fileno(file);
    void *buf;
    if ((buf = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        printf("Failed to open mapping.\n");
        exit(EXIT_FAILURE);
    }

    uint64_t *file_start = (uint64_t *)buf;
    cur = (index_record *)(file_start + 1);

    pthread_t threads[fs_args->threads - 1];
    for (int i = 1; i < fs_args->threads; i++) {
        thread_args *args = (thread_args *) malloc(sizeof(thread_args));
        args->block_size = fs_args->block_size;
        args->thread_num = i;
        args->buf = (index_record *) buf;

        if (pthread_create(&threads[i - 1], NULL, sort_in_memory, args) != 0) {
            printf("Failed to create %d thread.\n", i);
            exit(0);
        }
    }

    thread_args *args = (thread_args *) malloc(sizeof(thread_args));
    args->block_size = fs_args->block_size;
    args->thread_num = 0;
    args->buf = (index_record *) buf;
    sort_in_memory(args);

    for (int i = 1; i < fs_args->threads; i++)
        pthread_join(threads[i - 1], NULL);

    munmap(buf, file_size);
    fclose(file);
    return NULL;
}
