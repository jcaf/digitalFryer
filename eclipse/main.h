/*
 * main.h
 *
 *  Created on: Dec 1, 2020
 *      Author: jcaf
 */

#ifndef MAIN_H_
#define MAIN_H_
	#include <stdint.h>

	//Tiempos de Ignicion
	#define TchispaIg 500//ms Tiempo de Chispa de igninicion
	#define TdelayGasBurned 0//ms Tiempo prudente que puede demorar en prenderse el gas (Si es que lo existiera, sino seria 0)
	//Tiempos de deteccion de llamas

	//Tiempos de coccion y control
	#define Tprecalentamiento
	#define Tcoccion
//	#define TmaxToEndCoccion
//
//	//Temperaturas
//	#define TmprtPrecalentamiento
//	#define TmprtCoccion
#define TMPR_MAX 218//C i 425F -> E-5 = FRYER TOO HOT


	//Constantes
	#define KintentosIg 3//intentos de ignicion

	//Alarmas
	#define CodeAlerta0
	#define CodeAlerta1


	/* C = (F-32)/1.8
	 * F= (C*1.8) + 32
	 */
	enum TmprtUnits {Farenheith, Celsius};




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




//ikey layout
#define KB_LYOUT_LEFT_STARTSTOP 0
#define KB_LYOUT_LEFT_SLEEP 1
#define KB_LYOUT_LEFT_DOWN 2
#define KB_LYOUT_LEFT_UP 3
//
#define KB_LYOUT_PROGRAM 4
//
#define KB_LYOUT_RIGHT_STARTSTOP 5
#define KB_LYOUT_RIGHT_SLEEP 6
#define KB_LYOUT_RIGHT_DOWN 	7
#define KB_LYOUT_RIGHT_UP	8
//





#endif /* MAIN_H_ */
