#include <stdio.h>
#include <stdlib.h>
#include <plib.h>
#include <p32xxxx.h>

#include "vcxo.h"


int setupEdgeCount()
{
    OpenTimer1(T1_ON | T1_SOURCE_EXT | T1_PS_1_1, T1PR); //no prescalor other than 1_1 work's?!
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_1);
    return 0;
}

