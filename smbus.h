/*
 * File:   smbus.h
 * Author: Administrator
 *
 * Created on 12. M�rz 2013, 11:11
 */

#ifndef SMBUS_H
#define	SMBUS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define CMD_SYNCSTATUS    0xCC
#define CMD_TEMPSENS      0xBB
#define CMD_GETRSSI       0xAA
#define CMD_GETPACKETLOSS 0xDD
#define CMD_TIMESTAMPLAG  0xEE
#define CMD_MEANERROR     0xAC
#define CMD_BREAK         0x12
#define CMD_RESET         0x99


void setupSMBus(int pbclockfreq);


#ifdef	__cplusplus
}
#endif

#endif	/* SMBUS_H */

