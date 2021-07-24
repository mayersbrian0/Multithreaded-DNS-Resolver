#ifndef SHARED_ARRAY_H
#define SHARED_ARRAY_H

#include "multi-lookup.h"
#define MAX_NAME_LENGTH 255
#define ARRAY_SIZE 10

struct SharedArray {
    int counter;
    char arr[ARRAY_SIZE][MAX_NAME_LENGTH];
    int exit_count;
    int req_size;
    pthread_mutex_t arr_mutex;
    pthread_mutex_t exit_count_mutex;
    sem_t space_avail;
    sem_t items_avail; 
};

int initilize(struct SharedArray * sharedarr, int req_count);
int readToSharedArr(struct SharedArray * sharedarr, char * copy, int count);
int writeToSharedArr(struct SharedArray * sharedarr, char* value);
int destroy(struct SharedArray * sharedarr);


#endif 