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
			unsigned update:1;
			unsigned __a;
		}bf;
	};

		void blink_reset(int8_t BLINK_TOGGLE_STATE);
		void blink_set(struct _blink *b);
		void blink_timing(void);

	#define BLINK_TOGGLE_SET_BLANK 0	//Mostrar directamente blanco
	#define BLINK_TOGGLE_SET_TEXT 1		//Mostrar diretamente el texto

	//#define BLINK_UPDATE 1

#endif /* BLINK_BLINK_H_ */
