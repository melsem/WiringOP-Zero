/*
 * readall.c:
 *	The readall functions - getting a bit big, so split them out.
 *	Copyright (c) 2012-2013 Gordon Henderson
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "OrangePi.h"
#include "../wiringPi/OrangePi.h"
#include "../wiringPi/wiringPi.h"

extern int wpMode ;

#ifndef TRUE
#  define       TRUE    (1==1)
#  define       FALSE   (1==2)
#endif

/*
 * doReadallExternal:
 *	A relatively crude way to read the pins on an external device.
 *	We don't know the input/output mode of pins, but we can tell
 *	if it's an analog pin or a digital one...
 *********************************************************************************
 */

static void doReadallExternal (void)
{
  int pin ;

  printf ("+------+---------+--------+\n") ;
  printf ("|  Pin | Digital | Analog |\n") ;
  printf ("+------+---------+--------+\n") ;

  for (pin = wiringPiNodes->pinBase ; pin <= wiringPiNodes->pinMax ; ++pin)
    printf ("| %4d |  %4d   |  %4d  |\n", pin, digitalRead (pin), analogRead (pin)) ;

  printf ("+------+---------+--------+\n") ;
}


/*
 * doReadall:
 *	Read all the GPIO pins
 *	We also want to use this to read the state of pins on an externally
 *	connected device, so we need to do some fiddling with the internal
 *	wiringPi node structures - since the gpio command can only use
 *	one external device at a time, we'll use that to our advantage...
 *********************************************************************************
 */

static char *alts [] =
{
  "IN", "OUT", "ALT5", "ALT4", "ALT0", "ALT1", "ALT2", "OFF"
} ;


/*
 * readallPhys:
 *	Given a physical pin output the data on it and the next pin:
 *| BCM | wPi |   Name  | Mode | Val| Physical |Val | Mode | Name    | wPi | BCM |
 *********************************************************************************
 */

static void readallPhys (int physPin)
{
  int pin ;

  if (physPinToGpio (physPin) == -1)
    printf ("  |      |    ") ;
  else
    printf ("  |  %3d | %3d", physPinToGpio (physPin), physToGpio_BP [physPin]) ;

  printf (" | %s", physNames [physPin]) ;

  if (physToGpio_BP [physPin] == -1)
    printf (" |      |  ") ;
  else
  {
    /**/ if (wpMode == WPI_MODE_GPIO)
      pin = physPinToGpio (physPin) ;
    else if (wpMode == WPI_MODE_PHYS)
      pin = physPin ;
    else
      pin = physToGpio_BP [physPin] ;

    printf (" | %4s", alts [getAlt (pin)]) ;
    printf (" | %d", digitalRead (pin)) ;
  };

// Pin numbers:

  printf (" | %2d", physPin) ;
  ++physPin ;
  printf (" || %-2d", physPin) ;

// Same, reversed

  if (physToGpio_BP [physPin] == -1)
    printf (" |   |     ") ;
  else
  {
    /**/ if (wpMode == WPI_MODE_GPIO)
      pin = physPinToGpio (physPin) ;
    else if (wpMode == WPI_MODE_PHYS)
      pin = physPin ;
    else
      pin = physToGpio_BP [physPin] ;

    printf (" | %d", digitalRead (pin)) ;
    printf (" | %-4s", alts [getAlt (pin)]) ;
  };

  printf (" | %-5s", physNames [physPin]) ;

  if (physToGpio_BP [physPin] == -1)
    printf (" |     |     ") ;
  else
    printf (" | %-3d | %-3d ", physToGpio_BP [physPin], physPinToGpio (physPin)) ;

  printf (" |\n") ;
};


void cmReadall (void)
{
  int pin ;

  printf ("  +----------+-----+------+-------+\n") ;
  printf ("  |   GPIO   | wPi | Mode | Value |\n") ;
  printf ("  +----------+-----+------+-------+\n") ;

  for (pin = 0 ; pin < 384 ; ++pin)
  {
     if (wpiPinToGpio (pin) == -1)
	continue ;
    printf ("  | gpio-%-3d",wpiPinToGpio (pin)) ;
    printf (" | %-3d",pin) ;
    printf (" | %-4s", alts [getAlt (pin)]) ;
    printf (" | %s  |\n", digitalRead (pin) == HIGH ? "High" : "Low ") ;

  }

  printf ("  +----------+-----+------+-------+\n") ;
}


/*
 * abReadall:
 *	Read all the pins on the model A or B.
 *********************************************************************************
 */

void abReadall (int model, int rev)
{
  int pin ;
  char *type ;

  if (model == PI_MODEL_A)
    type = " A" ;
  else
    if (rev == PI_VERSION_2)
      type = "B2" ;
    else
      type = "B1" ;

  printf ("+-----+-----+---------+------+---+-Model %s-+---+------+---------+-----+-----+\n", type) ;
  printf ("| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |\n") ;
  printf ("+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+\n") ;
  for (pin = 1 ; pin <= 26 ; pin += 2)
    readallPhys (pin) ;

  if (rev == PI_VERSION_2) // B version 2
  {
    printf ("+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+\n") ;
    for (pin = 51 ; pin <= 54 ; pin += 2)
      readallPhys (pin) ;
  }

  printf ("+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+\n") ;
  printf ("| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |\n") ;
  printf ("+-----+-----+---------+------+---+-Model %s-+---+------+---------+-----+-----+\n", type) ;
}


/*
 * bPlusReadall:
 *	Read all the pins on the model B+
 *********************************************************************************
 */

void bPlusReadall (void)
{
  int pin ;

  printf ("+-----+-----+---------+------+---+--B Plus--+---+------+---------+-----+-----+\n") ;
  printf ("| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |\n") ;
  printf ("+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+\n") ;
  for (pin = 1 ; pin <= 40 ; pin += 2)
    readallPhys (pin) ;
  printf ("+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+\n") ;
  printf ("| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |\n") ;
  printf ("+-----+-----+---------+------+---+--B Plus--+---+------+---------+-----+-----+\n") ;
}

//add for BananaPro by lemaker team
void BPReadAll(void)
{
  int pin ;

  printf ("+-----+-----+---------+------+---+--Banana Pro--+------+---------+-----+-----+\n") ;
  printf ("| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |\n") ;
  printf ("+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+\n") ;
  for (pin = 1 ; pin <= 40 ; pin += 2)
    readallPhys (pin) ;
  printf ("+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+\n") ;
  printf ("| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |\n") ;
  printf ("+-----+-----+---------+------+---+--Banana Pro--+------+---------+-----+-----+\n") ;	
}
//end 2014.09.26

// changed by Christian Beckert
void OrangePiReadAll(void)
{
  int pin ;

  char name [80] ;
  FILE *fd ;

  if ((fd = fopen ("/proc/device-tree/model", "r")) != NULL)
  {
    if ( fgets (name, 80, fd) ){;}
    fclose (fd) ;
  }

  printf ("\n%-27s  %-30s%25s\n", "  +------+-----+----------+", name, "+----------+-----+------+"); //%-20s %-40s %19s
  printf ("  | GPIO | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | GPIO |\n") ;
  printf ("  +------+-----+----------+------+---+----++----+---+------+----------+-----+------+\n") ;

#if CONFIG_ORANGEPI_H3 || defined CONFIG_ORANGEPI_RK3399 || CONFIG_ORANGEPI_4 || CONFIG_ORANGEPI_PC2 || CONFIG_ORANGEPI_PRIME || CONFIG_ORANGEPI_WIN
  for (pin = 1 ; pin <= 40; pin += 2)

#elif CONFIG_ORANGEPI_ZERO || CONFIG_ORANGEPI_R1 || CONFIG_ORANGEPI_LITE2 || CONFIG_ORANGEPI_ZEROPLUS2_H3 || CONFIG_ORANGEPI_3 || CONFIG_ORANGEPI_ZEROPLUS || CONFIG_ORANGEPI_ZEROPLUS2_H5 || CONFIG_ORANGEPI_800
  for (pin = 1 ; pin <= 26 ; pin += 2)

#elif CONFIG_ORANGEPI_R1PLUS
  for (pin = 1 ; pin <= 13; pin += 2)

#elif CONFIG_ORANGEPI_ZERO2
  for (pin = 1 ; pin <= 34; pin += 2)

#endif

    readallPhys (pin) ;

#if CONFIG_ORANGEPI_ZERO
  printf ("  +------+-----+----------+------+---+----++----+---+------+----------+-----+------+\n") ;
  readallPhys (27) ;
#endif /* ORANGEPI_ZERO */

  printf ("  +------+-----+----------+------+---+----++----+---+------+----------+-----+------+\n") ;
  printf ("  | GPIO | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | GPIO |\n") ;
  printf ("%-27s  %-30s%25s\n\n", "  +------+-----+----------+", name, "+----------+-----+------+"); //%-20s %-40s %25s
}
// changed by Christian Beckert

void doReadall (void)
{
  int model, rev, mem, maker, overVolted ;

  if (wiringPiNodes != NULL)	// External readall
  {
    doReadallExternal () ;
    return ;
  }

  piBoardId (&model, &rev, &mem, &maker, &overVolted) ;

  /**/ if ((model == PI_MODEL_A) || (model == PI_MODEL_B))
    abReadall (model, rev) ;
  else if (model == PI_MODEL_BP) 
    bPlusReadall () ;
  else if (model == PI_MODEL_CM)
    cmReadall () ;
  else if (model == PI_MODEL_OPZ) //add for BananaPro by lemaker team
     OrangePiReadAll();	//guenter / // changed by Christian Beckert 
  else
    printf ("Oops - unable to determine board type... model: %d\n", model) ;
}
