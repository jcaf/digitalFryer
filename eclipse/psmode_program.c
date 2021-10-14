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
static struct _blink blink;

struct _pgrmmode pgrmode;

struct _Tcoccion Tcoccion;

int blinkIsActive=0;

void psmode_program(void)
{
	char str[10];

	//fryer.ps_program.sm0 avanza por la tecla e internamente
	if (fryer.ps_program.sm0 == 0)
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
	else if (fryer.ps_program.sm0 == 1)
	{
		MAX6675_formatText3dig(TCtemperature, str);
		lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);
		lcdan_print_string(str);
	}
	else if (fryer.ps_program.sm0 == 2)
	{
		lcdan_set_cursor(DISP_CURSOR_BASKETLEFT_START_X, 0);
		lcdan_print_string("SET  ");
		fryer.ps_program.sm0++;
	}
	else if (fryer.ps_program.sm0 == 3)
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

				if (--Tcoccion.TC <= Tcoccion.min)
				{
					Tcoccion.TC = Tcoccion.min;
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

				if (++Tcoccion.TC >= Tcoccion.max)
				{
					Tcoccion.TC = Tcoccion.max;
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
				MAX6675_formatText3dig(Tcoccion.TC, str);
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
		if (fryer.ps_program.sm0 == 2)
		{
			blink.timerBlink_K = PSMODE_PROGRAM_BLINK_TIMER_KMAX;
			blink_set(&blink);
			blinkIsActive = 1;
		}
		else if (fryer.ps_program.sm0 == 4)//EXIT
		{
			fryer.ps_program = ps_reset;

			fryer.psmode = PSMODE_OPERATIVE;
			fryer.ps_operative = ps_reset;
			psmode_operative_init();

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

}
