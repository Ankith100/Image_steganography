/*
Project : LSB Image Steganography
Name	: Gurram Anka Rao
Input	: Pass file through command line.
Encoding ./a.out -e <.bmp file> <secret_file> [output file]
Decoding./a.out -d <.stego.bmp> [output file]
Output: Secret message is Encoded/Decoded from the given bmp file.
*/

#include<stdio.h>
#include"encode.h"
#include"decode.h"
#include"types.h"
#include"common.h"
int main( int argc, char *argv[] )
{   

	// Declaring EncodeInfo variables
	EncodeInfo encInfo;	

	//Declaring an DecodeInfo variables
	DecodeInfo decInfo;							

	// Store state of each function call
	int state;								

	// if option given for encode(-e),and argv minimum 4
	if( argc > 3 && check_operation_type(argv) == e_encode )		
	{						
		// -e org.bmp secret.txt  -> min argc that is 4
		state = read_and_validate_encode_args( argc,argv,&encInfo);	

		// validate the arguments extention .bmp .txt and output file name
		if( state == e_success )
		{
			printf("Info : Validation is successful!\n");

			//encoding will done in do_encoding() definition
			state = do_encoding(&encInfo);

			// if  condition encode is succesfully completed
			if( state  == e_success )
			{
				printf("Info    :Encoding is done Succesfully!\n");
			}
			else
			{
				printf("Error   : Encoding Failed!\n");
				return -1;
			}
		}
		else
		{
			printf("Error : Arguments are not in proper formats\n%56s\n","use .bmp for image and .txt for secrect file");
			return -1;
		}
	}

	// given else if option is for decode(-d)
	else if( argc > 2 && check_operation_type(argv) == e_decode )		
	{

		// validating extensions of  files & assign new name if not given
		state = read_and_validate_decode_args( argc,argv,&decInfo);
		if( state == e_success )
		{
			printf("Info    : Validation is Successful!\n");

			// Decoding will done in do_decoding()
			state = do_decoding(&decInfo);		
			if( state == e_success )
			{
				printf("Info    :   Decoding is done succesfully!\n");
			}
			else
			{
				printf("Error   :   Decoding Failed!\n");
				return -1;
			}
		}
		else
		{
			printf("Error : Arguments are not in proper formats\n%30s\n","Use only .bmp files");
			return -1;
		}
	}
	else
	{
		// printing formats and error codes
		printf("\nError : input format is not valid!\n%40s\n\n","Refer the following Format");	
		printf("/lsb_steg -e File_name.bmp Secret_text.txt output.bmp\n<output.bmp> is optional\n");
		printf("\n/lsb_steg -d Encode_image.bmp Secret_file\n<secret> is optional and give name WITHOUT EXTENSION\n");
		printf("\nopeartion  :   %s for encoding\n%17s for decoding\n\n","-e","-d");
	}
	return 0;
}
