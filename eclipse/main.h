/*
 * main.h
 *
 *  Created on: Dec 1, 2020
 *      Author: jcaf
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "types.h"
#include "system.h"
#include "lcdan/lcdan.h"
#include "lcdan/lcdan_aux.h"
#include "ikb/ikb.h"
#include "timing/timing.h"
#include "utils/utils.h"
#include "blink/blink.h"
#include "MAX6675/MAX6675.h"
#include "Temperature/temperature.h"
#include "SPI/SPI.h"


#define DISP_CURSOR_BASKETLEFT_START_X 0x00
#define DISP_CURSOR_BASKETRIGHT_START_X 0x0B

enum _PSMODE
{
	PSMODE_OPERATIVE,
	PSMODE_PROGRAM,
};

enum E_KBMODE
{
	KBMODE_DEFAULT = 0,
	KBMODE_EDIT_COOKCYCLE,
	KBMODE_EDIT_TSETPOINT
};
enum DISPLAY_OWNER
{
	DISPLAY_TIMING = 0,
	DISPLAY_EDITCOOKCYCLE,
};

struct _pgrmmode
{
		struct _pgrmmode_bf
		{
				unsigned unitTemperature:1;//Degrees Fahrenheit or Celsius
				unsigned __a:7;//reserved
		}bf;
		int8_t numBaskets;//Default=1;
		int8_t language;//Languages, default SPANISH

};

struct _basket
{
		struct _display_basket
		{
				struct _cursor
				{
						int8_t x;
						int8_t y;
				}cursor;

				struct _bf_display_basket
				{
						unsigned print_cookCycle :1;
						unsigned __a:7;
				}bf;

				int8_t owner;
		}display;

		struct _kb_basket
		{
				int8_t startStop;
				int8_t sleep;
				int8_t down;
				int8_t up;
				//
				int8_t mode;
		} kb;

		struct _cookCycle
		{
				struct _t time;
				int8_t counterTicks;

				struct _bf_cookCycle
				{
						unsigned on :1;
						unsigned forceCheckControl :1;
						unsigned blinkDone :1;
						unsigned __a:5;
				} bf;

				struct _editcycle
				{
						int16_t timerTimeout;

						struct _bf_editcycle
						{
								unsigned blinkIsActive :1;
								unsigned __a:7;
						} bf;

				} editcycle;

		} cookCycle;

		struct _bf_basket
		{
				unsigned user_startStop:1;
				unsigned prepareReturnToKBDefault:1;
				unsigned __a:6;

		}bf;

		struct _blink blink;
};


#define BASKET_MAXSIZE 2
#define BASKET_LEFT 0
#define BASKET_RIGHT 1

struct _process
{
		int8_t sm0;
};


struct _fryer
{
		struct _basket basket[BASKET_MAXSIZE];
		//int8_t kb_mode;
		int8_t psmode;

		struct _process ps_program;
		struct _process ps_operative;
};
extern const struct _process ps_reset;
extern struct _fryer fryer;


struct _job
{
		int8_t sm0;
		uint16_t counter0;
		//uint16_t counter1;

		struct _job_f
		{
				unsigned enable:1;
				unsigned job:1;
				unsigned lock:1;
				unsigned recorridoEnd:1;
				unsigned __a:4;
		}f;
};
extern struct _job job_captureTemperature;
extern const struct _job job_reset;

struct _mainflag
{
		unsigned sysTickMs :1;
		unsigned __a:7;
};
extern struct _mainflag mainflag;

#define SYSTICK_MS 10//10ms


extern struct _t time_k[BASKET_MAXSIZE];


//////////////////////////////////////////
//Tiempos de Ignicion
#define TchispaIg 500//ms Tiempo de Chispa de igninicion
#define TdelayGasBurned 0//ms Tiempo prudente que puede demorar en prenderse el gas (Si es que lo existiera, sino seria 0)
//Tiempos de deteccion de llamas

//	//Temperaturas
//	#define TmprtPrecalentamiento
//	#define TmprtCoccion
//#define TMPR_MAX 218//C i 425F -> E-5 = FRYER TOO HOT


//Constantes
#define KintentosIg 3//intentos de ignicion

//Alarmas
#define CodeAlerta0
#define CodeAlerta1


/* C = (F-32)/1.8
 * F= (C*1.8) + 32
 */



#define PORTWxSOL_GAS_QUEMADOR 		PORTC
#define PORTRxSOL_GAS_QUEMADOR 		PINC
#define CONFIGIOxSOL_GAS_QUEMADOR 	DDRC
#define PINxKB_SOL_GAS_QUEMADOR		2

#define PORTWxSOL_GAS_PILOTO 	PORTD
#define PORTRxSOL_GAS_PILOTO 	PIND
#define CONFIGIOxSOL_GAS_PILOTO 	DDRD
#define PINxKB_SOL_GAS_PILOTO	6

#define PORTWxBUZZER 	PORTB
#define PORTRxBUZZER 	PINB
#define CONFIGIOxBUZZER 	DDRB
#define PINxBUZZER	2

#define PORTWxCHISPERO_ONOFF 	PORTB
#define PORTRxCHISPERO_ONOFF  	PINB
#define CONFIGIOxCHISPERO_ONOFF 	DDRB
#define PINxCHISPERO_ONOFF	3

#define PORTWxOC2 		PORTD
#define PORTRxOC2 		PIND
#define CONFIGIOxOC2 	DDRD
#define PINxKB_OC2			7

#define PORTWxFLAME_DETECC 		PORTA
#define PORTRxFLAME_DETECC 		PINA
#define CONFIGIOxFLAME_DETECC 	DDRA
#define PINxFLAME_DETECC			0


//ikey layout
#define KB_LYOUT_LEFT_STARTSTOP 0
#define KB_LYOUT_LEFT_SLEEP 1
#define KB_LYOUT_LEFT_DOWN 2
#define KB_LYOUT_LEFT_UP 3
//
#define KB_LYOUT_PROGRAM 4

#define KB_LYOUT_RIGHT_DOWN 5	//7
#define KB_LYOUT_RIGHT_UP	6	//8
#define KB_LYOUT_RIGHT_SLEEP 7	//6
#define KB_LYOUT_RIGHT_STARTSTOP 8	//5


/*
 *
 */
int8_t KFRYER_ON_SEG = 6; //6s total de encedido
int8_t KFRYER_OFF_SEG = 2; //2s total de encedido


#endif /* MAIN_H_ */
