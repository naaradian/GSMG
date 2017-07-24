/*
 * sdcard.c
 *
 *  Created on: 5 мая 2017 г.
 *      Author: user
 */
#include "deffs.h"
#include "MMC.h"
#include <msp430.h>

unsigned short cursector;
unsigned short sd_data;
unsigned int  cardSize;

int sdcard_init( void)
{
  unsigned char status = 1;
  unsigned short timeout = 150;
#ifdef DPRINT
  unsigned short tmp;
#endif
  cardSize = 0;
  while((status) && (timeout--))
  {
     status = mmcInit();
  }
   cardSize =  mmcReadCardSize();
   if(cardSize < 500) cardSize = 0; //do not use wrong sd card
#ifdef DPRINT
   tmp = (unsigned short)cardSize;
#endif
   cardSize *= 2000l; //in sectors
   wait_ms(100);
   uStateInit();
   wait_ms(100);
   uStateKeep();
#ifdef DPRINT
   printf("\n\r MMC/SD-card status %x  Size : %d (K) current sector : %d  sd_data : %d \n\r", status, tmp, cursector, sd_data);
#endif
   return 0;
}

void AddSdCard(unsigned char * buffer1)
{
    mmcWriteSector(cursector, buffer1);
    wait_ms(100); //?
    cursector++;
    if(cursector >= cardSize)  cursector = START_SECTOR;
    if( sd_data < cardSize)  sd_data++;
    uStateKeep();
}

char GetSdCard(unsigned char * buffer1)
{
   char ret = 0;
   if(sd_data)
    {
      sd_data--;
      if(cursector > START_SECTOR) cursector--;
      else cursector = cardSize - 1;
      mmcReadSector(cursector, buffer1);
      uStateKeep();
      ret = 1;
     }
#ifndef CONTINUOUS_SEND
   else
   {
   send_enabled = 0;
   reload = 1;
   modem_down();
   }
#endif
   return ret;
}

void GetSdCardBase(unsigned char * buffer1)
{
  unsigned char buffer[SD_BUFF_SIZE];
  mmcReadSector(BASE_SECTOR, buffer);
  memcpy(buffer1, buffer, STATE_SIZE);
}

void AddSdCardBase(unsigned char * buffer1)
{
  unsigned  char buffer[SD_BUFF_SIZE];
  memcpy(buffer, buffer1, STATE_SIZE);
  mmcWriteSector(BASE_SECTOR, buffer);
  wait_ms(100); //?
}

