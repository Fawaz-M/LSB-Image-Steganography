#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"


/*----------------------------------------------------------
 * Read and validate decode arguments
 *---------------------------------------------------------*/
Status read_and_validate_decode_args(int argc,
                                     char *argv[],
                                     DecodeInfo *decInfo)
{
    if (argc != 3 && argc != 4)
    {
        printf("Error: Invalid number of arguments\n");
        printf("Format: ./a.out -d stego.bmp [output.txt]\n");
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") != NULL)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        printf("Error: Stego image must be a .bmp file\n");
        return e_failure;
    }

    /* If output file is given */
    if (argc == 4)
    {
        if (strstr(argv[3], ".txt") != NULL)
        {
            decInfo->secret_fname = argv[3];
        }
        else
        {
            printf("Error: Output file must be a .txt file\n");
            return e_failure;
        }
    }
    else
    {
        /* If output file is not given, use default file */
        decInfo->secret_fname = "decoded.txt";
    }

    return e_success;
}


/*----------------------------------------------------------
 * Open stego image
 *---------------------------------------------------------*/
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image =
        fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        printf("ERROR: Unable to open stego image file %s\n",
               decInfo->stego_image_fname);

        return e_failure;
    }

    return e_success;
}


/*----------------------------------------------------------
 * Decode one byte from LSB
 *---------------------------------------------------------*/
Status decode_byte_from_lsb(char *arr,
                            char *data)
{
    int i;
    char ch = 0;

    for (i = 0; i < 8; i++)
    {
        ch = ch << 1;
        ch = ch | (arr[i] & 1);
    }

    *data = ch;

    return e_success;
}


/*----------------------------------------------------------
 * Decode integer from LSB
 *---------------------------------------------------------*/
Status decode_size_from_lsb(char *arr,
                            int *data)
{
    int i;
    int size = 0;

    for (i = 0; i < 32; i++)
    {
        size = size << 1;
        size = size | (arr[i] & 1);
    }

    *data = size;

    return e_success;
}


/*----------------------------------------------------------
 * Decode data from image
 *---------------------------------------------------------*/
Status decode_data_from_image(char *str,
                              int size,
                              FILE *fptr_stego_image)
{
    int i;
    char arr[8];

    for (i = 0; i < size; i++)
    {
        if (fread(arr, 8, 1,
                  fptr_stego_image) != 1)
        {
            return e_failure;
        }

        if (decode_byte_from_lsb(arr,
                                 &str[i]) == e_failure)
        {
            return e_failure;
        }
    }

    str[size] = '\0';

    return e_success;
}


/*----------------------------------------------------------
 * Decode magic string
 *---------------------------------------------------------*/
Status decode_magic_string(DecodeInfo *decInfo)
{
    int magic_size;
    char arr[32];

    /* Read magic string size */
    if (fread(arr, 32, 1,
              decInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }

    /* Decode magic string size */
    if (decode_size_from_lsb(arr,
                             &magic_size) == e_failure)
    {
        return e_failure;
    }

    /* Validate magic size */
    if (magic_size <= 0 ||
        magic_size >= sizeof(decInfo->magic))
    {
        printf("ERROR: Invalid magic string size\n");
        return e_failure;
    }

    /* Decode magic string */
    if (decode_data_from_image(decInfo->magic,
                               magic_size,
                               decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    return e_success;
}


/*----------------------------------------------------------
 * Decode secret file extension
 *---------------------------------------------------------*/
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    int extn_size;
    char arr[32];

    /* Read extension size */
    if (fread(arr, 32, 1,
              decInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }

    /* Decode extension size */
    if (decode_size_from_lsb(arr,
                             &extn_size) == e_failure)
    {
        return e_failure;
    }

    /* Validate extension size */
    if (extn_size <= 0 ||
        extn_size >= MAX_FILE_SUFFIX)
    {
        printf("ERROR: Invalid file extension size\n");
        return e_failure;
    }

    /* Decode extension */
    if (decode_data_from_image(decInfo->extn_secret_file,
                               extn_size,
                               decInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    return e_success;
}


/*----------------------------------------------------------
 * Decode secret file size
 *---------------------------------------------------------*/
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    int i;
    unsigned char *ptr;
    char arr[8];

    ptr = (unsigned char *)&decInfo->size_secret_file;

    for (i = 0; i < sizeof(long); i++)
    {
        /* Read 8 image bytes */
        if (fread(arr, 8, 1,
                  decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }

        /* Decode one byte */
        if (decode_byte_from_lsb(arr,
                                 (char *)&ptr[i]) == e_failure)
        {
            return e_failure;
        }
    }

    return e_success;
}


/*----------------------------------------------------------
 * Decode secret file data
 *---------------------------------------------------------*/
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    long i;
    char ch;
    char arr[8];

    for (i = 0;
         i < decInfo->size_secret_file;
         i++)
    {
        /* Read 8 image bytes */
        if (fread(arr, 8, 1,
                  decInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }

        /* Decode one character */
        if (decode_byte_from_lsb(arr,
                                 &ch) == e_failure)
        {
            return e_failure;
        }

        /* Write character to output file */
        if (fwrite(&ch, 1, 1,
                   decInfo->fptr_secret) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}


/*----------------------------------------------------------
 * Perform decoding
 *---------------------------------------------------------*/
Status do_decoding(DecodeInfo *decInfo)
{
    char input_magic[10];

    /* Open stego image */
    if (open_decode_files(decInfo) == e_failure)
    {
        printf("Error: Failed to open stego image\n");
        return e_failure;
    }

    /* Skip BMP header */
    if (fseek(decInfo->fptr_stego_image,
              54,
              SEEK_SET) != 0)
    {
        printf("ERROR: Failed to skip BMP header\n");

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /*------------------------------------------------------
     * Decode magic string
     *-----------------------------------------------------*/
    if (decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode magic string\n");

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /*------------------------------------------------------
     * Get magic string from user
     *-----------------------------------------------------*/
    printf("Enter magic string: ");

    if (scanf("%9s", input_magic) != 1)
    {
        printf("ERROR: Failed to read magic string\n");

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /*------------------------------------------------------
     * Compare magic strings
     *-----------------------------------------------------*/
    if (strcmp(input_magic, decInfo->magic) != 0)
    {
        printf("ERROR: Magic string mismatch\n");

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /*------------------------------------------------------
     * Decode secret file extension
     *-----------------------------------------------------*/
    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file extension\n");

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /*------------------------------------------------------
     * Open output file from CLA
     *-----------------------------------------------------*/
    decInfo->fptr_secret =
        fopen(decInfo->secret_fname, "wb");

    if (decInfo->fptr_secret == NULL)
    {
        printf("ERROR: Unable to create output file %s\n",
               decInfo->secret_fname);

        fclose(decInfo->fptr_stego_image);

        return e_failure;
    }


    /*------------------------------------------------------
     * Decode secret file size
     *-----------------------------------------------------*/
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file size\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }


    /*------------------------------------------------------
     * Decode secret file data
     *-----------------------------------------------------*/
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file\n");

        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_secret);

        return e_failure;
    }


    /*------------------------------------------------------
     * Close files
     *-----------------------------------------------------*/
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);


    printf("INFO: Decoding successful\n");
    printf("INFO: Output file: %s\n",
           decInfo->secret_fname);

    return e_success;
}