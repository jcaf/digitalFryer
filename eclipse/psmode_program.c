/*
 * psmode_program.c
 *
 *  Created on: Aug 24, 2021
 *      Author: jcaf
 */
#include "main.h"
#include "Temperature/temperature.h"

void psmode_program(void)
{
	char TCtemperature_str[10];

	if (fryer.ps_program.sm0 == 0)
	{
		lcdan_set_cursor(DISP_CURSOR_BASKETLEFT_START_X, 0);
		lcdan_print_string("OIL");
		fryer.ps_program.sm0++;
	}
	else if (fryer.ps_program.sm0 == 1)
	{
		MAX6675_formatText3dig(TCtemperature, TCtemperature_str);

		lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);
		lcdan_print_string(TCtemperature_str);
	}
	else if (fryer.ps_program.sm0 == 2)
	{
		lcdan_set_cursor(DISP_CURSOR_BASKETLEFT_START_X, 0);
		lcdan_print_string("SET");
		fryer.ps_program.sm0++;
	}
	else if (fryer.ps_program.sm0 == 3)
	{
		lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);

	}
/*
	//local blink - start
	//update blink struct through blink_set(struct _blink *b)
	if (.blink.bf.update)
	{
		.blink.bf.update = 0;
		//
		if (.blink.bf.toggle == BLINK_TOGGLE_SET_BLANK)
		{
			strcpy(str,"     ");
		}
		else
		{
			//build_cookCycle_string(&basket_temp[i].cookCycle.time, str);
		}
		//lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
		//lcdan_print_string(str);
		//
	}
	//local blink - end

*/
	//--------------------------------------------- al ultimo q evalue KB PROGRAM
	if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
	{
		ikb_key_was_read(KB_LYOUT_PROGRAM);

		if (++fryer.ps_program.sm0 == 4)//EXIT
		{
			fryer.psmode = PSMODE_PROGRAM;
			fryer.ps_program = ps_reset;
		}
	}
	//
}
