/*
 * Cold-Junction-Compensated K-Thermocoupleto-Digital Converter (0°C to +1023.5°C)
 */
#include "system.h"
#include "SPI/SPI.h"
#include "MAX6675.h"

/***************************************************************
MAX6675: Conversion Time typ: 0.17s max: 0.22s

Digitization
The ADC adds the cold-junction diode measurement
with the amplified thermocouple voltage and reads out
the 12-bit result onto the SO pin. A sequence of all
zeros means the thermocouple reading is 0°C. A
sequence of all ones means the thermocouple reading
is +1023.75°C.

    1023.75-> ((2^12) -1)
    x -> temp_read

****************************************************************/
float MAX6675_getFloatTmprCelsius(void)
{
    uint8_t spi_h;
    uint8_t spi_l;
    uint16_t temp_read  = 0x0000;
    //
    PinTo0(PORTWxSPI_CS,PINxKB_SPI_CS);
    _delay_us(1);//Tcss = 100ns

    	spi_h = SPI_MSTR_ExchangeData(DUMMY_BYTE_BY_EXCHANGE);
        spi_l = SPI_MSTR_ExchangeData(DUMMY_BYTE_BY_EXCHANGE);

	_delay_us(5);//Ttr = 100ns >5us asegura q sea por completo los ul
    PinTo1(PORTWxSPI_CS,PINxKB_SPI_CS);

    //
    if (spi_l & 0x04)
    {
        return -1;//disconnected
    }
    else
    {
        temp_read = (spi_h<<8) | (spi_l);
        temp_read = temp_read>>3;

        //return ( (temp_read * 1023.75)/ (4095) );
        //return ( (temp_read * MAX6675_TMPR_MAX)/ ((1<<12) -1) );
        return ( (temp_read * MAX6675_TMPR_MAX)/ (4095) );
    }
}

