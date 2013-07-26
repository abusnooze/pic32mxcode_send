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
#include "vcxo.h"
#include "_TxModuleMain.h"




BOOL txDone;
unsigned int T1Overflow = 0;

volatile unsigned char sendData = 0; //for SMBus
volatile unsigned char recvData = 0; //for SMBus
volatile unsigned char smbusCmdReceived = 0; //for SMBus

BOOL writeData2PacketRam(UINT32 dat)
{
    UINT8 tsData_8[PKT_MAX_PKT_LEN];
    BOOL bOk;

    tsData_8[0] = dat;
    tsData_8[1] = dat >> 8;
    tsData_8[2] = dat >> 16;
    tsData_8[3] = dat >> 24;
    bOk = ADF_MMapWrite(PKT_RAM_BASE_PTR, PKT_MAX_PKT_LEN, tsData_8);

    return bOk;
}

int main(void) {

    BOOL bOk = TRUE;
    TyMCR MCRregisters;
    UINT32 tsData_32;
    unsigned int pbclockfreq;
    int i;

    unsigned char dummyDat;
    UINT32 timestampIncrement;

    txDone = FALSE;

    /*---SYSTEM CONFIG---*/
    pbclockfreq = SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    DDPCONbits.JTAGEN = 0; //disable JTAG
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

    /*---PINMUXING (SWITCHING)---------------------------------------*/
    pinMux01();

    /*---SWITCHING---------------------------------------------------*/
    turnOffLED1;
    turnOffLED2;
    switchOnCounter; //enable clock division
    switch2ClockAnd(); //use AND Gatter instead of clock buffer
    //switch2ClockBuffer();

    /*---SETUP-------------------------------------------------------*/
    setupSMBus(pbclockfreq);              //I2C (SMBus slave)
    turnOnLED1;
    turnOnLED2;
    //setupPWM(pbclockfreq);
    setupADF();
    turnOffLED1;
    ADF_MCRRegisterReadBack(&MCRregisters); //read back the MCRRegisters
    //setupEdgeCount();

    /*---ENABLE INTERRUPTS------------------------------------------*/
    //INTEnableInterrupts();

    
    /*---LOOP-------------------------------------------------------*/
    tsData_32 = 1;
    writeData2PacketRam(tsData_32);


    timestampIncrement = (UINT32)(REFEDGES / 256); //256 = (12288000/48000)
    txDone = FALSE;
    bOk = bOk && ADF_PrepareTx();

    turnOffLED2;
    setupEdgeCount();
    INTEnableInterrupts(); //enable interrupts
    while(1){

        if (txDone){
            toggleLED1; //debugging
            tsData_32 += timestampIncrement;
            writeData2PacketRam(tsData_32);
            i = 50000;
            while(i--);
            dummyDat = 0xFF;
            bOk = bOk && ADF_MMapWrite(MCR_interrupt_source_0_Adr, 0x1, &dummyDat); //clear all interrupts in source 0 by writing 1's
            bOk = bOk && ADF_PrepareTx();
            txDone = FALSE;
            //bOk = bOk && ADF_GoToTxState();
        }
        
    }


    return (EXIT_SUCCESS);
}

/*
void __ISR(_TIMER_1_VECTOR, ipl1) T1Interrupt()
{
   T1Overflow++;
   if (T1Overflow == T1TURNS){
       ADF_GoToTxStateNow();
       T1Overflow = 0;
       txDone = TRUE;
   }
   mT1ClearIntFlag();

}*/

void __ISR(_TIMER_5_VECTOR, ipl6) T5Interrupt()
{
   ADF_GoToTxStateNow();
   T1Overflow = 0;
   txDone = TRUE;
   mT5ClearIntFlag();
}