#ifndef GUARD_CONTESTPAINTING_H
#define GUARD_CONTESTPAINTING_H

#include <gba/gba.h>

enum
{
    CONTESTRESULT_COOL = 9,
    CONTESTRESULT_BEAUTY = 13,
    CONTESTRESULT_CUTE = 2,
    CONTESTRESULT_SMART = 36,
    CONTESTRESULT_TOUGH = 6,
};

void sub_812FDA8(int);
void CB2_ContestPainting(void);

#endif
