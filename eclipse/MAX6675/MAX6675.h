#ifndef MAX6675_MAX6675_H_
#define MAX6675_MAX6675_H_

	#define MAX6675_TMPR_MIN  0
	#define MAX6675_TMPR_MAX  1023.75

	#define MAX6675_THERMOCOUPLED_OPEN -1
	#define MAX6675_TEMPERATURE_DEVIATION 0

	float MAX6675_getFloatTmprCelsius(void);
	int16_t MAX6675_get12bitsTemp(void);



#endif // MAX6675_MAX6675_H_
