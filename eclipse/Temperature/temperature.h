/*
 * temperature.h
 *
 *  Created on: Dec 3, 2020
 *      Author: jcaf
 */

#ifndef TEMPERATURE_TEMPERATURE_H_
#define TEMPERATURE_TEMPERATURE_H_

#define MAX6675_UTILS_LCD_PRINT3DIG_C
//#define MAX6675_UTILS_LCD_PRINTCOMPLETE_C

#ifdef MAX6675_UTILS_LCD_PRINT3DIG_C
	void MAX6675_convertIntTmptr2str_wformatPrint3dig(int16_t temper, char *str_out);
#endif

#ifdef MAX6675_UTILS_LCD_PRINTCOMPLETE_C
	void MAX6675_convertIntTmptr2str_wformatPrintComplete(int16_t temper, char *str_out);
#endif

#endif /* TEMPERATURE_TEMPERATURE_H_ */
