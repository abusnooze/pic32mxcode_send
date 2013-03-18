/*
 * File:   _TxModuleMain.c
 * Author: p2773
 *
 * Created on 18. Dezember 2012, 10:01
 */

#include <stdio.h>
#include <stdlib.h>
#include <plib.h>
#include <p32xxxx.h>

#include "adf7023_mint.h"
#include "si5326.h"
#include "configandmux.h"
#include "timestamping.h"
#include "switching.h"
#include "_TxModuleMain.h"

/*
#define tPREAMBEL_LEN   32  //byte
#define tPREAMBEL       0x55
#define tSYNCBYTE0      0x33
#define tSYNCBYTE1      0x33
#define tSYNCBYTE2      0xA6
#define tPAYLOADLEN     32   //240 //byte
*/

#define T1TURNS         60
#define T1PR            0xFFFF //==> packet send interval: T1TURNS * (T1PR+1)
                               //e.g.: 60*(65535+1) = 3932160, that's 0.32s between each packet @ 12.288MHz external clock intput.
                               //With a buffer length of e.g 512 at the receiver the measuring time window is 163,84s long (=2,75 minutes)

BOOL txDone;
unsigned int T1Overflow = 0;

/*
 *
 */
int main(void) {

    BOOL bOk = TRUE;
    TyMCR MCRregisters;
    ADFSTA_Reg temp1;
    int ttemp;
    UINT8 tmpDat;
    UINT8 tsData_8[PKT_MAX_PKT_LEN];
    UINT32 tsData_32;
    int m = 0;
    int spiChn;
    unsigned char bb_inDat;
    unsigned char MCRByte;
    unsigned char dummyDat;

    txDone = FALSE;

    SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    DDPCONbits.JTAGEN = 0; //disable JTAG

    /*configure phase 1*/
    SwitchOffSport();
    pinMux01();
    mPORTCClearBits(BIT_0); //set CLK_alt low (start off with low clock)
    SPI1_configMaster();
    SwitchADFSpi2Spi1();


    /*set up ADF7023*/
    bOk = bOk && ADF_Init();
    bOk = bOk && ADF_MCRRegisterReadBack(&MCRregisters); //read back the MCRRegisters



    /*Configure Counter*/
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    PORTSetPinsDigitalIn(IOPORT_A, BIT_4);      //T1CK
    OpenTimer1(T1_ON | T1_SOURCE_EXT | T1_PS_1_1, T1PR); //no prescalor other than 1_1 work's?!
    // set up the timer interrupt with a priority of 2
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_1);
    // enable multi-vector interrupts
    //INTEnableSystemMultiVectoredInt();


    /*config interrupt for TX DONE on IRQ3*/
    INTEnableInterrupts();
    /*
    //ConfigINT1(EXT_INT_PRI_3 | RISING_EDGE_INT | EXT_INT_ENABLE);
    INTSetVectorPriority(INT_VECTOR_EX_INT(1), INT_PRIORITY_LEVEL_3);		// set INT controller priority
    INTSetVectorSubPriority(INT_VECTOR_EX_INT(1), INT_SUB_PRIORITY_LEVEL_3);		// set INT controller sub-priority
    INTCONbits.INT1EP = 1; //edge polarity -> rising edge
    IFS0 &= ~0x0100; //clear interrupt flag
    IEC0 |= 0x0100; //enable INT1 interrupt
    INTEnable(INT_SOURCE_EX_INT(1), INT_ENABLED);		// enable the chn interrupt in the INT controller
    //ConfigINT1(EXT_INT_PRI_3 | RISING_EDGE_INT | EXT_INT_ENABLE);
    */


    /*WRITE test timestamp to packet ram-----------------------*/
    tsData_32 = 1;
    tsData_8[0] = tsData_32;
    tsData_8[1] = tsData_32 >> 8;
    tsData_8[2] = tsData_32 >> 16;
    tsData_8[3] = tsData_32 >> 24;
    bOk = bOk && ADF_MMapWrite(PKT_RAM_BASE_PTR, PKT_MAX_PKT_LEN, tsData_8);
    /*---------------------------------------------------------*/

    /*ADF: Go to RX state*/
        //bOk = bOk && ADF_GoToRxState();
    bOk = bOk && ADF_GoToTxState();

    //m=10000; while(m--);
    /*
    temp1.Reg = 0x00;
    while(temp1.Reg != 0x00000012){
        bOk = bOk && ADF_ReadStatus(&temp1);
        temp1.Reg = temp1.Reg & 0x0000001F;
    }
    bOk = bOk && ADF_MCRRegisterReadBack(&MCRregisters); //read back the MCRRegisters

    if(bOk == FALSE){
        bOk = TRUE;
    }
    */

    

    txDone = FALSE;
    while(1){
        //bOk = bOk && ADF_MMapRead(MCR_interrupt_source_0_Adr, 0x01, &MCRByte);
        //bOk = bOk && ADF_ReadStatus(&temp1);
        //if(bOk == FALSE){
        //    bOk = TRUE;
        //}
        //if (MCRByte & 0x10){
        if (txDone){

            mPORTBToggleBits(BIT_2); //debugging

            /*write timestamp to packet ram-------------*/
            tsData_32++;
            tsData_8[0] = tsData_32;
            tsData_8[1] = tsData_32 >> 8;
            tsData_8[2] = tsData_32 >> 16;
            tsData_8[3] = tsData_32 >> 24;
            bOk = bOk && ADF_MMapWrite(PKT_RAM_BASE_PTR, PKT_MAX_PKT_LEN, tsData_8);
            /*------------------------------------------*/

            txDone = FALSE;
            dummyDat = 0xFF;
            bOk = bOk && ADF_MMapWrite(MCR_interrupt_source_0_Adr, 0x1, &dummyDat); //clear all interrupts in source 0 by writing 1's
            if(bOk == FALSE){
                bOk = TRUE;
            }

            //m=200000;
            //while(m--);

            bOk = bOk && ADF_GoToTxState();
            //if(bOk == FALSE){
            //    bOk = TRUE;
            //}
            //MCRByte = 0x00;

            //m=10000;
            //while(m--);
  
        }
    }


    return (EXIT_SUCCESS);
}


void __ISR(_EXTERNAL_1_VECTOR, ipl3) INT1Interrupt()
{
   txDone = TRUE;
   mINT1ClearIntFlag();
   //INTClearFlag(INT_SOURCE_DMA(DMA_CHANNEL2));

}

void __ISR(_TIMER_1_VECTOR, ipl1) T1Interrupt()
{
   T1Overflow++;
   if (T1Overflow == T1TURNS){
       T1Overflow = 0;
       txDone = TRUE;
   }
   
   //mPORTBToggleBits(BIT_2);
   mT1ClearIntFlag();

}