// Copyright 2008  Segher Boessenkool  <segher@kernel.crashing.org>
// Licensed under the terms of the GNU GPL, version 2
// http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

#ifndef _VIDEO_H
#define _VIDEO_H

#include "types.h"

extern u8 screen[320*240];

void blit_screen(void);

#endif