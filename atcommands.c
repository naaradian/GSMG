#include "deffs.h"
#include <msp430.h>

char data_commandT1(char * Data, int tm , char type, int len )
{
    char ret;
#ifdef SIML
    static char TData[]={0,0,0,0,0};
    static char TDataB[]="\n\r 12,34,5678 OK\n\r";
#endif
    ret = dma_rcv(type);
    BInit(); // prepare receive buffer
    dma_send(Data, len);
    if(type != 14)  //if no auth to do not operating to message "connect"
    {
    DMA1CTL &= ~DMAEN;
    DMA1SZ = (unsigned short)1; //to have interrupt fom first received symbol
    DMA1CTL |= DMAEN;
    }

#ifdef SIML
  switch (type)
  {
  case 9 : dma_rcv(type);   BInit();  dma_send(&TData[0], 5); break;
  case 12 : dma_rcv(type);   BInit();  dma_send(&TDataB[0], 17); break;
  }
#endif //SIML
  return ret;
}

char modem_send_dataT1(char* Data, int len, char type)  //big packet
{
   return data_commandT1(Data,WAIT_ANS, type, len); //3000!
}

void modem_reset_r(void)
{
#ifndef GPS_PLATA
   P1DIR |= 0x01;
   wait_ms(1000);
   P1OUT &= ~0x01;
   leds_on();
   wait_ms(500);
   leds_off();
   P1OUT |= 0x01;
   wait_ms(1000);
   P1OUT &= ~0x01;
   wait_ms(500);
   P1OUT |= 0x01;
   wait_ms(1000);
   P1OUT &= ~0x01;
   wait_ms(500);
   P1OUT |= 0x01;
#endif
}

void modem_down(void)
{
#ifndef GPS_PLATA
    P1DIR |= 0x01;
     P1OUT |= 0x01;
    wait_ms(1000);
    P1OUT &= ~0x01;
    wait_ms(1000);
    P1OUT |= 0x01;
    wait_ms(1000);
#endif
}

void modem_on(void)
{
#ifndef GPS_PLATA
    P5DIR |= 0x40;                            // P1.0 = Output
    P5OUT &= ~0x40;
    wait_ms(1100);
    P5OUT |= 0x40;
#endif
}

void my_sprintf(unsigned char *dst, unsigned short val)
{
    unsigned char tmp;
    unsigned char *p = dst;
    unsigned char flag = 0;
    if(val < 0xfff) {*p =' ';} else {flag = 1; tmp = (val - 0xfff) >> 12;   if(tmp < 10) *p = tmp + 0x30; else *p = tmp + 87;}
    val &= 0xfff;
    p++;
    if(val < 0xff) {if(flag) *p ='0'; else *p = ' ';} else {flag = 1; tmp = (val - 0xff) >> 8;   if(tmp < 10) *p = tmp + 0x30; else *p = tmp + 87;}
    val &= 0xff;
    p++;
    if(val < 0xf) {if(flag) *p ='0'; else *p = ' ';} else {tmp = (val - 0xf) >> 8;   if(tmp < 10) *p = tmp + 0x30; else *p = tmp + 87;}
    val &= 0xf;
    p++;
    if(val < 10) *p = val + 0x30; else *p = val + 87;
}

void ReloadModem(void)
{
    static unsigned char wd;
    static unsigned char step = 0;
  //  int i;
    char auth[] =   {6,'X','X','X','X','#','1',6,'M','S','P','4','3','0',7,'V','1','7','0','7', '2','4',0};  //addctrl-z && whu 2 ?
    if(timerreload)
    {
      return;
    }
#ifdef DPRINT
    printf("\n\rstep:%d\n\r", step);
#endif
    switch(step)
    {
    case  0:   modem_down(); modem_reset_r();  timerreload = 2000; set_uart_9600();   dma_init();  wd = 20; break;
    case  1:   modem_send_dataT1("AT+IPR=57600\r", 14, 13); set_uart_57600(); timerreload = 200; break;
    case  2:   modem_send_dataT1("AT+CBC\r ", 8 , 12); timerreload = 100; break;
    case  3:   modem_send_dataT1("AT+CMEE=1\r", 11 , 6); timerreload = 100;  if((!bcl) && (wd--)) step = 1; break; //check previons request of battery
    case  4:   modem_send_dataT1("AT+CIPCLOSE\r", 13, 13); timerreload = 100; break;
    case  5:   modem_send_dataT1("AT+CBAND=\"UMTS_I_MODE\"\r", 25, 13); timerreload = 200; break;
    case  6:   modem_send_dataT1("AT+CIPMODE=1\r", 14, 13); timerreload = 500; break;
    case  7:   modem_send_dataT1("AT+CIPSHUT\r", 12, 13); timerreload = 500; break;
    case  8:   modem_send_dataT1("AT+CIPMODE=1\r", 14, 13); timerreload = 500; break;
    case  9:   modem_send_dataT1("AT+CIPHEAD=1\r", 12, 13); timerreload = 100; break;
    case 10:   modem_send_dataT1("AT+CSTT=\"internet\",\"\",\"\"\r", 26, 13); timerreload = 5000; break;
    case 11:   modem_send_dataT1("AT+CIICR\r", 10, 13); timerreload = 2000; break;
    case 12:   modem_send_dataT1("AT+CIFSR\r", 10, 13); timerreload = 2000; break;
    case 13:   modem_send_dataT1("ATE0\r", 6, 13); timerreload = 100; break;
    case 14:   modem_send_dataT1("AT+CIPSTART=\"TCP\",\"regatav6.ru\",\"60009\"\r", 47, 14); timerreload = 5000; wd = 0;  break;
    case 15:   my_sprintf((unsigned char *)&auth[1], (unsigned short)serial);
               modem_send_dataT1( auth, sizeof(auth), 9) ; timerreload = 3000; break;
    case 16:   if(dma_rcv(10)) //check auth
               {
                  reload = 0;
           //       printf("\n\r autorize ok\n\r");
                  auth_flag = 1;
               }
               else
               {
                  if(wd) {step = 14; wd--;} //try some times
            //      printf("\n\r wd = %d autorize wrong .?\n\r", wd);
               }
               break;
    default :  break;
    }
    if(step < 16) step ++;
    else step = 0;
}

void SendData(void)
{
#ifdef SIML
    static char TData[]={0,0,0,0,0};
#endif
    static unsigned char battery_send = 0;
    wd_reset();
    static char alarm_cnt = 0;
    static unsigned char last_send = 0;
    static unsigned short send_len; //to resend data if sending was wrong!!!!
    char TestData[] = {101, bcl};
//    int i;
    if(!tx_enable) return;

    if(reload)
    {
     ReloadModem();
     return;
    }

    if(!dma_rcv(9))
    {
       if(Fill_lifo_data)
       {

  //      printf("\n\r Send Lifo Data To Modem Wrong %d" , ALARM_V - alarm_cnt);
          if((MAX_GET_LiFO * SEND_DATA_SIZE - used_len > SEND_DATA_SIZE) && (alarm_cnt < ALARM_V))
           {
              Fill_lifo_data = 0;
          }
          else
          {
             if(alarm_cnt >= ALARM_V)
              {
                 alarm_cnt = 0;
       //          printf("\n\r...reload coord");
                 reload = 1;
                 return;
              }
          }    //max_get_lifo
       } //fill_lifo _data
       else //battery
       {
             if(alarm_cnt >= ALARM_V_BATT)
               {
                 alarm_cnt = 0;
       //          printf("\n\r...reload batt");
                 reload = 1;
                 return;
               }
       }
    }   //dma_rcv wrong
 else
    { //rcv ok
    if(last_send)
    {
  //       printf("\n\r Send Lifo Data To Modem Ok \n\r");
         Fill_lifo_data = 0;
         used_len = 0;
         last_send = 0;
     } //fill_lifo_data
     else
     {
   //   printf("\n\r Send Any Data To Modem Ok \n\r");
     }
     led_toggle(LED_MODEM);
     alarm_cnt = 0;
 }
  BInit(); // prepare receive buffer


  if(auth_flag)
    {
        auth_flag = 0;
        alarm_cnt = 0;
        if(used_len)
           {
           memcpy(TestData1, KeepTestData1,  used_len);
           send_len = used_len;
           Fill_lifo_data  = 1;
           }
    }

  if((Lifo_Used()) || Fill_lifo_data)
      {
        if(!Fill_lifo_data)
           {
            led_toggle(LED_SDCARD);
            send_len = UseLifo(&TestData1[0], &KeepTestData1[used_len]);
            used_len += send_len;
            Fill_lifo_data = 1;
           }
/*
#ifdef DPRINT
        for(i = 0; i < send_len;i++)
             {
                if(!(i% SEND_DATA_SIZE))  printf("\n\r>");
                 printf(" %d", TestData1[i]);
                 wd_reset();
             }
#endif
*/
         dma_send(TestData1, send_len);
         alarm_cnt++;
         last_send = 1;
         battery_send = 0;  //set delay to send battery
#ifdef SIML
  dma_rcv(9); BInit();  dma_send(&TData[0], 5);
#endif //SIML
     }//lifo used
  else
     {
//      printf("\n\r battery_send :%d", battery_send);
      if(battery_send < BATTERY_PERIOD)
        {
         battery_send++;
        }
      else
        {
   //     printf("\n\r Send battery :battery_send :%d", battery_send);
        battery_send = 0;
        dma_send(TestData, 2);
        alarm_cnt++;
        last_send = 0;
#ifdef SIML
  dma_rcv(9);   BInit();  dma_send(&TData[0], 5);
#endif //SIML
         }//battery send
     } //lifo not used
}






