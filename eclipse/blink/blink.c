/*
 * blink.c
 *
 *  Created on: Mar 5, 2021
 *      Author: jcaf
 */
#include "blink.h"

struct _blink *blink;

void blink_set(struct _blink *b)
{
	blink = b;//set address
}

void blink_reset(int8_t BLINK_TOGGLE_STATE)
{
	blink->timerBlink = 0x00;
	blink->bf.update = 1;//BLINK_UPDATE;
	//blink->bf.toggle = !BLINK_TOGGLE_BLANK;//mejor q sea 0
	blink->bf.toggle = BLINK_TOGGLE_STATE;
}

void blink_timing(void)
{
	if (++blink->timerBlink >= blink->timerBlink_K)
	{
		blink->timerBlink = 0x00;
		//
		blink->bf.toggle = !blink->bf.toggle;
		blink->bf.update = 1;//BLINK_UPDATE;
	}
}

/*
void ff(int i, )//ON DEMAND/// not job
{
	if (blinkIsActive == 1)
	{
		if (blink.bf.update == BLINK_UPDATE)//entrar y mostrar directamente bypaseando el turno por el timer
		{
			blink.bf.update = 0;
			//
			if (fryer.basket[i].blink.bf.toggle == BLINK_TOGGLE_SET_BLANK)
			{
				blink_clear(fx(i));//callback
			}
			else//BLINK_TOGGLE_SET_TEXT
			{
				blink_fill(fx(i));
			}
			blink_print_display(fx(i));
			//
		}
	}
}

 */
