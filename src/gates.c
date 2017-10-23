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

/* gates.c
 * Handles gates (kneestand) and checks if the player have completed the level
 */

#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "globals.h"
#include "backbuffer.h"
#include "sprites.h"
#include "settings.h"

int copytiles(int16 destX, int16 destY, int16 width, int16 height);
int drawlines(int16 destX, int16 destY, int16 width, int16 height);

int CROSSING_GATE(TITUS_level *level) { //Check and handle level completion, and if the player does a kneestand on a secret entrance
    check_finish(level);
    check_gates(level);
}


check_finish(TITUS_level *level) {
    TITUS_player *player = &(level->player);
    if (boss_alive) { //There is still a boss that needs to be killed!
        return;
    }
    if (level->levelid == 9) { //The level with a cage
        if ((level->player.sprite2.number != FIRST_OBJET + 26) &&
          (level->player.sprite2.number != FIRST_OBJET + 27)) {
            return;
        }
    }
    if (((player->sprite.x & 0xFFF0) != level->finishX) &&
      ((player->sprite.x & 0xFFF0) - 16 != level->finishX)) {
        return;
    }
    if (((player->sprite.y & 0xFFF0) != level->finishY) &&
      ((player->sprite.y & 0xFFF0) - 16 != level->finishY)) {
        return;
    }
#ifdef AUDIO_ENABLED
    SELECT_MUSIC(4);
    WAIT_SONG();
#endif
    CLOSE_SCREEN();
    NEWLEVEL_FLAG = true;
}

check_gates(TITUS_level *level) {
    TITUS_player *player = &(level->player);
    uint8 i;
    if ((CROSS_FLAG == 0) || //not kneestanding
      (NEWLEVEL_FLAG)) { //the player has finished the level
        return;
    }
    for (i = 0; i < level->gatecount; i++) {
        if ((level->gate[i].exists) &&
          (level->gate[i].entranceX == (player->sprite.x >> 4)) &&
          (level->gate[i].entranceY == (player->sprite.y >> 4))) {
            break;
        }
    }
    if (i == level->gatecount) {
        return; //No matching gate
    }
    player->sprite.speedX = 0;
    player->sprite.speedY = 0;
    CLOSE_SCREEN();
    uint8 orig_xlimit = XLIMIT;
    XLIMIT = level->width - screen_width;
    player->sprite.x = level->gate[i].exitX << 4;
    player->sprite.y = level->gate[i].exitY << 4;
    while (BITMAP_Y < level->gate[i].screenY) {
        D_SCROLL(level);
    }
    while (BITMAP_Y > level->gate[i].screenY) {
        U_SCROLL(level);
    }
    while (BITMAP_X < level->gate[i].screenX) {
        R_SCROLL(level);
    }
    while (BITMAP_X > level->gate[i].screenX) {
        L_SCROLL(level);
    }
    XLIMIT = orig_xlimit;
    NOSCROLL_FLAG = level->gate[i].noscroll;
    OPEN_SCREEN(level);
}


int CLOSE_SCREEN() {
    SDL_Rect dest;
    uint8 step_count = 10;
    uint16 rwidth = 320; //TODO: make this global
    uint16 rheight = 192;
    uint16 incX = rwidth / (step_count * 2);  //16
    uint16 incY = rheight / (step_count * 2); //10
    uint8 i;

	//First of all: make the screen black, at least the lower part of the screen
    dest.x = 0;
    dest.y = screen_height * 16 * scaling;
    dest.w = screen_width * 16 * scaling;
    dest.h = resheight - screen_height * 16 * scaling;
    SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));

    for (i = 0; i <= step_count; i++) {
        flip_screen(false); //quick flip TODO: move to other end of loop?

        //Clear top
        dest.x = 0;
        dest.y = 0;
        dest.w = screen_width * 16 * scaling;
        dest.h = i * incY * scaling;
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));
        
        //Clear left
        dest.x = 0;
        dest.y = 0;
        dest.w = i * incX * scaling;
        dest.h = screen_height * 16 * scaling;
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));
        
        //Clear bottom
        dest.x = 0;
        dest.y = (rheight - (i * incY)) * scaling;
        dest.w = screen_width * 16 * scaling;
        dest.h = i * incY * scaling;
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));
        
        //Clear right
        dest.x = (rwidth - (i * incX)) * scaling;
        dest.y = 0;
        dest.w = i * incX * scaling;
        dest.h = screen_height * 16 * scaling;
        SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));
    }
    flip_screen(false); //quick flip TODO: move to other end of loop?

    //SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    //SDL_Flip(screen);

}


int OPEN_SCREEN(TITUS_level *level) {
    SDL_Rect dest;
    int16 rwidth = 320; //TODO: make this global
    int16 rheight = 192;
    int8 step_count = 10;
    int16 blockX = 320 / (step_count * 2);  //16 (320: resolution width)
    int16 blockY = 192 / (step_count * 2); //9 (192: resolution height)
    int8 i, j;
    TFR_SCREENM(level); //Draw tiles on the backbuffer
    
    //BLACK_SCREEN
    dest.x = 0;
    dest.y = 0;
    dest.w = 320 * scaling;
    dest.h = 192 * scaling;
    SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, 0, 0, 0));

    j = step_count;
    for (i = 2; i <= step_count * 2; i += 2) {
        j--;
        flip_screen(false); //quick flip TODO: move to other end of loop?
        if (AMIGA_LINES) {
            drawlines( j * blockX * scaling, j * blockY * scaling, ((i * blockX) - blockX) * scaling, blockY * scaling); //Upper tiles
            drawlines(((j * blockX) + (i * blockX) - blockX) * scaling, j * blockY * scaling, blockX * scaling, i * blockY * scaling); //Right tiles
            drawlines(((j * blockX) + blockX) * scaling, (((j + 1) * blockY) + (i * blockY) - blockY) * scaling, ((i * blockX) - blockX) * scaling, blockY * scaling); //Bottom tiles
            drawlines( j * blockX * scaling, ((j * blockY) + blockY) * scaling, blockX * scaling, i * blockY * scaling); //Left tiles
        }
        copytiles( j * blockX * scaling, j * blockY * scaling, ((i * blockX) - blockX) * scaling, blockY * scaling); //Upper tiles
        copytiles(((j * blockX) + (i * blockX) - blockX) * scaling, j * blockY * scaling, blockX * scaling, i * blockY * scaling); //Right tiles
        copytiles(((j * blockX) + blockX) * scaling, (((j + 1) * blockY) + (i * blockY) - blockY) * scaling, ((i * blockX) - blockX) * scaling, blockY * scaling); //Bottom tiles
        copytiles( j * blockX * scaling, ((j * blockY) + blockY) * scaling, blockX * scaling, i * blockY * scaling); //Left tiles
	}
    flip_screen(false); //quick flip TODO: move to other end of loop?
}

int copytiles(int16 destX, int16 destY, int16 width, int16 height) {
    SDL_Rect src, dest;
    bool firstX, firstY;

//    int16 sepX = BITMAP_XM * 16 * scaling;
//    int16 sepY = BITMAP_YM * 16 * scaling;
    int16 sepXi = (S_COLUMNS - BITMAP_XM) * 16 * scaling;
    int16 sepYi = (S_LINES - BITMAP_YM) * 16 * scaling;

    // Tile screen:  | Output screen:
    //               |
    // D | C         | A | B
    // -   -         | -   -
    // B | A         | C | D
    //
    // The screens are splitted in 4 parts by BITMAP_XM and BITMAP_YM
    // The code below will move the 4 rectangles with tiles to their right place on the output screen

    //Upper left on screen (A)
    if ((destX < sepXi) &&
      (destY < sepYi)) {

/*
        src.x = sepX + destX;
        src.y = sepY + destY;
        dest.x = destX;
        dest.y = destY;
*/
        //src.w = width;
        int limitX = width / (16 * scaling) + BITMAP_XM;
        if ((width % (16 * scaling)) != 0) {
            limitX++;
        }
        //src.h = height;
        int limitY = height / (16 * scaling) + BITMAP_YM;
        if ((height % (16 * scaling)) != 0) {
            limitY++;
        }
        if (destX + width > sepXi) { //Both A and B
            //src.w = sepXi - destX;
            limitX = screen_width;
        }
        if (destY + height > sepYi) { //Both A and C
            //src.h = sepYi - destY;
            limitY = screen_height;
        }

        firstY = true;
        for (int i = BITMAP_YM + destY / (16 * scaling); i < limitY; i++) {
            firstX = true;
            for (int j = BITMAP_XM + destX / (16 * scaling); j < limitX; j++) {
                if (firstX) {
                    src.x = destX % (16 * scaling); //The remainder for the division
                } else {
                    src.x = 0;
                }
                src.w = 16 * scaling - src.x;
                if (j * 16 * scaling + src.w > BITMAP_XM * 16 * scaling + width) {
                    //src.w = 
                }
                if (firstY) {
                    src.y = destY % (16 * scaling);
                } else {
                    src.y = 0;
                }
                src.h = (16 * scaling) - src.y;
                dest.x = (j - BITMAP_XM) * 16 * scaling + src.x;
                dest.y = (i - BITMAP_YM) * 16 * scaling + src.y;
                //SDL_BlitSurface(level->tile[level->tile[tile_screen[i][j]].animation[tile_anim]].tiledata, &src, screen, &dest);
                firstX = false;
            }
            firstY = false;
        }
    }

    //Upper right on screen (B)
    if ((destY < sepYi) &&
      (destX + width > sepXi)) {

        src.x = destX - sepXi;
        src.y = sepYi + destY;
        src.w = width;
        int limitY = height / (16 * scaling) + BITMAP_YM;
        if ((height % (16 * scaling)) != 0) {
            limitY++;
        }
        dest.x = destX;
        dest.y = destY;
        if (destX < sepXi) { //Both B and A
            src.x = 0;
            src.w = width - (sepXi - destX);
            dest.x = sepXi;
        }
        if (destY + height > sepYi) { //Both B and D
            //src.h = sepYi - destY;
            //tilesY = screen_height;
        }

        firstY = true;
        for (int i = BITMAP_YM + destY / (16 * scaling); i < limitY; i++) {
            firstX = true;
            for (int j = 0; j < BITMAP_XM; j++) {
                if (firstY) {
                    src.y = destY % (16 * scaling);
                } else {
                    src.y = 0;
                }
                src.h = (16 * scaling) - src.y;
                dest.x = (j + screen_width - BITMAP_XM) * 16 * scaling;
                dest.y = (i - BITMAP_YM) * 16 * scaling + src.y;
                //SDL_BlitSurface(level->tile[level->tile[tile_screen[i][j]].animation[tile_anim]].tiledata, &src, screen, &dest);
            }
        }
    }

    //Lower left on screen (C)
	for (int i = 0; i < BITMAP_YM; i++) {
		for (int j = BITMAP_XM; j < screen_width; j++) {
			dest.x = (j - BITMAP_XM) * 16 * scaling;
			dest.y = (i + screen_height - BITMAP_YM) * 16 * scaling;
			//surface = SDL_DisplayFormat(level->tile[level->tile[tile_screen[i][j]].animation[tile_anim]].tiledata);
			//SDL_BlitSurface(surface, &src, screen, &dest);
			//SDL_FreeSurface(surface);
			//SDL_BlitSurface(level->tile[level->tile[tile_screen[i][j]].animation[tile_anim]].tiledata, &src, screen, &dest);
		}
	}

    //Lower right on screen (D)
	for (int i = 0; i < BITMAP_YM; i++) {
		for (int j = 0; j < BITMAP_XM; j++) {
			dest.x = (j + screen_width - BITMAP_XM) * 16 * scaling;
			dest.y = (i + screen_height - BITMAP_YM) * 16 * scaling;
			//surface = SDL_DisplayFormat(level->tile[level->tile[tile_screen[i][j]].animation[tile_anim]].tiledata);
			//SDL_BlitSurface(surface, &src, screen, &dest);
			//SDL_FreeSurface(surface);
			//SDL_BlitSurface(level->tile[level->tile[tile_screen[i][j]].animation[tile_anim]].tiledata, &src, screen, &dest);
		}
	}




/*
    //Upper left on screen (A)
    if ((destX < sepXi) &&
      (destY < sepYi)) {
        src.x = sepX + destX;
        src.y = sepY + destY;
        dest.x = destX;
        dest.y = destY;
        src.w = width;
        src.h = height;
        if (destX + width > sepXi) { //Both A and B
            src.w = sepXi - destX;
        }
        if (destY + height > sepYi) { //Both A and C
            src.h = sepYi - destY;
        }
		
        SDL_BlitSurface(tilescreen, &src, screen, &dest);
    }

    //Upper right on screen (B)
    if ((destY < sepYi) &&
      (destX + width > sepXi)) {
        src.x = destX - sepXi;
        src.y = sepY + destY;
        src.w = width;
        src.h = height;
        dest.x = destX;
        dest.y = destY;
        if (destX < sepXi) { //Both B and A
            src.x = 0;
            src.w = width - (sepXi - destX);
            dest.x = sepXi;
        }
        if (destY + height > sepYi) { //Both B and D
            src.h = sepYi - destY;
        }
        SDL_BlitSurface(tilescreen, &src, screen, &dest);
    }

    //Lower left on screen (C)
    if ((destX < sepXi) &&
      (destY + height > sepYi)) {
        src.x = sepX + destX;
        src.y = destY - sepYi;
        src.w = width;
        src.h = height;
        dest.x = destX;
        dest.y = destY;
        if (destX + width > sepXi) { //Both C and D
            src.w = sepXi - destX;
        }
        if (destY < sepYi) { //Both C and A
            src.y = 0;
            src.h = height - (sepYi - destY);
            dest.y = sepYi;
        }
        SDL_BlitSurface(tilescreen, &src, screen, &dest);
    }
    
    //Lower right on screen (D)
    if (((destX + width) > sepXi) &&
      ((destY + height) > sepYi)) {
        src.x = destX - sepXi;
        src.y = destY - sepYi;
        src.w = width;
        src.h = height;
        dest.x = destX;
        dest.y = destY;
        if (destX < sepXi) { //Both D and C
            src.x = 0;
            src.w = width - (sepXi - destX);
            dest.x = sepXi;
        }
        if (destY < sepYi) { //Both D and B
            src.y = 0;
            src.h = height - (sepYi - destY);
            dest.y = sepYi;
        }
			
        SDL_BlitSurface(tilescreen, &src, screen, &dest);
    }
*/
}

int drawlines(int16 destX, int16 destY, int16 width, int16 height) {
    SDL_Rect dest;
    int r_t, g_t, b_t;
    int i, j, r, g, b;



    //Testing: Amiga lines

    r_t = -128;
    g_t = -128;
    b_t = 0;
    dest.x = destX * scaling;
    dest.w = width * scaling;
    for (i = 0; i < screen_height * 16; i++) {
        dest.y = i * scaling;
        dest.h = scaling;
        r_t++;
        g_t++;
        b_t++;
        r = r_t;
        g = g_t;
        b = b_t;
        if (r < 0) {
            r = 0;
        } else if (r > 255) {
            r = 255;
        }
        if (g < 0) {
            g = 0;
        } else if (g > 255) {
            g = 255;
        }
        if (b < 0) {
            b = 0;
        } else if (b > 255) {
            b = 255;
        }
        
        if ((i >= destY) && (i < destY + height)) {
            SDL_FillRect(screen, &dest, SDL_MapRGB(screen->format, (uint8)r, (uint8)g, (uint8)b));
        }
    }

}
