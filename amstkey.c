#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

static const struct timespec p50 = {0, 2e7}; // fiftith of a second
static struct termios save_settings, raw_settings;

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
    // todo translate byte array into amstrad key character
    return c[0];
}

int line_input(char *inbuf)
{
    restore_canon();
    printf("? ");
    fflush(stdout);
    int num=read(STDIN_FILENO, inbuf, 255);
    restore_raw();
    return num;
}

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
    while( (c = inkeys()) != 'a')
    {
        fprintf(stdout, "> %c\n", c);
        fflush(stdout);
    }

    restore_canon();
}

