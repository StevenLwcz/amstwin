#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define clear_screen "\x1b[2J"
// #define move_up_n "\x1b[10A"
// #define move_down_n "\x1b[10B"
#define enter_alt_screen "\x1b[?1049h"
#define exit_alt_screen "\x1b[?1049l"
#define hide_cursor "\x1b[?25l"
#define show_cursor "\x1b[?25h"
#define reset_colours "\x1b]104\x07"
#define clear_screen "\x1b[2J"

struct amstwin 
{
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

static int curx = -1;
static int cury = -1;

#define MAX_SEQBUF 30
static char seqbuf[MAX_SEQBUF];

#define MAX_WINDOWS 8
static struct amstwin window[MAX_WINDOWS] = {1,80,1,50,1,0,
                                             1,80,1,50,1,0,
                                             1,80,1,50,1,0,
                                             1,80,1,50,1,0,
                                             1,80,1,50,1,0,
                                             1,80,1,50,1,0,
                                             1,80,1,50,1,0,
                                             1,80,1,50,1,0};

struct basic_colours 
{
    int no;
    int red;
    int green;
    int blue;
    char name[15];
};

#define MAX_PALETTE 27
static struct basic_colours colour_palette[MAX_PALETTE] =
{
        0,    0,    0,    0, "black",
        1,    0,    0,  128, "blue",
        2,    0,    0,  255, "bright blue",
        3,  128,    0,    0, "red",
        4,  128,    0,  128, "magenta",
        5,  128,    0,  255, "Mauve",
        6,  255,    0,    0, "bright red",
        7,  255,    0,  128, "purple",
        8,  255,    0,  255, "bright magenta",
        9,    0,  128,    0, "green",
       10,    0,  128,  128, "cyan",
       11,    0,  128,  255, "sky blue",
       12,  128,  128,    0, "yellow",
       13,  128,  128,  128, "white",
       14,  128,  128,  255, "pastel blue",
       15,  255,  128,    0, "orange",
       16,  255,  128,  128, "pink",
       17,  255,  128,  255, "pastel magenta",
       18,    0,  255,    0, "bright green",
       19,    0,  255,  128, "sea green",
       20,    0,  255,  255, "bright cyan",
       21,  128,  255,    0, "lime",
       22,  128,  255,  128, "pastel green",
       23,  128,  255,  255, "pastel cyan",
       24,  255,  255,    0, "bright yellow",
       25,  255,  255,  128, "pastel yellow",
       26,  255,  255,  255, "bright white"
};

/* default palette colours for 0-15 */
#define MAX_COLOUR 16
static const int inks[MAX_COLOUR] = { 1, 24, 20,  6, 26,  0,  2,  8, 
                            10, 12, 14, 16, 18, 22, 17, 11};



static void reset_colour()
{
    int num = write(STDOUT_FILENO, reset_colours, 6);
}

void end_window()
{
    reset_colour();
    int num = write(STDOUT_FILENO, show_cursor, 6);
    num = write(STDOUT_FILENO, exit_alt_screen, 8);
}

void end_amstwin(int sig)
{
    end_window();
    exit(0);
}

void cls(int stream)
{
   int len = sprintf(seqbuf, "\x1b[48;5;%dm%s", window[stream].paper, clear_screen);
   write(STDOUT_FILENO, seqbuf, len);
}

/* \x1b]4; colour ;rgb:FF/FF/FF\x1b\
 */
static void init_colour(int c, int r, int g, int b)
{
   int len = sprintf(seqbuf, "\x1b]4;%d;rgb:%02X/%02X/%02X\x1b\\", c, r, g, b, MAX_SEQBUF);
   write(STDOUT_FILENO, seqbuf, len);
}

/* Initialise the 1st 16 colours to the Amstrad BASIC scheme */
static void init_colours()
{
    for (int i=0;i<MAX_COLOUR;i++)
    {
        int c = inks[i];
        init_colour(i, colour_palette[c].red, colour_palette[c].green, colour_palette[c].blue);
    }
}

/* TO DO get screen size
 * do I need one for screen resize
 */
void init_window()
{
    signal(SIGINT, end_amstwin);
    signal(SIGTERM, end_amstwin);

    int num = write(STDOUT_FILENO, enter_alt_screen, 8);
    num = write(STDOUT_FILENO, clear_screen, 4);
    // num = write(STDOUT_FILENO, hide_cursor, 6);
    init_colours();
    cls(0);
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
 *
 * CSI 38;5; foreground colour m  256 colour
 * CSI 48;5; background colour m  256 colour
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

static void scroll_window()
{

}

static void print_window(int window)
{

}

void pen(int stream, int p)
{
   if (p >= 0 && p < MAX_COLOUR)
       window[stream].pen = p;
   else
       printf("Error pen out of range %d\n", p);
       /* TODO improve error handling */
}

void paper(int stream, int p)
{
   if (p >= 0 && p < MAX_COLOUR)
       window[stream].paper = p;
   else
       printf("Error paper out of range %d\n", p);
       /* TODO improve error handling */
}

       /* TODO improve error handling */
void ink(int i, int colour)
{
    if (i >= 0 && i < MAX_COLOUR)
    {
        if (colour >= 0 && colour < MAX_PALETTE)
        {
            init_colour(i, colour_palette[colour].red, colour_palette[colour].green, colour_palette[colour].blue); 
        }
        else
            printf("Error ink colour out of range $d\n", colour);
    }
    else
        printf("Error ink number out of range $d\n", i);

}

int main()
{
    char buffer[MAX_SEQBUF];
    init_window();

    new_window(1, 10, 26, 10, 26);
    cls(1);
    locate_stream(1,1,1);

    for (int p = 0; p < MAX_COLOUR; p++)
    {
       paper(1, p);
       int len = sprintf(buffer, "%4d", p, MAX_SEQBUF);
       print_stream(1, buffer);

    }
    sleep(7);
    for (int i = 0; i < 30; i++)
        init_colour(i, 0, 0, 0);
    sleep(7);

    reset_colour();
    init_colour(0, 0, 0, 0);
    print_stream(1, "RESET");
    ink(0, 0);
    for (int i = 2; i < MAX_COLOUR; i++)
    {
        ink(i, i+11);
    }
    sleep(10);
    end_window();


}
int test2()
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
           pen(1, x);
           paper(1, y);
           int p = x + y * 16;
           int len = sprintf(buffer, "%4d", p, MAX_SEQBUF);
           print_stream(1, buffer);
        }
    }

    sleep(20);
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
