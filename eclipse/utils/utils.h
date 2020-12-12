/*
 * utils.h
 *
 *  Created on: Dec 10, 2020
 *      Author: jcaf
 */

#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

//#define _dec2bcd_
//#define _nibble2asciihex_
//#define _asciihex2uint8_t_
//#define _asciihexNum2uint16_t_
//#define _dtostrf_paddingleft_ofzeros_
#define _paddingLeftwZeroes_
#define _paddingLeftwBlanks_


	#ifdef _dec2bcd_
	uint8_t dec2bcd(uint8_t dec);
	#endif

	#ifdef _nibble2asciihex_
	char nibble2asciihex(char c);//nibbleBin_to_asciiHex
	#endif

	#ifdef _asciihex2uint8_t_
	uint8_t asciihex2uint8_t(char asciihex);
	#endif

	#ifdef _asciihexNum2uint16_t_
	uint16_t asciihexNum2uint16_t(char *asciihexNum, uint8_t ndigs);
	#endif

	#ifdef _dtostrf_paddingleft_ofzeros_
	void dtostrf_paddingleft_ofzeros(char *p);
	#endif

	#ifdef _paddingLeftwZeroes_
	int8_t paddingLeftwZeroes(char *istr, int ndigs_format);
	#endif

	#ifdef _paddingLeftwBlanks_
	int8_t paddingLeftwBlanks(char *istr, int ndigs_format);
	#endif


#endif /* UTILS_UTILS_H_ */
