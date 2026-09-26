#ifndef TYPES_H
#define TYPES_H

#define MAX_FILE_SUFFIX 10

typedef unsigned int uint;

typedef enum
{
    e_success,
    e_failure
} Status;

typedef enum
{
    e_encode,
    e_decode,
    e_unsupported
} OperationType;

#endif