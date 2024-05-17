#ifndef RING_H
#define RING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>

#define LEN_MESSAGE 255

typedef struct {
    u_int8_t data[LEN_MESSAGE];
    u_int16_t hash;
    u_int8_t size;
    u_int8_t type;
} Message;

typedef struct Ring_node {
    int32_t shmid_curr;
    int32_t shmid_next;
    int32_t shmid_prev;
    Message message[LEN_MESSAGE];
    bool flag_is_busy;
} Node;

typedef struct Ring {
    int32_t shmid;
    size_t consumed;
    size_t produced;
    int32_t shmid_begin;
    int32_t shmid_tail;
} Ring;

Ring *init_ring();

void clear_buff(Ring *);

Message *pop_message(Ring *);

Node *create_node();

void allocate_node(Ring **begin);

void push_message(Ring *ring, Message *message);

#endif //RING_H

