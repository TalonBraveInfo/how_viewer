/* OpenHoW
 * Copyright (C) 2017-2018 Mark Sowden <markelswo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <PL/platform_graphics.h>
#include <PL/platform_graphics_camera.h>
#include <PL/platform_filesystem.h>

#include "pork_engine.h"
#include "pork_input.h"

#include "client_frontend.h"
#include "client_font.h"
#include "client_display.h"

unsigned int frontend_state = FE_MODE_INIT;

/* texture assets, these are loaded and free'd at runtime */
PLTexture *fe_background    = NULL;
PLTexture *fe_press         = NULL;
PLTexture *fe_any           = NULL;
PLTexture *fe_key           = NULL;

void FrontendInputCallback(int key, bool is_pressed) {
    if(frontend_state == FE_MODE_START && is_pressed) {
        /* todo, play 'ting' sound! */

        /* we've hit our key, we can take away this
         * callback now and carry on to whatever */
        SetKeyboardFocusCallback(NULL);
        SetFrontendState(FE_MODE_MAIN_MENU);
        return;
    }
}

void InitFrontend(void) {
    /* load in all the assets we'll be using for the frontend */
    fe_background = LoadBasicTexture("fe/title/titlemon.bmp");

    InitFonts();
}

void ProcessFrontendInput(void) {
    switch(frontend_state) {
        default:break;

        case FE_MODE_START: {
            /* this is... kind of a hack... but ensures that
             * nothing will take away our check for a key during
             * the 'start' screen, e.g. bringing the console up */
            SetKeyboardFocusCallback(FrontendInputCallback);
        } break;
    }
}

void SimulateFrontend(void) {
    if(frontend_state == FE_MODE_INIT) {
        /* by this point we'll make an assumption that we're
         * done initializing, bump up the progress, draw that
         * frame and then switch over to our 'start' state */
        if(GetLoadingProgress() < 100) {
            SetLoadingDescription("LOADING FRONTEND RESOURCES");
            SetLoadingProgress(100);

            /* load in some of the assets we'll be using on the
             * next screen before proceeding... */
            fe_press = LoadBasicTexture("fe/title/press.bmp");
            fe_any = LoadBasicTexture("fe/title/any.bmp");
            fe_key = LoadBasicTexture("fe/title/key.bmp");
            fe_background = LoadBasicTexture("fe/title/title.bmp");
            return;
        }

        SetFrontendState(FE_MODE_START);
        return;
    }
}

/************************************************************/

char loading_description[256];
uint8_t loading_progress = 0;

void SetLoadingBackground(const char *name) {
    if(fe_background != NULL) {
        plDeleteTexture(fe_background, true);
    }

    char screen_path[PL_SYSTEM_MAX_PATH];
    snprintf(screen_path, sizeof(screen_path), "fe/briefing/%s.bmp", name);
    if(!plFileExists(screen_path)) {
        snprintf(screen_path, sizeof(screen_path), "fe/loadmult.bmp");
    }

    fe_background = LoadBasicTexture(screen_path);

    DrawPork(0);
}

void SetLoadingDescription(const char *description) {
    if(g_state.is_dedicated) {
        return;
    }

    snprintf(loading_description, sizeof(loading_description), "%s ...", description);
}

void SetLoadingProgress(uint8_t progress) {
    if(g_state.is_dedicated) {
        return;
    }

    if(progress > 100) {
        progress = 100;
    }

    loading_progress = progress;
    DrawPork(0);
}

uint8_t GetLoadingProgress(void) {
    return loading_progress;
}

/************************************************************/

/* Hogs of War's menu was designed
 * with a fixed resolution in mind
 * and scales poorly with anything
 * else. For now we'll just keep
 * things fixed and worry about this
 * later. */
#define FRONTEND_MENU_WIDTH     640
#define FRONTEND_MENU_HEIGHT    480

void DrawLoadingScreen(void) {
    int c_x = (GetViewportWidth() / 2) - FRONTEND_MENU_WIDTH / 2;
    int c_y = (GetViewportHeight() / 2) - FRONTEND_MENU_HEIGHT / 2;
    plDrawTexturedRectangle(c_x, c_y, FRONTEND_MENU_WIDTH, FRONTEND_MENU_HEIGHT, fe_background);

    /* originally I wrote some code ensuring the menu bar
     * was centered... that was until I found out that on
     * the background, the slot for the bar ISN'T centered
     * at all. JOY... */
    static const int bar_w = 330;
    int bar_x = c_x + 151; //c_x + (FRONTEND_MENU_WIDTH / 2) - bar_w / 2;
    int bar_y = c_y + 450;
    if(loading_progress > 0) {
        plDrawRectangle(plCreateRectangle(
                PLVector2(bar_x, bar_y),
                PLVector2(((float)(bar_w) / 100) * loading_progress, 18),
                PL_COLOUR_INDIAN_RED,
                PL_COLOUR_INDIAN_RED,
                PL_COLOUR_RED,
                PL_COLOUR_RED
        ));
    }

    if(loading_description[0] != ' ' && loading_description[0] != '\0') {
        DrawBitmapString(g_fonts[FONT_CHARS2], bar_x + 2, bar_y + 1, 4, 1.f, loading_description);
    }
}

void DrawFrontend(void) {
    /* render and handle the main menu */
    if(frontend_state != FE_MODE_GAME) {
        int c_x = (GetViewportWidth() / 2) - FRONTEND_MENU_WIDTH / 2;
        int c_y = (GetViewportHeight() / 2) - FRONTEND_MENU_HEIGHT / 2;
        switch(frontend_state) {
            default:break;

            case FE_MODE_INIT: {
                /* here we display the background for the init once,
                 * taking advantage of the fact that we're not going
                 * to clear the buffer initially - this gives us an
                 * opportunity to unload the texture and load in the
                 * final one */
                static bool is_background_drawn = false;
                if(!is_background_drawn) {
                    plDrawTexturedRectangle(c_x, c_y, FRONTEND_MENU_WIDTH, FRONTEND_MENU_HEIGHT, fe_background);
                    plDeleteTexture(fe_background, true);
                    is_background_drawn = true;
                }

                /* first we'll draw in a little rectangle representing
                 * the incomplete portion of the load */
                static bool is_load_drawn = false;
                if(!is_load_drawn) {
                    plSetBlendMode(PL_BLEND_DEFAULT);
                    plDrawRectangle(plCreateRectangle(
                            PLVector2(c_x, c_y + 464),
                            PLVector2(FRONTEND_MENU_WIDTH, 16),
                            PLColour(0, 0, 0, 150),
                            PLColour(0, 0, 0, 150),
                            PLColour(0, 0, 0, 150),
                            PLColour(0, 0, 0, 150)
                    ));
                    plSetBlendMode(PL_BLEND_DEFAULT);
                    is_load_drawn = true;
                }

                /* and now we're going to draw the loading bar. */
                plDrawRectangle(plCreateRectangle(
                        PLVector2(c_x, c_y + 464),
                        PLVector2(loading_progress, 16),
                        PL_COLOUR_INDIAN_RED,
                        PL_COLOUR_INDIAN_RED,
                        PL_COLOUR_RED,
                        PL_COLOUR_RED
                ));
            } break;

            case FE_MODE_START: {
                plDrawTexturedRectangle(c_x, c_y, FRONTEND_MENU_WIDTH, FRONTEND_MENU_HEIGHT, fe_background);

                /* this will probably be rewritten later on, but below we're setting up
                 * where each of the little blocks of text will be and then moving them
                 * up and down until the player presses an action (which is handled via
                 * our crappy callback) */

                int press_x = (FRONTEND_MENU_WIDTH / 2) - (180 / 2) + c_x;
                int press_y = c_y + 435;
                /* todo, these should be bouncing... */
                plDrawTexturedRectangle(press_x, press_y, 62, 16, fe_press);
                plDrawTexturedRectangle(press_x += 70, press_y, 40, 16, fe_any);
                plDrawTexturedRectangle(press_x += 48, press_y - 4, 39, 20, fe_key);
            } break;

            case FE_MODE_MAIN_MENU: {
                plDrawTexturedRectangle(c_x, c_y, FRONTEND_MENU_WIDTH, FRONTEND_MENU_HEIGHT, fe_background);
            } break;

            case FE_MODE_LOADING: {
                DrawLoadingScreen();
            } break;
        }

        return;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * */

unsigned int GetFrontendState(void) {
    return frontend_state;
}

void SetFrontendState(unsigned int state) {
    if(state == frontend_state) {
        LogDebug("attempted to set debug state to an already existing state!\n");
        return;
    }

    LogDebug("changing frontend state to %u...\n", state);
    switch(state) {
        default: {
            LogWarn("invalid frontend state, %u, aborting\n", state);
            return;
        }

        case FE_MODE_MAIN_MENU: {
            /* remove the textures we loaded in
             * for the start screen - we won't
             * be needing them again... */
            plDeleteTexture(fe_press, true);
            plDeleteTexture(fe_any, true);
            plDeleteTexture(fe_key, true);
            plDeleteTexture(fe_background, true);

            fe_background = LoadBasicTexture("fe/pigbkpc1.bmp");
        } break;

        case FE_MODE_START: {

        } break;

        case FE_MODE_GAME: {

        } break;

        case FE_MODE_LOADING: {
            loading_description[0] = '\0';
            loading_progress = 0;
        } break;
    }
    frontend_state = state;
}

