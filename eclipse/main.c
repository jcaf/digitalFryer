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
 [root@JCAFPC Release]# avrdude -c usbasp -B5 -p m32 -U flash:w:atmega32Pulsonic.hex
 [root@JCAFPC Release]# avrdude -c usbasp -B1 -p m32 -V -U flash:w:atmega32Pulsonic.hex (SIN VERIFICAR)
 [jcaf@JCAFPC Release]$ avrdude -c usbasp -B5 -p m32 (ONLY A RESET)

 NUEVO
 [root@JCAFPC Release]# avrdude -c usbasp -B0.3 -p m32 -V -U flash:w:atmega328p.hex (MAS RAPIDO!)
 Tambien puede ser sin -BX.. cuando ya esta bien configurado los fuses:
 [root@JCAFPC Release]# avrdude -c usbasp -p m32 -U flash:w:atmega328p.hex

 5)
 8:50 a. m.
 GRABAR LA EEPROM
 [jcaf@jcafpc Release]$ avrdude -c usbasp -B4 -p m32 -V -U eeprom:w:atmega32Pulsonic.eep

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
} main_flag = { 0 };

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

int main(void)
{
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
	ikb_init();

	pinGetLevel_init(); //with Changed=flag activated at initialization

	ConfigOutputPin(CONFIGIOxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);
	PinTo0(PORTWxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);

	ConfigOutputPin(CONFIGIOxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);
	PinTo0(PORTWxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);

	ConfigOutputPin(CONFIGIOxBUZZER, PINxBUZZER);
	PinTo1(PORTWxBUZZER, PINxBUZZER);
	_delay_ms(5);
	PinTo0(PORTWxBUZZER, PINxBUZZER);

	lcdan_init();
	InitSPI_MASTER();
	//
	TCNT0 = 0x00;
	TCCR0 = (1 << WGM01) | (1 << CS02) | (0 << CS01) | (1 << CS00); //CTC, PRES=1024
	OCR0 = CTC_SET_OCR_BYTIME(10e-3, 1024); //TMR8-BIT @16MHz @PRES=1024-> BYTIME maximum = 16ms
	TIMSK |= (1 << OCIE0);
	sei();
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
						MAX6675_convertIntTmptr2str_wformatPrint3dig(
								temper_measured, temper_measured_str);
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
							MAX6675_convertIntTmptr2str_wformatPrint3dig(
									temper_measured, temper_measured_str);
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

				ikb_job();
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_STARTSTOP))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_STARTSTOP);
					//lcdan_print_string("0");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_SLEEP))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_SLEEP);
					//lcdan_print_string("1");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_DOWN))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_DOWN);
					//lcdan_print_string("2");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_LEFT_UP))
				{
					ikb_key_was_read(KB_LYOUT_LEFT_UP);
					//lcdan_print_string("3");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
				{
					ikb_key_was_read(KB_LYOUT_PROGRAM);
					//lcdan_print_string("4");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_STARTSTOP))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_STARTSTOP);
					//lcdan_print_string("5");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_SLEEP))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_SLEEP);
					//lcdan_print_string("6");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_DOWN))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_DOWN);
					//lcdan_print_string("7");
				}
				if (ikb_key_is_ready2read(KB_LYOUT_RIGHT_UP))
				{
					ikb_key_was_read(KB_LYOUT_RIGHT_UP);
					//lcdan_print_string("8");
				}

			}
		}
		//-------------------
		//lcdan_set_cursor(6, 0);
		//lcdan_print_string(temper_measured_str);

		if (pinGetLevel_level(PGLEVEL_LYOUT_SWONOFF)== 0)
		{
			if (sm0 == 0)
			{
				igDeteccFlama_resetJob();
				sm0++;
			}
			else if (sm0 == 1)
			{
				if (igDeteccFlama_doJob())
				{
					sm0++;    	//OK...Ignicion+deteccion de flama OK
					//PID_Control -> setpoint = Tprecalentamiento

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
				}
			}
			else if (sm0 == 2)
			{
				//Precalentamiento

				if (temper_measured >= Temper_Precalentamiento)
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
			//
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
		main_flag.f10ms = 0;
	}

	return 0;
}
ISR(TIMER0_COMP_vect)
{
	//PinToggle(PORTC,0);
	isr_flag.f10ms = 1;
}
