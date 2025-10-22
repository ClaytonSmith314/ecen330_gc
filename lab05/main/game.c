
#include "game.h"
#include <stdio.h>

#include "config.h"
#include "board.h"
#include "graphics.h"
#include "nav.h"
#include "com.h"
#include "esp_log.h"

#include "hw.h"
#include "lcd.h"
#include "joy.h"
#include "pin.h"


#define NIBBLE_SHIFT 4
#define BOARD_SIZE 9
#define RIGHT_NIBBLE 0x0F

enum TicStates {
    INIT_ST,
    NEW_GAME_ST,
    WAIT_MARK_ST,
    MARK_ST,
    WAIT_RESTART_ST
} tictac_sm;

// Sets the initial state for tictac_sm
void game_init() {
    tictac_sm = INIT_ST;
}

// prints which players next turn is
// - turn: the player to print
void game_next_plr(mark_t turn) {
    if(turn==X_m) {
        graphics_drawMessage("next player: X", WHITE, BLACK);
    }
    if(turn==O_m) {
        graphics_drawMessage("next player: O", WHITE, BLACK);
    }
}

// runs the tick function for tictac_sm
void game_tick() {
    static int8_t r, c;
    static mark_t turn = X_m;

    // state transfer
    switch (tictac_sm) {
        case INIT_ST: {
            tictac_sm = NEW_GAME_ST;
        } break;
        case NEW_GAME_ST: {
            tictac_sm = WAIT_MARK_ST;
            game_next_plr(turn);
            uint8_t temp; //flush coms
            while(!com_read(&temp, sizeof(temp)));
        } break;
        case WAIT_MARK_ST: {
            // if btn A is pressed, transition of the space is blank
            // and send data to other
            if (pin_get_level(HW_BTN_A)==0) {
                nav_get_loc(&r, &c);
                if(board_get(r, c)==no_m) {
                    tictac_sm = MARK_ST;
                }
                uint8_t rc = (r<<NIBBLE_SHIFT)|c;
                com_write(&rc, sizeof(rc));
            } else {
                uint8_t rc;
                int count = com_read(&rc,1);
                if (count>0) {
                    r = rc>>NIBBLE_SHIFT;
                    c = rc&RIGHT_NIBBLE;
                    if(board_get(r, c)==no_m) {
                        tictac_sm = MARK_ST;
                    }
                }
            }
        } break;
        case MARK_ST: {
            //check if need to restart game
            if (board_winner(X_m)||board_winner(O_m)||
            board_mark_count()==BOARD_SIZE) {
                tictac_sm = WAIT_RESTART_ST;
            } else {
                tictac_sm = WAIT_MARK_ST;
                game_next_plr(turn);
            }
        } break;
        case WAIT_RESTART_ST: {
            if (pin_get_level(HW_BTN_START)==0) {
                tictac_sm = NEW_GAME_ST;
            }
        } break;
    }

    // actions switch statement
    switch (tictac_sm) {
        case INIT_ST: {
        } break;
        case NEW_GAME_ST: {
            //setup new game
            lcd_fillRect(0,0,LCD_W,LCD_H,BLACK);
            graphics_drawGrid(WHITE);
            board_clear();
            turn = X_m;
            nav_set_loc(1,1);
        } break;
        case WAIT_MARK_ST: {

        } break;
        case MARK_ST: {
            // if turn is X, put X on board
            if(turn==X_m) {
                board_set(r, c, X_m);
                graphics_drawX(r, c, RED);
                if (board_winner(X_m)) {
                    graphics_drawMessage("Player X won!", WHITE, BLACK);
                }
            }
            // if turn is O, place O on board
            if(turn==O_m) {
                board_set(r, c, O_m);
                graphics_drawO(r, c, BLUE);
                if (board_winner(O_m)) {
                    graphics_drawMessage("Player O won!", WHITE, BLACK);
                }
            }
            turn = (turn==X_m)? O_m : X_m;
            // if there is no winner, check if draw. If not, next turn
            if (!(board_winner(X_m)||board_winner(O_m))) {
                if (board_mark_count()==BOARD_SIZE) {
                    graphics_drawMessage("Draw!", WHITE, BLACK);
                } else {
                    game_next_plr(turn);
                }
            }
        } break;
        case WAIT_RESTART_ST: {
            
        } break;
    }
}