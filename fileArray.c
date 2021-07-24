#include "fileArray.h"

int create_input_file_array(struct inputFileArray *value, char **argv, int valid_files[], int valid_count)
{
    value->file_loc = 0;
    value->size = valid_count;
    value->log_file = argv[3];
    value->res_log_file = argv[4];

    if (pthread_mutex_init(&(value->mutex_IA), NULL) == -1) return -1;
    if (pthread_mutex_init(&(value->log_mutex), NULL) == -1) return -1;
    if (pthread_mutex_init(&(value->res_log_mutex), NULL) == -1) return -1;
    
    if ( (value->input_file = malloc(sizeof(char *)* valid_count)) == NULL) return -1;
    for (int i =0; i < valid_count; i = i +1)
    {
        value->input_file[i] = argv[valid_files[i]];
    }
    

    return 0;
}

int delete_input_file_array(struct inputFileArray* value, int valid_count)
{
    pthread_mutex_destroy(&(value->mutex_IA));
    pthread_mutex_destroy(&(value->log_mutex));
    pthread_mutex_destroy(&(value->res_log_mutex));
    
    free(value->input_file);
    return 0;
}