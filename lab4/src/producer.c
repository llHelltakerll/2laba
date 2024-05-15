#define _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/random.h>
#include <time.h>

#include "ring.h"

uint16_t get_hash(uint8_t* data, size_t len) 
{
    uint16_t result;
    
    for (size_t i = 0 ; i < len; i++)
        result += data[i];

    return result / len;
}

Message generate_message() 
{
    srand((unsigned)time(NULL));
    const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; 
    
    Message message;
    uint16_t random;

    do
    {
        getrandom(&random, sizeof(random), 0);
        message.size = random = random % 257;
        if (random != 0)
            break;
    }
    while(1);
        
    for(int i = 0; i < random; i++)
        message.data[i] = letters[rand() % 53];

    message.hash = get_hash(message.data, message.size);
    message.type = rand() % 256;

    return message;
}

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

    sem_t* producer = sem_open("/producer", 0);
    sem_t* mutex = sem_open("/mutex", 0);

    int descriptor = shm_open("/mem", O_RDWR, S_IRUSR | S_IWUSR);
    Ring* ring = (Ring*)mmap(NULL, sizeof(Ring), PROT_READ | PROT_WRITE, MAP_SHARED , descriptor, 0);

    while (flag) 
    {
        sem_wait(producer);
        sem_wait(mutex);
        
        Message mes = generate_message();
        if (push_data(ring, &mes))
        {
            ring->added_messages_counter++;
            printf("- %s\nMessage: %s\n\n", argv[0], mes.data);
        }

        sem_post(mutex);
        sem_post(producer); 
       
        sleep(2);
    }
    munmap(ring, sizeof(Ring));
    sem_close(producer);
    sem_close(mutex);
    
    return 0;
}   