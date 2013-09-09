#include <stdint.h>
#include <stdlib.h>


static char base64_encoding_table[] = {
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
'w', 'x', 'y', 'z', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', '+', '/'};
static char * base64_decoding_table = NULL;
static int base64_mod_table[] = {0, 2, 1};


char * base64_encode( const char * i_data, int i_length, int & o_length)
{
	o_length = 4 * ((i_length + 2) / 3);

	char * encoded_data = new char[o_length];
	if (encoded_data == NULL) return NULL;

	for (int i = 0, j = 0; i < i_length;) {

		uint32_t octet_a = i < i_length ? i_data[i++] : 0;
		uint32_t octet_b = i < i_length ? i_data[i++] : 0;
		uint32_t octet_c = i < i_length ? i_data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		encoded_data[j++] = base64_encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = base64_encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = base64_encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = base64_encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < base64_mod_table[i_length % 3]; i++)
		encoded_data[o_length - 1 - i] = '=';

	return encoded_data;
}

/* DECODING IS NOT USED IN AFANASY:

void base64_build_decoding_table()
{
	base64_decoding_table = new char[256];

	for( int i = 0; i < 64; i++)
		base64_decoding_table[(char)base64_encoding_table[i]] = i;
}

char * base64_decode( const char * i_data, int i_length, int & o_length)
{
	if( base64_decoding_table == NULL) base64_build_decoding_table();

	if( i_length % 4 != 0) return NULL;

	o_length = i_length / 4 * 3;
	if( i_data[i_length - 1] == '=') (o_length)--;
	if( i_data[i_length - 2] == '=') (o_length)--;

	char * decoded_data = new char[o_length];
	if( decoded_data == NULL ) return NULL;

	for( int i = 0, j = 0; i < i_length;)
	{
		uint32_t sextet_a = i_data[i] == '=' ? 0 & i++ : base64_decoding_table[i_data[i++]];
		uint32_t sextet_b = i_data[i] == '=' ? 0 & i++ : base64_decoding_table[i_data[i++]];
		uint32_t sextet_c = i_data[i] == '=' ? 0 & i++ : base64_decoding_table[i_data[i++]];
		uint32_t sextet_d = i_data[i] == '=' ? 0 & i++ : base64_decoding_table[i_data[i++]];

		uint32_t triple = (sextet_a << 3 * 6)
		+ (sextet_b << 2 * 6)
		+ (sextet_c << 1 * 6)
		+ (sextet_d << 0 * 6);

		if( j < o_length ) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
		if( j < o_length ) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
		if( j < o_length ) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
	}

	return decoded_data;
}

void base64_base64_cleanup()
{
	if( base64_build_decoding_table )
		delete [] base64_decoding_table;
}

DECODING IS NOT USED IN AFANASY */
