typedef struct
{
    int len;
    char *buf;
} amst_string_t;

void init_amstkey();
int line_input(char *inbuf);
void inkeys(amst_string_t *str); // inkey$
int inkey(int key); // inkey()
void restore_canon();

