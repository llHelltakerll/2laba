#include "func.h"

int fd;
struct flock fl;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Wrong number of arguments\n");
        return -1;
    }

    FILE *file;
    if ((file = fopen(argv[1], "r")) == NULL) {
        create_file(argv[1]);
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        printf("Failed to open file.\n");
        exit(EXIT_FAILURE);
    }

    menu();

    close(fd);
    return 0;
}
