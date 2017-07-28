
#include <msp430.h>
#include "deffs.h"

unsigned int t1;
unsigned int spitimer;
char bcl;
unsigned char init_finished;
unsigned char restart;
unsigned short serial;
char Fill_lifo_data;
unsigned char reload;
unsigned int timerreload;
char TestData1[TX_SEND_SIZE]; // buffer for resend data
char KeepTestData1[TX_SEND_SIZE];  //buffer for sended data
unsigned short used_len;
unsigned char auth_flag;
unsigned char spilen;
unsigned char send_enabled;
unsigned char need_set_alarm;
unsigned char tx_enable;
unsigned short setted_size;

#ifdef SPISIML
long dt;
#endif

void params_init()
{
#ifdef SPISIML
    dt = 1492420083;
#endif
    Fill_lifo_data = 0;
    restart = 0;
    serial = 0;
    reload = 1;
    timerreload = 0;
    used_len = 0;
    auth_flag = 0;
    spitimer  = 0;
    spilen = 0;
    send_enabled = 1; //start with sending some
    tx_enable = 1;
}

void leds_init()
{
    P2DIR |= 0x02 + 0x04 + 0x08;              // P1.0 = Output
    P2OUT &= ~0x02;                           // Clear P1.0
    P2OUT &= ~0x04;
    P2OUT &= ~0x08;
}

void leds(char num, char state)
{
    switch(num)
    {
    case 0 : if(state > 0) {  P2OUT |= 0x02;} else { P2OUT &= ~0x02; } break;
    case 1 : if(state > 0) {  P2OUT |= 0x04;} else { P2OUT &= ~0x04; } break;
    case 2 : if(state > 0) {  P2OUT |= 0x08;} else { P2OUT &= ~0x08; } break;
    default :  break;
    }
}

void leds_on(void)
{
    P2OUT |= 0x0E;
}

void leds_off(void)
{
    P2OUT &= ~0x0E;
}

void led_toggle(char num)
{
    if(!key_pressed())  //toggle if key pressed only
    {
        P2OUT &= 0x01; //get off leds
    }
    else
    {
    switch(num)
       {
       case 0 :  P2OUT ^= 0x02; break;
       case 1 :  P2OUT ^= 0x04; break;
       case 2 :  P2OUT ^= 0x08; break;
       default :  break;
       }
    }
}

void wd_reset(void)
{
   if(!restart) WDTCTL = WDT_ARST_1000;
}

unsigned char key_pressed(void)
{
   return((P2IN & BIT0) ? 0 : 1);
}

void int_init()
{
    WDTCTL = WDTPW + WDTHOLD;
    TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
    TA1CCR0 = TIMER_A_50; // to have 50 ms interrupt period
    TA1CTL = TASSEL_2 + MC_2 + TACLR;         // SMCLK, contmode, clear TAR
    t1 = 0;
     __bis_SR_register(GIE);
}

void wait_ms(unsigned int ms)
{
   unsigned int t;
   for (t=0; t<(ms<<2); t++)
   {
    wd_reset();
    wait_us(250);
   }
}

void PortsInit(void)
{
  P1DIR = BIT0; //output
  P2DIR = 0x00;
  P3DIR = 0x00;
  P4DIR = 0x00;
  P5DIR = 0x00;

  P1OUT = BIT0; //output high
  P2OUT = BIT0; //p2.0 : input with pullup resistor
  P3OUT = 0x00;
  P4OUT = 0x00;
  P5OUT = 0x00;

  P2REN = BIT0;
}

void init(void)
{
    PortsInit();
    BInit();
    params_init();
    leds_init();
    SBInit();
    dma_init();
    spi_init();
    int_init();
    sdcard_init();
#ifndef CONTINUOUS_SEND
    rtc_init();
#endif
    WDTCTL = WDT_ARST_1000;
}

int main(void)
{
     init();
    __bis_SR_register(LPM3_bits + GIE);
    __no_operation();
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
  t1 += 50;
  if(timerreload > 50) timerreload -= 50;
  else timerreload = 0;
  spitimer  += 50;
  TA1CCR0 = TIMER_A_50;
  spi_task();
  wd_reset();
}
