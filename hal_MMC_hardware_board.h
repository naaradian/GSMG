//----------------------------------------------------------------------------
//  This include file contains definitions specific to the hardware board.
//----------------------------------------------------------------------------
// ********************************************************
//
//   
//
//            MSP430F169                  MMC Card 
//         -----------------          -----------------
//     /|\|              XIN|-   /|\ |                 |
//      | |                 |     |  |                 |
//      --|RST          XOUT|-    |--|Pin4/Vcc         |
//        |                 |        |                 |
//        |                 |        |                 |
//        |            P5.6 |<-------|Pin6/CD          |
//        |            P5.0 |------->|Pin1/CS          |
//        |                 |        |                 |
//        |      P5.2/SOMI1 |------->|Pin7/DOUT        |
//        |      P5.1/SIMO1 |<-------|Pin2/DIN         |
//        |      P5.3/UCLK1 |------->|Pin5/CLK         |
//        |                 |        |                 |
//        |                 |     |--|Pin3/GND         |
//                                |
//                                =
//
//  Pin configuration at MSP430F169:
//  --------------------------------
//  MSP430F169      MSP Pin        MMC             MMC Pin
//  -------------------------------------------------------------
//  P5.0              48           ChipSelect       1
//  P5.2 / SOMI       46           DataOut          7
//                                 GND              3 (0 V)
//                                 VDD              4 (3.3 V)
//  P5.3 / UCLK1      47           Clock            5
//  P5.6              44           CardDetect       6
//  P5.1 / SIMO       45           DataIn           2
//  -------------------------------------------------------------



//#include "pin_names.h"
#include <msp430.h>
#include "deffs.h"
                                     // MSP430 device being used.
//#define GPS_PLATA

#ifdef GPS_PLATA
// SPI port definitions              // Adjust the values for the chosen
#define SPI_PxSEL         P1SEL      // interfaces, according to the pin
#define SPI_PxDIR         P1DIR      // assignments indicated in the
#define SPI_PxIN          P1IN       // chosen MSP430 device datasheet.
#define SPI_PxOUT         P1OUT
#define SPI_SIMO          BIT3
#define SPI_SOMI          BIT2
#define SPI_UCLK          BIT7

#else
// SPI port definitions              // Adjust the values for the chosen
#define SPI_PxSEL         P3SEL      // interfaces, according to the pin
#define SPI_PxDIR         P3DIR      // assignments indicated in the
#define SPI_PxIN          P3IN       // chosen MSP430 device datasheet.
#define SPI_PxOUT         P3OUT
#define SPI_SIMO          BIT4
#define SPI_SOMI          BIT6
#define SPI_UCLK          BIT5  //ok


#endif

//----------------------------------------------------------------------------
// SPI/UART port selections.  Select which port will be used for the interface 
//----------------------------------------------------------------------------
//#define SPI_SER_INTF      SER_INTF_USART1  // Interface to MMC
#define SPI_SER_INTF      SER_INTF_BITBANG  // Interface to MMC

// SPI port definitions              // Adjust the values for the chosen
#define MMC_PxSEL         SPI_PxSEL      // interfaces, according to the pin
#define MMC_PxDIR         SPI_PxDIR      // assignments indicated in the
#define MMC_PxIN          SPI_PxIN       // chosen MSP430 device datasheet.
#define MMC_PxOUT         SPI_PxOUT      
#define MMC_SIMO          SPI_SIMO
#define MMC_SOMI          SPI_SOMI
#define MMC_UCLK          SPI_UCLK


#ifdef GPS_PLATA
// Chip Select
#define MMC_CS_PxOUT      P1OUT
#define MMC_CS_PxDIR      P1DIR
#define MMC_CS            BIT1
#else
// Chip Select
#define MMC_CS_PxOUT      P3OUT
#define MMC_CS_PxDIR      P3DIR
#define MMC_CS            BIT7
#endif

#ifdef GPS_PLATA
// Card Detect
#define MMC_CD_PxIN       P1IN
#define MMC_CD_PxDIR      P1DIR
#define MMC_CD            0x40
#else
// Card Detect
#define MMC_CD_PxIN       P3IN
#define MMC_CD_PxDIR      P3DIR
#define MMC_CD            BIT3
#endif


#define CS_LOW()    MMC_CS_PxOUT &= ~MMC_CS               // Card Select
#define CS_HIGH()   while(!halSPITXDONE); MMC_CS_PxOUT |= MMC_CS  // Card Deselect

#define DUMMY_CHAR 0xFF
