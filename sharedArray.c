#include "sharedArray.h"

int initilize(struct SharedArray * sharedarr, int req_count)
{
    sharedarr->counter = 0;
    sharedarr->exit_count = 0;
    sharedarr->req_size = req_count;

    sem_init(&(sharedarr->space_avail),0, 10);
    sem_init(&(sharedarr->items_avail),0, 0);
    pthread_mutex_init(&(sharedarr->arr_mutex), 0);
    pthread_mutex_init(&(sharedarr->exit_count_mutex),0); 

    return 0;
}

int readToSharedArr(struct SharedArray * sharedarr, char *copy, int count)
{
    sem_wait(&(sharedarr->items_avail));
    pthread_mutex_lock(&(sharedarr->arr_mutex));

    if (sharedarr->counter == 0) //leave function on end condition, if a thread is stuck waiting on an empty array 
    {
        pthread_mutex_unlock(&(sharedarr->arr_mutex));
        fprintf(stdout, "thread %lu resolved %d hostnames\n", pthread_self(), count);
        fflush(stdout);
        sem_post(&(sharedarr->items_avail));
        pthread_exit(0);
    }

    memset(copy, '\0', MAX_NAME_LENGTH);
    sharedarr->counter = sharedarr->counter - 1;
    strncpy(copy, sharedarr->arr[sharedarr->counter], MAX_NAME_LENGTH);

    pthread_mutex_unlock(&(sharedarr->arr_mutex));
    sem_post(&(sharedarr->space_avail));

    return 0;
}

int writeToSharedArr(struct SharedArray * sharedarr, char *value) 
{
    sem_wait(&(sharedarr->space_avail));
    pthread_mutex_lock(&(sharedarr->arr_mutex));

    strncpy(sharedarr->arr[sharedarr->counter], value, MAX_NAME_LENGTH);
    sharedarr->counter = sharedarr->counter + 1;

    pthread_mutex_unlock(&(sharedarr->arr_mutex));
    sem_post(&(sharedarr->items_avail)); 

    return 0;
}

int destroy(struct SharedArray * sharedarr)
{
    sem_destroy(&(sharedarr->space_avail));
    sem_destroy(&(sharedarr->items_avail));
    pthread_mutex_destroy(&(sharedarr->arr_mutex));
    pthread_mutex_destroy(&(sharedarr->exit_count_mutex));

    return 0;
}