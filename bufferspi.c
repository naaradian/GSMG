#include "deffs.h"
#include <msp430.h>
char SRcvBuff[SRCV_BUFF_SIZE];
signed int SBuffSize;
signed int  SHead;
signed int STail;
signed int LastHead;
char fl_sl;

void SBInit()
{
   SBuffSize = SRCV_BUFF_SIZE;  //have variable  sizes of buffer!!!!!!!
   SHead = 0l;
   STail = 0l;
   memset(SRcvBuff, 0, SRCV_BUFF_SIZE);
   LastHead = 0;
   fl_sl = 0;
}
void SBAdd(char elem)
{
  if(LastHead < SBuffSize)  LastHead ++;
   SRcvBuff[SHead] = elem;
  SHead++;
  if(SHead == SBuffSize) SHead = 0;
}

char SBGet()
{
   if(LastHead)  LastHead --;
   STail++;    //t
    if(STail == SBuffSize)
   {
     STail = 0l;
     return SRcvBuff[SBuffSize-1];
   }
  else
  {
   return SRcvBuff[STail-1];   //t
  }
}

char SBGetLast()
{
    if(LastHead)  LastHead --;
    SHead--;    //t
     if(SHead  < 0)
    {
      SHead = SBuffSize - 1;
      return SRcvBuff[SBuffSize-1];
    }
   else
   {
    return SRcvBuff[SHead];   //t
   }
}

unsigned SBUsed()
{
  int n = SHead - STail;
  if( n >= 0 ) return (unsigned)n;
  else return (unsigned)(n+SBuffSize);
}
 unsigned SBUsedLast()
 {
  return (unsigned)(LastHead);
 }

unsigned char Modify(unsigned char byte)
{
     unsigned char ret;
     switch(byte)
     {
         case '0': ret = '\\' ; break;
         case '1': ret = '$'; break;
     }
      return ret;
}

char SParseBuffer(char type)
{
   char ret = 0;
   static int cnt;
   char b;
   static char Ans[ONLINE_DATA_SIZE];
   while( SBUsed())
        {
          b = SBGet();
         switch(b)
         {
         case '\\' : fl_sl = 1; break;
         case '$':
//#ifdef DPRINT
   //      if((cnt != 11) && (cnt != 10))
   //     if(1)
   //      {
   //       printf("\n\r cnt = %d ", cnt);
   //      }
//#endif
             cnt = 0;  Ans[cnt++] = 1; break; //set first element to 1
         case '*':
             if (cnt == ONLINE_DATA_SIZE-1)
             {
               if(CheckCrc(Ans, ONLINE_DATA_SIZE))
               {
                   ret = 1;
                   if(!serial) serial = (Ans[ONLINE_DATA_SIZE - 4] << 8) + Ans[ONLINE_DATA_SIZE - 3];
#ifndef CONTINUOUS_SEND
#ifdef DAY_SHEDULE
                   correct_rtc(&Ans[1]);
#endif //DAY_SHEDULE
#endif //f CONTINUOUS_SEND
                   Lifo_Add(Ans);
//#ifdef DPRINT
//               printf("\n\r * crc ok  serial : %d", serial);
//#endif

#ifndef GPS_PLATA
                 led_toggle(LED_SPI);
#endif
               } //crc
               else
               {
#ifdef DPRINT
                  printf("\n\r crc wrong");
#endif
#ifndef GPS_PLATA
                   led_toggle(LED_SPI);
#endif
               } //crc
               cnt = 0;
             } //cnt =...
             else
             {
                 Ans[cnt++] = b;
        //         printf("_%d", Ans[cnt -1]);
             }
             break;
#ifdef USE_SERIAL_GPS
         case '+':  //printf("finish \n\r ");
               if (cnt == ONLINE_DATA_SIZE-1)
               {
                 if(CheckCrc(Ans, ONLINE_DATA_SIZE))
                 {
                     ret = 1;
                     if(!serial) serial = (Ans[ONLINE_DATA_SIZE - 4] << 8) + Ans[ONLINE_DATA_SIZE - 3];
//  #ifdef DPRINT
//                    printf("\n\r+ crc ok ");
//  #endif
  #ifndef GPS_PLATA
                   led_toggle(LED_SPI);
  #endif
                 } //crc
                 else
                 {
  #ifdef DPRINT
                    printf("\n\r crc wrong");
  #endif
  #ifndef GPS_PLATA
                     led_toggle(LED_SPI);
  #endif
                 } //crc
                 cnt = 0;
               } //cnt =...
               else
               {
                   Ans[cnt++] = b;
 //                  printf("_%d", Ans[cnt -1]);
               }
               break;

#endif
         default :
        if(cnt < ONLINE_DATA_SIZE)
         {
           if(!fl_sl)
           { Ans[cnt++] = b; }
           else { Ans[cnt++] = Modify(b); fl_sl = 0;}
 //          printf("_%d", Ans[cnt -1]);
         } //if cnt <
        } //switch
     } //while
  return ret;
}


