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
    int i;

    unsigned char dummyDat;
    UINT32 timestampIncrement;

    txDone = FALSE;

    /*---SYSTEM CONFIG---*/
    SYSTEMConfig(GetSystemClock(), SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    DDPCONbits.JTAGEN = 0; //disable JTAG
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

    /*---PINMUXING (SWITCHING)---------------------------------------*/
    SwitchOffSport(); //remove with new HW
    pinMux01();
    SwitchADFSpi2Spi1(); //remove with new HW

    /*---SETUP-------------------------------------------------------*/
    setupEdgeCount();
    setupADF();
    ADF_MCRRegisterReadBack(&MCRregisters); //read back the MCRRegisters

    /*---ENABLE INTERRUPTS------------------------------------------*/
    INTEnableInterrupts();

    
    /*---LOOP-------------------------------------------------------*/
    tsData_32 = 1;
    writeData2PacketRam(tsData_32);
    i = 50000;
    while(i--);
    bOk = bOk && ADF_GoToTxState();

    timestampIncrement = (UINT32)((T1TURNS*T1PR) / (12288000/48000)); //64 x 61436 / 256 = 3931904 / 256 = 15359
    //timestampIncrement = 1;
    txDone = FALSE;
    while(1){

        if (txDone){
            mPORTBToggleBits(BIT_2); //debugging
            tsData_32 += timestampIncrement;
            writeData2PacketRam(tsData_32);
            i = 50000;
            while(i--);
            txDone = FALSE;
            dummyDat = 0xFF;
            bOk = bOk && ADF_MMapWrite(MCR_interrupt_source_0_Adr, 0x1, &dummyDat); //clear all interrupts in source 0 by writing 1's
            bOk = bOk && ADF_GoToTxState(); 
        }
        
    }


    return (EXIT_SUCCESS);
}

void __ISR(_TIMER_1_VECTOR, ipl1) T1Interrupt()
{
   T1Overflow++;
   if (T1Overflow == T1TURNS){
       T1Overflow = 0;
       txDone = TRUE;
   }
   mT1ClearIntFlag();

}