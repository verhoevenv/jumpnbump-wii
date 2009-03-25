/*
 * input.c
 * Copyright (C) 1998 Brainchild Design - http://brainchilddesign.com/
 *
 * Copyright (C) 2001 Chuck Mason <cemason@users.sourceforge.net>
 *
 * Copyright (C) 2002 Florian Schulze <crow@icculus.org>
 *
 * This file is part of Jump'n'Bump.
 *
 * Jump'n'Bump is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Jump'n'Bump is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../globals.h"

#ifndef WIN32
#include <wiiuse/wpad.h>
#endif

static int num_joys=0;
static SDL_Joystick *joys[4];

/* assumes joysticks have at least one button, could check numbuttons first? */
#define JOY_LEFT(num) (num_joys>num && SDL_JoystickGetAxis(joys[num], 0)<-3200)
#define JOY_RIGHT(num) (num_joys>num && SDL_JoystickGetAxis(joys[num], 0)>3200)
/* I find using the vertical axis to be annoying -- dnb */
#define JOY_JUMP(num) (num_joys>num && SDL_JoystickGetButton(joys[num], 0))

#ifdef WIN32

#define WII_LEFT(num) 0
#define WII_RIGHT(num) 0
//TODO: jump with wiggle
#define WII_JUMP(num) 0

#define WII_EXP_LEFT(num) 0
#define WII_EXP_RIGHT(num) 0

#define WII_EXP_JUMP(num) 0

#else

#define WII_LEFT(num) (WPAD_ButtonsHeld(WPAD_CHAN_ ## num)&WPAD_BUTTON_UP)
#define WII_RIGHT(num) (WPAD_ButtonsHeld(WPAD_CHAN_ ## num)&WPAD_BUTTON_DOWN)
//TODO: jump with wiggle
#define WII_JUMP(num) (WPAD_ButtonsHeld(WPAD_CHAN_ ## num)&WPAD_BUTTON_2)

#define WII_EXP_LEFT(num) (ext_but[num] & 1)
#define WII_EXP_RIGHT(num) (ext_but[num] & 2)

#define WII_EXP_JUMP(num) (ext_but[num] & 4)

#endif

int calib_joy(int type)
{
	return 1;
}

void update_player_actions(void)
{
	int tmp;

#ifndef WIN32
	struct expansion_t ext_data[4];
	int ext_but[4] = {0,0};
	WPAD_Expansion(WPAD_CHAN_0, &ext_data[0]);
	WPAD_Expansion(WPAD_CHAN_1, &ext_data[1]);
	WPAD_Expansion(WPAD_CHAN_2, &ext_data[2]);
	WPAD_Expansion(WPAD_CHAN_3, &ext_data[3]);
	for(tmp=0;tmp<4;tmp++)
	{
        switch(ext_data[tmp].type)
        {
        case WPAD_EXP_NUNCHUK:
            if (ext_data[tmp].nunchuk.js.pos.x < (ext_data[tmp].nunchuk.js.center.x + ext_data[tmp].nunchuk.js.min.x)/2)
                ext_but[tmp] |= 1;
            if (ext_data[tmp].nunchuk.js.pos.x > (ext_data[tmp].nunchuk.js.center.x + ext_data[tmp].nunchuk.js.max.x)/2)
                ext_but[tmp] |= 2;
            if (WPAD_ButtonsHeld(tmp) & (WPAD_NUNCHUK_BUTTON_Z | WPAD_NUNCHUK_BUTTON_C))
                ext_but[tmp] |= 4;
            break;
        case WPAD_EXP_CLASSIC:
            if (WPAD_ButtonsHeld(tmp) & (WPAD_CLASSIC_BUTTON_LEFT))
                ext_but[tmp] |= 1;
            if (WPAD_ButtonsHeld(tmp) & (WPAD_CLASSIC_BUTTON_RIGHT))
                ext_but[tmp] |= 2;
            if (WPAD_ButtonsHeld(tmp) & (WPAD_CLASSIC_BUTTON_A | WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_X | WPAD_CLASSIC_BUTTON_Y))
                ext_but[tmp] |= 4;
            break;
        case WPAD_EXP_GUITARHERO3:
            //Clueless right now... never seen one, no idea what would be nice.
            break;
        }
	}
#endif

	if (client_player_num < 0) {
		tmp = (key_pressed(KEY_PL1_LEFT) == 1) || JOY_LEFT(3) || WII_LEFT(0);
		if (tmp != player[0].action_left)
			tellServerPlayerMoved(0, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL1_RIGHT) == 1) || JOY_RIGHT(3) || WII_RIGHT(0);
		if (tmp != player[0].action_right)
			tellServerPlayerMoved(0, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL1_JUMP) == 1) || JOY_JUMP(3) || WII_JUMP(0);
		if (tmp != player[0].action_up)
			tellServerPlayerMoved(0, MOVEMENT_UP, tmp);

		tmp = (key_pressed(KEY_PL2_LEFT) == 1) || JOY_LEFT(2) || WII_LEFT(1);
		if (tmp != player[1].action_left)
			tellServerPlayerMoved(1, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL2_RIGHT) == 1) || JOY_RIGHT(2) || WII_RIGHT(1);
		if (tmp != player[1].action_right)
			tellServerPlayerMoved(1, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL2_JUMP) == 1) || JOY_JUMP(2) || WII_JUMP(1);
		if (tmp != player[1].action_up)
			tellServerPlayerMoved(1, MOVEMENT_UP, tmp);

		tmp = (key_pressed(KEY_PL3_LEFT) == 1) || JOY_LEFT(1) || WII_LEFT(2);
		if (tmp != player[2].action_left)
			tellServerPlayerMoved(2, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL3_RIGHT) == 1) || JOY_RIGHT(1) || WII_RIGHT(2);
		if (tmp != player[2].action_right)
			tellServerPlayerMoved(2, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL3_JUMP) == 1) || JOY_JUMP(1) || WII_JUMP(2);
		if (tmp != player[2].action_up)
			tellServerPlayerMoved(2, MOVEMENT_UP, tmp);

		tmp = (key_pressed(KEY_PL4_LEFT) == 1) || JOY_LEFT(0) || WII_LEFT(3);
		if (tmp != player[3].action_left)
		tellServerPlayerMoved(3, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL4_RIGHT) == 1) || JOY_RIGHT(0) || WII_RIGHT(3);
		if (tmp != player[3].action_right)
		tellServerPlayerMoved(3, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL4_JUMP) == 1) || JOY_JUMP(0) || WII_JUMP(3);
		if (tmp != player[3].action_up)
		tellServerPlayerMoved(3, MOVEMENT_UP, tmp);

		tmp = (key_pressed(KEY_PL5_LEFT) == 1) || WII_EXP_LEFT(0);
		if (tmp != player[4].action_left)
			tellServerPlayerMoved(4, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL5_RIGHT) == 1) || WII_EXP_RIGHT(0);
		if (tmp != player[4].action_right)
			tellServerPlayerMoved(4, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL5_JUMP) == 1) || WII_EXP_JUMP(0);
		if (tmp != player[4].action_up)
			tellServerPlayerMoved(4, MOVEMENT_UP, tmp);

		tmp = (key_pressed(KEY_PL6_LEFT) == 1) || WII_EXP_LEFT(1);
		if (tmp != player[5].action_left)
			tellServerPlayerMoved(5, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL6_RIGHT) == 1) || WII_EXP_RIGHT(1);
		if (tmp != player[5].action_right)
			tellServerPlayerMoved(5, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL6_JUMP) == 1) || WII_EXP_JUMP(1);
		if (tmp != player[5].action_up)
			tellServerPlayerMoved(5, MOVEMENT_UP, tmp);

		tmp = (key_pressed(KEY_PL7_LEFT) == 1) || WII_EXP_LEFT(2);
		if (tmp != player[6].action_left)
			tellServerPlayerMoved(6, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL7_RIGHT) == 1) || WII_EXP_RIGHT(2);
		if (tmp != player[6].action_right)
			tellServerPlayerMoved(6, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL7_JUMP) == 1) || WII_EXP_JUMP(2);
		if (tmp != player[6].action_up)
			tellServerPlayerMoved(6, MOVEMENT_UP, tmp);

		tmp = (key_pressed(KEY_PL8_LEFT) == 1) || WII_EXP_LEFT(3);
		if (tmp != player[7].action_left)
			tellServerPlayerMoved(7, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL8_RIGHT) == 1) || WII_EXP_RIGHT(3);
		if (tmp != player[7].action_right)
			tellServerPlayerMoved(7, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL8_JUMP) == 1) || WII_EXP_JUMP(3);
		if (tmp != player[7].action_up)
			tellServerPlayerMoved(7, MOVEMENT_UP, tmp);
	} else {
		tmp = (key_pressed(KEY_PL1_LEFT) == 1) || JOY_LEFT(0) || WII_LEFT(0);
		if (tmp != player[client_player_num].action_left)
			tellServerPlayerMoved(client_player_num, MOVEMENT_LEFT, tmp);
		tmp = (key_pressed(KEY_PL1_RIGHT) == 1) || JOY_RIGHT(0) || WII_RIGHT(0);
		if (tmp != player[client_player_num].action_right)
			tellServerPlayerMoved(client_player_num, MOVEMENT_RIGHT, tmp);
		tmp = (key_pressed(KEY_PL1_JUMP) == 1) || JOY_JUMP(0) || WII_JUMP(0);
		if (tmp != player[client_player_num].action_up)
			tellServerPlayerMoved(client_player_num, MOVEMENT_UP, tmp);
	}
}

void init_inputs(void)
{
	int i;

	num_joys = SDL_NumJoysticks();
	for(i = 0; i < 4 && i < num_joys; ++i)
		joys[i] = SDL_JoystickOpen(i);

	main_info.mouse_enabled = 0;
	main_info.joy_enabled = 0;

#ifndef WIN32
    WPAD_Init();
#endif
}
