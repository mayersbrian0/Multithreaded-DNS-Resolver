#ifndef FILE_ARRAY_H
#define FILE_ARRAY_H

#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

struct inputFileArray {
    char **input_file;
    int file_loc;
    int size;
    char *log_file;
    char *res_log_file;
    pthread_mutex_t mutex_IA;
    pthread_mutex_t log_mutex;
    pthread_mutex_t res_log_mutex;
};

int create_input_file_array(struct inputFileArray *value, char **argv, int valid_files[], int valid_count);
int delete_input_file_array(struct inputFileArray* value, int valid_count);

#endif