#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <locale.h>

#define MAX_CHILDREN 100
#define nullptr (char *)NULL

extern char **environ;

int compareStrings(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

char *getEnvPath(char *envVar, char **envp)
{
    for (int i = 0; envp[i] != NULL; i++)
    {
        if (strncmp(envp[i], envVar, strlen(envVar)) == 0)
        {
            char *value = strchr(envp[i], '=');

            if (value != NULL)
            {
                return value + 1;
            }
            else
            {
                return NULL;
            }

            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[], char *envp[])
{
    if (argc < 2)
    {
        printf("File name missing!\n");
      //  return 1;
    }
    setlocale(LC_COLLATE, "C");

    int env_count = 0;
    while (environ[env_count] != NULL)
    {
        env_count++;
    }

    char **sorted_environ = malloc(sizeof(char *) * (env_count + 1));
    if (!sorted_environ)
    {
        perror("malloc failed");
        return 1;
    }

    for (int i = 0; i < env_count; i++)
    {
        sorted_environ[i] = environ[i];
    }
    sorted_environ[env_count] = NULL;

    qsort(sorted_environ, env_count, sizeof(char *), compareStrings);

    for (int i = 0; sorted_environ[i] != NULL; i++)
    {
        printf("%s\n", sorted_environ[i]);
    }

    free(sorted_environ);

    char *child_path;
    int child_counter = 0;
    while (1)
    {
        printf("Parent process: ");
        char input;
        scanf(" %c", &input);

        if (input == '+')
        {
            if (child_counter < MAX_CHILDREN)
            {

                child_path = getenv("CHILD_PATH");
                if (!child_path)
                {
                    fprintf(stderr, "CHILD_PATH not set.\n");
                    return 1;
                }

                pid_t pid = fork();
                if (pid == 0)
                {
                    char child_program[20];
                    snprintf(child_program, sizeof(child_program), "child_%02d", child_counter);

                    char child_path1[255];

                    snprintf(child_path1, sizeof(child_path1), "%s/child", child_path);

                    execl(child_path1, child_program, argv[1], nullptr);

                    perror("exec failed");
                    exit(1);
                }
                else if (pid > 0)
                {
                    child_counter++;
                    wait(NULL);
                }
                else
                {
                    perror("fork failed");
                    exit(1);
                }
            }
            else
            {
                printf("Reached the maximum number of children.\n");
            }
        }
        else if (input == '*')
        {
            if (child_counter < MAX_CHILDREN)
            {
                char *envVar = "CHILD_PATH";
                child_path = getEnvPath(envVar, envp);

                if (!child_path)
                {
                    fprintf(stderr, "CHILD_PATH not set.\n");
                    return 1;
                }

                pid_t pid = fork();
                if (pid == 0)
                {
                    char child_program[20];
                    snprintf(child_program, sizeof(child_program), "child_%02d", child_counter);

                    char child_path1[255];

                    snprintf(child_path1, sizeof(child_path1), "%s/child", child_path);
                    printf("%s\n", child_path1);

                    execl(child_path1, child_program, "env.txt", nullptr);

                    perror("exec failed");
                    exit(1);
                }
                else if (pid > 0)
                {
                    child_counter++;
                    wait(NULL);
                }
                else
                {
                    perror("fork failed");
                    exit(1);
                }
            }
            else
            {
                printf("Reached the maximum number of children.\n");
            }
        }
        else if (input == '&')
        {
            if (child_counter < MAX_CHILDREN)
            {
                char *envVar = "CHILD_PATH";
                child_path = getEnvPath(envVar, envp);

                char *child_path = getenv("CHILD_PATH");
                if (!child_path)
                {
                    fprintf(stderr, "CHILD_PATH not set.\n");
                    return 1;
                }

                pid_t pid = fork();
                if (pid == 0)
                {
                    char child_program[20];
                    snprintf(child_program, sizeof(child_program), "child_%02d", child_counter);

                    char child_path1[255];

                    snprintf(child_path1, sizeof(child_path1), "%s/child", child_path);
                    printf("%s\n", child_path1);

                    execl(child_path1, child_program, "env.txt", nullptr);

                    perror("exec failed");
                    exit(1);
                }
                else if (pid > 0)
                {
                    child_counter++;
                    wait(NULL);
                }
                else
                {
                    perror("fork failed");
                    exit(1);
                }
            }
            else
            {
                printf("Reached the maximum number of children.\n");
            }
        }
        else if (input == 'q')
        {
            break; // Exit the loop and terminate the program
        }
        else
        {
            printf("Invalid input. Try again.\n");
        }
    }

    return 0;
}
