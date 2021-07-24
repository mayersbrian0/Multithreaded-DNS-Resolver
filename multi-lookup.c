#include "multi-lookup.h"

#define ERROR1(S) { fprintf(stderr, "%s error %s\n", argv[0], S); exit(-1);}
#define ERROR2(S) { fprintf(stdout, "%s error %s\n", argv[0], S); exit(-1);}

struct ThreadArgs {
    struct inputFileArray in_file;
    struct SharedArray sa;
};

void *requester(void *ptr)
{
    
    FILE *fp, *log_fp; //values to read line form the file 
    char *filename;
    char *line = NULL;
    size_t len;
    ssize_t lineVal;
    int file_count = 0;
    char name[MAX_NAME_LENGTH];

    struct ThreadArgs* value = ( struct ThreadArgs *)ptr;
    while (1)
    {
        /*
            Begin of Critical Section for choosing files
        */
        pthread_mutex_lock(&(value->in_file.mutex_IA)); //acquire mutex
        if (value->in_file.file_loc == value->in_file.size) //Condition wherer requester thread finishes 
        {
            pthread_mutex_unlock(&(value->in_file.mutex_IA));
            
            pthread_mutex_lock(&(value->sa.exit_count_mutex));
            fprintf(stdout, "thread %lu serviced %d files\n", pthread_self(), file_count); //print out results 
            fflush(stdout);
            value->sa.exit_count = value->sa.exit_count + 1; //increment exit count
            pthread_mutex_unlock(&(value->sa.exit_count_mutex));
            if (line) //deallocate the line after use 
            {
                free(line);
            }

            pthread_exit(0);
        }

        filename = value->in_file.input_file[value->in_file.file_loc];
        //printf("Thread: %u has file %s\n", pthread_self(), filename);
        value->in_file.file_loc = value->in_file.file_loc + 1;

        pthread_mutex_unlock(&(value->in_file.mutex_IA)); //release mutex
        /*
            End of Critical Section for choosing files
        */
        
        fp = fopen(filename, "r");
        while ( (lineVal = getline(&line,&len, fp)) != -1) 
        {
            if (line[lineVal -1] == '\n')
            {
                line[lineVal -1] = '\0';
            }
            writeToSharedArr(&(value->sa), line);//add value to shared array

            // Start Critical Section of adding to log file 
            
            log_fp = fopen(value->in_file.log_file, "a");

            pthread_mutex_lock(&(value->in_file.log_mutex));
            fprintf(log_fp, "%s\n", line);
            pthread_mutex_unlock(&(value->in_file.log_mutex));

            fclose(log_fp);
            

            //End of Critical Section of adding to log file
        }
        file_count = file_count + 1;
        fclose(fp);
        
    }

}

void *resolver(void *ptr) 
{
    FILE *log_fp;
    int count = 0;
    char name[MAX_NAME_LENGTH];
    char IPstring[MAX_IP_LENGTH] = "NOT";

    struct ThreadArgs* value = ( struct ThreadArgs *)ptr;

    while (1)
    {
        /*
        Exit Condition
        */
        pthread_mutex_lock(&(value->sa.exit_count_mutex));
        if (value->sa.exit_count == value->sa.req_size) //if all the resolver threads have left
        {
            pthread_mutex_unlock(&(value->sa.exit_count_mutex));

            pthread_mutex_lock(&(value->sa.arr_mutex));
            if (value->sa.counter == 0)
            {
                pthread_mutex_unlock(&(value->sa.arr_mutex)); 
                fprintf(stdout, "thread %lu resolved %d hostnames\n", pthread_self(), count);
                fflush(stdout);
                sem_post(&(value->sa.items_avail)); //wake up any threads trapped waiting on value
                pthread_exit(0);
            }
            pthread_mutex_unlock(&(value->sa.arr_mutex));
        }
        pthread_mutex_unlock(&(value->sa.exit_count_mutex));
        /*
        Exit Condition
        */

        readToSharedArr(&(value->sa), name,count); //end condition is also in this function
        
        if (dnslookup(name, IPstring, 46) == UTIL_FAILURE) {strncpy(IPstring,"NOT RESOLVED",MAX_IP_LENGTH);}
        else {count = count + 1;} //add to number of strings that were resolved
        
        log_fp= fopen(value->in_file.res_log_file, "a"); //add to the results file
        pthread_mutex_lock(&(value->in_file.res_log_mutex));
        fprintf(log_fp, "%s, %s\n", name, IPstring);
        pthread_mutex_unlock(&(value->in_file.res_log_mutex));
        fclose(log_fp);
        
    } 
}


/*
Entry point to the program
*/
int main(int argc, char **argv)
{
    
    int file;
    char *ptr;
    int req_count, res_count, valid_count = 0;
    int valid_files[argc - 5]; //stores valid file indexs 
    struct ThreadArgs pass_vals;
    struct timeval start,end;
    gettimeofday(&start,NULL);

    //Error Checking on input 
    if (argc < 6) ERROR2("Incorrect Number of Arguments");
    req_count = strtol(argv[1], &ptr, 10);
    if (*ptr != '\0') ERROR1("Req Count must be a number");
    res_count = strtol(argv[2],&ptr,10);
    if (*ptr != '\0') ERROR1("Res Count must be a number");    
    if (-1 == (file = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC,0666))) ERROR1("Request Log File Not Valid");
    close(file);
    if (-1 == (file = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC,0666))) ERROR1("Resolve Log File Not Valid");
    close(file);
    if (req_count <= 0 || res_count <= 0) ERROR1("Need Number of threads between 1-10");
    if (req_count > (int)MAX_REQUESTER || res_count > (int)MAX_RESOLVER) ERROR1("Need Number of threads between 1-10");
    close(file);
    if ((argc - 5) > (int)MAX_INPUT_FILES) ERROR1("To Many Input Files");
    
    pthread_t req_main[req_count], res_main[res_count]; //thread pool

    for (int i = 5; i < argc; i = i +1) //checking for valid files
    {
        if (-1 == (file = open(argv[i], O_RDONLY,0666))) fprintf(stderr, "Invalid Input File %s\n", argv[i]); //check for invalid file names 
        else {valid_count = valid_count + 1; valid_files[valid_count -1] = i;}//count the number of valid files and store the index in argv
        close(file);
    }

    //allocate space for the files
    if ( valid_count == 0 || (create_input_file_array(&(pass_vals.in_file), argv, valid_files, valid_count) == -1)) ERROR1("Error Allocating Input Files");
    initilize(&(pass_vals.sa), req_count); //pass the shared array structure to be initalized and req_count whcih helps for end condition
    
    for (int i = 0; i < req_count; i = i +1)
    {
        pthread_create(&req_main[i], NULL, requester, &pass_vals);
    }

    for (int i = 0; i < res_count; i = i+1)
    {
        pthread_create(&res_main[i], NULL, resolver, &pass_vals);
    }
    
    //Join all the threads
    for (int i = 0; i < req_count; i = i +1)
    {
        pthread_join(req_main[i], NULL);
    }
    
    for (int i = 0; i < res_count; i = i +1)
    {
        pthread_join(res_main[i], NULL);
    }
    
    destroy(&(pass_vals.sa)); //destorys the shared array 
    delete_input_file_array(&(pass_vals.in_file), valid_count); //deletes the input file array 
    
    gettimeofday(&end,NULL);

    fprintf(stdout,"%s: total time is %.5f seconds\n", argv[0], (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)*1e-6);
    return 0;
}