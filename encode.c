#include<stdio.h>
#include<string.h>
#include"types.h"
#include"encode.h"
#include"common.h"

// calaculate encode bytes
int encodedbytes;	

/* Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
	// Src Image file
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
	// Do Error handling
	if (encInfo->fptr_src_image == NULL)
	{
		//perror("fopen");
		fprintf(stderr, "Error   :   Unable to open file %s\n", encInfo->src_image_fname);

		return e_failure;
	}

	// Secret file
	encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
	// Do Error handling
	if (encInfo->fptr_secret == NULL)
	{
		//perror("fopen");
		fprintf(stderr, "Error   :   Unable to open file %s\n", encInfo->secret_fname);

		return e_failure;
	}

	// Stego Image file
	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
	// Do Error handling
	if (encInfo->fptr_stego_image == NULL)
	{
		//perror("fopen");
		fprintf(stderr, "Error   :   Unable to open file %s\n", encInfo->stego_image_fname);

		return e_failure;
	}

	// No failure return e_success
	return e_success;
}

/* Function Definitions */
/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{   
	uint width, height;
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET);

	printf("Info    :   Image information\n");
	// Read the width (an int) 
	fread(&width, sizeof(int), 1, fptr_image);
	printf("             Width = %u\n", width);

	// Read the height (an int) 
	fread(&height, sizeof(int), 1, fptr_image);
	printf("             Height = %u\n", height);

	// Return image capacity
	return width * height * 3;
} 

/*
   Check type of operation
Input:Array of command line arguments
Ouput:Integer indicating type of operation
*/
OperationType check_operation_type( char *argv[] )
{  
	// checking if option given for -e
	if( !strcmp(argv[1],"-e") )		
		return e_encode;

	// checking if option given for -d
	else if ( !strcmp(argv[1],"-d") )	
		return e_decode;
	else
		return e_unsupported;
}

/*
   Read and validate command line arguments.
Input: Command line arguments.
Output: Validate args and return -e_success/e_failure.

*/
Status read_and_validate_encode_args( int argc, char *argv[], EncodeInfo *encInfo )
{
	int i,flag1 = 0,flag2 = 0,length = strlen(argv[2]);

	// check original.bmp file extension
	for(i = 0; i < length; i++ )	
	{
		// check if extension is bmp
		if( !strcmp(argv[2] + i,".bmp") )	
		{
			// copying file name address to encInfo.src_image_fname
			encInfo->src_image_fname = argv[2];	
			// increment flag to say .bmp extension validation is succesful
			flag1++;	   
			break;
		}
	}
	length = strlen(argv[3]);

	// check secret.txt file extention
	for( i = 0; i < length; i++)
	{
		//check if secret file given with extension
		if( *(argv[3] + i) == '.' )			
		{
			// copying the extension to array extn_secret_file
			strcpy(encInfo->extn_secret_file,argv[3]+i); 

			//copying file name address to encInfo.secret_fname
			encInfo->secret_fname = argv[3];		

			// increment flag to say .txt extention validaton is succesful
			flag1++;					
			break;
		}
	}
	// checking if output file name is provided or not
	if( argc == 5 )					
	{
		length = strlen(argv[4]);	

		// checking the extention of output file name
		for( i = 0; i < length; i++ )		
		{
			if( !strcmp(argv[4] + i,".bmp") )
			{

				// if valid copy the string address toencInfo.stego_image_fname
				encInfo->stego_image_fname = argv[4];	

				//output file name is given and do not enter the block to assign new name
				flag2 = 1;			
				flag1++;			
				break;
			}
			flag2 = -1;					
		}
	}

	// output file name is not valid
	if ( flag2 == -1 )	
		printf("Info    :	output file name extention is not valid, creating output file with name %s.bmp\n",DEFAULT_STEGO_NAME);

	// name is not valid or ouput file name is not given
	if( flag2 == 0 || flag2 == -1 )	
	{
		printf("Info    :	Output file name is not given\n Assigning default name \"%s.bmp\"\n",DEFAULT_STEGO_NAME);
		flag1++;					

		// default name will .bmp
		encInfo->stego_image_fname = DEFAULT_STEGO_NAME".bmp";	
	}

	// three files names are stored successfully then flag will be 3
	if( flag1 == 3 )					
		return e_success;
	else
		return e_failure;
}


/*
   Check image capacity to hold secret file
Input: Structure pointer
Output: validate and return success/failure.
*/  
Status check_capacity( EncodeInfo *encInfo )
{ 
	// store original image size
	encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);	

	// store secret.txt file size    
	encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);		

	// if image size is enough to hold
	if( encInfo->image_capacity > 54 + 16 + 32 + 32 + 32 + encInfo->size_secret_file * 8 )
		return e_success;								
	else							
		return e_failure;
}

/*
   Get secret file size
Input: File pointer of secret file
Output: Length of the file.
*/
uint get_file_size( FILE *fptr )
{
	uint size;

	// moving file pointer to end of the file
	fseek(fptr,0L,SEEK_END);	

	// fptr file size will be file size    
	size = ftell(fptr);		
	printf("Info    :   Size of secret file is %uBytes\n",size);

	// rewind to get back fptr to starting file
	rewind(fptr);		
	return size;
}


/*
   Copy 54 bytes header to destination file.
Input: File pointer to source and destination files.
Output: Copy header and return success/failure.
*/
Status copy_bmp_header( FILE *fptr_src_image , FILE *fptr_dest_image )
{
	char ch[HEADER_SIZE];

	// move file pointer to begining
	rewind(fptr_src_image);

	// header of bmp is starting from location 0
	rewind(fptr_dest_image);

	//  redaing 54 bytes binary src and sorting to ch
	fread(ch,sizeof(char),HEADER_SIZE,fptr_src_image);

	// writing 54 bytes data from ch to dest file
	fwrite(ch,sizeof(char),HEADER_SIZE,fptr_dest_image);
	return e_success;
}

/*
   Encode magic string passing parameters.
Input: Magic string .
Output: magic string is encoded. 
*/
Status encode_magic_string( const char *magic_string, EncodeInfo *encInfo )
{
	// store magic_string size
	int len = strlen(magic_string);

	// function calling to encode the string data,2 bytes of string ae passing
	if( encode_data_to_image(magic_string,len,encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo->image_data) == e_success )
	{	

		// record bytes encoded
		encodedbytes += len;
		printf("Info    :   Encode data to image is done successfully!\n");
		return e_success;
	}
	else
	{
		printf("Error   :   Failed to encode the data to image\n");
		return e_failure;
	}
}

/*Encode string data to image call from encode_magic_string.
Input : Magic data to image
output : Magic data to image is encoded.
*/
Status encode_data_to_image( const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, char *image_data )
{
	// flag to record any failure have been occured
	int i,j,state,flag = 0;	

	// store the scanned value from src    
	char ch;			

	// encode the lopp byte by byte
	for( i = 0; i < size; i++ )
	{
		// reading 8 bytes from src image to ch
		fread(image_data,sizeof(char),MAX_IMAGE_BUF_SIZE,fptr_src_image);

		// if error occured while reading the fptr_src_image
		if( ferror(fptr_src_image) )			
			flag++;

		// function call to encode 8 bits of data to 8 bytes
		state = encode_byte_to_lsb(data[i],image_data); 			
		if( state == e_failure )
			flag++;

		// writing 1 byte to steg_image output bmp
		fwrite(image_data,sizeof(char),MAX_IMAGE_BUF_SIZE,fptr_stego_image);	

		// if error occured while writing the fptr_stego_image
		if( ferror(fptr_stego_image) )					
			flag++;
	}

	// if no error occurred
	if( flag == 0 )		
	{
		printf("Info    :   Encode bytes to lsb done succcesfully!\n");
		return e_success;
	}
	else
	{
		printf("Error   :   Failed to encode bytes to lsb\n");
		return e_failure;
	}
}

/*
   Function to encode 1byte into lsb's of 8bytes
Input: Data to be encoded, 8 byte image data.
Output: The data is encoded into 8 bytes.
*/

Status encode_byte_to_lsb( char data, char *image_buffer )
{
	int i,bitdata = 0;		

	// loop 8 times that is to extract and encode 8 bits
	for( i = 0; i < MAX_IMAGE_BUF_SIZE; i++ )
	{
		// bits from msb of data
		bitdata = data & 0x80;	

		// bitdata will contain 0x80  or 0x00
		if( bitdata )			

			// set lsb bit of pixel data
			image_buffer[i] |= 0x01;				
		else

			// clear lsb bit of pixel data
			image_buffer[i] &= 0xfe;

		// moving next bit -1 to msb
		data <<= 1;		
	}
	return e_success;
}

/*function to encode integer number to given buffer data lsb*/
Status encode_size_to_lsb( int size, char *buffer )
{
	int i, bitdata = 0;

	// encode 4 bytes of data
	for( i = 0; i < 32; i++)
	{
		// 1 bit form msb to bit data
		bitdata = size & 0x80000000;			
		if( bitdata )		

			// set lsb data
			buffer[i] |= 1;			
		else

			// clear lsb bit
			buffer[i] &= 0xfffffffe;		

		// move next bit to msb
		size <<= 1;					
	}
	return e_success;
}

/*function to encode the integer numbers, that is size of extension or file*/
Status encode_size( int size, EncodeInfo *encInfo )
{
	int i;

	// read and store pixel data from original image to encode byte
	char buffer[32];					
	for( i = 0; i < 32; i++ )

		//reading from image and stroing to buffer[]
		fread(&buffer[i],sizeof(char),1l,encInfo->fptr_src_image);
	if( encode_size_to_lsb(size,buffer) == e_success )
	{
		for( i = 0; i < 32; i++ )

			// write the encoded data from buffer to stego.bmp
			fwrite(&buffer[i],sizeof(char),1l,encInfo->fptr_stego_image);	
		encodedbytes += sizeof(int);		
		return e_success;
	}
	else
		return e_failure;
}

/**
  Encode secret file extension.
Input: File extension.
Output: Encode extn into destination file.
*/
Status encode_secret_file_extn( const char *file_extn, EncodeInfo *encInfo)
{
	// store the size of data that is.txt
	int len = strlen(file_extn);			
	if( encode_data_to_image(file_extn,len,encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo->image_data) == e_success )
	{
		encodedbytes += len;				

		//call to encode the string data to image
		return e_success;				
	}
	else
		return e_failure;
}

/*function called at the time to encode secret file size*/
Status encode_secret_file_size( long size, EncodeInfo *encInfo )
{
	// calling size encode function
	encode_size(size,encInfo);				
}

/*
   Encode secret file data into the destination image
Input : Structure pointer.
Output: Encode secret data into destination file and return e_success/.

*/
Status encode_secret_file_data( EncodeInfo *encInfo )
{
	int i,j;
	for( i = 0; i < encInfo->size_secret_file; i++ )
	{
		// read 1 byte from secret file
		fread(encInfo->secret_data,sizeof(char),1l,encInfo->fptr_secret);

		// read 8 byte from org.bmp to encode 1 byte data
		fread(encInfo->image_data,sizeof(char),MAX_IMAGE_BUF_SIZE,encInfo->fptr_src_image);

		// encode 1byte of data to buffer data
		encode_byte_to_lsb(*encInfo->secret_data,encInfo->image_data);

		// writing encoded data from buffer to stego.bmp
		fwrite(encInfo->image_data,sizeof(char),MAX_IMAGE_BUF_SIZE,encInfo->fptr_stego_image);
	}
	encodedbytes += encInfo->size_secret_file;						   
	return e_success;
}

/*
   Copy remaing data into destination image
Input : File pointers to source and destination images.
Output: Copy remaining image data into destination image.
*/

Status copy_remaining_img_data( FILE *fptr_src, FILE *fptr_dest )
{
	char ch;

	// loop untill EOF is reached in org.bmp
	while( 1 )					
	{
		// read 1 byte binary from src to ch
		fread(&ch,sizeof(char),1,fptr_src);		

		// break if EOF is reached src org.bmp
		if( feof(fptr_src) )				
			break;

		// writing 1 byte data from ch to dest file
		fwrite(&ch,sizeof(char),1,fptr_dest);		
	}
	return e_success;
}

/*Encoding starts from here*/
Status do_encoding( EncodeInfo *encInfo )
{ 
	// state to record success or failure
	int state;					

	//  open the file and assign file pointers if success
	state = open_files(encInfo);			
	if( state == e_success )
	{
		printf("Info    :   File opened Successfully!\n");

		// checking capacity of given org.bmp to hold secret.txt
		state = check_capacity(encInfo);		
		if( state == e_success )
		{
			printf("Info    :   Capacity check done successfully!\n");

			//copying bmp header which is of 54Bytes
			state = copy_bmp_header(encInfo->fptr_src_image , encInfo->fptr_stego_image);	
			if( state == e_success )
			{
				printf("Info    :   Header copied Successfully!\n");
				state = encode_magic_string(MAGIC_STRING,encInfo);
				if( state == e_success )
				{
					printf("Info    :   Magic string encoded successfully!\n");

					// function calling to store SIZE OF EXTENSION
					state = encode_size(strlen(encInfo->extn_secret_file),encInfo);	
					if(state == e_success )
					{
						printf("Info    :   Secret file extenson size encoded Successfully!\n");

						// function call to .txt
						state = encode_secret_file_extn(encInfo->extn_secret_file,encInfo);
						if( state == e_success )
						{
							printf("Info    :   Secret file extension encoded Successfully!\n");

							// function call to encode file size
							state = encode_secret_file_size(encInfo->size_secret_file,encInfo);	
							if( state == e_success )
							{
								printf("Info    :   Size of secret file has been encoded successfully!\n");

								// function call to encode secret file data
								state = encode_secret_file_data(encInfo);		
								if( state == e_success )
								{
									printf("Info    :   Secret file data encoded Successfully!\n");

									// call to copy remaining data
									state = copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image);
									if( state == e_success )								
									{
										printf("Info    :   Remaining data are copied successfully!\nInfo    :   Total Bits Encoded - %dbits\n",encodedbytes*8);
										//closing all files
										fclose(encInfo->fptr_src_image);
										fclose(encInfo->fptr_secret);
										fclose(encInfo->fptr_stego_image);
										return e_success;
									}
									else
									{
										printf("Error   :   Failed to copy the remaining data to output bmp\n");
										return e_failure;
									}
								}
								else
								{
									printf("Error   :   Failed to encode secret file data\n");
									return e_failure;
								}
							}
							else
							{
								printf("Error   :   Failed to encode the size of secret file\n");
								return e_failure;
							}
						}
						else
						{
							printf("Error    :   Failed to encode secret file extension\n");
							return e_failure;
						}
					}
					else
					{
						printf("Error   :   Failed to encode secret file extension\n");
						return e_failure;
					}
				}
				else
				{
					printf("Error   :   Failed to encode magic string\n");
					return e_failure;
				}
			}
			else
			{
				printf("Error   :   Failed to copy the header of the bmp file!\n");
				return e_failure;
			}
		}
		else
		{
			printf("Error   :   Encoding is not possible because capacity of given file is less\n");
			return e_failure;
		}
	}
	else
	{
		printf("Error   :   Failed to open the files\n%31s\n","File doesn't exist!");
		return e_failure;
	}
}
