/* next step sort out colours when scrolling */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>

// #define move_up_n "\x1b[10A"
// #define move_down_n "\x1b[10B"
#define home "\x1b[H"
#define enter_alt_screen "\x1b[?1049h"
#define exit_alt_screen "\x1b[?1049l"
#define hide_cursor "\x1b[?25l"
#define show_cursor "\x1b[?25h"
#define reset_colours "\x1b]104\x07"
#define clear_screen "\x1b[2J"

struct amstwin 
{
    int left; // 0 based
    int col;
    int top;  // 0 based
    int line;
    int pen;
    int paper;
    int x; // relative 0 based
    int y; // relative 0 based
}; 

typedef struct sqr_colour
{
    char pen;
    char paper;
} sqr_colour_t;

static int curpen = -1;
static int curpaper = -1;

static int curx = -1; // 1 based
static int cury = -1;
//:static int curpos = -1;

#define MAX_SEQBUF 30
static char seqbuf[MAX_SEQBUF];

#define MAX_WINDOWS 8
static struct amstwin window[MAX_WINDOWS] = {0,0,0,0,1,0,0,0,
                                             0,0,0,0,1,0,0,0,
                                             0,0,0,0,1,0,0,0,
                                             0,0,0,0,1,0,0,0,
                                             0,0,0,0,1,0,0,0,
                                             0,0,0,0,1,0,0,0,
                                             0,0,0,0,1,0,0,0,
                                             0,0,0,0,1,0,0,0};

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


static struct winsize ws;
static char *screen = NULL;
static sqr_colour_t *scr_colours = NULL;

static void reset_colour()
{
    int num = write(STDOUT_FILENO, reset_colours, 6);
}

void end_window()
{
    reset_colour();
    int num = write(STDOUT_FILENO, show_cursor, 6);
    num = write(STDOUT_FILENO, clear_screen, 4);
    num = write(STDOUT_FILENO, exit_alt_screen, 8);
    free(screen);
    // free(scr_colours);
}

void end_amstwin(int sig)
{
    end_window();
    exit(0);
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

// y is 0 based
static char *get_left_pos(int s, int y)
{
    return screen + window[s].left + (window[s].top + y) * ws.ws_col;
}

/* relative to zero */
static int get_cur_pos_num(int s)
{
    return (window[s].left + window[s].x) + (window[s].top + window[s].y) * ws.ws_col;
}

/* add screen address */
static char *get_cur_pos(int s)
{
    return screen + get_cur_pos_num(s);
}

static sqr_colour_t *get_colour_left_pos(int s, int y)
{
    return scr_colours + (window[s].left + (window[s].top + y) * ws.ws_col);
}

static sqr_colour_t *get_colour_cur_pos(int s)
{
    return scr_colours + (window[s].left + window[s].x + (window[s].top + window[s].y) * ws.ws_col);
}

void locate_stream_internal(int s, int x, int y)
{
    x += window[s].left;
    y += window[s].top;
    int len = sprintf(seqbuf, "\x1b[%d;%dH", y, x, MAX_SEQBUF);
    write(STDOUT_FILENO, seqbuf, len);
}

/* CSI row; column H */
/* BASIC locate command is 1,1 based and so is ESC[col;lineH */
/* locate really needs to be delayed in case the window needs scrolled */
void locate_stream(int s, int x, int y)
{
    /* TODO check if x and y are outside window */
    window[s].x = x - 1;
    window[s].y = y - 1;
    if (curx != window[s].x || cury != window[s].y)
    {
        x += window[s].left;
        y += window[s].top;
        int len = sprintf(seqbuf, "\x1b[%d;%dH", y, x, MAX_SEQBUF);
        write(STDOUT_FILENO, seqbuf, len);
        curx = x; cury = y;
    }
}

void locate(int x, int y)
{
    locate_stream(0, x, y);
}

static void write_pen(int pen)
{
    int len = sprintf(seqbuf, "\x1b[38;5;%dm", pen, MAX_SEQBUF);
    write(STDOUT_FILENO, seqbuf, len);
}

static void write_paper(int paper)
{
    int len = sprintf(seqbuf, "\x1b[48;5;%dm", paper, MAX_SEQBUF);
    write(STDOUT_FILENO, seqbuf, len);
}

static void setcolour(sqr_colour_t *dest, const sqr_colour_t src, const int len)
{
    for (int i = 0; i < len; i++) *dest++ = src;
}

void cls(int s)
{
    curpaper = window[s].paper;
    write_paper(curpaper);

    char buf[window[s].col];

    memset(buf, ' ', window[s].col);

    for (int y = 0; y < window[s].line; y++)
    {
        locate_stream(s, 1, y+1);
        write(STDOUT_FILENO, buf, window[s].col);
        memcpy(get_left_pos(s, y), buf, window[s].col); 
        // memset(get_left_pos(s, y), ' ', window[s].col);
    }

    sqr_colour_t cbuf[window[s].col];
    setcolour(cbuf, (sqr_colour_t){window[s].pen, curpaper}, window[s].col);

    for (int i = 0; i < window[s].line; i++)
        memcpy(get_colour_left_pos(s, i), cbuf, sizeof(cbuf));

    locate_stream(s, 1, 1);
}

/* 
 * do I need a signal handler to catch screen resize
 */

void new_window(int stream, int left, int right, int top, int bottom)
{
    window[stream].left = left - 1;
    window[stream].col = right - left + 1;
    window[stream].top = top - 1;
    window[stream].line = bottom - top + 1;
    locate_stream(stream, 1, 1);
}

void init_window()
{
    signal(SIGINT, end_amstwin);
    signal(SIGTERM, end_amstwin);

    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

    int scr_size = ws.ws_row * ws.ws_col;
    screen = malloc(scr_size);
    for (int i = 0; i < MAX_WINDOWS; i++)
        new_window(i, 1, ws.ws_col, 1, ws.ws_row);

    scr_colours = malloc(scr_size * sizeof(sqr_colour_t));

    int num = write(STDOUT_FILENO, enter_alt_screen, 8);
    num = write(STDOUT_FILENO, clear_screen, 4);
    // num = write(STDOUT_FILENO, hide_cursor, 6);
    init_colours();
    cls(0);
}

static void scroll_window(int s, int num)
{
    for (int i = 0; i < window[s].line - num; i++)
    {
        memcpy(get_left_pos(s, i), get_left_pos(s, i + num), window[s].col);
        memcpy(get_colour_left_pos(s, i), get_colour_left_pos(s, i + num), window[s].col * sizeof(sqr_colour_t));
    }

    window[s].y -= num;

    for (int j = 0; j < num; j++)
    {
        char *pos = get_left_pos(s, window[s].line - j);
        memset(pos, ' ', window[s].col);
        setcolour(get_colour_left_pos(s, window[s].line - j), 
                  (sqr_colour_t){window[s].pen, window[s].paper}, 
                  window[s].col);
    }
}

static void print_window(int s)
{
    for (int i = 0; i < window[s].line; i++)
    {
        char *pos = get_left_pos(s, i);
        locate_stream_internal(s, 1, i+1);
        sqr_colour_t *cc = get_colour_left_pos(s, i);
        write_pen(cc->pen);
        write_paper(cc->paper);
        sqr_colour_t *tmp = cc;
        int len = 0;
        for (int j = 0; j < window[s].col; j++)
        {
            tmp++;
            len++;
            if (tmp->pen != cc->pen || tmp->paper != cc->paper)
            {
                write(STDOUT_FILENO, pos, len);
                pos+=len;
                len = 0;
                if (tmp->pen != cc->pen)
                   write_pen(tmp->pen);
                if (tmp->paper !=  cc->paper)
                   write_paper(tmp->paper);
                cc = tmp;
            }
        }
        write(STDOUT_FILENO, pos, len);
        pos+=len;
    }
    locate_stream_internal(s, 1, window[s].y+1);
    curx = 1; cury = window[s].y + 1;
}

/* TODO text may be bigger than window */
static void print_wrap(int s, char *buf)
{
   int len = strlen(buf);
   int first = window[s].col - window[s].x;
   memcpy(get_cur_pos(s), buf, first);
   sqr_colour_t cc = {window[s].pen, window[s].paper};
   setcolour(get_colour_cur_pos(s), cc, first);

   buf += first;
   len -= first;
   char *pos = get_left_pos(s, ++window[s].y);
   
   while (len > window[s].col)
   {
       memcpy(pos, buf, window[s].col);
       setcolour(get_colour_left_pos(s, window[s].y), cc, window[s].col);
       window[s].y++;
       len -= window[s].col;
       pos += ws.ws_col;
       buf += window[s].col;
   }
   memset(pos, ' ', window[s].col);
   memcpy(pos, buf, len);
   setcolour(get_colour_left_pos(s, window[s].y), cc, len);
   window[s].y++;
   window[s].x=0;
   curx = -1; cury = -1;
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
void print_stream_cr(int stream, char *buf, bool cr)
{
   int len = strlen(buf);
   if (window[stream].x + len > window[stream].col)
   {
       int lines = (window[stream].x + len) / window[stream].col;
       if (window[stream].y + lines >= window[stream].line)
           scroll_window(stream, lines);
       print_wrap(stream, buf);
       print_window(stream);
   }
   else
   {
       if (window[stream].y == window[stream].line)
       {
           scroll_window(stream, 1);
           print_window(stream);
           curpen = -1; curpaper = -1;
       }
       if (window[stream].pen != curpen)
       {
           curpen = window[stream].pen;
           write_pen(curpen);
       }
       if (window[stream].paper != curpaper)
       {
           curpaper = window[stream].paper;
           write_paper(curpaper);
       }

       if (window[stream].x != curx || window[stream].y != cury)
       {
           locate_stream_internal(stream, window[stream].x + 1, window[stream].y + 1);
           // curx = window[stream].x ;
       }

       /* update colours */
       setcolour(get_colour_cur_pos(stream), (sqr_colour_t){curpen, curpaper}, len);

       write(STDOUT_FILENO, buf, len);
       memcpy(get_cur_pos(stream), buf, len);
       if (cr)
       {
           window[stream].x = 0;
           window[stream].y += 1;
           if (window[stream].left < 2)
           {
               write(STDOUT_FILENO, "\n", 1);
           }
           else
           {
               int len = sprintf(seqbuf, "\n\x1b[%dC", window[stream].left, MAX_SEQBUF); // move cursor right %d columns
               write(STDOUT_FILENO, seqbuf, len);
           }
       }
       else
       {
           curx += len; 
           window[stream].x += len; 
       }
   }
}

void print_stream(int stream, char *buf)
{
    print_stream_cr(stream, buf, false);
}

void print_0(char *buf)
{
    print_stream(0, buf);
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
