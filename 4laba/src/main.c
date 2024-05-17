#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include "ring.h"
#include <time.h>

#define BUFFER_SIZE 6

pid_t *childs = NULL;
size_t total_size = 0;

sem_t *items;
sem_t *free_space;
sem_t *mutex;
volatile bool IS_RUNNING = true;

void producer(int32_t);

void consumer(int32_t);

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
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
        allocate_node(&ring_queue);

    printf("Shmid segment : %d\n", ring_queue->shmid);

    menu(ring_queue);

    close_semaphores();
    return 0;
}

void initialize_semaphores(void) {
    sem_unlink("free_space");
    sem_unlink("items");
    sem_unlink("mutex");

    free_space = sem_open("free_space", O_CREAT, 0777, 0);
    if (free_space == SEM_FAILED) {
        perror("Failed to open free_space semaphore");
        exit(EXIT_FAILURE);
    }

    items = sem_open("items", O_CREAT, 0777, BUFFER_SIZE);
    if (items == SEM_FAILED) {
        perror("Failed to open items semaphore");
        exit(EXIT_FAILURE);
    }

    mutex = sem_open("mutex", O_CREAT, 0777, 1);
    if (mutex == SEM_FAILED) {
        perror("Failed to open mutex semaphore");
        exit(EXIT_FAILURE);
    }
}

void menu(Ring *ring_queue) {
    int status;
    char ch;
    do {
        printf("##########################");
        printf("\nSelect an action:\n");
        printf("p - Add a producer\n");
        printf("c - Add a consumer\n");
        printf("q - Quit\n");
        printf("##########################\n");
        ch = getchar();
        switch (ch) {
            case 'p': {
                pid_t pid = fork();
                if (pid == 0) {
                    producer(ring_queue->shmid);
                } else {
                    childs = (pid_t *) realloc(childs, (total_size + 1) * sizeof(pid_t));
                    childs[total_size++] = pid;
                }
                break;
            }
            case 'c': {
                pid_t pid = fork();
                if (pid == 0) {
                    consumer(ring_queue->shmid);
                } else {
                    childs = (pid_t *) realloc(childs, (total_size + 1) * sizeof(pid_t));
                    childs[total_size++] = pid;
                }
                break;
            }
            case 'q': {
                for (size_t i = 0; i < total_size; ++i) {
                    kill(childs[i], SIGUSR1);
                    kill(childs[i], SIGKILL);
                }
                clear_buff(ring_queue);
                IS_RUNNING = false;
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
    sem_unlink("mutex");

    sem_close(mutex);
    sem_close(items);
    sem_close(free_space);

    printf("Semaphores closed and unlinked.\n");
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

void consumer(int32_t shmid) {
    Ring *queue = shmat(shmid, NULL, 0);
    do {
        sem_wait(free_space);
        sem_wait(mutex);
        sleep(2);
        Message *message = pop_message(queue);
        if (message != NULL) {
            display_message(message);
            free(message);
        }
        sem_post(mutex);
        sem_post(items);
        printf("Consumed from CHILD with PID = %d\n", getpid());
        printf("Total messages retrieved = %lu\n", queue->consumed);
    } while (IS_RUNNING);
    shmdt(queue);
}

void producer(int32_t shmid) {
    Ring *queue = shmat(shmid, NULL, 0);
    do {
        sem_wait(items);
        sem_wait(mutex);
        sleep(2);
        Message new_message = generate_message();
        push_message(queue, &new_message);
        sem_post(mutex);
        sem_post(free_space);
        printf("Produced from CHILD with PID = %d\n", getpid());
        printf("Total objects created = %lu\n", queue->produced);
    } while (IS_RUNNING);
    shmdt(queue);
}