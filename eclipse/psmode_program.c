/*
 * psmode_program.c
 *
 *  Created on: Aug 24, 2021
 *      Author: jcaf
 */
#include "main.h"
#include "psmode_program.h"
#include "psmode_operative.h"
#include "Temperature/temperature.h"
#include "indicator/indicator.h"

#define PSMODE_PROGRAM_BLINK_TIMER_KMAX (400/SYSTICK_MS)//Xms/10ms de acceso


struct _pgrmmode pgrmode;

/* Farenheiths*/
struct _Tcoccion EEMEM TMPRTURE_COCCION = {
		.TC = 350,
		.max = 450,
		.min = 50
};
struct _Tcoccion tmprture_coccion;

int blinkIsActive=0;



int8_t psmode_program(void)
{
	char codret = 0;
	char str[10];
	//
	static struct _blink blink;
	blink_set(&blink);
	//

	//fryer.ps_program.sm0 avanza por la tecla e internamente
	if (fryer.ps_program.sm0 == 0)
	{
		struct _key_prop key_prop = { 0 };
		key_prop = propEmpty;
		key_prop.uFlag.f.onKeyPressed = 1;
		for (int i=0; i<BASKET_MAXSIZE; i++)
		{
			ikb_setKeyProp(fryer.basket[i].kb.program ,key_prop);//
		}
		lcdan_clear();

		fryer.ps_program.sm0++;
		//
		blink.timerBlink_K = PSMODE_PROGRAM_BLINK_TIMER_KMAX;

	}
	else if (fryer.ps_program.sm0 == 1)
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
		fryer.ps_program.sm0++;
	}
	else if (fryer.ps_program.sm0 == 2)
	{
		MAX6675_formatText3dig(TCtemperature, str);
		lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);
		lcdan_print_string(str);
	}
	else if (fryer.ps_program.sm0 == 3)
	{
		lcdan_set_cursor(DISP_CURSOR_BASKETLEFT_START_X, 0);
		lcdan_print_string("SET  ");
		fryer.ps_program.sm0++;
	}
	else if (fryer.ps_program.sm0 == 4)
	{
		for (int i=0; i<BASKET_MAXSIZE; i++)
		{
			if (ikb_key_is_ready2read(fryer.basket[i].kb.down))
			{
				ikb_key_was_read(fryer.basket[i].kb.down);
				//
				if (!ikb_inReptt(fryer.basket[i].kb.down))
				{
					indicator_setKSysTickTime_ms(75/SYSTICK_MS);
					indicator_On();
				}
				//

				blink_reset(BLINK_TOGGLE_SET_TEXT);

				if (--tmprture_coccion.TC <= tmprture_coccion.min)
				{
					tmprture_coccion.TC = tmprture_coccion.min;
				}
			}
			if (ikb_key_is_ready2read(fryer.basket[i].kb.up))
			{
				ikb_key_was_read(fryer.basket[i].kb.up);
				//
				if (!ikb_inReptt(fryer.basket[i].kb.up))
				{
					indicator_setKSysTickTime_ms(75/SYSTICK_MS);
					indicator_On();
				}
				//

				blink_reset(BLINK_TOGGLE_SET_TEXT);

				if (++tmprture_coccion.TC >= tmprture_coccion.max)
				{
					tmprture_coccion.TC = tmprture_coccion.max;
				}
			}
		}
	}

	//--------------------------------------------
	if (blinkIsActive)
	{
		if (blink.bf.update)
		{
			blink.bf.update = 0;
			//
			if (blink.bf.toggle == BLINK_TOGGLE_SET_BLANK)
			{
				strcpy(str,"   ");
			}
			else
			{
				MAX6675_formatText3dig(tmprture_coccion.TC, str);
			}
			lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);
			lcdan_print_string(str);
		}
	}


	//al ultimo q evalue KB PROGRAM
	if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
	{
		ikb_key_was_read(KB_LYOUT_PROGRAM);

		fryer.ps_program.sm0++;
		//
		if (fryer.ps_program.sm0 == 3)
		{
			//blink.timerBlink_K = PSMODE_PROGRAM_BLINK_TIMER_KMAX;
			//blink_set(&blink);
			blinkIsActive = 1;
		}
		else if (fryer.ps_program.sm0 == 5)//EXIT
		{
			codret = 1;

			eeprom_update_block((struct _Tcoccion *)&tmprture_coccion , (struct _Tcoccion *)&TMPRTURE_COCCION, sizeof(struct _Tcoccion) );

			blinkIsActive = 0;
		}
		indicator_setKSysTickTime_ms(75/SYSTICK_MS);
		indicator_On();
	}
	//
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	if (mainflag.sysTickMs)
	{
		blink_timing();
	}
	//+++++++++++++++++++++++++++++++++++++++++++++++++

	return codret;
}
