/*
 * temperature.c
 *
 *  Created on: Dec 3, 2020
 *      Author: jcaf
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "temperature.h"

#ifdef MAX6675_UTILS_LCD_PRINT3DIG_C
/*****************************************************
Format with 3 digits 999C
*****************************************************/
void MAX6675_convertIntTmptr2str_wformatPrint3dig(int16_t temper, char *str_out)
{
    char buff[10];

    if (temper< 0)
    {
        if (temper == -1)
            {strcpy(str_out,"N.C ");}//4posit
        return;
    }
    else
    {
        itoa(temper, buff, 10);//convierte

        //4 positions to display
        strcpy(str_out,"   C");

        if (temper< 10)
        {
            strncpy(&str_out[2], buff, 1);
        }
        else if (temper<100)
        {
            strncpy(&str_out[1], buff, 2);
        }
        else if (temper<1000)
        {
            strncpy(&str_out[0], buff, 3);
        }
        else
        {
            strncpy(&str_out[0], buff, 4);
        }
    }
}
#endif
/*****************************************************
*****************************************************/
#ifdef MAX6675_UTILS_LCD_PRINTCOMPLETE_C
void MAX6675_convertIntTmptr2str_wformatPrintComplete(int16_t temper, char *str_out)
{
    char buff[10];

    if (temper < 0)
    {
        if (temper == -1)
            {strcpy(str_out,"N.C  ");}
        return;
    }
    else
    {
        itoa(temper, buff, 10);

        //5 positions to display: 1023 + C
        strcpy(str_out,"    C");

        if (temper< 10)
        {
            strncpy(&str_out[3], buff, 1);
        }
        else if (temper<100)
        {
            strncpy(&str_out[2], buff, 2);
        }
        else if (temper<1000)
        {
            strncpy(&str_out[1], buff, 3);
        }
        else
        {
            strncpy(&str_out[0], buff, 4);
        }
    }
}
#endif

