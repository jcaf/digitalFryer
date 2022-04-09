/*
 * psmode_viewTemp.c
 *
 *  Created on: Apr 7, 2022
 *      Author: jcaf
 */

#include "main.h"
#include "psmode_program.h"
#include "psmode_operative.h"
#include "Temperature/temperature.h"
#include "indicator/indicator.h"

//static struct _blink blink;
//struct _pgrmmode pgrmode;


int8_t psmode_viewTemp(void)
{
	char codret = 0;
	char str[10];

	//fryer.ps_program.sm0 avanza por la tecla e internamente
	if (fryer.ps_viewTemp.sm0 == 0)
	{
		struct _key_prop key_prop = { 0 };
		key_prop = propEmpty;
		key_prop.uFlag.f.onKeyPressed = 1;
		for (int i=0; i<BASKET_MAXSIZE; i++)
		{
			ikb_setKeyProp(fryer.basket[i].kb.program ,key_prop);//
		}
		lcdan_clear();

		fryer.ps_viewTemp.sm0++;
	}
	else if (fryer.ps_viewTemp.sm0 == 1)
	{
		lcdan_set_cursor(DISP_CURSOR_BASKETLEFT_START_X, 0);
		lcdan_print_string("OIL  ");
		//
		lcdan_set_cursor(0x0E, 0);//PRINT SYMBOL DEGREE
		lcdan_write_data(0b11011111);//

		if (pgrmode.bf.unitTemperature == FAHRENHEIT)
		{
			lcdan_write_data('F');
		}
		else
		{
			lcdan_write_data('C');
		}
		fryer.ps_viewTemp.sm0++;
	}
	else if (fryer.ps_viewTemp.sm0 == 2)
	{
		MAX6675_formatText3dig(TCtemperature, str);
		lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);
		lcdan_print_string(str);
	}
	else if (fryer.ps_viewTemp.sm0 == 3)
	{
		fryer.ps_viewTemp.sm0 = 0;
		//
		if (fryer.ps_viewTemp.sm0 == 3)//EXIT
		{
			codret = 1;
			fryer.ps_viewTemp = ps_reset;
			fryer.ps_program = ps_reset;
			fryer.ps_operative = ps_reset;
		}
	}

	//al ultimo q evalue KB PROGRAM
	if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
	{
		ikb_key_was_read(KB_LYOUT_PROGRAM);

		fryer.ps_viewTemp.sm0++;
		//
		indicator_setKSysTickTime_ms(75/SYSTICK_MS);
		indicator_On();
	}

	return codret;
}
