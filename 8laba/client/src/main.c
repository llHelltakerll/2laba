#define _POSIX_C_SOURCE 200809L

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <strings.h>

#define MAX_LEN 1024

int CLOSE = 0;

char request[MAX_LEN];

void processing_request(int fd_client);

void processing_request_from_file(int fd_client);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Wrong arguments!\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo criteria = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = 0,
            .ai_flags = 0
    };
    struct addrinfo *addr_info = NULL;

    if (getaddrinfo(NULL, argv[1], &criteria, &addr_info) != 0) {
        exit(EXIT_FAILURE);
    }

    int fd_client = socket(criteria.ai_family, criteria.ai_socktype, criteria.ai_addrlen);

    if (connect(fd_client, addr_info->ai_addr, addr_info->ai_addrlen) == -1) {
        printf("Failed to connect!\n");
        exit(EXIT_FAILURE);
    }

    char buff[MAX_LEN];
    read(fd_client, buff, MAX_LEN);
    printf("%s", buff);

    while (CLOSE == 0) {
        printf("> ");
        fgets(request, MAX_LEN, stdin);
        if (request[0] == '@') {
            request[strlen(request) - 1] = '\0';
            processing_request_from_file(fd_client);
        } else {
            processing_request(fd_client);
        }
    }

    close(fd_client);
    return 0;
}

void processing_request(int fd_client) {
    write(fd_client, request, MAX_LEN);
    CLOSE = (strncasecmp(request, "QUIT", strlen("QUIT")) == 0) ? 1 : 0;
    ssize_t bytes_read = recv(fd_client, request, MAX_LEN, 0);
    request[bytes_read] = '\0';
    if (bytes_read > 0) {
        fputs(request, stdout);
    } else {
        fprintf(stderr, "Disconnected from server.\n");
        close(fd_client);
        exit(EXIT_FAILURE);
    }
}

void processing_request_from_file(int fd_client) {
    FILE *file = fopen(request + 1, "r");
    if (file == NULL) {
        printf("File to open file.\n");
        return;
    }
    while (!feof(file)) {
        fgets(request, MAX_LEN, file);
        printf("> %s", request);
        processing_request(fd_client);
        if (CLOSE == 1) {
            return;
        }
    }
}
