#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"


/*----------------------------------------------------------
 * Structure for decoding information
 *---------------------------------------------------------*/
typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Magic String info */
    char magic[10];

    /* Secret File info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    long size_secret_file;

} DecodeInfo;


/*----------------------------------------------------------
 * Function Prototypes
 *---------------------------------------------------------*/

/* Read and validate decode arguments */
/* Read and validate Decode args from argv */

 OperationType check_operation_type(char *argv[]);

Status read_and_validate_decode_args(int argc,
                                     char *argv[],
                                     DecodeInfo *decInfo);


/* Open files */

Status open_decode_files(DecodeInfo *decInfo);


/* Decode one byte from LSB */

Status decode_byte_from_lsb(char *arr,
                            char *data);


/* Decode integer from LSB */

Status decode_size_from_lsb(char *arr,
                            int *data);


/* Decode data from image */

Status decode_data_from_image(char *data,
                              int size,
                              FILE *fptr_stego_image);


/* Decode magic string */

Status decode_magic_string(DecodeInfo *decInfo);


/* Decode secret file extension */

Status decode_secret_file_extn(DecodeInfo *decInfo);


/* Decode secret file size */

Status decode_secret_file_size(DecodeInfo *decInfo);


/* Decode secret file data */

Status decode_secret_file_data(DecodeInfo *decInfo);


/* Perform decoding */

Status do_decoding(DecodeInfo *decInfo);


#endif