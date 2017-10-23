/*   
 * Copyright (C) 2008 - 2012 The OpenTitus team
 *
 * Authors:
 * Eirik Stople
 *
 * "Titus the Fox: To Marrakech and Back" (1992) and
 * "Lagaf': Les Aventures de Moktar - Vol 1: La Zoubida" (1991)
 * was developed by, and is probably copyrighted by Titus Software,
 * which, according to Wikipedia, stopped buisness in 2005.
 *
 * OpenTitus is not affiliated with Titus Software.
 *
 * OpenTitus is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 3  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

/* keyboard.c
 * Keyboard functions
 */

#include "SDL/SDL.h"
#include "keyboard.h"
#include "globals.h"
#include "common.h"

//Probably not the best way, but it works...
#define HAVE_CONFIG_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef AUDIO_ENABLED
#include "audio.h"
#endif

int waitforbutton() {
    SDL_Event event;
    int waiting = 1;
    while (waiting > 0)
    {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                waiting = -1;

#ifdef __PSP2__
            if (event.type == SDL_JOYBUTTONDOWN) {
                if (event.jbutton.button == KEY_RETURN || event.jbutton.button == KEY_ENTER || event.jbutton.button == KEY_SPACE)
#else
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == KEY_RETURN || event.key.keysym.sym == KEY_ENTER || event.key.keysym.sym == KEY_SPACE)
#endif
                    waiting = 0;

#ifdef __PSP2__
                if (event.jbutton.button == KEY_ESC)
#else
                if (event.key.keysym.sym == SDLK_ESCAPE)
#endif
                    waiting = -1;

#ifdef AUDIO_ENABLED
#if __PSP2__
                if (event.jbutton.button == KEY_MUSIC) {
#else
                if (event.key.keysym.sym == KEY_MUSIC) {
#endif
					AUDIOMODE++;
					if (AUDIOMODE > 1) {
						AUDIOMODE = 0;
					}
					if (AUDIOMODE == 1) {
						startmusic();
					}
                }
#endif
            }
        }
		titus_sleep();
#ifdef AUDIO_MIKMOD_SINGLETHREAD
        checkmodule();
#endif

#ifdef AUDIO_SDL_MIXER
        checkaudio();
#endif

    }
    return (waiting);
}

