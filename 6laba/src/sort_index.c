#define _POSIX_C_SOURCE 200112L

#include "func.h"

int size;
int blocks;
int threads;

pthread_barrier_t barrier;
pthread_mutex_t mutex;
index_record *cur;


int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Wrong number of parameters.\n");
        exit(EXIT_FAILURE);
    }

    size    = atoi(argv[1]);
    blocks  = atoi(argv[2]);
    threads = atoi(argv[3]);

    if ((size % 256 != 0) || (threads < 2 || threads > 8000) || (blocks % 2 != 0 || blocks < threads)) {
        printf("Wrong arguments.\n");
        exit(EXIT_FAILURE);
    }

    init_barrier_mutex();

    file_sort_args *fs_args = (file_sort_args *) malloc(sizeof(file_sort_args));

    fs_args->block_size = size / blocks;
    fs_args->threads = threads;
    fs_args->file_name = argv[4];

    pthread_t id;
    if (pthread_create(&id, NULL, sort_file_in_memory, fs_args) != 0) {
        printf("Error while creating 0 thread.\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(id, NULL) != 0) {
        printf("Error executing process.\n");
        exit(EXIT_FAILURE);
    }

    destroy_barrier_mutex();
    return 0;
}
