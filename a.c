#include <stdlib.h>
#include <string.h>

typedef struct sqr_colour
{
    char pen;
    char paper;
} sqr_colour_t;


void main()
{
 int x = 10;
 int y = 10;
 sqr_colour_t *p = malloc(40);
 sqr_colour_t *q = p;
 memset(p, ' ', 40);
 *p = (sqr_colour_t){1,2};
 p+=(x + y);
 *p = (sqr_colour_t){3,4};
}
