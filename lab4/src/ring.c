#include "ring.h"

#include <stdio.h>

void init_ring(Ring* const ring) {
   if (ring == NULL) return;

   ring->head = 0;
   ring->tail = 0;
   ring->added_messages_counter = 0;
   ring->removed_messages_counter = 0;
} 

bool is_full(const Ring* const ring) {
    return ((ring->head + 1) % RING_SIZE) == ring->tail;
}

bool is_empty(const Ring* const ring) {
    return (ring->head == 0 && ring->tail == 0);
}

bool push_data(Ring* const ring, const Message* const data) {
    if (is_full(ring)) 
        return false;

    ring->data[ring->head] = *data;
    ring->head = (ring->head + 1) % RING_SIZE;

    return true;
}

Message get_data(Ring* const ring) {
    if (is_empty(ring))
    {
        Message empty_mes = {0};
        return empty_mes;
    } 

    Message data = ring->data[ring->tail];

    ring->tail = (ring->tail + 1) % RING_SIZE;

    return data;
}

void pop_data(Ring* const ring) {
    ring->tail = (ring->tail + 1) % RING_SIZE;
}