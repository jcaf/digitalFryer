/*
 Programmer USBasp, fix permissions:
 https://andreasrohner.at/posts/Electronics/How-to-fix-device-permissions-for-the-USBasp-programmer/
 Permanent Solution
 The more sensible solution is to add a udev rule for the device. A udev rule simply matches certain properties of a device after it is connected and performs certain actions on it, like changing the file permissions.
 The firmware package already contains a template for a udev rule in the directory bin/linux-nonroot/99-USBasp.rules and a small script to install it. The installation is trivial, because you just have to copy the 99-USBasp.rules file to /etc/udev/rules.d/.
 $ sudo cp 99-USBasp.rules /etc/udev/rules.d/
 Customizing the udev rule
 The rule from the firmware package just sets the file permissions to 666, which is a bit crude. I wrote my own version of it for Arch Linux. In Arch Linux the group uucp is used for "Serial and USB devices such as modems, handhelds, RS-232/serial ports", so it makes sense to use it for the USBasp device.
 # Set Group for USBasp
 SUBSYSTEM=="usb", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="05dc", GROUP="uucp"
 You also have to add your user to that group. For this to take effect you have to log out and then log in again.
 gpasswd -a youruser uucp
 Please note that the group is called uucp only in Arch Linux. Other distributions use a different group for the same thing. Ubuntu for example uses the group dialout.

 Atmega32
 ----------------------------------------
 1)    http://www.engbedded.com/fusecalc/
 lock bits:
 http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p
 2) verificar que responda el atmega (ONLY A RESET)
 [jcaf@jcafpc ~]$ avrdude -c usbasp -B5 -p m32

 3) programar fuse (sin preservar EEPROM)
 -U lfuse:w:0xbf:m -U hfuse:w:0xcf:m
 [jcaf@jcafpc ~]$ avrdude -c usbasp -B5 -p m32 -U lfuse:w:0xbf:m -U hfuse:w:0xcf:m

 4) GRABAR EL CODIGO FUENTE CON EL COMANDO ACOSTUMBRADO
 [root@JCAFPC Release]# avrdude -c usbasp -B5 -p m32 -U flash:w:digitalFryer.hex
 [root@JCAFPC Release]# avrdude -c usbasp -B1 -p m32 -V -U flash:w:digitalFryer.hex (SIN VERIFICAR)
 [jcaf@JCAFPC Release]$ avrdude -c usbasp -B5 -p m32 (ONLY A RESET)

 NUEVO
 [root@JCAFPC Release]# avrdude -c usbasp -B0.3 -p m32 -V -U flash:w:digitalFryer.hex (MAS RAPIDO!)
 Tambien puede ser sin -BX.. cuando ya esta bien configurado los fuses:
 [root@JCAFPC Release]# avrdude -c usbasp -p m32 -U flash:w:digitalFryer.hex

 5)
 8:50 a. m.
 GRABAR LA EEPROM
 [jcaf@jcafpc Release]$ avrdude -c usbasp -B4 -p m32 -V -U eeprom:w:digitalFryer.eep

 6) programar fuse (PRESERVANDO EEPROM)

 avrdude -c usbasp -B5 -p m32 -U lfuse:w:0xbf:m -U hfuse:w:0xc7:m

 7) Verificar los fuses
 [jcaf@jcafpc Release]$ avrdude -c usbasp -B4 -p m32 -U lfuse:r:-:i -v

 +++++++++++++++++++++++
 Acomodar para Atmega32A
 proteger flash (modo 3): lectura y escritura

 avrdude -c usbasp -B10 -p m32 -U lock:w:0xFC:m

 (ignorar el error de 0x3C... pues los 2 bits de mayor peso no estan implentados)

 25 en 2020: BRAYAN 0.35 - 0.6 A TODAS LAS TARJETAS, OK MOTOR UNIPOLAR

 MODO DE FUNCIONAMIENTO:


 */

#include <avr/io.h>
#include "main.h"
#include "types.h"
#include "system.h"
#include "lcdan/lcdan.h"
#include "lcdan/lcdan_aux.h"
#include "ikb/ikb.h"
#include "MAX6675.h"
#include "SPI/SPI.h"
#include "Temperature/temperature.h"
#include "pinGetLevel/pinGetLevel.h"
#include "igDeteccFlama.h"
#include "timing/timing.h"
#include "utils/utils.h"
#include "adc.h"

//Temperaturas
int16_t Temper_Precalentamiento = 10;
int16_t Temper_Coccion = 300;

/////////////////////////////////////////

void solenoideGasQuemadores_on(void)
{

}
void solenoideGasQuemadores_off(void)
{

}
void solenoideGasPiloto_on(void)
{

}
void solenoideGasPiloto_off(void)
{
}

volatile struct _isr_flag
{
	unsigned f10ms :1;
	unsigned __a :7;
} isr_flag = { 0 };

struct _main_flag
{
	unsigned f10ms :1;
	unsigned blink_toggle: 1;
	unsigned blink_isActive: 1;
	unsigned __a:5;

} main_flag = { 0 };

#define BLINK_TIMER_KMAX (500/10)//500ms/10ms de acceso
int8_t blink_timer = BLINK_TIMER_KMAX;//0;

int16_t temper_measured; //expose global
//extern int16_t temper_measured;//expose global


struct _basket
{
	struct _t cookCycle_time;

	int8_t cookCycle_timer;

	struct _basket_bf
	{
		unsigned cookCycle_on:1;
		unsigned __a;

	}bf;
};
struct _fryer
{
	struct _basket leftBasket;
	struct _basket rightBasket;

	int8_t kbStage;
}fryer;

void chispero(void);

/*
 * >> x=0:2*pi/16:2*pi
 * >> y=(sin(x)*127)+127
 */
#define SENO_NUMPOINTS 16
uint8_t seno[SENO_NUMPOINTS]={127, 176, 217, 244, 254, 244, 217, 176, 127, 78, 37, 10, 0, 10, 37, 78};

int main(void)
{
	struct _basket leftBasket_temp;
	struct _basket rightBasket_temp;
	//
	int8_t kb_mode = 0;
	int8_t process_mode = 0;
	//


	//Tiempo Necesario para estabilizar la tarjeta
//	__delay_ms(2000);//estabilizar tarjeta de deteccion

	//Active pull-up
	PinTo1(PORTWxKB_KEY0, PINxKB_KEY0);
	PinTo1(PORTWxKB_KEY1, PINxKB_KEY1);
	PinTo1(PORTWxKB_KEY2, PINxKB_KEY2);
	PinTo1(PORTWxKB_KEY3, PINxKB_KEY3);
	PinTo1(PORTWxKB_KEY4, PINxKB_KEY4);
	PinTo1(PORTWxKB_KEY5, PINxKB_KEY5);
	PinTo1(PORTWxKB_KEY6, PINxKB_KEY6);
	PinTo1(PORTWxKB_KEY7, PINxKB_KEY7);
	PinTo1(PORTWxKB_KEY8, PINxKB_KEY8);
	__delay_ms(1);
	ikb_init();

	pinGetLevel_init(); //with Changed=flag activated at initialization

	//
	PinTo0(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
	ConfigOutputPin(CONFIGIOxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);

	PinTo0(PORTWxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);
	ConfigOutputPin(CONFIGIOxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);

	PinTo0(PORTWxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);
	ConfigOutputPin(CONFIGIOxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);
	//
	ConfigOutputPin(CONFIGIOxBUZZER, PINxBUZZER);
	PinTo1(PORTWxBUZZER, PINxBUZZER);
	__delay_ms(5);
	PinTo0(PORTWxBUZZER, PINxBUZZER);

	//PinTo1(PORTWxFLAME_DETECC, PINxFLAME_DETECC);//Active pull-up
	//ConfigInputPin(PORTRxFLAME_DETECC, PINxFLAME_DETECC);

	lcdan_init();
	InitSPI_MASTER();


	//
	//ADC_init(ADC_MODE_SINGLE_END);


	//Config to 10ms, antes de generar la onda senoidal
	TCNT0 = 0x00;
	TCCR0 = (1 << WGM01) | (1 << CS02) | (0 << CS01) | (1 << CS00); //CTC, PRES=1024
	OCR0 = CTC_SET_OCR_BYTIME(10e-3, 1024); //TMR8-BIT @16MHz @PRES=1024-> BYTIME maximum = 16ms

/*
	//Config to 61uS para generar la onda senoidal
	TCNT0 = 0x00;
	TCCR0 = (1 << WGM01) | (0 << CS02) | (1 << CS01) | (0 << CS00); //CTC, PRES=8
	OCR0 = CTC_SET_OCR_BYTIME(62.5E-6, 8); //TMR8-BIT @16MHz @PRES=1024-> BYTIME maximum = 16ms
*/
	//Config TRM2 PWM OC2 PIN (8-Bits)
	//Prescaler = 1, No inverted OC2 pin
	TCNT2 = 0;
	OCR2 = 127;
	TCCR2 = (1<< WGM21) |(1<< WGM20) | (1<< COM21) | (0<< COM20) | (0<< CS22) | (0<< CS21) | (1<< CS20);//NO Preescaling
	ConfigOutputPin(CONFIGIOxOC2, PINxKB_OC2);

	//
	TIMSK |= (1 << OCIE0);
	sei();

	PinTo1(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);

	PinTo1(PORTWxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);


	/*
	while (1)
	{
		PinTo1(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
		_delay_ms(1000);
		_delay_ms(1000);
		PinTo0(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
		_delay_ms(1000);
		_delay_ms(1000);
		_delay_ms(1000);
		_delay_ms(1000);

	}
*/


	#define FLAME_IS_ON 0
	#define FLAME_IS_OFF 1

/*
while(1)
{
	pinGetLevel_job();
	__delay_ms(20);

	if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_FLAME_DETECC))
	{
		lcdan_set_cursor(0, 0);
		//if has changed PGLEVEL_LYOUT_FLAME_DETECC, then read the level. After apply "pinGetLevel_clearChange"

		if (pinGetLevel_level(PGLEVEL_LYOUT_FLAME_DETECC)== FLAME_IS_ON)
		//if (PinRead(PORTRxFLAME_DETECC, PINxFLAME_DETECC) == FLAME_IS_ON)
		{
			lcdan_print_PSTRstring(PSTR("ON "));
			//
			PinTo0(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
			//
			__delay_ms(1500);
			PinTo1(PORTWxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);
			__delay_ms(2000);//en este tiempo sale con presion y puede generar un bajon en la deteccion
		}
		else
		{
			lcdan_print_PSTRstring(PSTR("OFF"));
			PinTo1(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
			//
			PinTo1(PORTWxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);

			PinTo0(PORTWxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);
		}

		pinGetLevel_clearChange(PGLEVEL_LYOUT_FLAME_DETECC);
	}

}
*/



//while(1)
//{
//	flama = (ADC_read(ADC_CH_0)*5.0)/1023;
//	lcdan_set_cursor(0, 0);
//	if (flama > 2.0)
//	{
//		lcdan_print_PSTRstring(PSTR("ON "));
//	}
//	else
//	{
//		lcdan_print_PSTRstring(PSTR("OFF"));
//	}
//
//	lcdan_set_cursor(0, 1);
//	dtostrf(flama, 0,2, voltaje_flama);
//	lcdan_print_string(voltaje_flama);
//
//	_delay_ms(100);
//}

	//
	//ConfigOutputPin(DDRC,0);
	int c = 0;

	//
	int MAX6675_ConversionTime_access = 0;
	float TmprFactCorr = +2;
	int16_t TmprCelsius = 0;
	int16_t temper_mean_val = 0;
	int8_t temper_mean_n_samples = 0;
	char temper_measured_str[10] = "";
	//

	int8_t sm0 = 0;
	//k-load from EEPROM
	struct _t lefttime_k = { 3, 1 };    //mm:ss
	struct _t righttime_k = { 4, 30 };    //mm:ss

	char str[10];
	char buff[10];

	while (1)
	{
		if (isr_flag.f10ms)
		{
			isr_flag.f10ms = 0;
			main_flag.f10ms = 1;
		}

		//----------------------
		if (main_flag.f10ms)
		{
			if (++c == 2)    //20ms
			{
				c = 0;

				if (++MAX6675_ConversionTime_access == 11) //MAX6675 has max 0.22s
				{
					MAX6675_ConversionTime_access = 0;

					TmprCelsius = (int) MAX6675_getFloatTmprCelsius();
					//
					if (TmprCelsius < 0)    //error cable disconnected ?
					{
						temper_measured = TmprCelsius;
						//reset counters
						temper_mean_n_samples = 0x00;
						temper_mean_val = 0x00;
						//
						MAX6675_convertIntTmptr2str_wformatPrint3dig(temper_measured, temper_measured_str);
					}
					else
					{
						temper_mean_val += TmprCelsius;

						#define TEMPER_K_MULT_DIV_SHIFT 3//MULT_DIV_SHIFT -
						#define TEMPER_MEAN_SAMPLES (0x001<<TEMPER_K_MULT_DIV_SHIFT)//4

						if (++temper_mean_n_samples >= TEMPER_MEAN_SAMPLES)
						{
							temper_measured = ((int16_t) (temper_mean_val
									>> TEMPER_K_MULT_DIV_SHIFT)) + TmprFactCorr; //div usando shift
							//
							temper_mean_n_samples = 0x00;
							temper_mean_val = 0x00;

							//
							MAX6675_convertIntTmptr2str_wformatPrint3dig(temper_measured, temper_measured_str);
						}
					}
				}
				pinGetLevel_job();
				//---------------------------
				if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_SWONOFF))
				{
					lcdan_set_cursor(0, 0);
					//change the flow
					if (pinGetLevel_level(PGLEVEL_LYOUT_SWONOFF)== 0)	//active in low
					{
						sm0 = 0x00;
						//lcdan_print_PSTRstring(PSTR("3:00        3:00"));
					}
					else
					{
						lcdan_print_PSTRstring(PSTR("OFF"));
					}
					pinGetLevel_clearChange(PGLEVEL_LYOUT_SWONOFF);
				}
chispero();

				ikb_job();
				/*
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_STARTSTOP))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_STARTSTOP);
					lcdan_print_string("0");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_SLEEP))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_SLEEP);
					lcdan_print_string("1");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_DOWN))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_DOWN);
					lcdan_print_string("2");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_UP))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_UP);
					lcdan_print_string("3");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
				{
					ikb_key_was_read(KB_LYOUT_PROGRAM);
					lcdan_print_string("4");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_STARTSTOP))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_STARTSTOP);
					lcdan_print_string("8");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_SLEEP))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_SLEEP);
					lcdan_print_string("7");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_DOWN))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_DOWN);
					lcdan_print_string("5");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_UP))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_UP);
					lcdan_print_string("6");
				}
				*/
			}
		}
		//-------------------
		//lcdan_set_cursor(6, 0);
		//lcdan_print_string(temper_measured_str);

		if (pinGetLevel_level(PGLEVEL_LYOUT_SWONOFF)== 0)
		{
			if (sm0 == 0)
			{
				fryer.leftBasket.cookCycle_time.min = lefttime_k.min;
				fryer.leftBasket.cookCycle_time.sec = lefttime_k.sec;
				fryer.rightBasket.cookCycle_time.min = righttime_k.min;
				fryer.rightBasket.cookCycle_time.sec = righttime_k.sec;

				//print Left time
				//build left mm:ss
				itoa(fryer.leftBasket.cookCycle_time.min, buff,10);
				paddingLeftwBlanks(buff, 2);
				strcpy(str,buff);
				strcat(str,":");
				itoa(fryer.leftBasket.cookCycle_time.sec, buff, 10);
				paddingLeftwZeroes(buff, 2);
				strcat(str,buff);
				lcdan_set_cursor(0, 0);
				lcdan_print_string(str);
				//

				//print Right time
				//build left mm:ss
				itoa(fryer.rightBasket.cookCycle_time.min, buff,10);
				paddingLeftwBlanks(buff, 2);
				strcpy(str,buff);
				strcat(str,":");
				itoa(fryer.rightBasket.cookCycle_time.sec, buff, 10);
				paddingLeftwZeroes(buff, 2);
				strcat(str,buff);
				lcdan_set_cursor(0x0B, 0);//xx:xx
				lcdan_print_string(str);
				//

				igDeteccFlama_resetJob();
				sm0++;
			}
			else if (sm0 == 1)
			{
				if (igDeteccFlama_doJob())
				{
					sm0++;    	//OK...Ignicion+deteccion de flama OK
					//PID_Control -> setpoint = Tprecalentamiento


				}
			}
			else if (sm0 == 2)
			{
				//Precalentamiento

				if (1)//(temper_measured >= Temper_Precalentamiento)
				{
					PinTo1(PORTWxBUZZER, PINxBUZZER);
					_delay_ms(50);
					PinTo0(PORTWxBUZZER, PINxBUZZER);

					//buzzer beep + LCD +
					//PID_Control -> setpoint = T

					fryer.leftBasket.cookCycle_timer = 0x00;
					fryer.rightBasket.cookCycle_timer = 0x00;

					sm0++;

					//
					fryer.leftBasket.bf.cookCycle_on = 1;
					fryer.rightBasket.bf.cookCycle_on = 1;
				}
			}
			else if (sm0 == 3)
			{

			}

			if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
			{
				ikb_key_was_read(KB_LYOUT_PROGRAM);
				//lcdan_print_string("4");
				if (++kb_mode >= 3)
				{
					kb_mode = 0x00;
				}
			}

			//int8_t change_cookCyle = 0

			int8_t cookCycle_show = 0;

			enum E_KB_MODE
			{
				DEFAULT = 0,
				EDIT_COOKCYCLE,
				EDIT_TSETPOINT
			};

			if (kb_mode == DEFAULT)
			{
				if ( (ikb_key_is_ready2read(KB_LYOUT_LEFT_DOWN)) || (ikb_key_is_ready2read(KB_LYOUT_LEFT_UP)) )
				{
					ikb_key_was_read(KB_LYOUT_LEFT_DOWN);
					ikb_key_was_read(KB_LYOUT_LEFT_UP);

					//
					fryer.leftBasket.bf.cookCycle_on = 0;//off

					//solo muestra blinkeando el valor...

					leftBasket_temp.cookCycle_time.min = lefttime_k.min;
					leftBasket_temp.cookCycle_time.sec = lefttime_k.sec;

					//++--
					blink_timer = BLINK_TIMER_KMAX;
					main_flag.blink_toggle = 0;
					main_flag.blink_isActive = 1;
					//--++

					//change layout

					struct _key_prop key_prop = {0};

					key_prop = propEmpty;
					key_prop.uFlag.f.onKeyPressed = 1;
					key_prop.uFlag.f.reptt = 1;
					key_prop.numGroup = 1;
					key_prop.repttTh.breakTime = (uint16_t) 300.0 / KB_PERIODIC_ACCESS;
					key_prop.repttTh.period = (uint16_t) 50.0 / KB_PERIODIC_ACCESS;

					ikb_setKeyProp(KB_LYOUT_LEFT_DOWN, key_prop);
					ikb_setKeyProp(KB_LYOUT_LEFT_UP, key_prop);
					//

					kb_mode = EDIT_COOKCYCLE;
				}

			}
			if (kb_mode == EDIT_COOKCYCLE)
			{
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_DOWN))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_DOWN);

					time_dec(&leftBasket_temp.cookCycle_time);

				}
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_UP))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_UP);

					time_inc(&leftBasket_temp.cookCycle_time);

					//
					blink_timer = BLINK_TIMER_KMAX;
					main_flag.blink_toggle = 1;

				}
			}

			//+++++++++++++++++++++++++++++++++++++++++++++++++
			if (fryer.leftBasket.bf.cookCycle_on)
			{
				if (main_flag.f10ms)
				{
					if (++fryer.leftBasket.cookCycle_timer == 100)
					{
						fryer.leftBasket.cookCycle_timer = 0;

						time_dec(&fryer.leftBasket.cookCycle_time);
						//
						//print Left time mm:ss
						itoa(fryer.leftBasket.cookCycle_time.min, buff,10);
						paddingLeftwBlanks(buff, 2);
						strcpy(str,buff);
						strcat(str,":");
						itoa(fryer.leftBasket.cookCycle_time.sec, buff, 10);
						paddingLeftwZeroes(buff, 2);
						strcat(str,buff);
						lcdan_set_cursor(0, 0);
						lcdan_print_string(str);
					}
				}
			}
			if (fryer.rightBasket.bf.cookCycle_on)
			{
				if (main_flag.f10ms)
				{
					if (++fryer.rightBasket.cookCycle_timer == 100)
					{
						fryer.rightBasket.cookCycle_timer = 0;

						time_dec(&fryer.rightBasket.cookCycle_time);
						//
						//print Right time mm:ss
						itoa(fryer.rightBasket.cookCycle_time.min, buff,10);
						paddingLeftwBlanks(buff, 2);
						strcpy(str,buff);
						strcat(str,":");
						itoa(fryer.rightBasket.cookCycle_time.sec, buff, 10);
						paddingLeftwZeroes(buff, 2);
						strcat(str,buff);
						lcdan_set_cursor(0x0B, 0);//xx:xx
						lcdan_print_string(str);
						//
					}
				}
			}
		}
		else    											//OFF
		{

		}



		//---------------------------
		//++--Blinking zone


		if (main_flag.blink_isActive == 1)
		{
			if (main_flag.f10ms)
			{
				if (++blink_timer >= BLINK_TIMER_KMAX)//
				{
					blink_timer = 0x00;
					//
					main_flag.blink_toggle = !main_flag.blink_toggle;
					if (main_flag.blink_toggle == 1)
					{
						strcpy(str,"     ");
						//
						lcdan_set_cursor(0, 0);
						lcdan_print_string(str);
					}
					else
					{
						//print Left time mm:ss
						itoa(leftBasket_temp.cookCycle_time.min, buff,10);
						paddingLeftwBlanks(buff, 2);
						strcpy(str,buff);
						strcat(str,":");
						itoa(leftBasket_temp.cookCycle_time.sec, buff, 10);
						paddingLeftwZeroes(buff, 2);
						strcat(str,buff);
						//
						lcdan_set_cursor(0, 0);
						lcdan_print_string(str);
					}


				}
			}
			//--++Blinking zone

		}


		//---------------------------
		main_flag.f10ms = 0;
	}

	return 0;
}

ISR(TIMER0_COMP_vect)
{
	//PinToggle(PORTC,0);
	isr_flag.f10ms = 1;
}


/*
 //ISR para generar la onda senoidal
ISR(TIMER0_COMP_vect)
{
//	PinToggle(PORTWxOC2,PINxKB_OC2);

	 // Si se usa el flag de desborde del TOV2, entonces cada 4 Overflows se escribe el DC

	//update D.C = OCR2

	static int8_t c;

	if ( ++c >= 16)
	{
		c = 0;
	}
	OCR2 = seno[c];
}
*/

void chispero(void)
{
/*
	if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_CHISPERO))
	{
		//lcdan_set_cursor(0, 1);
		//change the flow
		if (pinGetLevel_level(PGLEVEL_LYOUT_CHISPERO)== 0)	//active in low
		{
			PinTo1(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
		}
		else
		{
			PinTo0(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
		}
		pinGetLevel_clearChange(PGLEVEL_LYOUT_CHISPERO);
	}
*/
}
//
void duty()
{

}
