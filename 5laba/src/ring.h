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
#define RING_SIZE 5

typedef struct {
    u_int8_t data[LEN_MESSAGE];
    u_int16_t hash;
    u_int8_t size;
    u_int8_t type;
} Message;

typedef struct Ring_node {
    struct Ring_node *next;
    struct Ring_node *prev;
    Message message;
    bool is_used;
} Ring_node;

typedef struct Ring {
    Ring_node *begin;
    Ring_node *tail;
    size_t produced;
    size_t consumed;
    size_t size_queue;
} Ring;

Ring_node *create_node();

Ring *init_ring();

void append(Ring **, bool);

bool erase(Ring **);

void clear_ring(Ring **);

void push_message(Ring *ring, Message *message);

Message *pop_message(Ring *ring);

void print_ring_nodes(const Ring *ring);

#endif //RING_H

