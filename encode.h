#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include "types.h"


/*----------------------------------------------------------
 * Structure for encoding information
 *---------------------------------------------------------*/
typedef struct _EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;
    char magic[10];

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    long size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;


/*----------------------------------------------------------
 * Function Prototypes
 *---------------------------------------------------------*/
 OperationType check_operation_type(char *argv[]);

/* Read and validate encode arguments */
/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(int argc,
                                     char *argv[],
                                     EncodeInfo *encInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Open files */
Status open_files(EncodeInfo *encInfo);

/* Check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Copy BMP header */
Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_stego_image);

/* Encode one byte into LSB */
void encode_byte_to_lsb(char data,
                        char *arr);

/* Encode integer into LSB */
void encode_size_to_lsb(int data,
                        char *arr);

/* Encode string into image */
Status encode_data_to_image(char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image);

/* Encode magic string */
Status encode_magic_string(char *magic_string,
                           EncodeInfo *encInfo);

/* Encode secret file extension */
Status encode_secret_file_extn(char *file_extn,
                               EncodeInfo *encInfo);

/* Encode secret file size */
Status encode_secret_file_size(long file_size,
                               EncodeInfo *encInfo);

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo);

/* Copy remaining image data */
Status copy_remaining_data(FILE *fptr_src_image,
                           FILE *fptr_stego_image);

/* Perform encoding */
Status do_encoding(EncodeInfo *encInfo);

#endif