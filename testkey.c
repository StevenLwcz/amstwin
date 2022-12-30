#include <stdio.h>
#include "amstkey.h"

void main()
{
    init_amstkey();
    char inbuf[255];
    int num = line_input(inbuf);
    inbuf[num] = 0;
    printf("%s", inbuf);
    
    char c[5];
    char k;
    while((k = inkeys(c)) != 'a')
    {
        if (k > 0)
        {
          fprintf(stdout, "> %s\n", c);
          fflush(stdout);
        }
    }

    int loop=1;
    while (loop++)
    {
      for (int i = 0; i<81; i++)
      {
        int rc = inkey(i);
        if (rc != -1)
            printf("Key %d, %d\n", i, rc);
        rc = inkey(67);
        if (rc == 128)
           loop=-1;
      }
    }
    int rc = inkey(58);
          printf("Key %d, %d\n", 58, rc);

    restore_canon();
}

