#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include "ring.h"
#include <time.h>

pthread_t *threads = NULL;
size_t thread_count = 0;

pthread_mutex_t mutex;
sem_t *items;
sem_t *free_space;
_Thread_local bool IS_RUNNING = true;

size_t consumer_passes = 0;
size_t producer_passes = 0;

void *producer(void *arg);

void *consumer(void *arg);

Message generate_message(void);

void handler_stop_proc();

void display_message(const Message *message);

void initialize_semaphores(void);

void menu(Ring *ring_queue);

void close_semaphores(void);

int main(void) {
    srand(time(NULL));

    signal(SIGUSR1, handler_stop_proc);

    initialize_semaphores();

    Ring *ring_queue = NULL;
    for (size_t i = 0; i < RING_SIZE; ++i)
        append(&ring_queue, false);

    menu(ring_queue);

    close_semaphores();
    return 0;
}

void initialize_semaphores(void) {
    sem_unlink("free_space");
    sem_unlink("items");
    sem_unlink("MUTEX");

    free_space = sem_open("free_space", O_CREAT, 0777, 0);
    if (free_space == SEM_FAILED) {
        perror("Failed to open free_space semaphore");
        exit(EXIT_FAILURE);
    }

    items = sem_open("items", O_CREAT, 0777, RING_SIZE);
    if (items == SEM_FAILED) {
        perror("Failed to open items semaphore");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&mutex, NULL);
}

void menu(Ring *ring_queue) {
    int status;
    char ch;
    do {
        printf("##########################");
        printf("\nSelect an action:\n");
        printf("p - Add a producer\n");
        printf("c - Add a consumer\n");
        printf("+ - Increase queue size\n");
        printf("- - Decrease queue size\n");
        printf("q - Quit\n");
        printf("##########################\n");
        fflush(stdin);
        ch = getchar();
        switch (ch) {
            case 'p': {
                if (ring_queue == NULL) {
                    fprintf(stderr, "Ring queue is not initialized.\n");
                    exit(EXIT_FAILURE);
                }
                if (ring_queue->size_queue < 1) {
                    printf("The size of ring is less then 1.\n");
                    break;
                }

                threads = (pthread_t *) (pthread_t *) realloc(threads, (thread_count + 1) * sizeof(pthread_t));

                int ret = pthread_create(&threads[thread_count], NULL, producer, (void *)ring_queue);

                if (ret != 0) {
                    fprintf(stderr, "Failed to create a producer thread.\n");
                    exit(EXIT_FAILURE);
                } else {
                    thread_count++;
                }
                break;
            }
            case 'c': {
                if (ring_queue == NULL) {
                    fprintf(stderr, "Ring queue is not initialized.\n");
                    exit(EXIT_FAILURE);
                }
                if (ring_queue->size_queue < 1) {
                    printf("The size of ring is less then 1.\n");
                    break;
                }

                threads = (pthread_t *) (pthread_t *) realloc(threads, (thread_count + 1) * sizeof(pthread_t));

                int ret = pthread_create(&threads[thread_count], NULL, consumer, (void *)ring_queue);

                if (ret != 0) {
                    fprintf(stderr, "Failed to create a producer thread.\n");
                    exit(EXIT_FAILURE);
                } else {
                    thread_count++;
                }
                break;
            }
            case '+': {
                pthread_mutex_lock(&mutex);
                append(&ring_queue, true);
                if (ring_queue != NULL) {
                    printf("Insert, count of places : %lu\n", ring_queue->size_queue);
                }
                sem_post(items);
                pthread_mutex_unlock(&mutex);
                break;
            }
            case '-': {
                pthread_mutex_lock(&mutex);
                sleep(2);
                bool flag_execute = erase(&ring_queue);
                if (flag_execute == false) {
                    producer_passes++;
                }
                if (flag_execute == true) {
                    consumer_passes++;
                    sem_wait(free_space);
                }
                if (ring_queue != NULL) {
                    printf("Extract, count of places : %lu\n", ring_queue->size_queue);
                }
                pthread_mutex_unlock(&mutex);
                break;
            }
            case 'q': {
                IS_RUNNING = false;
                for (size_t i = 0; i < thread_count - 1; ++i) {
                    pthread_cancel(threads[i]);
                }
                clear_ring(&ring_queue);
                break;
            }

            default: {
                printf("Incorrect input.\n");
                fflush(stdin);
                break;
            }
        }
        waitpid(-1, &status, WNOHANG);
        getchar();
    } while (IS_RUNNING);
}

void close_semaphores(void) {
    sem_unlink("free_space");
    sem_unlink("items");
    sem_unlink("MUTEX");

    pthread_mutex_destroy(&mutex);
    sem_close(items);
    sem_close(free_space);

    printf("Semaphores and mutex closed and unlinked.\n");
}

Message generate_message(void) {
    Message message = {
            .data = {0},
            .hash = 0,
            .size = 0,
            .type = 0
    };

    do {
        message.size = rand() % 257;
    } while (message.size == 0);

    size_t realSize = message.size;
    if (realSize == 256) {
        message.size = 0;
        realSize = (message.size == 0) ? 256 : message.size;
    }

    message.hash = 0;
    for (size_t i = 0; i < realSize; ++i) {
        message.data[i] = rand() % 256;
        message.hash += message.data[i];
    }

    return message;
}

void display_message(const Message *message) {
    for (int i = 0; i < message->size; ++i) {
        printf("%02X", message->data[i]);
    }
    printf("\n");
}

void handler_stop_proc() {
    IS_RUNNING = false;
}

void *consumer(void *arg) {
    Ring *queue = (Ring *) arg;
    signal(SIGUSR1, handler_stop_proc);
    do {
        sem_wait(free_space);
        pthread_mutex_lock(&mutex);
        if (consumer_passes != 0) {
            consumer_passes--;
            pthread_mutex_unlock(&mutex);
            continue;
        }
        sleep(2);
        Message *message = pop_message(queue);
        pthread_mutex_unlock(&mutex);
        sem_post(items);
        if (message != NULL) {
            display_message(message);
            free(message);
        }
        printf("Consumed from pthread with id = %lu\n", pthread_self());
        printf("Total messages retrieved = %lu\n", queue->consumed);

        struct timespec req, rem;
        req.tv_sec = 0;
        req.tv_nsec = 20000 * 1000;
        nanosleep(&req, &rem);

    } while (IS_RUNNING);

    return NULL;
}

void *producer(void *arg) {
    Ring *queue = (Ring *) arg;
    signal(SIGUSR1, handler_stop_proc);
    do {
        sem_wait(items);
        pthread_mutex_lock(&mutex);
        if (producer_passes != 0) {
            producer_passes--;
            pthread_mutex_unlock(&mutex);
            continue;
        }
        sleep(2);
        Message new_message = generate_message();
        push_message(queue, &new_message);
        pthread_mutex_unlock(&mutex);
        sem_post(free_space);
        printf("Produced from pthread with id = %lu\n", pthread_self());
        printf("Total objects created = %lu\n", queue->produced);

        struct timespec req, rem;
        req.tv_sec = 0;
        req.tv_nsec = 20000 * 1000;
        nanosleep(&req, &rem);

    } while (IS_RUNNING);

    return NULL;
}