#include "base64.h"
#include <string.h>

unsigned char b64_chr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char padding_char = '=';

int base64_encode(const unsigned char * sourcedata, size_t length, char * base64)
{
	int i=0, j=0;
	unsigned char trans_index=0;
	const int datalength = length;
	for (; i < datalength; i += 3){
		trans_index = ((sourcedata[i] >> 2) & 0x3f);
		base64[j++] = b64_chr[(int)trans_index];
		
		trans_index = ((sourcedata[i] << 4) & 0x30);
		if (i + 1 < datalength){
			trans_index |= ((sourcedata[i + 1] >> 4) & 0x0f);
			base64[j++] = b64_chr[(int)trans_index];
		}else{
			base64[j++] = b64_chr[(int)trans_index];
			
			base64[j++] = padding_char;
			
			base64[j++] = padding_char;
			
			break;
		}
		
		trans_index = ((sourcedata[i + 1] << 2) & 0x3c);
		if (i + 2 < datalength){
			trans_index |= ((sourcedata[i + 2] >> 6) & 0x03);
			base64[j++] = b64_chr[(int)trans_index];
			
			trans_index = sourcedata[i + 2] & 0x3f;
			base64[j++] = b64_chr[(int)trans_index];
		}
		else{
			base64[j++] = b64_chr[(int)trans_index];
			
			base64[j++] = padding_char;
			
			break;
		}
	}
	
	base64[j] = '\0';
	return 0;
}

unsigned int b64_int(unsigned int ch) {
	// ASCII to base64_int
	// 65-90  Upper Case  >>  0-25
	// 97-122 Lower Case  >>  26-51
	// 48-57  Numbers     >>  52-61
	// 43     Plus (+)    >>  62
	// 47     Slash (/)   >>  63
	// 61     Equal (=)   >>  64~
	if (ch==43)
		return 62;
	if (ch==47)
		return 63;
	if (ch==61)
		return 64;
	if ((ch>47) && (ch<58))
		return ch + 4;
	if ((ch>64) && (ch<91))
		return ch - 'A';
	if ((ch>96) && (ch<123))
		return (ch - 'a') + 26;
	return 0;
}

unsigned int b64e_size(unsigned int in_size) {
	
	// size equals 4*floor((1/3)*(in_size+2));
	int i, j = 0;
	for (i=0;i<in_size;i++) {
		if (i % 3 == 0)
			j += 1;
	}
	return (4*j);
}

unsigned int b64d_size(unsigned int in_size) {
	
	return ((3*in_size)/4);
}

unsigned int b64_decode(const unsigned char* in, unsigned int in_len, unsigned char* out) {
	
	unsigned int i=0, j=0, k=0, s[4];
	
	for (i=0;i<in_len;i++)
	{
		s[j++]=b64_int(*(in+i));
		if (j==4)
		{
			out[k+0] = ((s[0]&255)<<2)+((s[1]&0x30)>>4);
			if (s[2]!=64)
			{
				out[k+1] = ((s[1]&0x0F)<<4)+((s[2]&0x3C)>>2);
				if ((s[3]!=64))
				{
					out[k + 2] = ((s[2] & 0x03) << 6) + (s[3]);
					k += 3;
				}
				else
					k+=2;
			}
			else
				k+=1;
			j=0;
		}
	}
	
	return k;
}
