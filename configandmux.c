#include <stdio.h>
#include <stdlib.h>
#include <plib.h>
#include <p32xxxx.h>

#include "configandmux.h"


void pinMux01 (void)
{
    /*SPI1:*/
    PORTSetPinsDigitalIn(IOPORT_B, BIT_14);    //SCK1
    PORTSetPinsDigitalIn(IOPORT_C, BIT_8);      //SDI1
    PORTSetPinsDigitalOut(IOPORT_C, BIT_6);     //SDO1
    //PORTSetPinsDigitalIn(IOPORT_B, BIT_3);      //SS1, test, usually do not need
    PORTSetPinsDigitalOut(IOPORT_C, BIT_7);    //CS_ADF (SPI)

    /*SPI2 (I2S):*/
    PORTSetPinsDigitalIn(IOPORT_B, BIT_15);     //SCK2
    //PORTSetPinsDigitalOut(IOPORT_B, BIT_15);     //SCK2
    PORTSetPinsDigitalIn(IOPORT_C, BIT_3);      //SDI2
    PORTSetPinsDigitalOut(IOPORT_B, BIT_5);     //SDO2


    /*SPI Beaglebone (bitbang)*/
    PORTSetPinsDigitalOut(IOPORT_C, BIT_0);     //SCK_BB
    PORTSetPinsDigitalIn(IOPORT_C, BIT_2);      //SDI_alt
    PORTSetPinsDigitalOut(IOPORT_C, BIT_1);     //SDO_BB
    PORTSetPinsDigitalIn(IOPORT_B, BIT_8);      //CS_BB_IRQ


    /*INTERRUPT LINES*/
    PORTSetPinsDigitalIn(IOPORT_C, BIT_9);      //IRQ_GP3

    /*COUNTER (TIMER1)*/
    PORTSetPinsDigitalIn(IOPORT_A, BIT_4);      //T1CK

    /*CONTROL SWITCHES*/
    PORTSetPinsDigitalOut(IOPORT_B, BIT_13);    //CNTL_AND_A
    PORTSetPinsDigitalOut(IOPORT_A, BIT_1);    //CNTL_COUNTER
    PORTSetPinsDigitalOut(IOPORT_B, BIT_6);    //CNTL_BUFFER

    /*LEDS*/
    PORTSetPinsDigitalOut(IOPORT_C, BIT_5);     //LED 1
    PORTSetPinsDigitalOut(IOPORT_A, BIT_9);     //LED 2


    /*PWM (Output Compare OC1*/
    PORTSetPinsDigitalOut(IOPORT_B, BIT_7);    //PWM-OUT

    /*I2C (SMBus)*/
    PORTSetPinsDigitalOut(IOPORT_B, BIT_3);     //SCL2 (pin 24)
    PORTSetPinsDigitalOut(IOPORT_B, BIT_2);     //SDA2 (pin 23)

    /*TESTPINS*/
     PORTSetPinsDigitalOut(IOPORT_B, BIT_10); //PIN 8
     PORTSetPinsDigitalOut(IOPORT_B, BIT_11); //PIN 9

    /*PPS*/
     PPSUnLock;

      /*SPI1:*/
      PPSInput(2, SDI1, RPC8);
      PPSOutput(3, RPC6, SDO1);
      PPSInput(1, SS1, RPB3); //test, do not need usually the SS1 pin

      /*SPI2 (I2S):*/
      PPSInput(3, SDI2, RPC3);
      PPSOutput(2, RPB5, SDO2);
      PPSInput(4, SS2, RPC4);  //frame clock

      /*IRQ*/
      PPSInput(4,INT1,RPC9); //IRQ_GP3

      /*PWM (OC1)*/
      PPSOutput(1, RPB7, OC1);

     PPSLock;
}

/*************************************************************************/
void SPI1_configMaster(void)
{
    UINT spi_con1 = 0;
    UINT spi_con2 = 0;

    spi_con1 =  SPI_OPEN_MSTEN    |   //set master mode
                SPI_OPEN_MODE8    |   //8 bit mode
                //SPI_OPEN_CKP_HIGH |   //clock polarity: idle = HI, active = LO
                //--> commented out = idle LO, active HI
                SPI_OPEN_CKE_REV  |   //transmit from active clock to idle clock (--> transmit on falling clock edge if clock idle is LO)
                //SPI_OPEN_SMP_END  |   //input sampled at end of data output time
                //--> commented out => sample MISO at middle of data output time
                0;

    spi_con2 =  0;

    SpiChnOpenEx(SPI_CHANNEL1,spi_con1, spi_con2, 8);

}

void SPI1_configSlave(void)
{
    //SpiChnChangeMode(SPI_CHANNEL1, TRUE, TRUE, TRUE);
    //SpiChnClose(SPI_CHANNEL1);
    /* Note:        - The I/O pins used by the SPI module are returned to their reset configuration.
     *              - The SPI_OPEN_SSEN is used to decide if the SS pin has to be returned to the reset state.*/
    //SpiChnOpen(SPI_CHANNEL1, SPI_OPEN_SLVEN|SPI_OPEN_MODE32|SPI_OPEN_CKP_HIGH, 8); //open in enhanced buffer mode?
    //SpiChnOpen(SPI_CHANNEL1, SPI_OPEN_SLVEN|SPI_OPEN_MODE8, 8); //open in enhanced buffer mode?
    SpiChnOpenEx(SPI_CHANNEL1,SPI_OPEN_SLVEN|SPI_OPEN_MODE8, 0, 8);
}


void pinMux02 (void)
{
    /*SPI1:*/
    PORTSetPinsDigitalIn(IOPORT_B, BIT_14);    //SCK1
    //TRISB = TRISB | 0x00004000;
    PORTSetPinsDigitalIn(IOPORT_C, BIT_8);      //SDI1
    PORTSetPinsDigitalOut(IOPORT_C, BIT_6);     //SDO1

     PPSUnLock;
      /*SPI1:*/
      PPSInput(2, SDI1, RPC8);
      PPSOutput(3, RPC6, SDO1);
     PPSLock;
}

void SPI2_configI2S(void)
{
    UINT spi_con1 = 0, spi_con2 = 0;

    spi_con1 =     SPI_OPEN_FRMEN      |        // Enable the Framed SPI support. Otherwise the Framed SPI is disabled.
                   SPI_OPEN_FSP_IN     |        // Frame Sync Pulse (FSP) direction set to input (Frame Slave).
                   SPI_OPEN_MODE32     |	// Data mode: 24b
                   SPI_OPEN_SLVEN      |        // set the Slave mode
                   SPI_OPEN_CKP_HIGH   ;        // bit clock polarity

    spi_con2 =     SPI_OPEN2_AUDEN     |
                   SPI_OPEN2_AUDMONO   ;       // left channel = right channel


    SpiChnOpenEx(SPI_CHANNEL2,spi_con1, spi_con2, 8);
}