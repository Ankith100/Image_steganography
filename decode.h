#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4
#define DEFAULT_SECRET_FNAME "DecodedFile"

typedef struct _DecodeInfo
{
	/* Stego Image Info */
	char *stego_image_fname;
	FILE *fptr_stego_image;
	char image_data[MAX_IMAGE_BUF_SIZE];

	/* Secret File Info */
	char *secret_fname;
	FILE *fptr_secret;						
	int size_secret_file;
	int size_file_extn;
	char extn_file[MAX_FILE_SUFFIX+1];
	char secret_data[MAX_SECRET_BUF_SIZE];

	/*magic string*/
	char magic_string[3];


} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo);		

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);							

/* Get File pointers for stego file */
Status open_stego_file_for_decoding(DecodeInfo *decInfo);						

/* Get File pointers for secret file */
Status open_secret_file_for_decoding(DecodeInfo *decInfo);						

/* Store Magic String */
Status decode_magic_string(char *magic_string, DecodeInfo *decInfo);				

/* Decode secret file extenstion */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);					

/* Decode secret file extenstion */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo);				

/* Decode secret file size */
Status decode_secret_file_size(int *file_size, DecodeInfo *decInfo);				

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);						

/* Decode function, which does the real decoding */
Status decode_data_to_file(char *data, char *buffer, FILE *fptr_secret_file, FILE *fptr_stego_image); 

/* Encoding  extention size and file size */
Status decode_size(int *size, FILE *fptr, char *buffer);					

/* Decode size to lsb */
Status decode_bytes_from_lsb(char *buffer, char *ch);						

/*Authenticate magic string which is read from stego image*/
Status authenticate_magic_string(char *stego_magic_string, char *key_magic_string);

/*assign secret file name and concatenate with extension*/
Status assign_secret_file_name(DecodeInfo *decInfo);						

#endif
