/*
 * groupdig.c
 *
 *  Created on: Dec 10, 2020
 *      Author: jcaf
 */



/*
Display 7s-> codigos @transistor
LCD -> char -> posicion @x,y

unificando conceptos

7:25

7.25

#ifdef DISPLAY DP_INTERNAL
#ifdef DISPLAY_DP_EXTERNAL_LEDS

|7 .| 2 5

7.25//Claro mejor desde ASCII
1:59:26
*/

struct _dig
{
	struct _bf_prop
	{
		unsigned blink:1;
		unsigned __a:7;

	}bf_prop;

	int timer;

}dig;

#define GROUPDIG_SIZEMAX (4+1)

//char time_char[GROUPDIG_SIZEMAX+2];//place num - string (itoa o dtostr/sprintf)

struct _groupDig
{
	struct _dig dig[GROUPDIG_SIZEMAX];
	const char *strDig;
}groupDig;

void groupDig_setStrDig(const char *str)
{
	groupDig.strDig = str;
}

/*
void mmain(void)
{
	if (groupDig[0].dig.bf_prop.blink)
	{
		//inicia temporizacion
	}

}
*/
