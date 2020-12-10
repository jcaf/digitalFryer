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

5)￼￼￼
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
#include "system.h"
#include "lcdan/lcdan.h"
#include "lcdan/lcdan_aux.h"

int main(void)
{
    ConfigOutputPin(DDRC,2);
    PinTo1(PORTC,2);
    ConfigOutputPin(DDRC,2);


    lcdan_init();
    lcdan_print_string("Digital Fryer");

    while(1)
    {
        ;
    }


    return 0;
}
