/*
 * psmode_operative.c
 *
 *  Created on: Aug 26, 2021
 *      Author: jcaf
 */
#include "main.h"
#include "indicator/indicator.h"

#define PSMODE_OPERATIVE_BLINK_TIMER_KMAX (400/SYSTICK_MS)			//Xms/10ms de acceso
#define PSMODE_OPERATIVE_EDITCYCLE_TIMERTIMEOUT_K (3000/SYSTICK_MS)	//5000ms/10ms

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

struct _basket basket_temp[BASKET_MAXSIZE];//basket temp mantiene en todo el programa el valor guardado en la EEPROM desde q arranca la aplicacion

void psmode_operative_init(void)
{
	char str[20];

	//++--
	for (int i=0; i<BASKET_MAXSIZE; i++)
	{
		fryer.basket[i].blink.timerBlink_K =  PSMODE_OPERATIVE_BLINK_TIMER_KMAX;
		//
		eeprom_read_block((struct _t *)(&fryer.basket[i].cookCycle.time), (struct _t *)(&COOKTIME[i]), sizeof(struct _t));
		//carga desde la eeprom de manera temporal
		basket_temp[i].cookCycle.time = fryer.basket[i].cookCycle.time;


		//return to visualizing decrement-timing
		fryer.basket[i].cookCycle.counterTicks = 0x00;//reset counter
		fryer.basket[i].cookCycle.bf.forceCheckControl = 1;//forzar pase directo para poder visualizar y actuar si es 00:00


		fryer.basket[i].display.owner = DISPLAY_TIMING;
		fryer.basket[i].display.bf.print_cookCycle = 1;
		//
		//if (fryer.basket[i].display.bf.print_cookCycle == 1)
		if ((fryer.basket[i].display.bf.print_cookCycle == 1)	&& (fryer.viewmode == FRYER_VIEWMODE_COOK))
		{
			build_cookCycle_string(&fryer.basket[i].cookCycle.time, str);
			lcdan_clear();
			lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
			lcdan_print_string(str);
		}

		//
		kbmode_default(&fryer.basket[i].kb);
		fryer.basket[i].kbmode = KBMODE_DEFAULT;
	}
	//--+
}

#define FRYER_COOKCYCLE_USER_STARTED 1
#define FRYER_COOKCYCLE_USER_STOPPED 0

void p1(void)
{
	char str[20];
	for (int i=0; i<BASKET_MAXSIZE; i++)
	{
		blink_set(&fryer.basket[i].blink);

		if ((ikb_key_is_ready2read(fryer.basket[i].kb.down)) && (fryer.basket[i].cookCycle.bf.blinkDone == 0))
		{
			//ikb_key_was_read(fryer.basket[i].kb.down);
			if (!ikb_inReptt(fryer.basket[i].kb.down))
			{
				indicator_setKSysTickTime_ms(75/SYSTICK_MS);
				indicator_On();
			}

			//
			//
			fryer.basket[i].display.bf.print_cookCycle = 0;
			//fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STOPPED;//preserva el estado actual si esta corriendo el temporizador o no
			fryer.basket[i].cookCycle.editcycle.bf.blinkIsActive = 1;
			fryer.basket[i].kbmode = KBMODE_EDIT_COOKCYCLE;
			fryer.basket[i].display.owner = DISPLAY_EDITCOOKCYCLE;
			fryer.basket[i].cookCycle.editcycle.timerTimeout = 0x0000;//reset
			blink_reset(BLINK_TOGGLE_SET_TEXT);
			//

			time_dec(&basket_temp[i].cookCycle.time);
		}
		if ((ikb_key_is_ready2read(fryer.basket[i].kb.up))  && (fryer.basket[i].cookCycle.bf.blinkDone == 0))
		{
			//ikb_key_was_read(fryer.basket[i].kb.up);
			if (!ikb_inReptt(fryer.basket[i].kb.up))
			{
				indicator_setKSysTickTime_ms(75/SYSTICK_MS);
				indicator_On();
			}
			//
			fryer.basket[i].display.bf.print_cookCycle = 0;
			//fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STOPPED;
			fryer.basket[i].cookCycle.editcycle.bf.blinkIsActive = 1;
			fryer.basket[i].kbmode = KBMODE_EDIT_COOKCYCLE;
			fryer.basket[i].display.owner = DISPLAY_EDITCOOKCYCLE;
			fryer.basket[i].cookCycle.editcycle.timerTimeout = 0x0000;//reset
			blink_reset(BLINK_TOGGLE_SET_TEXT);
			//

			time_inc(&basket_temp[i].cookCycle.time);
		}
		//clear
		ikb_key_was_read(fryer.basket[i].kb.down);
		ikb_key_was_read(fryer.basket[i].kb.up);


		if (fryer.basket[i].kbmode == KBMODE_DEFAULT)
		{
			if (ikb_key_is_ready2read(fryer.basket[i].kb.startStop ) )
			{
				ikb_key_was_read(fryer.basket[i].kb.startStop);
				//
				indicator_setKSysTickTime_ms(1000/SYSTICK_MS);
				indicator_On();
				//
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
					eeprom_read_block((struct _t *)(&fryer.basket[i].cookCycle.time), (struct _t *)(&COOKTIME[i]), sizeof(struct _t));

					//return to viewing decrement timing
					fryer.basket[i].cookCycle.counterTicks = 0x00;
					fryer.basket[i].cookCycle.bf.forceCheckControl = 1;
					fryer.basket[i].display.owner = DISPLAY_TIMING;
					fryer.basket[i].display.bf.print_cookCycle = 1;
					fryer.basket[i].cookCycle.bf.blinkDone = 0;
					//
					//if (fryer.basket[i].display.bf.print_cookCycle == 1)
					if ((fryer.basket[i].display.bf.print_cookCycle == 1) && (fryer.viewmode == FRYER_VIEWMODE_COOK))
					{
						build_cookCycle_string(&fryer.basket[i].cookCycle.time, str);
						lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
						lcdan_print_string(str);
					}
				}
			}
		}
		else if (fryer.basket[i].kbmode == KBMODE_EDIT_COOKCYCLE)
		{
			//cancela blinking e inicia nuevo ciclo de coccion
			if (ikb_key_is_ready2read(fryer.basket[i].kb.startStop ) )
			{
				ikb_key_was_read(fryer.basket[i].kb.startStop);
				//
				fryer.basket[i].bf.prepareReturnToKBDefault = 1;
				fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STARTED;
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
				if (fryer.viewmode == FRYER_VIEWMODE_COOK)//add 2022
				{
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
				}
				//
			}
			//local blink - end

			if (mainflag.sysTickMs)
			{
				//Timeout : Limpiar teclas del basket correspodiente
				//cancela blinking y espera a un user_Start

				if (++fryer.basket[i].cookCycle.editcycle.timerTimeout >= PSMODE_OPERATIVE_EDITCYCLE_TIMERTIMEOUT_K)
				{
					fryer.basket[i].cookCycle.editcycle.timerTimeout = 0x0000;
					//
					fryer.basket[i].bf.prepareReturnToKBDefault = 1;

					if (fryer.basket[i].cookCycle.bf.on == 1)//osea en caliente se cambio el setpoint
					{
						cookCycle_hotUpdate(&basket_temp[i].cookCycle.time, &COOKTIME[i], &fryer.basket[i].cookCycle.time);
						//return with fryer.basket[i].cookCycle.time UPDATED!

					}
					else
					{
						fryer.basket[i].bf.user_startStop = FRYER_COOKCYCLE_USER_STOPPED;
					}

					//clear all keys * solo por ahora seria startstop
					//ikb_key_was_read(fryer.basket[i].kb.startStop);
					//ikb_key_was_read(fryer.basket[i].kb.sleep);
					//
				}
			}
		}
		if (fryer.basket[i].bf.prepareReturnToKBDefault == 1)//prepare all to return to KBMODE_DEFAULT
		{
			fryer.basket[i].bf.prepareReturnToKBDefault = 0;
			//
			//update eeprom
			//COOKTIME[i] = basket_temp[i].cookCycle.time;//update new cookCycle set-point
			eeprom_update_block((struct _t *)(&basket_temp[i].cookCycle.time), (struct _t *)(&COOKTIME[i]), sizeof(struct _t));

			//load from eeprom
			fryer.basket[i].cookCycle.time = basket_temp[i].cookCycle.time;


			fryer.basket[i].cookCycle.editcycle.bf.blinkIsActive = 0;
			//return to visualizing decrement-timing
			fryer.basket[i].cookCycle.counterTicks = 0x00;//reset counter
			fryer.basket[i].cookCycle.bf.forceCheckControl = 1;//forzar pase directo para poder visualizar y actuar si es 00:00
			fryer.basket[i].display.owner = DISPLAY_TIMING;
			fryer.basket[i].display.bf.print_cookCycle = 1;
			//return to kb
			//kbmode_setDefault(&fryer.basket[i].kb);
			fryer.basket[i].kbmode = KBMODE_DEFAULT;
		}


	}
}
void p2(void)
{

	//process
	for (int i=0; i<BASKET_MAXSIZE; i++)
	{
		blink_set(&fryer.basket[i].blink);

		if (fryer.basket[i].cookCycle.bf.on == 1)
		{
			if (mainflag.sysTickMs)
			{
				if (++fryer.basket[i].cookCycle.counterTicks == (1000/SYSTICK_MS) )//update cada 1s
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
						//
						indicator_setKSysTickTime_ms(1000/SYSTICK_MS);
						indicator_On();
					}
				}

				fryer.basket[i].cookCycle.bf.forceCheckControl = 0x00;
			}
		}



	}
}
void p3(void)
{
	char str[20];
	for (int i=0; i<BASKET_MAXSIZE; i++)
	{
		blink_set(&fryer.basket[i].blink);

		//print timing decrement mm:ss
		//if (fryer.basket[i].display.owner == DISPLAY_TIMING)
		if ((fryer.basket[i].display.owner == DISPLAY_TIMING) && (fryer.viewmode == FRYER_VIEWMODE_COOK))
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



	}
}

void psmode_operative(void)
{
	if (fryer.ps_operative.sm0 == 0)
	{
		psmode_operative_init();
		fryer.ps_operative.sm0++;
	}


	if (fryer.viewmode == FRYER_VIEWMODE_COOK)
	{
		p1();
	}

	p2();

	if (fryer.viewmode == FRYER_VIEWMODE_COOK)
	{
		p3();
	}

	if (fryer.viewmode == FRYER_VIEWMODE_COOK)
	{
		for (int i=0; i<BASKET_MAXSIZE; i++)
		{
			blink_set(&fryer.basket[i].blink);
			//
			//+++++++++++++++++++++++++++++++++++++++++++++++++
			//es necesario que este dentro del for() xq se cambia el puntero
			//de la estructura correspondiente a cada canastilla
			if (mainflag.sysTickMs)
			{
				blink_timing();
			}
			//+++++++++++++++++++++++++++++++++++++++++++++++++

		}
	}

	if (fryer.viewmode == FRYER_VIEWMODE_COOK)
	{
		if (ikb_getKeyStartPressed(KB_LYOUT_PROGRAM))
		{
			ikb_clearKeyStartPressed(KB_LYOUT_PROGRAM);
			//
			indicator_setKSysTickTime_ms(75/SYSTICK_MS);
			indicator_On();
		}
	}
}
