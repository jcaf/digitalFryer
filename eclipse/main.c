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


 */

#include "main.h"
#include "pinGetLevel/pinGetLevel.h"
#include "igDeteccFlama.h"
#include "adc.h"
#include "psmode_program.h"
#include "psmode_operative.h"


volatile struct _isr_flag
{
	unsigned sysTickMs :1;
	unsigned __a :7;
} isr_flag = { 0 };
struct _mainflag mainflag;

const struct _process ps_reset;
struct _fryer fryer;

struct _job job_captureTemperature;
const struct _job job_reset;

//k-load from EEPROM
struct _t time_k[BASKET_MAXSIZE] = { {0, 5}, {0,10} };    //mm:ss


int main(void)
{
	int8_t sm0 = 0;
	int c = 0;

	//++--
	//Kb init
	fryer.basket[BASKET_LEFT].kb.startStop = KB_LYOUT_LEFT_STARTSTOP;
	fryer.basket[BASKET_LEFT].kb.sleep = KB_LYOUT_LEFT_SLEEP;
	fryer.basket[BASKET_LEFT].kb.down = KB_LYOUT_LEFT_DOWN;
	fryer.basket[BASKET_LEFT].kb.up = KB_LYOUT_LEFT_UP;

	fryer.basket[BASKET_LEFT].display.cursor.x = DISP_CURSOR_BASKETLEFT_START_X;//0x00;
	fryer.basket[BASKET_LEFT].display.cursor.y = 0x00;

	//
	fryer.basket[BASKET_RIGHT].kb.startStop = KB_LYOUT_RIGHT_STARTSTOP;
	fryer.basket[BASKET_RIGHT].kb.sleep = KB_LYOUT_RIGHT_SLEEP;
	fryer.basket[BASKET_RIGHT].kb.down = KB_LYOUT_RIGHT_DOWN;
	fryer.basket[BASKET_RIGHT].kb.up = KB_LYOUT_RIGHT_UP;

	fryer.basket[BASKET_RIGHT].display.cursor.x = DISP_CURSOR_BASKETRIGHT_START_X;//0x0B;
	fryer.basket[BASKET_RIGHT].display.cursor.y = 0x00;
	//--++

	//Tiempo Necesario para estabilizar la tarjeta
	//__delay_ms(2000);//estabilizar tarjeta de deteccion

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
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (0 << COM20)
			| (0 << CS22) | (0 << CS21) | (1 << CS20);	//NO Preescaling
	ConfigOutputPin(CONFIGIOxOC2, PINxKB_OC2);

	//
	TIMSK |= (1 << OCIE0);
	sei();

	PinTo1(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
	PinTo1(PORTWxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);

	//x default en EEPROM
	Tcoccion.TC = 300;	//F
	Tcoccion.max = 390;	//390F-> 200C
	Tcoccion.min = 50;	//200F-> 100C
						//50F -> 10C
	while (1)
	{
		if (isr_flag.sysTickMs)
		{
			isr_flag.sysTickMs = 0;
			mainflag.sysTickMs = 1;
		}

		//----------------------
		if (mainflag.sysTickMs)
		{
			if (++c == (20/SYSTICK_MS))    //20ms
			{
				c = 0;
				//
				pinGetLevel_job();
				//---------------------------
				if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_SWONOFF))
				{
					lcdan_set_cursor(0, 0);
					//change the flow
					if (pinGetLevel_level(PGLEVEL_LYOUT_SWONOFF)== 0)	//active in low
					{
						sm0 = 0x00;
					}
					else
					{
						lcdan_print_PSTRstring(PSTR("OFF"));
					}
					pinGetLevel_clearChange(PGLEVEL_LYOUT_SWONOFF);
				}
				//chispero();
				ikb_job();
			}
		}
		temperature_job();
		//
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
					//buzzer beep + LCD + PID_Control -> setpoint = T
					sm0++;
					psmode_operative_init();
				}
			}
			else if (sm0 == 3)
			{
			}
			//---------------------------------------------------
			if (fryer.psmode == PSMODE_PROGRAM)
			{
				psmode_program();
			}
			else if (fryer.psmode == PSMODE_OPERATIVE)
			{
				psmode_operative();
			}
		}
		else //switch OFF
		{

		}

		//---------------------------
		mainflag.sysTickMs = 0;
	}

	return 0;
}

ISR(TIMER0_COMP_vect)
{
	isr_flag.sysTickMs = 1;
	//PinToggle(PORTC,0);
}

/*
//x=0:2*pi/16:2*pi
//y=(sin(x)*127)+127
#define SENO_NUMPOINTS 16
uint8_t seno[SENO_NUMPOINTS] = { 127, 176, 217, 244, 254, 244, 217, 176, 127, 78, 37, 10, 0, 10, 37, 78 };

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
#define FLAME_IS_ON 0
#define FLAME_IS_OFF 1
while (1)
{
	pinGetLevel_job();
	__delay_ms(20);

	if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_FLAME_DETECC))
	{
		lcdan_set_cursor(0, 0);
		//if has changed PGLEVEL_LYOUT_FLAME_DETECC, then read the level. After apply "pinGetLevel_clearChange"

		if (pinGetLevel_level(PGLEVEL_LYOUT_FLAME_DETECC) == FLAME_IS_ON)
		//if (PinRead(PORTRxFLAME_DETECC, PINxFLAME_DETECC) == FLAME_IS_ON)
		{
			lcdan_print_PSTRstring(PSTR("ON "));
			PinTo0(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
			__delay_ms(1500);
			PinTo1(PORTWxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);
			__delay_ms(2000);//en este tiempo sale con presion y puede generar un bajon en la deteccion
		}
		else
		{
			lcdan_print_PSTRstring(PSTR("OFF"));
			PinTo1(PORTWxCHISPERO_ONOFF, PINxCHISPERO_ONOFF);
			PinTo1(PORTWxSOL_GAS_PILOTO, PINxKB_SOL_GAS_PILOTO);
			PinTo0(PORTWxSOL_GAS_QUEMADOR, PINxKB_SOL_GAS_QUEMADOR);
		}
		pinGetLevel_clearChange(PGLEVEL_LYOUT_FLAME_DETECC);
	}

}
while (1)
{
	flama = (ADC_read(ADC_CH_0) * 5.0) / 1023;
	lcdan_set_cursor(0, 0);
	if (flama > 2.0)
	{
		lcdan_print_PSTRstring(PSTR("ON "));
	}
	else
	{
		lcdan_print_PSTRstring(PSTR("OFF"));
	}

	lcdan_set_cursor(0, 1);
	dtostrf(flama, 0, 2, voltaje_flama);
	lcdan_print_string(voltaje_flama);

	_delay_ms(100);
}

ConfigOutputPin(DDRC, 0);

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
