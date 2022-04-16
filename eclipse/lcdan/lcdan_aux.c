/*
 * File:   lcdan_aux.c
 * Author: jcaf
 *
 * Created on February 12, 2015, 10:19 AM
 */

//#include <xc.h>
#include <avr/io.h>
#include <stdint.h>
#include "../system.h"
#include "lcdan.h"
#include "lcdan_aux.h"


void lcdan_print_string(const char *p)
{
    while (*p)
    {
        lcdan_write_data(*p);
        p++;
    }
}
#if defined(__GNUC__) && defined(__AVR__)
#include <avr/pgmspace.h>
void lcdan_print_PSTRstring(const char *p)
{
    char c;

    while (1)
    {
        c = pgm_read_byte_near(p);
        if (c == '\0')
            break;
        else
            lcdan_write_data(c);
        p++;
    }
}
#endif
//display lcd

//x = col
//y = row


void lcdan_set_cursor(int8_t x, int8_t y)
{
    int8_t xy = LCDAN_BASEADDR_ROW_0;

    if (y > LCDAN_ROW-1)
        {y = LCDAN_ROW-1;}

    if (y == 0)
        xy = LCDAN_BASEADDR_ROW_0 + x;
    else if (y == 1)
        xy = LCDAN_BASEADDR_ROW_1 + x;
    else if (y == 2)
        xy = LCDAN_BASEADDR_ROW_2 + x;
    else if (y == 3)
        xy = LCDAN_BASEADDR_ROW_3 + x;

    lcdan_write_cmd(xy);
}

//ADDED

int8_t lcdan_str_get_center_hor(const char *str)
{
    int8_t pos = 0;
    uint8_t length = strlen(str);

    if (length < LCDAN_COL)
        pos = (LCDAN_COL - length) >> 1; //2

    return pos;
}
uint8_t lcdan_str_get_align_col(const char *str_src, int8_t col_align)
{
    if (col_align == LCDAN_STR_FORMAT_ALIGN_CENTER)
    {
        return  lcdan_str_get_center_hor(str_src);
    }
    else
    {
        return 0;
    }
    //else if ()//LEFT RIGHT
}
///////////////////////// unificar con sysPrint ////////////////////////////////
//special to print VSCROLL over LCD 20X4

//el programador sera responsable de:
//if (pos_i= 0) && (str_src >COL)
//--> no align, untouch str_src

//ojo trunca al tamano de columnas imprimibles en LCD alfanu..
//CONDITION: strlen(str_dest) >= LCDAN_STR_MAXSIZE
////////////////////////////////////////////////////////////////////////////////
void lcdan_str_lineformat_align(char *str_dest,  const char *str_src, int8_t pos_i /* = 0 */)
{
    int8_t i, pos_f, x;

    str_dest[LCDAN_STR_MAXSIZE-1] = '\0';         //[''   ...''0] posc[20]='\0'
    for (i = LCDAN_STR_MAXSIZE-2; i >= 0; i--)    //blank all elements
        str_dest[i] = ' ';

    if (pos_i < 0)
        pos_i = lcdan_str_get_align_col(str_src, LCDAN_STR_FORMAT_ALIGN_CENTER);

    pos_f = pos_i + strlen(str_src);

    if (pos_f > LCDAN_COL)
        pos_f = LCDAN_COL;//trunc to max pos= LCD_COL

    x = 0;
    for (i = pos_i; i < pos_f; i++, x++)
        str_dest[i] = str_src[x];
}

//////////////////////////////////////////////////////////
void lcdan_str_lineformat_align_P(char *str_dest,  const char *str_src, int8_t pos_i /* = 0 */)//default uint8_t posc_i=0
{
    char _str_src[LCDAN_STR_MAXSIZE];
    strncpy_P(_str_src,  (const char *)str_src, LCDAN_STR_MAXSIZE);
    _str_src[LCDAN_STR_MAXSIZE-1] = '\0';

    lcdan_str_lineformat_align(str_dest, (const char *) _str_src, pos_i);//default uint8_t posc_i=0
}
//////////////////////////////////////////////////////////
//3dot at end of line
//todos los arrays/buffers q imprimen en LCD DEBEN DE SER >= LCD_COL+1
void lcdan_str_lineformat_trimEOL3dots(char *str)
{
    str[LCDAN_COL-3] = '.';
    str[LCDAN_COL-2] = '.';
    str[LCDAN_COL-1] = '.';
    str[LCDAN_COL] = '\0';
}




//+++++++++++++++++++++++++++++++++++++
//char lcdanBuff[LCDAN_COL,LCDAN_ROW];
//int8_t col,row;
/*
*/


void lcdanBuff_clear(char str_dest[LCDAN_ROW][LCDAN_COL])
{
	for (int r=0; r<LCDAN_ROW; r++)
	{
		for (int c=0; c<LCDAN_COL; c++)
		{
			str_dest[r][c]=' ';
		}
	}
}
/*
*/
//void lcdanBuff_set_cursor(int8_t c, int8_t r)
//{
//	col = c;
//	row = r;
//}
/*
void lcdanBuff_print_string(int8_t col, char * str_dest, const char * str_src)
{
	int8_t i=0;
	while (str_src[i] != '\0')
	{
		if ( col >= LCDAN_COL)
		{
			//str_dest[LCDAN_COL]	= '\0';
			break;
		}
		else
		{
			str_dest[col] = str_src[i];
		}
		col++;
		i++;
	}
}
*/
void lcdanBuff_print_string(int8_t col, int8_t row, char str_dest[LCDAN_ROW][LCDAN_COL], const char * str_src)
{
	int8_t i=0;

	if (row>=LCDAN_ROW)
		row = (LCDAN_ROW-1);

	while (str_src[i] != '\0')
	{
		if ( col >= LCDAN_COL)
		{
			//str_dest[LCDAN_COL]	= '\0';
			break;
		}
		else
		{
			str_dest[row][col] = str_src[i];
		}
		col++;
		i++;
	}
}

void lcdanBuff_dump2device(const char str_src[LCDAN_ROW][LCDAN_COL])
{
	const uint8_t lcdan_address_4row[4]=
	{
			LCDAN_BASEADDR_ROW_0,
			LCDAN_BASEADDR_ROW_1,
			LCDAN_BASEADDR_ROW_2,
			LCDAN_BASEADDR_ROW_3,
	};

	for (int8_t r=0; r<LCDAN_ROW; r++)
	{
		lcdan_write_cmd(lcdan_address_4row[r]);//set cursor
		for (int8_t c=0; c<LCDAN_COL; c++)
		{
			lcdan_write_data(str_src[r][c]);
		}
	}
}


