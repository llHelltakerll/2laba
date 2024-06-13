#ifndef INC_7LABA_FUNC_H
#define INC_7LABA_FUNC_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define MAX_RECORDS 10
#define MAX_NAME_LEN 80
#define MAX_ADDRESS_LEN 80

typedef struct record {
    char name[MAX_NAME_LEN];
    char address[MAX_ADDRESS_LEN];
    int semester;
} record_t;

extern int fd;
extern struct flock fl;

void print_record(int record_index);

void get_record(int record_index, record_t *record);

void modify_record(int record_index, record_t *record);

void save_record(record_t *record, record_t *new_record, int record_index);

void create_file(const char *filename);

void menu(void);

#endif //INC_7LABA_FUNC_H
