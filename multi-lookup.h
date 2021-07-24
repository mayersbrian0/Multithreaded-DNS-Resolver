/*
File: multi-lookup.h
Author: Brian Mayers
*/

#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include "util.h"
#include "fileArray.h"
#include "sharedArray.h"

#define ARRAY_SIZE 10
#define MAX_INPUT_FILES 100
#define MAX_REQUESTER 10
#define MAX_RESOLVER 10
#define MAX_NAME_LENGTH 255
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

void *resolver(void *ptr);
void *requester(void *ptr);



#endif 