#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define clear_screen "\x1b[2J"
// #define move_up_n "\x1b[10A"
// #define move_down_n "\x1b[10B"
#define enter_alt_screen "\x1b[?1049h"
#define exit_alt_screen "\x1b[?1049l"
#define hide_cursor "\x1b[?25l"
#define show_cursor "\x1b[?25h"
#define reset_colours "\x1b]104\x07"

struct amstwin {
    int left;
    int right;
    int top;
    int bottom;
    int pen;
    int paper;
    // int x;
    // int y;
};

static int curpen = -1;
static int curpaper = -1;
#define MAX_SEQBUF 30
static char seqbuf[MAX_SEQBUF];

static int curx = -1;
static int cury = -1;

struct amstwin window[8] = {1,80,1,50,1,0,
                            1,80,1,50,1,0,
                            1,80,1,50,1,0,
                            1,80,1,50,1,0,
                            1,80,1,50,1,0,
                            1,80,1,50,1,0,
                            1,80,1,50,1,0,
                            1,80,1,50,1,0};

/* TO DO get screen size
 * set up signal handler for interupt
 * do I need one for screen resize
 */
void init_window()
{
    int num = write(STDOUT_FILENO, enter_alt_screen, 8);
    num = write(STDOUT_FILENO, clear_screen, 4);
    // num = write(STDOUT_FILENO, hide_cursor, 6);
}

void end_window()
{
    int num = write(STDOUT_FILENO, show_cursor, 6);
    num = write(STDOUT_FILENO, reset_colours, 6);
    num = write(STDOUT_FILENO, exit_alt_screen, 8);
}

/* CSI row; column H */
/* BASIC locate command is 1,1 based */
void locate(int x, int y)
{
   // TODO optimise when x or y is the same especially x
   int len = sprintf(seqbuf, "\x1b[%d;%dH", --y, --x, MAX_SEQBUF);
   write(STDOUT_FILENO, seqbuf, len);
}

void locate_stream(int s, int x, int y)
{
    x += window[s].left;
    y += window[s].top;
    locate(x, y);
}


/* if text is longer than width it is wrapped round
 * if last line is bottom then window scrolls up (colours kept)
 * ; no line feed
 * , tab
 * Windows overlap and share same screen
 */
void print_stream(int stream, char *buf)
{
   if (window[stream].pen != curpen)
   {
       curpen = window[stream].pen;
       int len = sprintf(seqbuf, "\x1b[38;5;%dm", curpen, MAX_SEQBUF);
       write(STDOUT_FILENO, seqbuf, len);
   }
   if (window[stream].paper != curpaper)
   {
       curpaper = window[stream].paper;
       int len = sprintf(seqbuf, "\x1b[48;5;%dm", curpaper, MAX_SEQBUF);
       write(STDOUT_FILENO, seqbuf, len);
   }
    write(STDOUT_FILENO, buf, strlen(buf));
}

void print(char *buf)
{
    print_stream(0, buf);
}

void new_window(int stream, int left, int right, int top, int bottom)
{
    window[stream].left = left;
    window[stream].right = right;
    window[stream].top = top;
    window[stream].bottom = bottom;
}

void scroll_window()
{

}

void print_window(int window)
{

}

/* for now we will use the number direct 
 * CSI 38;5; colour m  256 colour
 */
void pen(int stream, int p)
{
   window[stream].pen = p;
}

/* for now we will use the number direct 
 * CSI 48;5; colour m  256 colour
 */
void paper(int stream, int p)
{
   window[stream].paper = p;
}

/* \x1b]4; colour ;rgb:FF/FF/FF\x1b\
 */
void init_colour(int c, int r, int g, int b)
{
   int len = sprintf(seqbuf, "\x1b]4;%d;rgb:%02X/%02X/%02X\x1b\\", c, r, g, b, MAX_SEQBUF);
   write(STDOUT_FILENO, seqbuf, len);
}

int main()
{

    char buffer[MAX_SEQBUF];
    init_window();

    new_window(1, 10, 26, 10, 26);
    locate_stream(1,1,1);
    for (int y = 0; y<16; y++) 
    {
        locate_stream(1, 1, y);
        for (int x=0; x<16; x++)
        {
           int p = x + y * 16;
           pen(1, p);
           paper(1, 255 - p);
           int len = sprintf(buffer, "%4d", p, MAX_SEQBUF);
           print_stream(1, buffer);
        }
    }
    sleep(5);

    for (int c=0; c<256; c++)
    {
        init_colour(c, 0, 0, 0);
    }
    sleep(5);
    end_window();

}
int test1()
{
    char buffer[MAX_SEQBUF];
    init_window();
    init_colour(0, 0, 0, 0);
    // init_colour(12, 255, 255, 255);
    locate(1,1);
    pen(0, 12);
    print("Hello");
    sleep(2);
    init_colour(12, 255, 255, 0);
    locate(2,2);
    print("Hello");
    sleep(2);
    init_colour(12, 0, 255, 255);
    sleep(2);
    pen(0, 3);
    print("pen 3");
    init_colour(3, 0, 255, 255);
    sleep(2);
    init_colour(12, 127, 255, 127);
    sleep(2);
    locate(2,4);
    print("Hello");
    locate(5,10);
    pen(0, 1);
    print("Hello There");
    locate(5,20);
    pen(0, 2);
    print("Hello over here");
    new_window(1, 10, 26, 10, 26);
    locate_stream(1,1,1);
    for (int y = 0; y<16; y++) 
    {
        locate_stream(1, 1, y);
        for (int x=0; x<16; x++)
        {
           int p = x + y * 16;
           pen(1, p);
           paper(1, 255 - p);
           int len = sprintf(buffer, "%4d", p, MAX_SEQBUF);
           print_stream(1, buffer);
        }
    }

    sleep(40);
    end_window();

}
