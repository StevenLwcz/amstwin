// int utf8len(const *char buff);

.global utf8len

/*
 *  01111111 - 7 - 7F
 *  110xxxxx 10xxxxxx - 11 - 7FF
 *  1110xxxx 10xxxxxx 10xxxxxx - 16 - FFFF
 *  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx - 21 - 1FFFFF
 */

.text

/* input x0 = address of UTF8 character
 * output x0 byte len
 */

utf8len:
    ldrb w4, [x0], 1
    mvn w0, w4
    lsl w0, w0, 24
    clz w0, w0
    cbz w0, len1
    ret
len1:
    mov x0, 1
    ret
