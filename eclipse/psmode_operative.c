/*
 * psmode_operative.c
 *
 *  Created on: Aug 26, 2021
 *      Author: jcaf
 */
#include "main.h"

//build to print Left time mm:ss
void build_cookCycle_string(struct _t *t, char *str)
{
	char buff[20];
	itoa(t->min, buff, 10);
	paddingLeftwBlanks(buff, 2);
	strcpy(str, buff);
	strcat(str, ":");
	itoa(t->sec, buff, 10);
	paddingLeftwZeroes(buff, 2);
	strcat(str, buff);
}
void kbmode_setDefault(struct _kb_basket *kb)
{
	struct _key_prop prop = { 0 };
	prop = propEmpty;
	prop.uFlag.f.onKeyPressed = 1;

	ikb_setKeyProp(kb->startStop ,prop);
	ikb_setKeyProp(kb->sleep,prop);
	ikb_setKeyProp(kb->down ,prop);
	ikb_setKeyProp(kb->up ,prop);

//	for (int i = 0; i < KB_NUM_KEYS; i++)
//	{
//		ikb_setKeyProp(i, prop);
//	}
}
void kbmode_setEditCookCycle(struct _kb_basket *kb)
{
	struct _key_prop key_prop = { 0 };
	key_prop = propEmpty;
	key_prop.uFlag.f.onKeyPressed = 1;
	key_prop.uFlag.f.reptt = 1;
	key_prop.numGroup = 0;
	key_prop.repttTh.breakTime = (uint16_t) 200.0 / KB_PERIODIC_ACCESS;
	key_prop.repttTh.period = (uint16_t) 50.0 / KB_PERIODIC_ACCESS;

	ikb_setKeyProp(kb->down, key_prop);
	ikb_setKeyProp(kb->up, key_prop);
	//ikb_setKeyProp(KB_LYOUT_LEFT_DOWN, key_prop);
	//ikb_setKeyProp(KB_LYOUT_LEFT_UP, key_prop);
}


void cookCycle_hotUpdate(struct _t *TcookCycle_setPoint_toUpdate, struct _t *TcookCycle_setPoint_current,struct _t *Tcookcycle_timingrunning)
{
	int16_t TcookCycle_toUpdate_inSecs = (TcookCycle_setPoint_toUpdate->min * 60)+ TcookCycle_setPoint_toUpdate->sec;
	int16_t TcookCycle_setPoint_inSecs = (TcookCycle_setPoint_current->min * 60)+ TcookCycle_setPoint_current->sec;
	int16_t Trunning_inSecs = (Tcookcycle_timingrunning->min * 60)+ Tcookcycle_timingrunning->sec;

	int32_t diff_inSec = TcookCycle_toUpdate_inSecs	- (TcookCycle_setPoint_inSecs - Trunning_inSecs);

	if (diff_inSec <= 0)	//Trunc
	{
		Tcookcycle_timingrunning->min = 0;
		Tcookcycle_timingrunning->sec = 0;
	}
	else
	{
		Tcookcycle_timingrunning->min = (int) (diff_inSec / 60.0);
		Tcookcycle_timingrunning->sec = (int) (diff_inSec % 60);	//modulo;
	}
}

struct _basket basket_temp[BASKET_MAXSIZE];


char str[20];

void psmode_operative(void)
{
	//
	#define FRYER_COOKCYCLE_USER_STARTED 1
	#define FRYER_COOKCYCLE_USER_STOPPED 0
	//

	//
	if (fryer.ps_operative.sm0 == 0)
	{

	}


	for (int i=0; i<BASKET_MAXSIZE; i++)
	{
		blink_set(&fryer.basket[i].blink);

		if (fryer.basket[i].kb.mode == KBMODE_DEFAULT)
		{
			if (ikb_key_is_ready2read(fryer.basket[i].kb.startStop ) )
			{
				ikb_key_was_read(fryer.basket[i].kb.startStop);
				//
				if (fryer.basket[i].bf.user_startStop == FRYER_COOKCYCLE_USER_STOPPED)
				{
					fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STARTED;
					//
					fryer.basket[i].cookCycle.bf.on = 1;

				}
				else
				{
					fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STOPPED;
					//
					//STOP decrement timming
					fryer.basket[i].cookCycle.bf.on = 0;
					//load from eeprom
					fryer.basket[i].cookCycle.time.min = time_k[i].min;
					fryer.basket[i].cookCycle.time.sec = time_k[i].sec;
					//return to viewing decrement timing
					fryer.basket[i].cookCycle.counterTicks = 0x00;
					fryer.basket[i].cookCycle.bf.forceCheckControl = 1;
					fryer.basket[i].display.owner = DISPLAY_TIMING;
					fryer.basket[i].display.bf.print_cookCycle = 1;
					fryer.basket[i].cookCycle.bf.blinkDone = 0;
					//
					if (fryer.basket[i].display.bf.print_cookCycle == 1)
					{
						build_cookCycle_string(&fryer.basket[i].cookCycle.time, str);
						lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
						lcdan_print_string(str);
					}
				}
			}

			if (( ikb_key_is_ready2read(fryer.basket[i].kb.down) || ikb_key_is_ready2read(fryer.basket[i].kb.up) ) && \
					(fryer.basket[i].cookCycle.bf.blinkDone == 0))
			{
				//clear keys
				//ikb_key_was_read(fryer.basket[i].kb.down);
				//ikb_key_was_read(fryer.basket[i].kb.up);


				fryer.basket[i].display.bf.print_cookCycle = 0;

				//solo muestra blinkeando el valor...
				basket_temp[i].cookCycle.time.min = time_k[i].min;
				basket_temp[i].cookCycle.time.sec = time_k[i].sec;
				//++--
				blink_reset(BLINK_TOGGLE_SET_BLANK);

				fryer.basket[i].cookCycle.editcycle.bf.blinkIsActive = 1;
				fryer.basket[i].display.owner = DISPLAY_EDITCOOKCYCLE;
				//--++
				fryer.basket[i].cookCycle.editcycle.timerTimeout = 0x0000;//reset

				//change keyboard layout
				kbmode_setEditCookCycle(&fryer.basket[i].kb);
				fryer.basket[i].kb.mode = KBMODE_EDIT_COOKCYCLE;
				fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STOPPED;
				//
			}
			ikb_key_was_read(fryer.basket[i].kb.down);
			ikb_key_was_read(fryer.basket[i].kb.up);
		}
		else if (fryer.basket[i].kb.mode == KBMODE_EDIT_COOKCYCLE)
		{
			if (ikb_key_is_ready2read(fryer.basket[i].kb.down))
			{
				ikb_key_was_read(fryer.basket[i].kb.down);

				time_dec(&basket_temp[i].cookCycle.time);
				//
				blink_reset(BLINK_TOGGLE_SET_TEXT);
				fryer.basket[i].cookCycle.editcycle.timerTimeout = 0x0000;//reset
			}

			if (ikb_key_is_ready2read(fryer.basket[i].kb.up))
			{
				ikb_key_was_read(fryer.basket[i].kb.up);

				time_inc(&basket_temp[i].cookCycle.time);
				//
				blink_reset(BLINK_TOGGLE_SET_TEXT);
				fryer.basket[i].cookCycle.editcycle.timerTimeout = 0x0000;//reset
			}

			//
			if (ikb_key_is_ready2read(fryer.basket[i].kb.startStop ) )
			{
				ikb_key_was_read(fryer.basket[i].kb.startStop);
				//
				fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STARTED;;
				//
				fryer.basket[i].bf.prepareReturnToKBDefault = 1;
				//cancela blinking e inicia nuevo ciclo de coccion
				fryer.basket[i].cookCycle.bf.on = 1;
				}
			//
		}

		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		if (fryer.basket[i].cookCycle.editcycle.bf.blinkIsActive == 1)
		{
			//local blink - start
			//update blink struct through blink_set(struct _blink *b)
			if (fryer.basket[i].blink.bf.update)
			{
				fryer.basket[i].blink.bf.update = 0;
				//
				if (fryer.basket[i].blink.bf.toggle == BLINK_TOGGLE_SET_BLANK)
				{
					strcpy(str,"     ");
				}
				else
				{
					build_cookCycle_string(&basket_temp[i].cookCycle.time, str);
				}
				lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
				lcdan_print_string(str);
				//
			}
			//local blink - end

			if (mainflag.sysTickMs)
			{
				//Timeout : Limpiar teclas del basket correspodiente
				if (++fryer.basket[i].cookCycle.editcycle.timerTimeout >= EDITCYCLE_TIMERTIMEOUT_K)
				{
					fryer.basket[i].cookCycle.editcycle.timerTimeout = 0x0000;
					//
					if (fryer.basket[i].cookCycle.bf.on == 1)
					{
						cookCycle_hotUpdate(&basket_temp[i].cookCycle.time, &time_k[i], &fryer.basket[i].cookCycle.time);
						//return with fryer.basket[i].cookCycle.time UPDATED!
					}
					fryer.basket[i].bf.prepareReturnToKBDefault = 1;
					//cancela blinking y espera a un user_Start

					//clear all keys * solo por ahora seria startstop
					ikb_key_was_read(fryer.basket[i].kb.startStop);
					ikb_key_was_read(fryer.basket[i].kb.sleep);
					//
				}
			}
		}
		if (fryer.basket[i].bf.prepareReturnToKBDefault == 1)//prepare all to return to KBMODE_DEFAULT
		{
			fryer.basket[i].bf.prepareReturnToKBDefault = 0;
			//
			//update eeprom
			time_k[i] = basket_temp[i].cookCycle.time;//update new cookCycle set-point
			//load from eeprom
			fryer.basket[i].cookCycle.time.min = time_k[i].min;
			fryer.basket[i].cookCycle.time.sec = time_k[i].sec;
			fryer.basket[i].cookCycle.editcycle.bf.blinkIsActive = 0;
			//return to visualizing decrement-timing
			fryer.basket[i].cookCycle.counterTicks = 0x00;//reset counter
			fryer.basket[i].cookCycle.bf.forceCheckControl = 1;//forzar pase directo para poder visualizar y actuar si es 00:00
			fryer.basket[i].display.owner = DISPLAY_TIMING;
			fryer.basket[i].display.bf.print_cookCycle = 1;
			//return to kb
			kbmode_setDefault(&fryer.basket[i].kb);
			fryer.basket[i].kb.mode = KBMODE_DEFAULT;
		}

		//
		if (fryer.basket[i].cookCycle.bf.on == 1)
		{
			if (mainflag.sysTickMs)
			{
				if (++fryer.basket[i].cookCycle.counterTicks == 100)//update cada 1s
				{
					fryer.basket[i].cookCycle.counterTicks = 0x00;
					//
					time_dec(&fryer.basket[i].cookCycle.time);
					fryer.basket[i].cookCycle.bf.forceCheckControl = 1;
					if (fryer.basket[i].display.owner == DISPLAY_TIMING)
					{
						fryer.basket[i].display.bf.print_cookCycle = 1;
					}
					//
				}
			}
			//
			if (fryer.basket[i].cookCycle.bf.forceCheckControl == 1)
			{
				if (fryer.basket[i].cookCycle.time.sec == 0)
				{
					if (fryer.basket[i].cookCycle.time.min == 0)
					{
						fryer.basket[i].cookCycle.bf.on = 0;
						//
						//PinTo1(PORTWxBUZZER, PINxBUZZER);__delay_ms(15);PinTo0(PORTWxBUZZER, PINxBUZZER);
						blink_reset(BLINK_TOGGLE_SET_TEXT);
						fryer.basket[i].display.bf.print_cookCycle = 0;
						fryer.basket[i].cookCycle.bf.blinkDone = 1;
					}
				}

				fryer.basket[i].cookCycle.bf.forceCheckControl = 0x00;
			}
		}

		//print timing decrement mm:ss
		if (fryer.basket[i].display.owner == DISPLAY_TIMING)
		{
			if (fryer.basket[i].display.bf.print_cookCycle == 1)
			{
				build_cookCycle_string(&fryer.basket[i].cookCycle.time, str);
				lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
				lcdan_print_string(str);
				//
				fryer.basket[i].display.bf.print_cookCycle = 0;
			}
			//blink
			if (fryer.basket[i].cookCycle.bf.blinkDone == 1)
			{
				if (fryer.basket[i].blink.bf.toggle == BLINK_TOGGLE_SET_BLANK)
				{
					strcpy(str,"     ");
				}
				else//BLINK_TOGGLE_SET_TEXT
				{
					strcpy(str,"DONE ");
				}
				lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
				lcdan_print_string(str);

			}
		}
		//
		//+++++++++++++++++++++++++++++++++++++++++++++++++
		if (mainflag.sysTickMs)
		{
			blink_timing();
		}
		//+++++++++++++++++++++++++++++++++++++++++++++++++
	}//for
	//
	if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
	{
		ikb_key_was_read(KB_LYOUT_PROGRAM);

		fryer.psmode = PSMODE_PROGRAM;
		fryer.ps_program = ps_reset;

	}

}