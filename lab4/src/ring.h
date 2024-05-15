#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DATA_MAX_LEN 255

#define RING_SIZE 10

typedef struct Message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    uint8_t data[DATA_MAX_LEN];
} Message;

typedef struct Ring {
    Message data[RING_SIZE];
    size_t head;
    size_t tail;
    size_t added_messages_counter;
    size_t removed_messages_counter;
} Ring;

void init_ring(Ring* const ring);

bool is_full(const Ring* const ring);
bool is_empty(const Ring* const ring);

bool push_data(Ring* const ring, const Message* const data);
Message get_data(Ring* const ring);
void pop_data(Ring* const ring);