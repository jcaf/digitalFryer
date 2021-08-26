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
	unsigned sysTickMs :1;
	unsigned __a :7;
} isr_flag = { 0 };


struct _mainflag mainflag;

#define BLINK_TIMER_KMAX (400/10)//Xms/10ms de acceso
#define EDITCYCLE_TIMERTIMEOUT_K (5000/10)//5000ms/10ms

const struct _process ps_reset;
struct _fryer fryer;

struct _job job_captureTemperature;
const struct _job job_reset;

void chispero(void);

/*
 * >> x=0:2*pi/16:2*pi
 * >> y=(sin(x)*127)+127
 */
#define SENO_NUMPOINTS 16
uint8_t seno[SENO_NUMPOINTS] = { 127, 176, 217, 244, 254, 244, 217, 176, 127,
		78, 37, 10, 0, 10, 37, 78 };

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


int main(void)
{
	int8_t sm0 = 0;
	int c = 0;

	struct _basket basket_temp[BASKET_MAXSIZE];

	//k-load from EEPROM
	struct _t time_k[BASKET_MAXSIZE] = { {0, 5}, {0,10} };    //mm:ss

	//

//	float TmprFactCorr = +2;
//	int16_t TmprCelsius = 0;
//	int16_t temper_mean_val = 0;
//	int8_t temper_mean_n_samples = 0;
//	char temper_measured_str[10] = "";
	//
	char str[20];


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
	TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21) | (0 << COM20)
			| (0 << CS22) | (0 << CS21) | (1 << CS20);	//NO Preescaling
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
			if (++c == 2)    //20ms
			{
				c = 0;

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
			}
		}

		if (pinGetLevel_level(PGLEVEL_LYOUT_SWONOFF)== 0)
		{
			if (sm0 == 0)
			{
				for (int i=0; i<BASKET_MAXSIZE; i++)
				{
					fryer.basket[i].blink.timerBlink_K =  BLINK_TIMER_KMAX;
					//+-
					fryer.basket[i].cookCycle.time.min = time_k[i].min;
					fryer.basket[i].cookCycle.time.sec = time_k[i].sec;
					fryer.basket[i].display.owner = DISPLAY_TIMING;
					fryer.basket[i].display.bf.print_cookCycle = 1;
					//
					if (fryer.basket[i].display.bf.print_cookCycle == 1)
					{
						build_cookCycle_string(&fryer.basket[i].cookCycle.time, str);
						lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
						lcdan_print_string(str);
					}
					//-+
				}

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

					sm0++;
					//++--
					for (int i=0; i<BASKET_MAXSIZE; i++)
					{
						fryer.basket[i].cookCycle.time.min = time_k[i].min;
						fryer.basket[i].cookCycle.time.sec = time_k[i].sec;
						fryer.basket[i].display.owner = DISPLAY_TIMING;
						fryer.basket[i].display.bf.print_cookCycle = 1;
						//
						if (fryer.basket[i].display.bf.print_cookCycle == 1)
						{
							build_cookCycle_string(&fryer.basket[i].cookCycle.time, str);
							lcdan_set_cursor(fryer.basket[i].display.cursor.x, fryer.basket[i].display.cursor.y);
							lcdan_print_string(str);
						}
					}
					//--+
				}
			}
			else if (sm0 == 3)
			{

			}
			//----------------------------------------------------
			//desde aqui todo es compartido
			if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
			{
				ikb_key_was_read(KB_LYOUT_PROGRAM);

//				if (++fryer.basket[i].kb.mode >= 3)
//				{
//					fryer.basket[i].kb.mode = 0x00;
//				}
				/*
				 * 1st touch -> OIL (TEMPERATURA ACTUAL)
				 * 2nd touch -> SET (TEMPERATURA DE MANTENIMIENTO)
				 * 				USA LAS TECLAS DE LA DERECHA
				 * 3th touch -> sale y muestra el tiempo
				 *
				 *SYSTEMMODE_OPERATIVE,
				SYSTEMMODE_PROGRAM,
				 */

			}

			//---------------------------------------------------
			if (fryer.psmode == PSMODE_PROGRAM)
			{
				psmode_program();

			}
			else if (fryer.psmode == PSMODE_OPERATIVE)
			{
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

							if (fryer.basket[i].bf.user_startStop == 0)
							{
								fryer.basket[i].bf.user_startStop = 1;
								//
								fryer.basket[i].cookCycle.bf.on = 1;

							}
							else
							{
								fryer.basket[i].bf.user_startStop = 0;
								fryer.basket[i].cookCycle.bf.on = 0;
								//load from eeprom
								fryer.basket[i].cookCycle.time.min = time_k[i].min;
								fryer.basket[i].cookCycle.time.sec = time_k[i].sec;
								//return to decrementing timing
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

						if ( ikb_key_is_ready2read(fryer.basket[i].kb.down) || ikb_key_is_ready2read(fryer.basket[i].kb.up) )
						{
							ikb_key_was_read(fryer.basket[i].kb.down);
							ikb_key_was_read(fryer.basket[i].kb.up);

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
						}
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

								if (fryer.basket[i].cookCycle.bf.on == 1)
								{
									cookCycle_hotUpdate(&basket_temp[i].cookCycle.time, &time_k[i], &fryer.basket[i].cookCycle.time);
									//return with fryer.basket[i].cookCycle.time UPDATED!
								}
								//update eeprom
								time_k[i] = basket_temp[i].cookCycle.time;//update new cookCycle set-point
								//load from eeprom
								fryer.basket[i].cookCycle.time.min = time_k[i].min;
								fryer.basket[i].cookCycle.time.sec = time_k[i].sec;
								//

								fryer.basket[i].cookCycle.editcycle.bf.blinkIsActive = 0;
								//return to decrementing timing
								fryer.basket[i].cookCycle.counterTicks = 0x00;//reset counter
								fryer.basket[i].cookCycle.bf.forceCheckControl = 1;//forzar pase directo para poder visualizar y actuar si es 00:00
								fryer.basket[i].display.owner = DISPLAY_TIMING;
								fryer.basket[i].display.bf.print_cookCycle = 1;
								//
								//clear all keys * solo por ahora seria startstop
								ikb_key_was_read(fryer.basket[i].kb.startStop);
								ikb_key_was_read(fryer.basket[i].kb.sleep);
								//
								kbmode_setDefault(&fryer.basket[i].kb);
								fryer.basket[i].kb.mode = KBMODE_DEFAULT;
								//
							}
						}
					}
					////+++++++++++++++++++++++++++++++++++++++++++++++++
					//if (main_flag.f10ms)
					//{
					//blink_timing();
					//}
					////+++++++++++++++++++++++++++++++++++++++++++++++++

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
	//PinToggle(PORTC,0);
	isr_flag.sysTickMs = 1;
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
