#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"


/* Read and validate Encode arguments from argv */

Status read_and_validate_encode_args(int argc,
                                     char *argv[],
                                     EncodeInfo *encInfo)
{
    if (argc < 4 || argc > 5)
    {
        printf("Error: Invalid number of arguments\n");
        printf("Format: ./a.out -e source.bmp secret.txt [stego.bmp]\n");
        return e_failure;
    }

    /* Validate source image */

    if (strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("Error: Source image must be a .bmp file\n");
        return e_failure;
    }


    /* Validate secret file */

    if (strstr(argv[3], ".txt") != NULL)
    {
        encInfo->secret_fname = argv[3];

        strcpy(encInfo->extn_secret_file, ".txt");
    }
    else
    {
        printf("Error: Secret file must be a .txt file\n");
        return e_failure;
    }


    /* Validate output file */

    if (argc == 4)
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    else
    {
        if (strstr(argv[4], ".bmp") != NULL)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            printf("Error: Stego image must be a .bmp file\n");
            return e_failure;
        }
    }

    return e_success;
}


/* Get image size */

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width;
    uint height;

    fseek(fptr_image, 18, SEEK_SET);

    fread(&width, sizeof(uint), 1, fptr_image);
    fread(&height, sizeof(uint), 1, fptr_image);

    rewind(fptr_image);

    return width * height * 3;
}


/* Get file size */

uint get_file_size(FILE *fptr)
{
    long size;

    fseek(fptr, 0, SEEK_END);

    size = ftell(fptr);

    rewind(fptr);

    return (uint)size;
}


/* Open files */

Status open_files(EncodeInfo *encInfo)
{
    /* Open source image */

    encInfo->fptr_src_image =
        fopen(encInfo->src_image_fname, "rb");

    if (encInfo->fptr_src_image == NULL)
    {
        printf("ERROR: Unable to open source image file %s\n",
               encInfo->src_image_fname);

        return e_failure;
    }


    /* Open secret file */

    encInfo->fptr_secret =
        fopen(encInfo->secret_fname, "rb");

    if (encInfo->fptr_secret == NULL)
    {
        printf("ERROR: Unable to open secret file %s\n",
               encInfo->secret_fname);

        fclose(encInfo->fptr_src_image);

        return e_failure;
    }


    /* Create stego image */

    encInfo->fptr_stego_image =
        fopen(encInfo->stego_image_fname, "wb");

    if (encInfo->fptr_stego_image == NULL)
    {
        printf("ERROR: Unable to create stego image file %s\n",
               encInfo->stego_image_fname);

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);

        return e_failure;
    }

    return e_success;
}


/* Check capacity */

Status check_capacity(EncodeInfo *encInfo)
{
    long secret_file_size;
    long size_of_info;
    long required_bits;

    printf("Enter magic string: ");

    if (scanf("%9s", encInfo->magic) != 1)
    {
        printf("ERROR: Failed to read magic string\n");
        return e_failure;
    }


    /* Get secret file size */

    if (fseek(encInfo->fptr_secret, 0, SEEK_END) != 0)
    {
        printf("ERROR: Unable to seek secret file\n");
        return e_failure;
    }

    secret_file_size = ftell(encInfo->fptr_secret);

    if (secret_file_size < 0)
    {
        printf("ERROR: Unable to get secret file size\n");
        return e_failure;
    }

    encInfo->size_secret_file = secret_file_size;

    rewind(encInfo->fptr_secret);


    /* Get image capacity */

    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);


    /*
       Required information:

       Magic string
       Magic string size
       Secret file extension
       Extension size
       Secret file size
       Secret file data
    */

    size_of_info =
        strlen(encInfo->magic)
        + sizeof(int)
        + strlen(encInfo->extn_secret_file)
        + sizeof(int)
        + sizeof(long)
        + secret_file_size;


    required_bits = size_of_info * 8;


    if (required_bits <= encInfo->image_capacity)
    {
        return e_success;
    }

    return e_failure;
}


/* Copy BMP header */

Status copy_bmp_header(FILE *fptr_src_image,
                       FILE *fptr_stego_image)
{
    char header[54];

    rewind(fptr_src_image);

    if (fread(header, 1, 54, fptr_src_image) != 54)
    {
        printf("ERROR: Unable to read BMP header\n");
        return e_failure;
    }

    if (fwrite(header, 1, 54, fptr_stego_image) != 54)
    {
        printf("ERROR: Unable to write BMP header\n");
        return e_failure;
    }

    return e_success;
}


/* Encode one byte into LSB */

void encode_byte_to_lsb(char data,
                        char *arr)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        /* Clear LSB */

        arr[i] = arr[i] & (~1);

        /* Set LSB */

        arr[i] =
            arr[i] |
            ((data >> (7 - i)) & 1);
    }
}


/* Encode integer into LSB */

void encode_size_to_lsb(int data,
                        char *arr)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        /* Clear LSB */

        arr[i] = arr[i] & (~1);

        /* Set LSB */

        arr[i] =
            arr[i] |
            ((data >> (31 - i)) & 1);
    }
}


/* Encode data into image */

Status encode_data_to_image(char *data,
                            int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    int i;

    for (i = 0; i < size; i++)
    {
        char arr[8];


        /* Read 8 bytes from source image */

        if (fread(arr, 8, 1,
                  fptr_src_image) != 1)
        {
            return e_failure;
        }


        /* Encode one character */

        encode_byte_to_lsb(data[i], arr);


        /* Write modified 8 bytes */

        if (fwrite(arr, 8, 1,
                   fptr_stego_image) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}


/* Encode magic string */

Status encode_magic_string(char *magic_string,
                           EncodeInfo *encInfo)
{
    int magic_size;
    char arr[32];


    /* Get magic string size */

    magic_size = strlen(magic_string);


    /* Read 32 bytes from image */

    if (fread(arr, 32, 1,
              encInfo->fptr_src_image) != 1)
    {
        return e_failure;
    }


    /* Encode magic string size */

    encode_size_to_lsb(magic_size, arr);


    /* Write modified image bytes */

    if (fwrite(arr, 32, 1,
               encInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }


    /* Encode actual magic string */

    if (encode_data_to_image(magic_string,
                             magic_size,
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    return e_success;
}


/* Encode secret file extension */

Status encode_secret_file_extn(char *file_extn,
                               EncodeInfo *encInfo)
{
    int extn_size;
    char arr[32];


    /* Get extension size */

    extn_size = strlen(file_extn);


    /* Read 32 bytes from image */

    if (fread(arr, 32, 1,
              encInfo->fptr_src_image) != 1)
    {
        return e_failure;
    }


    /* Encode extension size */

    encode_size_to_lsb(extn_size, arr);


    /* Write modified bytes */

    if (fwrite(arr, 32, 1,
               encInfo->fptr_stego_image) != 1)
    {
        return e_failure;
    }


    /* Encode extension */

    if (encode_data_to_image(file_extn,
                             extn_size,
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }

    return e_success;
}


/* Encode secret file size */

Status encode_secret_file_size(long file_size,
                               EncodeInfo *encInfo)
{
    int i;

    unsigned char *ptr;

    char arr[8];


    ptr = (unsigned char *)&file_size;


    /*
       Encode each byte of long value
    */

    for (i = 0; i < sizeof(long); i++)
    {
        /* Read 8 image bytes */

        if (fread(arr, 8, 1,
                  encInfo->fptr_src_image) != 1)
        {
            return e_failure;
        }


        /* Encode byte */

        encode_byte_to_lsb(ptr[i], arr);


        /* Write modified bytes */

        if (fwrite(arr, 8, 1,
                   encInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}


/* Encode secret file data */

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    char arr[8];


    rewind(encInfo->fptr_secret);


    /* Read secret file character by character */

    while (fread(&ch, 1, 1,
                 encInfo->fptr_secret) == 1)
    {
        /* Read 8 image bytes */

        if (fread(arr, 8, 1,
                  encInfo->fptr_src_image) != 1)
        {
            return e_failure;
        }


        /* Encode character */

        encode_byte_to_lsb(ch, arr);


        /* Write modified bytes */

        if (fwrite(arr, 8, 1,
                   encInfo->fptr_stego_image) != 1)
        {
            return e_failure;
        }
    }

    return e_success;
}


/* Copy remaining image data */

Status copy_remaining_data(FILE *fptr_src_image,
                           FILE *fptr_stego_image)
{
    char buffer[1024];

    size_t bytes_read;


    while ((bytes_read =
            fread(buffer, 1,
                  sizeof(buffer),
                  fptr_src_image)) > 0)
    {
        if (fwrite(buffer, 1,
                   bytes_read,
                   fptr_stego_image) != bytes_read)
        {
            return e_failure;
        }
    }

    return e_success;
}


/* Perform encoding */

Status do_encoding(EncodeInfo *encInfo)
{
    /* Open all files */

    if (open_files(encInfo) == e_failure)
    {
        printf("Error: Failed to open files\n");

        return e_failure;
    }


    /* Check image capacity */

    if (check_capacity(encInfo) == e_failure)
    {
        printf("Error: Insufficient capacity in source image\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }


    /* Copy BMP header */

    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_stego_image) == e_failure)
    {
        printf("Error: Failed to copy BMP header\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }


    /* Encode magic string */

    if (encode_magic_string(encInfo->magic,
                            encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode magic string\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }


    /* Encode secret file extension */

    if (encode_secret_file_extn(encInfo->extn_secret_file,
                                encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode secret file extension\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }


    /* Encode secret file size */

    if (encode_secret_file_size(encInfo->size_secret_file,
                                encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode secret file size\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }


    /* Encode secret file data */

    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: Failed to encode secret file data\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }


    /* Copy remaining image data */

    if (copy_remaining_data(encInfo->fptr_src_image,
                            encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to copy remaining image data\n");

        fclose(encInfo->fptr_src_image);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_stego_image);

        return e_failure;
    }


    /* Close all files */

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);


    printf("Encoding completed successfully\n");

    return e_success;
}