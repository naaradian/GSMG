#include <msp430.h>
#include <stdio.h>
#include <time.h>
#include "deffs.h"

#ifndef CONTINUOUS_SEND
void rtc_init(void)
{

  RTCCTL01 |= RTCMODE  | RTCAIE; //calendar mode; do not use rtcrdyie every sec interrupt
#ifdef VIEW_TIME
  RTCCTL01 |= RTCRDYIE;//for debug to view time every sec
#endif
  RTCCTL01 |= RTCHOLD; // RTC hold
  // Set any time
  RTCYEAR = 2017;
  RTCMON = 7;
  RTCDAY = 28;
  RTCDOW = 5;
  RTCHOUR = 9;
  RTCMIN = 1;
  RTCSEC = 0;

#ifndef DAY_SHEDULE
  RTCADOW  &= ~0x80;
  RTCADAY  &= ~0x80;
  RTCAHOUR &= ~0x80;
  if((RTCMIN+ MIN_ALARM) < 60)
 {
  RTCAMIN =  RTCMIN + MIN_ALARM | 0x80; // RTC Minute Alarm and enable
 }
  else
 {
   RTCAMIN =  (RTCMIN + MIN_ALARM - 60) | 0x80; // RTC Minute Alarm and enable
 }
#else
  RTCAHOUR = HOUR_ALARM | 0x80;
  RTCAMIN =  MIN_ALARM | 0x80;
  RTCADOW  &= ~0x80;
  RTCADAY  &= ~0x80;
#endif
  RTCCTL01 &= ~(RTCHOLD);
}

#ifdef DAY_SHEDULE

unsigned char no_before_alarm(unsigned char min)
{
    if( (!(MIN_ALARM)) && ((min == 59) || (!min))) {return 0;}
    else if((min == MIN_ALARM) || (min == (MIN_ALARM - 1))){ return 0;}
    return 1;
}

void correct_rtc(char * pepoch)
{
    unsigned long epoch = (unsigned long)*pepoch + ((unsigned long)*(pepoch + 1) << 8) +  ((unsigned long)*(pepoch + 2) << 16) +  ((unsigned long)*(pepoch +3) << 24);
    unsigned long sec_of_day;
    unsigned char hour, min, sec;
    unsigned long div;
    div =  24l * 60l * 60l;
    sec_of_day = epoch % div;
    hour       = sec_of_day / (60 * 60);
    min     = sec_of_day % (60 * 60) / 60;
    sec     = epoch % 60;
//    printf("\n\r %d:%d:%d ", hour, min, sec);
    if(no_before_alarm(min))
    {
    RTCCTL01 |= RTCHOLD;
    RTCHOUR = hour;
    RTCMIN = min;
    RTCSEC = sec;
    RTCCTL01 &= ~(RTCHOLD);
    }
}

#endif

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
#ifdef VIEW_TIME
#ifdef DPRINT
    static int cnt = 0;
    cnt++;
    printf("\n\r rtc int %d ", cnt);
    printf("  %d:%d:%d",RTCHOUR, RTCMIN,RTCSEC);
    printf(" alarm :  %d:%d",HOUR_ALARM, MIN_ALARM);
#endif
#endif

  switch(__even_in_range(RTCIV,16))
  {
    case 0: break;                          // No interrupts
    case 2: break;                          // RTCRDYIFG
    case 4: break;
    case 6: //printf(" rtc 6 %d:%d",RTCHOUR, RTCMIN);
            send_enabled = 1;
#ifndef DAY_SHEDULE
        if((RTCMIN+ MIN_ALARM) < 60)
        {
         RTCAMIN =  RTCMIN + MIN_ALARM | 0x80; // RTC Minute Alarm and enable
        }
         else
        {
          RTCAMIN =  (RTCMIN + MIN_ALARM - 60) | 0x80; // RTC Minute Alarm and enable
        }
 #endif
        break;                          // RTCAIFG
    case 8:  break;                          // RT0PSIFG
    case 10: break;                         // RT1PSIFG
    case 12: break;                         // Reserved
    case 14: break;                         // Reserved
    case 16: break;                         // Reserved
    default: break;
  }
}
#endif // CONTINUOUS_SEND
