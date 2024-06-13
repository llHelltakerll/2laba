#include <stdbool.h>
#include "func.h"

void menu(void) {
    record_t current_record;
    int record_index = -1;
    char ch;

    bool IN_PROGRESS = true;
    do {
        printf("##########################\n"
               "Select an action:\n"
               "l - LST\n"
               "g - GET\n"
               "p - PUT\n"
               "q - QUIT\n"
               "##########################\n");
        printf("Command: ");

        fflush(stdin);
        ch = getchar();
        switch (ch) {
            case 'l':
                for (int i = 0; i < MAX_RECORDS; i++) {
                    print_record(i);
                }
                break;
            case 'g':
                printf("Enter the record number: ");
                scanf("%d", &record_index);
                print_record(record_index);
                break;
            case 'p':
                if (record_index == -1) {
                    printf("Get the record with the GET option before using the PUT option.\n");
                    break;
                }
                record_t new_record;

                get_record(record_index, &current_record);

                printf("Enter the new name for the record: ");
                scanf("%s", new_record.name);

                printf("Enter the new address for the record: ");
                scanf("%s", new_record.address);

                printf("Enter the new semester for the record: ");
                scanf("%d", &new_record.semester);

                save_record(&current_record, &new_record, record_index);
                break;
            case 'q':
                IN_PROGRESS = false;
                break;
            default:
                printf("Wrong command\n");
                fflush(stdin);
                break;

        }
        getchar();
    } while (IN_PROGRESS);
}

void print_record(int record_index) {
    record_t record;

    off_t offset = record_index * sizeof(record);
    lseek(fd, offset, SEEK_SET);

    read(fd, &record, sizeof(record));
    printf("############\n"
           "Record    %d:\n"
           "Name:     %s;\n"
           "Address:  %s;\n"
           "Semester: %d.\n"
           "############\n\n", record_index, record.name, record.address, record.semester);
}

void get_record(int record_index, record_t *record) {
    off_t offset = record_index * sizeof(*record);
    lseek(fd, offset, SEEK_SET);

    read(fd, record, sizeof(*record));
}

void modify_record(int record_index, record_t *record) {
    off_t offset = record_index * sizeof(*record);
    lseek(fd, offset, SEEK_SET);

    write(fd, record, sizeof(*record));
}

void save_record(record_t *record, record_t *new_record, int record_index) {
    if (record_index < 0 || record_index >= MAX_RECORDS) {
        printf("Invalid record_t number.\n");
        return;
    }

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = record_index * sizeof(*record);
    fl.l_len = sizeof(*record);

    while (fcntl(fd, F_SETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) { // обе ошибки, согласно POSIX
            // Получить блокировку в данный момент нет возможности
            printf("Record %d is locked.\n", record_index);
        } else {
            exit(EXIT_FAILURE);
        }
    }

    record_t current_record;
    lseek(fd, record_index * sizeof(current_record), SEEK_SET);
    if (read(fd, &current_record, sizeof(current_record)) == -1) {
        printf("Failed to read record.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(current_record.name, record->name) != 0 || strcmp(current_record.address, record->address) != 0 ||
        current_record.semester != record->semester) {
        printf("Record %d has been modified by another process. Trying again.\n", record_index);
        get_record(record_index, record);

        fl.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &fl);

        save_record(record, new_record, record_index);
    } else {
        modify_record(record_index, new_record);

        fl.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &fl);
    }
}

void create_file(const char *filename) {
    struct record arrayRecords[10];

    for (int i = 0; i < 10; i++) {
        sprintf(arrayRecords[i].name, "Name%d", i);
        sprintf(arrayRecords[i].address, "Address%d", i);
        arrayRecords[i].semester = i;
    }

    FILE *file;
    if ((file = fopen(filename, "r")) != NULL) {
        printf("Failed to open. File already exists.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    file = fopen(filename, "w");
    if (file == NULL) {
        printf("Failed to open file.\n");
        exit(EXIT_FAILURE);
    }

    fwrite(arrayRecords, sizeof(arrayRecords), 1, file);
    fclose(file);
}
