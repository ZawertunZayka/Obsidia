// Copyright 2016-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include <stdlib.h>

#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"
#include "d_event.h"
#include "g_game.h"
#include "d_main.h"
#include "gamepad.h"
#include "lprintf.h"

#include "obsidia_doom.h"


//The gamepad uses keyboard emulation, but for compilation, these variables need to be placed
//somewhere. THis is as good a place as any.
int usejoystick=0;
int joyleft, joyright, joyup, joydown;


//atomic, for communication between joy thread and main game thread
static int translateKey(int c) {
	switch (c) {
		case 0xB5: case 'w': case 'W': return key_up;
		case 0xB6: case 's': case 'S': return key_down;
		case 0xB4: case 'a': case 'A': return key_left;
		case 0xB7: case 'd': case 'D': return key_right;
		case ' ': case 'f': case 'F': return key_fire;
		case 'e': case 'E': return key_use;
		case 0x0d: return key_menu_enter;
		case 0x1b: case 'q': case 'Q': return key_escape;
		case 'm': case 'M': return key_map;
		case 'r': case 'R': return key_speed;
		default: return (c >= '1' && c <= '9') ? c : 0;
	}
}


void gamepadPoll(void)
{
	static int oldKey=0;
	int newKey=translateKey(obsidia_doom_poll_key());
	event_t ev;
	if (newKey == oldKey) return;
	if (oldKey) { ev.type=ev_keyup; ev.data1=oldKey; D_PostEvent(&ev); }
	if (newKey) { ev.type=ev_keydown; ev.data1=newKey; D_PostEvent(&ev); }
	oldKey=newKey;
}

void gamepadInit(void)
{
	lprintf(LO_INFO, "gamepadInit: Initializing game pad.\n");
}

void jsInit() {
	/* CardKB is polled synchronously from I_StartTic. */
}
