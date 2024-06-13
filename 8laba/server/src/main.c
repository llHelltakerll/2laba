#define _POSIX_C_SOURCE 200809L

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAX_LEN 1024

char request[MAX_LEN];
char curr[MAX_LEN];
char listDir[MAX_LEN];

void print_dir(DIR *dir);

void read_file(const char *filename, char **buffer, size_t *size);

void *handle_client(void *arg);

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    char **argv;
} client_info_t;

void copy_argv(client_info_t *client_info, int argc, char *argv[]);

void get_client_info_str(client_info_t *client_info, char *client_info_str, size_t len);

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Wrong arguments");
        exit(EXIT_FAILURE);
    }

    struct addrinfo criteria = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
            .ai_flags = AI_PASSIVE,
            .ai_protocol = 0,
            .ai_canonname = NULL,
            .ai_addr = NULL,
            .ai_next = NULL
    };
    struct addrinfo *addr_info = NULL;

    if (getaddrinfo(NULL, argv[1], &criteria, &addr_info) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errno));
        exit(EXIT_FAILURE);
    }

    int socfd = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);

    bind(socfd, addr_info->ai_addr, addr_info->ai_addrlen);

    printf("myserver %s\n", argv[1]);
    printf("Ready.\n");

    listen(socfd, SOMAXCONN);

    while (1) {
        client_info_t *client_info = malloc(sizeof(client_info_t));
        client_info->client_addr_len = sizeof(client_info->client_addr);

        client_info->client_socket = accept(socfd,
                                            (struct sockaddr *) &client_info->client_addr,
                                            &client_info->client_addr_len);

        copy_argv(client_info, argc, argv);

        if (client_info->client_socket == -1) {
            perror("accept");
            free(client_info);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_info) != 0) {
            perror("pthread_create");
            close(client_info->client_socket);
            free(client_info);
            continue;
        }

        pthread_detach(thread);
    }

    close(socfd);

    return 0;
}

void copy_argv(client_info_t *client_info, int argc, char *argv[]) {
    client_info->argv = malloc(argc * sizeof(char *));
    if (client_info->argv == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < argc; ++i) {
        client_info->argv[i] = strdup(argv[i]);
        if (client_info->argv[i] == NULL) {
            perror("Failed to duplicate string");
            exit(EXIT_FAILURE);
        }
    }
}

void get_client_info_str(client_info_t *client_info, char *client_info_str, size_t len) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_info->client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_info->client_addr.sin_port);
    snprintf(client_info_str, len, "%s:%d", client_ip, client_port);
}

void *handle_client(void *arg) {
    client_info_t *client_info = (client_info_t *) arg;

    strncpy(curr, ".", 2);

    const char *dir_name = client_info->argv[2];
    DIR *d = opendir(dir_name);
    if (d == NULL) {
        printf("The current directory could not be opened.");
        exit(EXIT_FAILURE);
    }

    getcwd(curr, MAX_LEN);

    print_dir(d);

    const char *filename = client_info->argv[3];
    char *serverinfo = NULL;

    size_t file_size = 0;
    read_file(filename, &serverinfo, &file_size);
    write(client_info->client_socket, serverinfo, file_size);
    bool is_new_dir = false;

    const time_t timer = time(NULL);
    struct tm *tp = localtime(&timer);
    char str_date[40];
    strftime(str_date, 40, "%d.%m.%Y %H:%M:%S", tp);

    char client_info_str[INET_ADDRSTRLEN + 6];
    get_client_info_str(client_info, client_info_str, sizeof(client_info_str));

    printf("%s - Client connected: %s\n", str_date, client_info_str);

    bool IS_RUN = true;
    while (IS_RUN) {
        ssize_t bytes_read = recv(client_info->client_socket, request, MAX_LEN, 0);
        size_t size_query = strlen(request);
        if (bytes_read > 0) {
            tp = localtime(&timer);
            strftime(str_date, 40, "%d.%m.%Y %H:%M:%S", tp);
            get_client_info_str(client_info, client_info_str, sizeof(client_info_str));

            if (strncasecmp(request, "ECHO", strlen("ECHO")) == 0) {
                printf("%s - %s : %s\n", client_info_str, str_date, "ECHO");
                write(client_info->client_socket, request + strlen("ECHO") + 1, size_query);
            } else if (strncasecmp(request, "INFO", strlen("INFO")) == 0) {
                printf("%s - %s : %s\n", client_info_str, str_date, "INFO");
                write(client_info->client_socket, serverinfo, file_size);
            } else if (strncasecmp(request, "CD", strlen("CD")) == 0) {
                printf("%s - %s : %s\n", client_info_str, str_date, "CD");
                request[size_query - 1] = '\0';
                if ((d = opendir(request + 3)) == NULL) {
                    write(client_info->client_socket, "", 1);
                } else {
                    strncpy(curr, request + 3, size_query);
                    chdir(curr);
                    getcwd(curr, MAX_LEN);
                    d = opendir(curr);
                    request[size_query - 1] = '\n';
                    write(client_info->client_socket, request + 3, size_query - 3);
                    is_new_dir = true;
                }
            } else if (strncasecmp(request, "LIST", strlen("LIST")) == 0) {
                printf("%s - %s : %s\n", client_info_str, str_date, "LIST");
                if (is_new_dir == true) {
                    print_dir(d);
                    is_new_dir = false;
                }
                write(client_info->client_socket, listDir, strlen(listDir));
            } else if (strncasecmp(request, "QUIT", strlen("QUIT")) == 0) {
                printf("%s - %s : %s\n", client_info_str, str_date, "QUIT");
                IS_RUN = false;
            } else {
                printf("%s - %s : %s\n", client_info_str, str_date, "UNKNOWN COMMAND");
                write(client_info->client_socket, "Unknown command!\n", strlen("Unknown command!\n"));
            }
        } else if (bytes_read == 0) {
            printf("The client closed the connection.\n");
            break;
        } else {
            fprintf(stderr, "Error to read data.\n");
        }
    }

    tp = localtime(&timer);
    strftime(str_date, 40, "%d.%m.%Y %H:%M:%S", tp);
    printf("%s - Client disconnected: %s\n", str_date, client_info_str);

    free(serverinfo);
    close(client_info->client_socket);
    free(client_info);
    pthread_exit(NULL);
}

void print_dir(DIR *dir) {
    struct dirent *d = NULL;
    struct stat sb;
    memset(listDir, '\0', strlen(listDir));

    strncat(listDir, curr, strlen(curr));
    strncat(listDir, "\n", 2);
    while ((d = readdir(dir))) {
        char fullpath[MAX_LEN * 3];

        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
            continue;
        }
        snprintf(fullpath, sizeof(fullpath), "%s/%s", curr, d->d_name);

        if (lstat(fullpath, &sb) == -1) {
            continue;
        }
        if ((sb.st_mode & __S_IFMT) == __S_IFDIR) {
            strncat(listDir, d->d_name, strlen(d->d_name));
            strncat(listDir, "/", 2);
        } else if ((sb.st_mode & __S_IFMT) == __S_IFREG) {
            strncat(listDir, d->d_name, strlen(d->d_name));
        } else if ((sb.st_mode & __S_IFMT) == __S_IFLNK) {
            char target_path[MAX_LEN];
            ssize_t bytes = readlink(fullpath, target_path, sizeof(target_path) - 1);
            if (bytes != -1) {
                lstat(target_path, &sb);
                if ((sb.st_mode & __S_IFMT) == __S_IFDIR) {
                    strncat(listDir, d->d_name, strlen(d->d_name) + 1);
                    strncat(listDir, " --> ", strlen(" --> ") + 1);
                    strncat(listDir, target_path, strlen(target_path) + 1);
                } else if ((sb.st_mode & __S_IFMT) == __S_IFDIR) {
                    strncat(listDir, d->d_name, strlen(d->d_name) + 1);
                    strncat(listDir, " -->> ", strlen(" -->> ") + 1);
                    strncat(listDir, target_path, strlen(target_path) + 1);
                }
            }
        }
        strncat(listDir, "\n", 2);
    }
    closedir(dir);
}

// Read 'filename' file
void read_file(const char *filename, char **buffer, size_t *size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open file.\n");
        return;
    }

    if (buffer == NULL) {
        fclose(file);
        return;
    }

    // Get size of file allocate memory and read file
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *buffer = (char *) realloc(*buffer, *size * sizeof(char));
    fread(*buffer, sizeof(char), *size, file);
    fclose(file);
}
