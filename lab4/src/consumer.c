#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <unistd.h>

#include "ring.h"

bool flag = true;

void sig1_handler(int signo) 
{
    if (signo != SIGUSR1)
        return;

    flag = false;
}

int main(int argc, char** argv) 
{
    signal(SIGUSR1, sig1_handler);

    sem_t* consumer = sem_open("/consumer", 0);
    sem_t* mutex = sem_open("/mutex", 0);

    int descriptor = shm_open("/mem", O_RDWR, S_IRUSR | S_IWUSR);
    Ring* ring = (Ring*)mmap(NULL, sizeof(Ring), PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);

    while (flag) 
    {
        sem_wait(consumer);
        sem_wait(mutex);

        Message mes = get_data(ring);

        if (mes.data != 0)
        {
            ring->removed_messages_counter++;
            printf("- %s Message: %s\n\n", argv[0], mes.data);
        }

        sem_post(mutex);
        sem_post(consumer);

        sleep(2);
    }

    munmap(ring, sizeof(Ring));
    sem_close(consumer);
    sem_close(mutex);

    return 0;
}