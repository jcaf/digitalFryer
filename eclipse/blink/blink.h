/*
 * blink.h
 *
 *  Created on: Mar 5, 2021
 *      Author: jcaf
 */

#ifndef BLINK_BLINK_H_
#define BLINK_BLINK_H_

#include <stdint.h>
	struct _blink
	{
		uint8_t timerBlink;
		uint8_t timerBlink_K;

		struct _bf_blink
		{
			unsigned toggle:1;
			unsigned bypass:1;
			unsigned __a;
		}bf;
	};

		void blink_set(struct _blink *b);
		void blink_timing(void);

	#define BLINK_TOGGLE_BLANK 1
	#define BLINK_BYPASS_TIMER 1

#endif /* BLINK_BLINK_H_ */
