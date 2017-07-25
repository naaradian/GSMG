/*
 * deffs.h
 *
 *  Created on: 6 апр. 2017 г.
 *      Author: user
 */
#ifndef DEFFS_H_
#define DEFFS_H_


#include <stdlib.h>
#include <string.h>


//#define SEND_6
//#define SEND_DIFF
#define USE_SERIAL_GPS

//#define SPI_POLARITY_LOW

#define USE_SPI_DMA_INTERRUPT

//#define DAY_SHEDULE
#define CONTINUOUS_SEND

#define DPRINT

#ifdef DPRINT
#include <stdio.h>
#define MAX_GET_LiFO     (25)
#else
#define MAX_GET_LiFO     (40)
#endif



//#define GPS_PLATA
//#define SIML   //modem channel
//#define SPISIML //spi channel

#ifdef SEND_6
#ifdef SEND_DIFF
extern  unsigned char connect_flag;
#endif
#endif

#define SPI_DMA
#ifdef SPISIML
#define MASTER
#endif
#ifndef SEND_6
#define SEND_DATA_SIZE   (21)  //size for server now
#else
#define SEND_DATA_SIZE   (25)  //size for server 6 longs + 1 byte type = 1
#endif
#define SEND_DATA_SIZE_DIFF (6)   //diff frame 5 bytes + 1 byte of type = 0



#define TX_SEND_SIZE     (SEND_DATA_SIZE * MAX_GET_LiFO)

#define RCV_BUFF_SIZE    (100)

#ifndef USE_SPI_DMA_INTERRUPT
#define SRCV_BUFF_SIZE   (50)
#else
#define SRCV_BUFF_SIZE   (ONLINE_DATA_SIZE<<1)
#endif
#ifndef USE_SERIAL_GPS
#define ONLINE_DATA_SIZE (27) //size of one message in future format with $ and * and checksum
#else
#define ONLINE_DATA_SIZE (29)  //+ serial
#endif
//#define MSG_LEN          (ONLINE_DATA_SIZE - 2)
#define INT_SIZE         (40)
#define MSG_LEN          (25)
#define LIFO_SIZE        (18)
#define SD_BUFF_SIZE     (512)
#define WD_SPI           (10)
extern char Fill_lifo_data;
extern void BInit();
extern char ParseBuffer(char);
extern unsigned short rlen;
extern char RcvBuff[RCV_BUFF_SIZE];
extern void SBAdd(char);
extern void SBInit(void);
extern void dma_init(void);
extern void dma_send(char * , unsigned short );
extern char dma_rcv(char);
extern unsigned int t1;
extern void sendTextMessage(void);
extern char at_command(char * ,int , char);
extern char data_command(char *,int , char, int);
extern void setEchoMode(void);
extern void delay(unsigned short); // min 16 ms
extern void sendtext(char *);
extern void senddata(char);
extern void setTCPClientConnection(void);
void getID(void);
extern char modem_start(char);
extern char modem_init(void);
extern char modem_send_data(char*, int);
extern void SendData(void);
#define TIMES    (2)
#define ALARM_V  (15) //
#define ALARM_V_BATT (2)
#define WAIT_ANS  (950)
#define SEND_TIME (1000)
#ifndef SPISIML //spi channel
#define SPI_TIME  (1400)
#else
#define SPI_TIME  (800)
#endif
#define WD_PERIOD (500)
extern void spi_task(void);
extern void spi_init(void);
extern void spi_receive(void);
extern void spi_transmit(void);
extern void spi_send(char* ,int);
extern void spi_dma_rcv(void);
extern char SParseBuffer(char);
extern char CheckCrc(char *, int);
extern unsigned int spitimer;
extern char bcl;
extern char fl_sl;
extern long dt;
extern void Lifo_Add(char *);
extern unsigned int Lifo_Used(void);
extern unsigned char Lifo_Get(char * ,char *);
extern char Lifo_Test(void);
extern unsigned short UseLifo(char *, char *);
extern int sdcard_init(void);
extern unsigned short cursector;
extern unsigned short sd_data;
extern char GetSdCard(unsigned char *);
extern void AddSdCard(unsigned char *);
extern void TestSdCard(void);
extern unsigned int cardSize;
extern void leds(char, char);
extern void led_toggle(char);
#define CYCLES_PER_US 1    // for 1MHz (default)
#define CYCLES_PER_MS 1000*(CYCLES_PER_US)
#define wait_us(us) __delay_cycles((CYCLES_PER_US)*(us))
extern void wait_ms(unsigned int);
extern void modem_reset(void);
extern void modem_on(void);
extern void modem_reset(void);
extern void modem_on(void);
extern void wd_reset(void);
#define LED_SPI     (0)
#define LED_MODEM   (1)
#define LED_SDCARD  (2)
#define ON          (1)
#define OFF         (0)
//#define TIMER_A_16    (1046 * 16)
#define TIMER_A_50    (52300) //1046*50

extern void set_uart_38400(void);
extern void set_uart_9600(void);
extern unsigned char restart;
#define START_SECTOR    (1)  //zero sector use for keep cursector
#define BASE_SECTOR     (0)
#define STATE_SIZE      (sizeof(long) * 3)
#define SIGNATURE       (0xAA557E96)
extern void GetSdCardBase(unsigned char *);
extern void AddSdCardBase(unsigned char *);
extern unsigned short uStateInit(void);
extern void uStateKeep(void);
extern unsigned short serial;
#define BATTERY_PERIOD (40)
extern char Fill_lifo_data;
extern unsigned char key_pressed(void);
extern unsigned char reload;
extern unsigned int timerreload;
extern char TestData1[TX_SEND_SIZE]; // static to property resend data
extern char KeepTestData1[TX_SEND_SIZE];  //buffer for sended data
char * CurrentKeep;
extern unsigned short used_len;
extern unsigned char auth_flag;
extern void leds_on();
extern void leds_off();
extern unsigned char spilen;
extern void rtc_init(void);
extern void correct_rtc(char *);
#define HOUR_ALARM  (9)
#define MIN_ALARM   (3)
extern void set_uart_57600(void);
extern unsigned char send_enabled;
extern void modem_down(void);
extern unsigned char need_set_alarm;
extern unsigned char tx_enable;
extern char dma_rcv_int(char);

#endif /* DEFFS_H_ */
