
// Name                : Mohamed Fawaz.K.H
// Date                : 03/09/2026
// Project Name        : LSB Image Steganography
// Project Description : This project securely hides secret information 
//                       inside a BMP image using LSB (Least Significant 
//                       Bit) steganography. It also allows the hidden
//                       data to be extracted from the image without 
//                       significantly changing its appearance.








#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

int main(int argc, char *argv[])
{
    OperationType res;

    /* Check minimum arguments */
    if (argc < 2)
    {
        printf("Error: Insufficient arguments\n");
        printf("Usage:\n");
        printf("Encoding: ./a.out -e source.bmp secret.txt [stego.bmp]\n");
        printf("Decoding: ./a.out -d stego.bmp [output.txt]\n");
        return 0;
    }

    /* Check operation type */
    res = check_operation_type(argv);

    if (res == e_encode)
    {
        EncodeInfo encInfo;

        /* Validate encoding arguments */
        if (read_and_validate_encode_args(argc, argv, &encInfo) == e_success)
        {
            /* Start encoding */
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding completed successfully\n");
            }
            else
            {
                printf("Error: Encoding failed\n");
                return 0;
            }
        }
        else
        {
            printf("Error: Invalid encoding arguments\n");
            return 0;
        }
    }
   else if (res == e_decode)
    {
        /* Decode operation */

        DecodeInfo decInfo;

        if (read_and_validate_decode_args(argc, argv, &decInfo) == e_success)
        {
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding completed successfully\n");
            }
            else
            {
                printf("Error: Decoding failed\n");
                return 0;
            }
    }
    else
    {
        printf("Error: Invalid decode arguments\n");
        return 0;
    }
}
    else
    {
        printf("Error: Unsupported operation\n");
        printf("Use -e for encoding or -d for decoding\n");
        return 0;
    }

    return 0;
}