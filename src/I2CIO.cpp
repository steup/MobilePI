// ---------------------------------------------------------------------------
// Created by Francisco Malpartida on 20/08/11.
// Copyright 2011 - Under creative commons license 3.0:
//        Attribution-ShareAlike CC BY-SA
//
// This software is furnished "as is", without technical support, and with no 
// warranty, express or implied, as to its usefulness for any purpose.
//
// Thread Safe: No
// Extendable: Yes
//
// @file I2CIO.h
// This file implements a basic IO library using the PCF8574 I2C IO Expander
// chip.
// 
// @brief 
// Implement a basic IO library to drive the PCF8574* I2C IO Expander ASIC.
// The library implements basic IO general methods to configure IO pin direction
// read and write uint8_t operations and basic pin level routines to set or read
// a particular IO port.
//
//
// @version API 1.0.0
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#include <inttypes.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <stdio.h>

#include "I2CIO.h"

int dev;

// CLASS VARIABLES
// ---------------------------------------------------------------------------


// CONSTRUCTOR
// ---------------------------------------------------------------------------
I2CIO::I2CIO(){
   _i2cAddr  = 0x0;
   _dirMask  = 0xFF;    // mark all as INPUTs
   _shadow   = 0x0;     // no values set
   _dev      = 0;
}

// PUBLIC METHODS
// ---------------------------------------------------------------------------

//
// begin
int I2CIO::begin (  uint8_t i2cAddr )
{
   _i2cAddr = i2cAddr;

   _dev = open("/dev/i2c-1", O_RDWR);
   if(_dev < 0)
	  exit(_dev);
   printf("bus opened: %d\n", dev);

   if( int ret = ioctl(_dev, I2C_SLAVE, _i2cAddr) <0 )
	  exit(ret);

  printf("device locked: %x\n", _i2cAddr);

   _shadow = read();
   
   return ( _dev );
}

//
// pinMode
void I2CIO::pinMode ( uint8_t pin, uint8_t dir )
{
   if ( _dev )
   {
      if ( dir == 1 )
      {
         _dirMask &= ~( 1 << pin );
      }
      else 
      {
         _dirMask |= ( 1 << pin );
      }
   }
}

//
// portMode
void I2CIO::portMode ( uint8_t dir )
{
   
   if ( _dev )
   {
      if ( dir == 0 )
      {
         _dirMask = 0xFF;
      }
      else
      {
         _dirMask = 0x00;
      }
   }
}

//
// read
uint8_t I2CIO::read ( void )
{
   uint8_t retVal = 0;
   
   if ( _dev )
   {
      if(::read(_dev, &retVal, 1)!=1)
	perror("Error reading from i2c: ");
      retVal &= _dirMask;      
   }
   return ( retVal );
}

//
// write
int I2CIO::write ( uint8_t value )
{
   int status = 0;
   
   if ( _dev )
   {
      // Only write HIGH the values of the ports that have been initialised as
      // outputs updating the output shadow of the device
      _shadow = ( value & ~(_dirMask) );
   
      if(::write( _dev, &value, 1 )!=1)
	perror("Error reading from i2c: ");
   }
   return ( (status == 0) );
}

//
// digitalRead
uint8_t I2CIO::digitalRead ( uint8_t pin )
{
   uint8_t pinVal = 0;
   
   // Check if initialised and that the pin is within range of the device
   // -------------------------------------------------------------------
   if ( ( _dev ) && ( pin <= 7 ) )
   {
      // Remove the values which are not inputs and get the value of the pin
      pinVal = this->read() & _dirMask;
      pinVal = ( pinVal >> pin ) & 0x01; // Get the pin value
   }
   return (pinVal);
}

//
// digitalWrite
int I2CIO::digitalWrite ( uint8_t pin, uint8_t level )
{
   uint8_t writeVal;
   int status = 0;
   
   // Check if initialised and that the pin is within range of the device
   // -------------------------------------------------------------------
   if ( ( _dev ) && ( pin <= 7 ) )
   {
      // Only write to HIGH the port if the port has been configured as
      // an OUTPUT pin. Add the new state of the pin to the shadow
      writeVal = ( 1 << pin ) & ~_dirMask;
      if ( level == 1 )
      {
         _shadow |= writeVal;
                                                      
      }
      else 
      {
         _shadow &= ~writeVal;
      }
      status = this->write ( _shadow );
   }
   return ( status );
}

void I2CIO::delay(uint16_t value, TimeUnit unit){
  uint32_t nom, denom;
  convert(unit, TimeUnit::us, nom, denom);
  usleep(value*nom/denom);
}

//
// PRIVATE METHODS
// ---------------------------------------------------------------------------
