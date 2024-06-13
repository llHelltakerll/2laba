#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "structures.h"

void open_file_or_exit(const char *file_name, FILE **file, char *mode);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of parameters.\n");
        exit(EXIT_FAILURE);
    }

    if (atoi(argv[1]) % 256 != 0) {
        printf("Wrong arguments.\n");
        exit(0);
    }

    srand(time(NULL));

    index_hdr_s header;
    header.records = atoi(argv[1]);

    header.idx = (index_record *) malloc(header.records * sizeof(index_record));
    for (int i = 0; i < (int) header.records; i++) {
        header.idx[i].recno = i + 1;
        header.idx[i].time_mark = 15020 + rand() % (60378 - 15020 + 1);
        header.idx[i].time_mark +=
                0.5 * ((rand() % 24) * 60 * 60 + (rand() % 60) * 60 + rand() % 60) / (12 * 60 * 60);
    }

    FILE *file;
    open_file_or_exit(argv[2], &file, "wb");

    fwrite(&header.records, sizeof(header.records), 1, file);
    fwrite(header.idx, sizeof(index_record), header.records, file);

    fclose(file);
    free(header.idx);
    return 0;
}

void open_file_or_exit(const char *file_name, FILE **file, char *mode) {
    *file = fopen(file_name, mode);
    if (*file == NULL) {
        printf("Failed to open file.\n");
        exit(EXIT_FAILURE);
    }
}
