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

	void blink_reset(void)
	{
		blink->timerBlink = 0x00;
		blink->bf.bypass = BLINK_BYPASS_TIMER;
		blink->bf.toggle = BLINK_TOGGLE_BLANK;//mejor q sea 0
	}
	void blink_timing(void)
	{
		if (++blink->timerBlink >= blink->timerBlink_K)
		{
			blink->timerBlink = 0x00;
			//
			blink->bf.toggle = !blink->bf.toggle;
			blink->bf.bypass = BLINK_BYPASS_TIMER;
		}

	}
