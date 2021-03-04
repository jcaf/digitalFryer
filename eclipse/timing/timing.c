/*
 ============================================================================
 Name        : timing.c
 Author      : Jcaf
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

void time_set(struct _t *pt)
{
}

#ifdef TIME_HOURS_EXIST
	#define TIME_HOURS_NUMMAX 59
	#define TIME_HOURS_NUMMIN 0
#endif

#define TIME_MINUTES_NUMMAX 59
#define TIME_MINUTES_NUMMIN 0

#define TIME_SEC_NUMMAX 59
#define TIME_SEC_NUMMIN 0

/*
 * return: 1 if 0:0:0
 */
int8_t time_dec(struct _t *pt)
{
	//Check if nothing to do
	#ifdef TIME_LIM_BOTTOM
		if (pt->sec == pt->sec_limbottom)
			if (pt->min == pt->min_limbottom)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == pt->hour_limbottom)
				#endif
				return 1;//time is already 00:00:00

		//if time is initialize to 0:0:0 -> por seguridad retorna
		if (pt->sec == 0)
			if (pt->min == 0)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == 0)
				#endif
				return 1;//time is already 00:00:00
		//

	#else
		if (pt->sec == TIME_SEC_NUMMIN)
				if (pt->min == TIME_MINUTES_NUMMIN)
					#ifdef TIME_HOURS_EXIST
						if (pt->hour == TIME_HOURS_NUMMIN)
					#endif
					return 1;//time is already 00:00:00
	#endif

	//Anything to do
	pt->sec--;

	//In this tick time is 00:00:00
	#ifdef TIME_LIM_BOTTOM
		if (pt->sec == pt->sec_limbottom)
			if (pt->min == pt->min_limbottom)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == pt->hour_limbottom)
				#endif
				return 1;//time is already 00:00:00
	#else
		if (pt->sec == TIME_SEC_NUMMIN)
				if (pt->min == TIME_MINUTES_NUMMIN)
					#ifdef TIME_HOURS_EXIST
						if (pt->hour == TIME_HOURS_NUMMIN)
					#endif
					return 1;//time is already 00:00:00
	#endif

	//Decrement hours, min and seconds
	if (pt->sec < TIME_SEC_NUMMIN)
	{
		pt->sec = TIME_SEC_NUMMAX;

		pt->min--;

		#ifdef TIME_HOURS_EXIST
		if (pt->min < TIME_MINUTES_NUMMIN)
		{
			pt->min = TIME_MINUTES_NUMMAX;
			pt->hour--;
		}
		#endif
	}
	return 0;
}
/*
int8_t time_dec(struct _t *pt)
{
	//Check if nothing to do
	if (pt->sec == TIME_SEC_NUMMIN)
		if (pt->min == TIME_MINUTES_NUMMIN)
			#ifdef TIME_HOURS_EXIST
				if (pt->hour == TIME_HOURS_NUMMIN)
			#endif
			return 1;//time is already 00:00:00

	//Anything to do
	pt->sec--;

	//In this tick time is 00:00:00
	if (pt->sec == TIME_SEC_NUMMIN)
		if (pt->min == TIME_MINUTES_NUMMIN)
			#ifdef TIME_HOURS_EXIST
				if (pt->hour == TIME_HOURS_NUMMIN)
			#endif
			return 1;

	//Decrement hours, min and seconds
	if (pt->sec < TIME_SEC_NUMMIN)
	{
		pt->sec = TIME_SEC_NUMMAX;

		pt->min--;

		#ifdef TIME_HOURS_EXIST
		if (pt->min < TIME_MINUTES_NUMMIN)
		{
			pt->min = TIME_MINUTES_NUMMAX;
			pt->hour--;
		}
		#endif
	}
	return 0;
}
*/
/*
//Only min:seconds
int8_t time_dec(struct _t *pt)
{
	if (pt->sec == 0)
		if (pt->min == 0)
			return 1;


	pt->sec--;

	if (pt->sec == 0)
		if (pt->min == 0)
			return 1;//-> 00:00

	if (pt->sec < TIME_SEC_NUMMIN)
	{
		pt->min--;
		pt->sec = TIME_SEC_NUMMAX;
	}
	return 0;
}
*/

/*
 * return: 1 if 0:0:0
 */
int8_t time_inc(struct _t *pt)
{
	//Check if nothing to do
	#ifdef TIME_LIM_TOP
		if (pt->sec == pt->sec_limtop)
			if (pt->min == pt->min_limtop)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == pt->hour_limtop)
				#endif
				return 1;

		#ifdef TIME_HOURS_EXIST
			if (pt->hour > pt->hour_limtop)
				return 1;
		#endif
			if (pt->min > pt->min_limtop)
				return 1;
			else
				if (pt->min == pt->min_limtop)
					if (pt->sec > pt->sec_limtop)
						return 1;

	#else
		if (pt->sec == TIME_SEC_NUMMAX)
			if (pt->min == TIME_MINUTES_NUMMAX)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == TIME_HOURS_NUMMAX)
				#endif
				return 1;

		#ifdef TIME_HOURS_EXIST
			if (pt->hour > TIME_HOURS_NUMMAX)
				return 1;
		#endif
			if (pt->min > TIME_MINUTES_NUMMAX)
				return 1;
			else
				if (pt->min == TIME_MINUTES_NUMMAX)
					if (pt->sec > TIME_SEC_NUMMAX)
						return 1;
	#endif

	//Anything to do
	pt->sec++;

	//In this tick time is 00:00:00
	//Check if nothing to do
	#ifdef TIME_LIM_TOP
		if (pt->sec == pt->sec_limtop)
			if (pt->min == pt->min_limtop)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == pt->hour_limtop)
				#endif
						return 1;//time is already 00:00:00
	#else
		if (pt->sec == TIME_SEC_NUMMAX)
			if (pt->min == TIME_MINUTES_NUMMAX)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == TIME_HOURS_NUMMAX)
				#endif
				return 1;//time is already 00:00:00
	#endif


	//Increment hours, min and seconds
	if (pt->sec > TIME_SEC_NUMMAX)
	{
		pt->sec = TIME_SEC_NUMMIN;

		pt->min++;
		#ifdef TIME_HOURS_EXIST
		if (pt->min > TIME_MINUTES_NUMMAX)
		{
			pt->min = TIME_MINUTES_NUMMIN;
			pt->hour++;
		}
		#endif

	}
	return 0;
}

/*
 int8_t time_inc(struct _t *pt)
{
	//Check if nothing to do
		if (pt->sec == TIME_SEC_NUMMAX)
			if (pt->min == TIME_MINUTES_NUMMAX)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == TIME_HOURS_NUMMAX)
				#endif
				return 1;//time is already 00:00:00

		//Anything to do
		pt->sec++;

		//In this tick time is 00:00:00
		if (pt->sec == TIME_SEC_NUMMAX)
			if (pt->min == TIME_MINUTES_NUMMAX)
				#ifdef TIME_HOURS_EXIST
					if (pt->hour == TIME_HOURS_NUMMAX)
				#endif
				return 1;

		//Increment hours, min and seconds
		if (pt->sec > TIME_SEC_NUMMAX)
		{
			pt->sec = TIME_SEC_NUMMIN;

			pt->min++;
			#ifdef TIME_HOURS_EXIST
			if (pt->min > TIME_MINUTES_NUMMAX)
			{
				pt->min = TIME_MINUTES_NUMMIN;
				pt->hour++;
			}
			#endif

		}
		return 0;
}
*/


struct _t mytime;
/*
int main(void)
{
	int8_t t;
	#ifdef TIME_HOURS_EXISTS
	mytime.hour = 59;
	#endif
	mytime.min = 0;
	mytime.sec = 14;


	#ifdef TIME_LIM_TOP
	#ifdef TIME_HOURS_EXIST
	mytime.hour_limtop = 0;
	#endif
	mytime.min_limtop = 0;
	mytime.sec_limtop = 15;
	#endif

	#ifdef TIME_LIM_BOTTOM
	#ifdef TIME_HOURS_EXIST
	mytime.hour_limbottom = 0;
	#endif
	mytime.min_limbottom = 45;
	mytime.sec_limbottom = 15;
	#endif
	do
	{
		t = time_dec(&mytime);
		//t = time_inc(&mytime);

		#ifdef TIME_HOURS_EXIST
		printf("%02d:", mytime.hour);
		#endif
		printf("%02d:", mytime.min);
		printf("%02d\n", mytime.sec);
	}while (t!= 1);
	printf("fin\n");

	return EXIT_SUCCESS;
}
*/
