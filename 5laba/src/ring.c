#include "ring.h"

#include <inttypes.h>

Ring_node *create_node() {
    Ring_node *buffer = (Ring_node *) calloc(1, sizeof(Ring_node));
    if (!buffer) {
        perror("Failed to allocate memory for node");
        exit(EXIT_FAILURE);
    }
    buffer->next = NULL;
    buffer->prev = NULL;
    buffer->is_used = false;

    memset(&buffer->message, 0, sizeof(Message));
    return buffer;
}

Ring *init_ring() {
    Ring *buffer = (Ring *) malloc(sizeof(Ring));
    if (!buffer) {
        perror("Failed to allocate memory for ring");
        exit(EXIT_FAILURE);
    }
    buffer->begin = NULL;
    buffer->tail = NULL;
    buffer->size_queue = 0;
    buffer->produced = 0;
    buffer->consumed = 0;
    return buffer;
}

void append(Ring **ring, bool flag_after) {
    if (ring == NULL)
        exit(-100);
    if (*ring == NULL) {
        *ring = init_ring();
        Ring_node *new_node = create_node();
        (*ring)->begin = (*ring)->tail = new_node;
        new_node->next = new_node->prev = new_node;
        (*ring)->size_queue++;
        return;
    }
    (*ring)->size_queue++;
    Ring_node *buffer = create_node();
    if (!flag_after) {
        buffer->next = (*ring)->begin;
        buffer->prev = (*ring)->begin->prev;
        (*ring)->begin->prev->next = buffer;
        (*ring)->begin->prev = buffer;
        (*ring)->begin = buffer;
    } else {
        buffer->prev = (*ring)->tail;
        buffer->next = (*ring)->tail->next;
        (*ring)->tail->next->prev = buffer;
        (*ring)->tail->next = buffer;
        (*ring)->tail = buffer;
    }
}

bool erase(Ring **ring) {
    if (ring == NULL || *ring == NULL || (*ring)->begin == NULL) {
        fprintf(stderr, "The queue is empty or not initialized.\n");
        exit(-100);
    }
    (*ring)->size_queue--;
    bool result = false;

    Ring_node *to_erase = (*ring)->tail;

    if ((*ring)->size_queue == 1) {
        printf("Only one element left in the ring, cannot erase.\n");
        result = to_erase->is_used;
        (*ring)->size_queue++;
    } else {
        to_erase->prev->next = to_erase->next;
        to_erase->next->prev = to_erase->prev;
        if (to_erase == (*ring)->begin) {
            (*ring)->begin = to_erase->next;
        }
        if (to_erase == (*ring)->tail) {
            (*ring)->tail = to_erase->prev;
        }
        result = to_erase->is_used;
        free(to_erase);
    }

    return result;
}

void clear_ring(Ring **ring) {
    if (ring == NULL || *ring == NULL)
        return;
    Ring_node *current = (*ring)->begin;
    Ring_node *next;
    for (size_t i = 0; i < (*ring)->size_queue; ++i) {
        next = current->next;
        free(current);
        current = next;
    }
    free(*ring);
    *ring = NULL;
}

void push_message(Ring *ring, Message *message) {
    if (ring == NULL) {
        printf("The ring is empty.\n");
        return;
    }
    if (ring->begin == NULL) {
        printf("There are 0 places in the ring.\n");
        return;
    }
    Ring_node *curr = ring->tail;
    if (curr->is_used == true) {
        printf("No free places.\n");
        return;
    }
    curr->message = *message;
    curr->is_used = true;
    ring->tail = ring->tail->next;
    ring->produced++;
}

Message *pop_message(Ring *ring) {
    if (ring == NULL) {
        printf("The ring is empty.\n");
        return NULL;
    }
    if (ring->begin == NULL) {
        printf("There are 0 places in the ring.\n");
        return NULL;
    }
    Ring_node *curr = ring->begin;
    if (curr->is_used == false) {
        printf("No messages to pop.\n");
        return NULL;
    }

    Message *popped_message = (Message *) malloc(sizeof(Message));
    if (!popped_message) {
        perror("Failed to allocate memory for message");
        exit(EXIT_FAILURE);
    }
    *popped_message = curr->message;

    curr->is_used = false;
    ring->begin = ring->begin->next;
    ring->consumed++;
    return popped_message;
}

void print_ring_nodes(const Ring *ring) {
    if (ring == NULL || ring->begin == NULL) {
        printf("The ring is empty.\n");
        return;
    }
    Ring_node *current = ring->begin;
    int count = 1;
    do {
        printf("Node %d\n", count++);
        current = current->next;
    } while (current != ring->begin);
}
