#include "button-target.h"
#include "backlight-target.h"
#include "lcd.h"
#include "kernel.h"
#include <stdbool.h>

static char game[9];
static int turn = 0;

#define ME  1
#define HIM 2

static int player = ME;

static int score(void)
{
    int win = 0;

    if(game[0] && game[0] == game[3] && game[0] == game[6])
        win = game[0];
    if(game[1] && game[1] == game[4] && game[1] == game[7])
        win = game[1];
    if(game[2] && game[2] == game[5] && game[2] == game[8])
        win = game[2];

    if(game[0] && game[0] == game[1] && game[0] == game[2])
        win = game[0];
    if(game[3] && game[3] == game[4] && game[3] == game[5])
        win = game[3];
    if(game[6] && game[6] == game[7] && game[6] == game[8])
        win = game[6];

    if(game[0] && game[0] == game[4] && game[0] == game[8])
        win = game[0];
    if(game[2] && game[2] == game[4] && game[2] == game[6])
        win = game[2];

    if(win == ME)
        return -1;
    if(win == HIM)
        return 1;

    return 0;
}

static void play(int i)
{
    if(game[i] != 0)
        return;

    game[i] = player;
    player = 3 - player;    /* next */
    turn++;
}

static int minmax(bool first)
{
    /*
     * ME : find lower score
     * HIM: find highest score
     */

    int max, i, best = 0, s;
    max = player == ME ? 1 : -1;

    s = score();
    if(s || turn == 9)
        return s;

    for(i=0; i<9; i++)
        if(game[i] == 0)
        {
            play(i);
            int max2 = minmax(false);

            /* undo */
            turn--;
            player = 3 - player;    /* next */
            game[i] = 0;

            if((player == ME  && max2 <= max) ||
               (player == HIM && max2 >= max))
            {
                max = max2;
                best = i;
            }
        }

    if(first)
        play(best);

    return max;
}

static void show(int x, int y)
{
    int i;

    for(i=0; i<3; i++)
        lcd_puts(0, i, "   ");

    for(i=0; i<9; i++)
        if(game[i] != 0)
            lcd_puts(i%3, i/3, game[i] == ME ? "0" : "X");

    lcd_puts(0, 6, "MORPION");
    lcd_puts(0, 7, "A LA CON");

    lcd_puts(8, 1, "quit : power");
    lcd_puts(8, 2, "reset:  home");

    lcd_drawrect(y*6 + 1, x*8, 5, 7);   /* cursor */
    lcd_drawrect(0, 0, 19, 24);

    lcd_update();
}

void morpion(void)
{
    int x = 0, y = 0;
    while(1)
    {
        int i, s;
        sleep(HZ/10);

        if(button_hold())
            continue;

        const int btn = button_read_device();
        if(btn & BUTTON_POWER)
        {
            lcd_clear_display();
            lcd_update();
            return;
        }

        if(btn & BUTTON_HOME)
        {
            _buttonlight_off();
            lcd_clear_display();
            lcd_update();
            turn = 0;
            player = ME;
            for(i=0; i<9; i++)
                game[i] = 0;
        }

        show(x, y);
        s = score();
        if(s || turn == 9)
        {
            _buttonlight_on();
            lcd_puts(0, 4, (s == -1) ? "WON" :
                  ( (s == 1 ) ? "PWNED" : "DRAW" ));
            lcd_update();
        }
        else if(player == HIM)
        {
            lcd_puts(6, 0, ".");
            lcd_update();
            minmax(true);
            lcd_puts(6, 0, " ");
            lcd_update();
        }
        else if(btn & BUTTON_SELECT)    /* my turn */
            play(x*3 + y);

        if(btn & BUTTON_DOWN)
            if(x < 2)
                x++;

        if(btn & BUTTON_LEFT)
            if(y > 0)
                y--;

        if(btn & BUTTON_UP)
            if(x > 0)
                x--;

        if(btn & BUTTON_RIGHT)
            if(y < 2)
                y++;
    }
}
