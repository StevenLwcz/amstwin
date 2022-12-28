#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

static const struct timespec p50 = {0, 2e6}; // fiftith of a second
// static const struct timespec p50 = {0, 2e7}; // fiftith of a second
static struct termios save_settings, raw_settings;

typedef struct
{ char key; 
  char code; 
} key_code_t;

static const key_code_t amst_key_mapping[255] = {
['1'] = {66, 0}, ['2'] = {64, 0}, ['3'] = {65, 0}, ['4'] = {57, 0}, ['5'] = {56, 0}, ['6'] = {49, 0}, ['7'] = {48, 0}, ['8'] = {41, 0}, ['9'] = {40, 0}, ['0'] = {33, 0}, ['-'] = {32, 0}, ['='] = {25, 0}, 
['q'] = {67, 0}, ['w'] = {59, 0}, ['e'] = {58, 0}, ['r'] = {50, 0}, ['t'] = {51, 0}, ['y'] = {43, 0}, ['u'] = {42, 0}, ['i'] = {35, 0}, ['o'] = {34, 0}, ['p'] = {27, 0}, ['['] = {26, 0}, [']'] = {17, 0}, 
['a'] = {69, 0}, ['s'] = {60, 0}, ['d'] = {61, 0}, ['f'] = {53, 0}, ['g'] = {52, 0}, ['h'] = {44, 0}, ['j'] = {45, 0}, ['k'] = {37, 0}, ['l'] = {36, 0}, [';'] = {29, 0}, ['\''] = {28, 0}, ['#'] = {19, 0}, 
['z'] = {71, 0}, ['x'] = {63, 0}, ['c'] = {62, 0}, ['v'] = {55, 0}, ['b'] = {54, 0}, ['n'] = {46, 0}, ['m'] = {38, 0}, [','] = {39, 0}, ['.'] = {31, 0}, ['/'] = {30, 0}, 

                 ['!'] = {64, 128}, ['"'] = {65, 128},              ['$'] = {56, 128}, ['%'] = {49, 128}, ['^'] = {48, 128}, ['&'] = {41, 128}, ['*'] = {40, 128}, ['('] = {33, 128}, [')'] = {32, 128}, ['_'] = {25, 128}, ['+'] = {24, 128}, 
['Q'] = {67, 128}, ['W'] = {59, 128}, ['E'] = {58, 128}, ['R'] = {50, 128}, ['T'] = {51, 128}, ['Y'] = {43, 128}, ['U'] = {42, 128}, ['I'] = {35, 128}, ['O'] = {34, 128}, ['P'] = {27, 128}, ['{'] = {26, 128}, ['}'] = {17, 128}, 
['A'] = {69, 128}, ['S'] = {60, 128}, ['D'] = {61, 128}, ['F'] = {53, 128}, ['G'] = {52, 128}, ['H'] = {44, 128}, ['J'] = {45, 128}, ['K'] = {37, 128}, ['L'] = {36, 128}, [':'] = {29, 128}, ['@'] = {28, 128}, ['~'] = {19, 128}, 
['|'] = {71, 128}, ['Z'] = {63, 128}, ['X'] = {62, 128}, ['C'] = {55, 128}, ['V'] = {54, 128}, ['B'] = {46, 128}, ['N'] = {38, 128}, ['M'] = {39, 128}, ['<'] = {31, 128}, ['>'] = {30, 128}, ['?'] = {22, 128} 

//57 = £ and 66 = ¬
};

// inkey() takes a key number and returns -1 not pressed
// , 0 pressed, 32 with shift, 128 with control, 
// 160 with shift and control - not sure how to so this on UNIX
// so mapping needed is from key number to character

// restore canonical mode 
void restore_canon()
{
   if (tcsetattr(STDIN_FILENO, TCSANOW, &save_settings) == -1)
        printf("tcsetattr failed\n");
}

// restore raw mode 
void restore_raw()
{
   if (tcsetattr(STDIN_FILENO, TCSANOW, &raw_settings) == -1)
        printf("tcsetattr failed\n");
}

char inkeys() // inkey$
{
    char c[5] = {0};
    read(STDIN_FILENO, &c, 5);
    nanosleep(&p50, NULL);
    // if no key pressed empty string returned - probably do at assember wrapper level
    // by setting len to zero
    // to do tranlate escape squences to something senible
    return c[0];
}

// need flag to suppress prompt and or new line character
int line_input(char *inbuf)
{
    restore_canon();
    fputs("? ", stdout);
    fflush(stdout);
    int num=read(STDIN_FILENO, inbuf, 255);
    restore_raw();
    return num;
}

static int key_status[80] = { -1 };

// key + shift = 32 , + control = 128 , + both = 160
    // TO DO deal with utf-8 char and escape
int inkey(int key) // inkey()
{
    //nanosleep(&p50, NULL);
    char c[5] = {0};
    for (int i = 0; i < 10; i++)
    {
        int num = read(STDIN_FILENO, &c, 5);
        if (num == 1) 
        {
            key_code_t kc = amst_key_mapping[c[0]];
            key_status[kc.key] = kc.code;
        }
    }
    int r = key_status[key];
    key_status[key] = -1;
    return r;
}

// 69 = a  return 0 , A also 69 but return 32
// char to kwynum  a -> 69  A->69 
// a->0 A -> 32 1 -> 128

void init_amstkey()
{
    if (tcgetattr(STDIN_FILENO, &save_settings) == -1)
        printf("tcgetattr failed\\n"); // todo better error termination

    raw_settings = save_settings;
    // raw mode
    raw_settings.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    raw_settings.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
                             INPCK | ISTRIP | IXON | PARMRK);
    // non block read
    raw_settings.c_cc[VMIN] = 0;
    raw_settings.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_settings) == -1)
        printf("tcsetattr failed\n");
}


void main()
{
    init_amstkey();
    char inbuf[255];
    int num = line_input(inbuf);
    inbuf[num] = 0;
    printf("%s", inbuf);
    
    char c;
    // while( (c = inkeys()) != 'a')
    // {
        // fprintf(stdout, "> %c\n", c);
        // fflush(stdout);
    // }
    int loop=1;
    while (loop++)
    {
      for (int i = 0; i<81; i++)
      {
        int rc = inkey(50);
        if (rc != -1)
            printf("Key %d, %d\n", 50, rc);
        rc = inkey(51);
        if (rc != -1)
            printf("Key %d, %d\n", 51, rc);
       rc = inkey(67);
       if (rc == 128)
          loop=-1;
      }
      // printf("Loop%d\n", loop);
    }
       int rc = inkey(58);
            printf("Key %d, %d\n", 58, rc);

    restore_canon();
}

