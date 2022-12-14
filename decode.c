#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"common.h"
#include"decode.h"

// flag to identify outut file name is given or not
int secret_fname_flag;							

/*
Description: Open files
Inputs: Stego image filename, Output filename 
Output: FILE pointers for above files

*/

Status open_stego_file_for_decoding( DecodeInfo *decInfo )
{
	// opening stego.bmp as read mode
	decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r");		

	// validation for opening the file
	if( decInfo->fptr_stego_image == NULL )				
	{
		printf("Error   :   Unable to open file \"%s\", file doesn't exist!\n",decInfo->stego_image_fname);
		return e_failure;
	}
	return e_success;
}

/*function to open secret file*/
Status open_secret_file_for_decoding( DecodeInfo *decInfo )
{
	// opening secretfile.txt as write mode
	decInfo->fptr_secret = fopen(decInfo->secret_fname,"w");

	// validation for open file
	if( decInfo->fptr_secret == NULL )				
	{
		printf("Error   :   Unable to open file \"%s\", file doesn't exist!\n",decInfo->secret_fname);
		return e_failure;
	}
	return e_success;
}

/*    Validate decode arguments.
Input: Command line arguments, Structure ponter
Output: validate arguments, and return e_success/e_failure.
*/
Status read_and_validate_decode_args( int argc, char *argv[], DecodeInfo *decInfo )
{
	// calculate strlen to loop length 
	int i, flag = 0, len = strlen(argv[2]);			

	// memory space to store  secret fram
	decInfo->secret_fname = malloc(75);				
	if( decInfo->secret_fname == NULL )
	{
		printf("Error   :   failed to assign memory space!\n");
		return e_failure;
	}
	for( i = 0; i < len; i++ )
	{
		// checking if condition is .bmp
		if( !strcmp(argv[2] + i,".bmp") )		
		{
			// given name is valid then assign to decinfo.steg_img_fname
			decInfo->stego_image_fname = argv[2];
			flag++;								
			break;
		}
	}
	// if optional argument is given
	if( argc == 4 )								
	{
		// loop comapre extension
		len = strlen(argv[3]);						
		for( i = 0; i < len; i++ )
		{
			// if user provided '.' in file
			if( *( argv[3] + i ) == '.' )				
			{
				// output given name is invalid
				secret_fname_flag = -1;						
				break;
			}
			// output given name is valid
			secret_fname_flag = 1;				
		}
	}

	// store given output file name in decinfo.secret_fname
	if( secret_fname_flag == 1 )
		strcpy(decInfo->secret_fname,argv[3]);		

	// if both file names are valid
	if( flag == 1 )					

		return e_success;
	else
		return e_failure;
}

/*
   Decode magic string
Input: Magic string.
Output: Decode magic string, and return e_success.
*/
Status decode_magic_string( char *magic_string, DecodeInfo *decInfo )
{
	// moving file pointer to 55 that is SEEK 
	fseek(decInfo->fptr_stego_image,54l,SEEK_SET);			
	// unsigned char to set bit
	unsigned char ch = 0;						

	// to know length of magic string #*
	int i,len = strlen(MAGIC_STRING);
	for( i = 0; i < len; i++ )
	{ 
		// read 8 bytes from image
		fread(decInfo->image_data,sizeof(char),8l,decInfo->fptr_stego_image);

		// decode lsb from image_uffer nad store to ch
		decode_bytes_from_lsb(decInfo->image_data,&ch);				
		magic_string[i] = ch;
		ch = 0;									
	}
	// decInfo->magic_string
	magic_string[i] = '\0';							
	printf("Info    :   Magic string was \"%s\"\n",magic_string);
	return e_success;
}

/*
   Decode stego magic string
Input: stego Magic string.
Output: Decode stego magic string,key magic string and return e_success.
*/

Status authenticate_magic_string( char *stego_magic_string, char *key_magic_string )
{
	if( !strcmp(stego_magic_string,key_magic_string) )	

		//comparing two magic string wheather they are equal
		return e_success;
	else
		return e_failure;
}

/*function to decode secret file size, and store to integer variable*/
Status decode_secret_file_extn_size( DecodeInfo *decInfo )
{
	int state;

	// function call to decode integer from image
	state =  decode_size(&decInfo->size_file_extn,decInfo->fptr_stego_image,decInfo->image_data); 
	if( state == e_success )
		return e_success;
	else
		return e_failure;
}

/*function to assign output secret file name*/
Status assign_secret_file_name( DecodeInfo *decInfo )
{
	if( secret_fname_flag == -1 )
	{
		// assign default name if given name not valid
		strcpy(decInfo->secret_fname,DEFAULT_SECRET_FNAME);

		printf("Error   :   GIVEN OUTPUT FILE NAME IS NOT VALID!!!!\n");
	}
	// if given output name is not given
	else if( secret_fname_flag == 0 )				
	{
		strcpy(decInfo->secret_fname,"decodefile");
		printf("Info    :   Output file name is not given\n");

		// assign name as adefault name decode file
		strcpy(decInfo->secret_fname,DEFAULT_SECRET_FNAME);				
	}
	strcat(decInfo->secret_fname,decInfo->extn_file);	
	printf("Info    :   Assigned%sFile Name is \"%s\"\n",secret_fname_flag == 1 ? " " : " new ",decInfo->secret_fname); 
	return e_success;
}

/*
   Decode 8 bits from image buffer and store it in data
Input: 8byte image data.
Output: Decoded byte.
*/
Status decode_bytes_from_lsb( char *buffer, char *ch )
{
	int i,bitdata = 0;		

	//bitdata to store each decoded lsb bit and check 1 or 0
	for( i = 0; i < 8; i++ )
	{
		// move bits to msb
		*ch <<= 1;		

		bitdata = buffer[i] & 0x01;
		// bitdata will either 0x00 or 0x01
		if( bitdata )			

			// clear bit and assign 0 to lsb
			*ch |= 0x01;		
	}
	return e_success;						        
}


/*function to decode integer from image and store to variable given*/
Status decode_size( int *size, FILE *fptr, char *buffer )
{
	int i,j,bitdata = 0;

	// clearing size before storing decode data
	*size = 0;				

	//loop integerbsize times
	for( i = 0; i < sizeof(int); i++ )						
	{
		// reading 8 bytes from image
		fread(buffer,sizeof(char),8l,fptr);					
		for( j = 0; j < MAX_IMAGE_BUF_SIZE; j++ )
		{
			// getting lsb of each charater byte
			*size <<= 1;
			bitdata = buffer[j] & 0x01;				
			if( bitdata )
				// lsb of image byte is 0		
				*size |= 0x01;				                                      
		}					
	}										
	return e_success;
}
/*
   Decode secret file extension.
Input: Structure pointer.
output: Secret file extension is decoded.
*/
Status decode_secret_file_extn( char *file_extn, DecodeInfo *decInfo )
{
	char ch,i;					

	// loop file extension size times
	for( i = 0; i < decInfo->size_file_extn; i++ )	
	{
		// reading 8 bytes from image and store to buffer
		fread(decInfo->image_data,sizeof(char),8l,decInfo->fptr_stego_image);

		// decode bits from 8 bytes read from image data
		decode_bytes_from_lsb(decInfo->image_data,&ch);				
		decInfo->extn_file[i] = ch;						
	}
	// making character array as string
	decInfo->extn_file[i] = '\0';						
	return e_success;
}

/*
   Decode secret file size.
Input: Structure pointer.
Output: File extension is decoded.
*/
Status decode_secret_file_size( int *file_size, DecodeInfo *decInfo )
{
	int state;

	// function call to decode 4 bytes from image
	state = decode_size(file_size,decInfo->fptr_stego_image,decInfo->image_data);  
	if( state == e_success )
	{
		printf("Info    :   Size of the secret file was %dBytes\n",*file_size);	   
		return e_success;
	}
	else
		return e_failure;
}

/*
   Decode secret file data
Input: Structure pointer.
Output: Secret file size is decoded and stored in structure.
*/
Status decode_secret_file_data( DecodeInfo *decInfo )
{
	// flag to record successful function return
	int i,state,flag = 0;							
	printf("Info    :   Secret file content is given below\n\n");

	// loop secret file size times
	for( i = 0; i < decInfo->size_secret_file; i++ )
	{
		// function call to decode data to file
		state = decode_data_to_file(decInfo->secret_data,decInfo->image_data,decInfo->fptr_secret,decInfo->fptr_stego_image);
		printf("%c",*decInfo->secret_data);			
		if( state == e_success )						
			flag++;
	}
	// if all functio calls are success
	if( flag == decInfo->size_secret_file )					
		return e_success;
	else
		return e_failure;
}

/*function to decode data to file*/
Status decode_data_to_file(char *data, char *buffer, FILE *fptr_secret_file, FILE *fptr_stego_image)
{
	// read 8 bytes from stego image to image data
	fread(buffer,sizeof(char),MAX_IMAGE_BUF_SIZE,fptr_stego_image);

	// function call to decode lsb from buffer to_data
	decode_bytes_from_lsb(buffer,data);		

	// writing the decode data to secret file
	fwrite(data,sizeof(char),MAX_SECRET_BUF_SIZE,fptr_secret_file);

	// if error registered file steem
	if( ferror(fptr_secret_file) || ferror(fptr_stego_image) )	
		return e_failure;
	else
		return e_success;
}


/*function to DO DECODING*/
Status do_decoding( DecodeInfo *decInfo )
{
	// state to record success or failure
	int state;									

	// openihng stego image
	state = open_stego_file_for_decoding(decInfo);	
	if( state == e_success )
	{
		printf("Info    :   File\" %s\" opened Successfully!\n",decInfo->stego_image_fname);

		// decoding magic string and storing to magic_string
		state = decode_magic_string(decInfo->magic_string,decInfo);		
		if( state == e_success )
		{
			printf("Info    :   Magic string decoded Successfully\n");

			// authenticate decoded magic string
			state = authenticate_magic_string(decInfo->magic_string,MAGIC_STRING);	
			if( state == e_success )
			{
				printf("Info    :   Authentication of magic string is Success!\n");

				// calculate secret file extension size
				state = decode_secret_file_extn_size(decInfo);		
				if( state == e_success )
				{
					printf("Info    :   Secret file extension size decoded successfully!\n");

					// decoding file extension and string to array
					state = decode_secret_file_extn(decInfo->extn_file,decInfo);	
					if( state == e_success )
					{
						printf("Info    :   Secret file extension decoded successfully!\n");
						printf("            The extension was \"%s\"\n",decInfo->extn_file);

						// function call to assign screrfile name from image
						state = assign_secret_file_name(decInfo);	
						if( state == e_success )
						{
							printf("Info    :   Secret file name assigned succesfully with decoded extension!\n");

							// opening secret file
							state = open_secret_file_for_decoding(decInfo);
							if( state == e_success )
							{
								printf("Info    :   File \"%s\" opened succesfully!\n",decInfo->secret_fname);

								// decoding secret file size
								state = decode_secret_file_size(&decInfo->size_secret_file,decInfo); 
								if( state == e_success )
								{
									printf("Info    :   Secret file size decoded successfully!\n");

									// decoding secret data and write to given file
									state = decode_secret_file_data(decInfo);			
									if( state == e_success )
									{
										printf("\n\nInfo    :   Secret file data decoded successfully!\n");

										/*closing all files*/
										fclose(decInfo->fptr_stego_image);
										fclose(decInfo->fptr_secret);

										/*free secret file name memory*/
										free(decInfo->secret_fname);
										return e_success;
									}
									else
									{
										printf("\n\nError   :   Failed to decode secret file data!\n");
										return e_failure;
									}
								}
								else
								{
									printf("Error   :   Failed to decode secret file size!\n");
									return e_failure;
								}
							}
							else
							{
								printf("Error   :   Failed to open the secret file\n");
								return e_success;
							}
						}
						else
						{
							printf("Error   :   Failed to assign secret file name\n");
							return e_failure;
						}
					}
					else
					{
						printf("Error   :   Failed to decode file extension!\n");
						return e_failure;
					}
				}
				else
				{
					printf("Error   :   Failed to decode file extension size!\n");
					return e_failure;
				}
			}
			else
			{
				printf("Error   :   Authentication of magic string Failed\n            \"%s\" is not an Encoded Image\n",decInfo->stego_image_fname);
				return e_failure;
			}
		}
		else
		{
			printf("Error   :   Failed to decode magic string\n");
		}
	}
	else
	{
		printf("Error   :   Failed to open the Files\n");
		return e_failure;
	}
	return e_failure;
}
