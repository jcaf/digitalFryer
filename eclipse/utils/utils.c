/*
 * utils.c
 *
 *  Created on: Dec 10, 2020
 *      Author: jcaf
 */
#include <stdint.h>
#include <string.h>
#include "utils.h"

#ifdef _dec2bcd_
uint8_t dec2bcd(uint8_t dec)
{
    return ( (dec/10)<<4 ) + (dec%10);
}
#endif

#ifdef _nibble2asciihex_
char nibble2asciihex(char c)//nibbleBin_to_asciiHex
{
    if (c < 10)
        return c+'0';
    else
        return (c-10) + 'A';
}
#endif

/*Only accept 1 ASCII-HEX
 *0 1 2 3 4 5 6 7 8 9 A B C D E F
 */
#ifdef _asciihex2uint8_t_
uint8_t asciihex2uint8_t(char asciihex)
{
    if ( (asciihex >= '0') && (asciihex <= '9'))
        return  asciihex-'0';
    else if ((asciihex >= 'A') && (asciihex <= 'F'))
        return  (asciihex-'A') + 10;
    else return 0;
}
#endif

/*
 * Convierte un numero de n cantidades de digitos ("ndigs") y lo convierte en uint16_t
 * 1F = 31
 */
#ifdef _asciihexNum2uint16_t_
uint16_t asciihexNum2uint16_t(char *asciihexNum, uint8_t ndigs)//
{
    uint16_t hex_power;
    uint16_t acc = 0;
    uint8_t num_max = ndigs;

    for (int8_t i=0; i<num_max; i++)
    {
        hex_power=1;
        ndigs--;
        for (int8_t h=0; h<ndigs; h++)
            {hex_power *= 16;}

        int8_t r = asciihex2bin(asciihexNum[i]);
        acc += (r*hex_power);
        //printf("r es: %i, hex_power es %i, acc es %i\n", r,hex_power,acc);
    }
    return acc;
}
#endif

#ifdef _dtostrf_paddingleft_ofzeros_
void dtostrf_paddingleft_ofzeros(char *p)
{
    while (*p !='.')
    {
        if (*p == ' ')
            *p='0';
        p++;
    }
}
#endif
/*
 *
 * char istr[4];
 * itoa(i, istr, 10)
 * istr
 * nformat = 3 -> 003'\n'
 */

#if defined (_paddingLeftwBlanks_) || defined (_paddingLeftwZeroes_)

int8_t paddingLeftw_(char *istr, int ndigs_format, char c)
{
	if (ndigs_format < strlen(istr) )
			return 0;

		//find position '\0'
		int cz;
		for (cz=0; cz<ndigs_format; cz++)
			if (istr[cz] == '\0')
				break;
		//count zeroes found >0

		if ( (cz == 0) || (cz == ndigs_format) )
				return 0;

		istr[ndigs_format] = '\0';

		//move >>
		for (int i=0; i< ndigs_format-cz; i++)
		{
			for (int j = ndigs_format-1; j>=0; j--)
			{
				istr[j] = istr[j-1];
			}
		}
		//complete with 'c' to left
		for (int i=0; i < ndigs_format-cz; i++)
			istr[i]=c;

		return 1;
	}

	#ifdef _paddingLeftwZeroes_
	int8_t paddingLeftwZeroes(char *istr, int ndigs_format)
	{
		return paddingLeftw_(istr, ndigs_format, '0');
	}
	#endif

	#ifdef _paddingLeftwBlanks_
	int8_t paddingLeftwBlanks(char *istr, int ndigs_format)
	{
		return paddingLeftw_(istr, ndigs_format, ' ');
	}
	#endif
#endif
