//   initialization waits for DCO to stabilize against ACLK.
//   ACLK = ~32.768kHz, MCLK = SMCLK = DCO ~ 1048kHz.  BRCLK = SMCLK/2

#include <msp430.h>
#include "deffs.h"

#ifdef SPI_DMA
char SPI_RX_Buff0[SRCV_BUFF_SIZE];

void spi_clear_buff(char * Buff, unsigned short len)
{
    while(len--)
  {
    SBAdd(*Buff);
    Buff++;
  }
}
#ifndef USE_SPI_DMA_INTERRUPT
void spi_dma_rcv(void)
{
    unsigned short len;
    unsigned short val;
    DMA2CTL &= ~(DMAEN); //170412
    val = DMA2SZ;
    DMA2CTL |= DMAEN;
    if(val < SRCV_BUFF_SIZE)
    {
        DMA2CTL &= ~(DMAEN);
        if(!spi_dma_rxbuf_flag)
        {
            __data16_write_addr((unsigned short) &DMA2DA,(unsigned long) &SPI_RX_Buff1[0]);
            spi_dma_rxbuf_flag = 1;
        }
        else
        {
            __data16_write_addr((unsigned short) &DMA2DA,(unsigned long) &SPI_RX_Buff0[0]);
             spi_dma_rxbuf_flag = 0;
        }
        DMA2SZ = (unsigned short)SRCV_BUFF_SIZE;
        DMA2CTL |= DMAEN;
        len = (unsigned short)SRCV_BUFF_SIZE - val; //4 -for test
        spi_dma_rxbuf_flag ? spi_clear_buff(&SPI_RX_Buff0[0], len) : spi_clear_buff(&SPI_RX_Buff1[0], len);
     }
}
#else
void spi_dma_rcv(void)
{
    if(spilen)
    {
    spi_clear_buff(&SPI_RX_Buff0[0], spilen);
    spilen = 0;
    DMA2CTL |= DMAEN;
    }
}
#endif
#endif

void spi_task(void)
{
   static char is_spi_ok = 0;
   if(spitimer < SPI_TIME) return;
    spitimer = 0;
#ifdef SPISIML
    spi_transmit();
#endif
#ifdef SPI_DMA
   spi_dma_rcv();
#endif
   if(SParseBuffer(0))
   {is_spi_ok = 0;}
   else{is_spi_ok++;}
   if(is_spi_ok > WD_SPI)
   {spi_init();  is_spi_ok = 0;  }
   if(send_enabled)
   {
   SendData();
   }
}

void spi_init(void)
{
#ifdef DPRINT
printf("\n\r spi init");
#endif
#ifndef GPS_PLATA
  PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs  
  P1MAP3 = PM_UCB0SIMO;                     // Map UCB0SIMO  to P1.3
  P1MAP2 = PM_UCB0SOMI;                     // Map UCB0SOMI to P1.2
  P1MAP4 = PM_UCB0CLK;                      // Map UCB0CLK output to P1.4
  PMAPPWD = 0;                              // Lock port mapping registers  
   
  P1DIR |= BIT2; //BIT2;            // ACLK, MCLK, SMCLK set out to pins
  P1SEL |= BIT3 + BIT2 + BIT4;   // debugging purposes.

  #else
  PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
    P2MAP1 = PM_UCA0SIMO;                     // Map UCA0SIMO output to P2.0
    P2MAP3 = PM_UCA0SOMI;                     // Map UCA0SOMI output to P2.2
    P2MAP5 = PM_UCA0CLK;                      // Map UCA0CLK output to P2.4
    PMAPPWD = 0;                              // Lock port mapping registers

   P2DIR |= BIT1 + BIT3 + BIT5;              // ACLK, MCLK, SMCLK set out to pins
   P2SEL |= BIT1 + BIT3 + BIT5;              // P2.0,2,4 for debugging purposes.
#endif
  UCB0CTL1 |= UCSWRST;  // **Put state machine in reset**

#ifdef MASTER
#ifndef  SPI_POLARITY_LOW
  UCB0CTL0 |= UCMST+UCSYNC+UCCKPL+UCMSB;    // 3-pin, 8-bit SPI master
#else
  UCB0CTL0 |= UCMST+UCSYNC+UCMSB;    // 3-pin, 8-bit SPI master
#endif
#else
#ifndef  SPI_POLARITY_LOW
  UCB0CTL0 = UCSYNC+UCCKPL+UCMSB;               //
#else
  UCB0CTL0 = UCSYNC+UCMSB;               //slave
#endif
#endif
                                          // Clock polarity high, MSB
  UCB0CTL1 |= UCSSEL_2;                     // SMCLK
  UCB0BR0 = 0;
  UCB0BR1 = 0;
  UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

#ifndef SPI_DMA
  UCB0IE |= UCRXIE;   //without this do not receive?
#endif

#ifdef SPISIML
   UCB0STAT |= UCLISTEN;  //temporary loop
#endif

#ifdef SPI_DMA
   DMACTL1 |= DMA2TSEL_18;                       //UCB0 receive ?
   DMACTL4 |= ROUNDROBIN;
  __data16_write_addr((unsigned short) &DMA2SA,(unsigned long) &UCB0RXBUF);
  __data16_write_addr((unsigned short) &DMA2DA,(unsigned long) &SPI_RX_Buff0[0]);
  DMA2SZ = INT_SIZE;                               // Block size// Block size

#ifdef USE_SPI_DMA_INTERRUPT
   DMA2CTL = DMADSTINCR_3+DMASBDB+DMALEVEL+DMADT_0+DMAEN  + DMAIE;  // dst increment  ???
#else
   DMA2CTL = DMADSTINCR_3+DMASBDB+DMALEVEL+DMADT_0+DMAEN;
#endif
#endif
}

char CheckCrc(char * Buff, int len)
{
 char ret = 0;
 int i;
 unsigned char crc= 0;
 for(i = 1; i < len - 2; i++)
 {
     crc+=Buff[i];
 }
 crc -=1;
 if(Buff[len-2] == crc) {ret = 1;}
 return ret;
}

#ifdef SPISIML

void AddCrc(char * Buff, int len)
{
 int i;
 unsigned char crc= 0;
 for(i = 1; i < len - 2; i++) //Buff[0] do not used (= $)
 {
     crc+=Buff[i];
 }
 crc -=1;
 Buff[len-2] = crc;
}

void spi_transmit(void)
{
//   struct Controller_ControlFrame {
//       long dt, lat, lon, alt, course, speed; // 6*4 bytes
//   };
#ifndef USE_SERIAL_GPS
 char TestData1[] ={'$','0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','l','m','n','o','p','q','r','*'};
#else

  char TestData1[] ={'$','0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','l','m','n','o','p','q',0x12,0x34,'t','*'};
 //char TestData1[] ={'$','0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','l','m','n','o','p','q',0x12,0x34,'t','+'};
 #endif
#ifndef SEND_6
//  long dt = 1492420083;
  double lat = 59.934280;
  double lon = 30.335099;
//  long course = 12345678;
//  long speed = 87654321;
   dt++;
   memcpy(&TestData1[1], &dt, sizeof(long));
   memcpy(&TestData1[1 + sizeof(long)], &lat, sizeof(double));
   memcpy(&TestData1[1 + sizeof(long) + sizeof(double)], &lon, sizeof(double));
//serial is in shablon
#else
    long dt = 1492420083;
    long lat = 59934280;
    long lon = 30335099;
    long alt = 10;
    long course = 12345678;
    long speed = 87654321;

     memcpy(&TestData1[1], &dt, sizeof(long));
     memcpy(&TestData1[1 + sizeof(long)]  , &lat, sizeof(long));
     memcpy(&TestData1[1 + sizeof(long)*2], &lon, sizeof(long));
     memcpy(&TestData1[1 + sizeof(long)*3], &alt, sizeof(long));
     memcpy(&TestData1[1 + sizeof(long)*4], &course, sizeof(long));
     memcpy(&TestData1[1 + sizeof(long)*5], &speed, sizeof(long));
     //serial is in shablon
#endif

   spi_send(TestData1, ONLINE_DATA_SIZE);
}

void spi_send(char* TestData1, int size)
{
 //   printf("\n\r spi send");
   int i;
   AddCrc( TestData1, size);

   UCB0TXBUF = TestData1[0];
   while (!(UCB0IFG&UCTXIFG));
   for(i = 1; i < size; i++) //t
   {
     while (!(UCB0IFG&UCTXIFG));
     switch(TestData1[i])
     {
     case '\\':  UCB0TXBUF = TestData1[i];
                  while (!(UCB0IFG&UCTXIFG));
                  __delay_cycles(40);
                  UCB0TXBUF = '0';  break;
     case '$' :   UCB0TXBUF = '\\';
                     while (!(UCB0IFG&UCTXIFG));
                   __delay_cycles(40);
                  UCB0TXBUF = '1';  break;
#ifdef SEND_DIFF
     case '^' :   UCB0TXBUF = '\\';  //diff frame
                     while (!(UCB0IFG&UCTXIFG));
                   __delay_cycles(40);
                  UCB0TXBUF = '2';  break;
#endif

     default :
                 UCB0TXBUF = TestData1[i];                 // Send next value
     }
      __delay_cycles(40);
    }
}
#endif //SIML

#ifndef SPI_DMA

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
    SBAdd(UCB0RXBUF);
}
#endif

#ifdef USE_SPI_DMA_INTERRUPT
#pragma vector=DMA_VECTOR
__interrupt void DMA_ISR(void)
{
//    static int cnt = 0;
 switch(__even_in_range(DMAIV,16))
  {
    case 0: break;
    case 2: tx_enable = 1;                  // DMA0IFG = DMA Channel 0
            break;
    case 4:
     //       cnt++;
       //     printf("i%d", cnt);
            timerreload = 50;
            DMA1CTL &= ~DMAEN;
            DMA1SZ = (unsigned short)RCV_BUFF_SIZE; //to have interrupt fom first received symbol
            DMA1CTL |= DMAEN;
            break;                              // DMA1IFG = DMA Channel 1
    case 6: spitimer = SPI_TIME;            //to momentally  data operating
            spilen =   INT_SIZE;
            break;                          // DMA2IFG = DMA Channel 2
    case 8: break;                          // DMA3IFG = DMA Channel 3
    case 10: break;                         // DMA4IFG = DMA Channel 4
    case 12: break;                         // DMA5IFG = DMA Channel 5
    case 14: break;                         // DMA6IFG = DMA Channel 6
    case 16: break;                         // DMA7IFG = DMA Channel 7
    default: break;
  }
}
#endif



