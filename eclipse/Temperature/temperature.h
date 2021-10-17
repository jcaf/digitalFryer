/*
 * temperature.h
 *
 *  Created on: Dec 3, 2020
 *      Author: jcaf
 */

#ifndef TEMPERATURE_TEMPERATURE_H_
#define TEMPERATURE_TEMPERATURE_H_

#define MAX6675_UTILS_LCD_PRINT3DIG
//#define MAX6675_UTILS_LCD_PRINT3DIG_C
//#define MAX6675_UTILS_LCD_PRINTCOMPLETE_C

#ifdef MAX6675_UTILS_LCD_PRINT3DIG
void MAX6675_formatText3dig(int16_t temper, char *str_out);
#endif

#ifdef MAX6675_UTILS_LCD_PRINT3DIG_C
	void MAX6675_formatText3dig_C(int16_t temper, char *str_out);
#endif

#ifdef MAX6675_UTILS_LCD_PRINTCOMPLETE_C
	void MAX6675_convertIntTmptr2str_wformatPrintComplete(int16_t temper, char *str_out);
#endif

int8_t MAX6675_job(void);
int8_t MAX6675_smoothAlg_nonblock_job(int16_t *TCtemperature);

extern int TCtemperature;
int8_t temperature_job(void);

enum _THTEMPERATURE_UNITS
{
	FAHRENHEIT= 0,
	CELSIUS
};

#endif /* TEMPERATURE_TEMPERATURE_H_ */
