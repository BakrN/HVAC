#include <stdio.h>
#include <stdlib.h>

#define ISVALID_SBUFFER(buffer) (                                         \
    {                                                                     \
        if (!buffer)                                                      \
        {                                                                 \
            fprintf(stderr, "Passed buffer wasn't created \n ");          \
            fflush(stderr);                                               \
            DEBUG_PRINT();                                                \
            exit(EXIT_FAILURE);                                           \
        }                                                                 \
        elseif(buffer->terminate_threads != 0)                            \
        {                                                                 \
            fprintf(stderr, "Tring to listen to terminated buffer  \n "); \
            fflush(stderr);                                               \
            DEBUG_PRINT();                                                \
            exit(EXIT_FAILURE);                                           \
        }                                                                 \
    })

#define FILE_HANDLER(x) (                                 \
    {                                                     \
        if (!x)                                           \
        {                                                 \
            \ 
        fprintf(stderr, "Failed to open/close file \n "); \
            fflush(stderr);                               \
            DEBUG_PRINT();                                \
            exit(EXIT_FAILURE);                           \
        }                                                 \
    })

//#define 
