/* 
 * File:   vcxo.h
 * Author: Administrator
 *
 * Created on 20. März 2013, 09:46
 */

#ifndef VCXO_H
#define	VCXO_H

#ifdef	__cplusplus
extern "C" {
#endif

//#define T1TURNS         64
//#define T1PR            61436  //==> packet send interval: T1TURNS * T1PR
                               //e.g.: 60*(65535) = 3932100, that's 0.32s between each packet @ 12.288MHz external clock intput.
                               //With a buffer length of e.g 512 at the receiver the measuring time window is 163,84s long (=2,75 minutes)
#define T45PR           12288000 //3931904
#define REFEDGES        T45PR

int setupEdgeCount(void);


#ifdef	__cplusplus
}
#endif

#endif	/* VCXO_H */

