#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("Child process: pid=%d, ppid=%d\n", getpid(), getppid());

    
    printf("%s : %s\n", argv[1], argv[0]);    

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen failed");
        exit(1);
    }

    char var_name[50];
    while (fscanf(file, "%s", var_name) != EOF) {
        char *var_value = getenv(var_name);
        if (var_value) {
            printf("%s: %s\n", var_name, var_value);
        } else {
            //printf("%s not found in environment.\n", var_name);
            continue;
        }
    }

    fclose(file);
    return 0;
}
