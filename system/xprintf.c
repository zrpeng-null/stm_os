/******************************************************************************
--´®¿Ú¿ØÖÆÖÕ¶Ë
--ÒÆÖ² : µ¹°ÎÂÜ²·
--Ê±¼ä : 2013/8/26
******************************************************************************/
#include "xprintf.h"


#if _USE_XFUNC_OUT
#include <stdarg.h>
void (*xfunc_out)(unsigned char);	/* Pointer to the output stream */
static char *outptr;
/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/
int xputc (char c)
{
    if (_CR_CRLF && c == '\n')
    {
        xputc('\r');    /* CR -> CRLF */
    }
    if (outptr)
    {
        *outptr++ = (unsigned char)c;
        return c;
    }
    if (xfunc_out)
    {
        xfunc_out((unsigned char)c);
    }
    return c;
}
/*------------------------------------*/
/* Put a string to the default device */
/*------------------------------------*/
int xputs (const char *str)
{
    const char *phead = str;
    while (*str)
    {
        xputc(*str++);
    }

    return str - phead;
}
/* Put a string to the specified device */
int xfputs (void(*func)(unsigned char), const char *str)
{
    const char *phead = str;
    void (*pf)(unsigned char);

    pf = xfunc_out;		/* Save current output device */
    xfunc_out = func;	/* Switch output to specified device */
    while (*str)		/* Put the string */
    {
        xputc(*str++);
    }
    xfunc_out = pf;		/* Restore output device */

    return str - phead;
}
/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  xprintf("%d", 1234);			"1234"
    xprintf("%6d,%3d%%", -200, 5);	"  -200,  5%"
    xprintf("%-6u", 100);			"100   "
    xprintf("%ld", 12345678L);		"12345678"
    xprintf("%04x", 0xA3);			"00a3"
    xprintf("%08LX", 0x123ABC);		"00123ABC"
    xprintf("%016b", 0x550F);		"0101010100001111"
    xprintf("%s", "String");		"String"
    xprintf("%-4s", "abc");			"abc "
    xprintf("%4s", "abc");			" abc"
    xprintf("%c", 'a');				"a"
    xprintf("%f", 10.0);            <xprintf lacks floating point support>
*/
/* Pointer to arguments */
static int xvprintf(const char *fmt, va_list arp)
{
    unsigned int r, i, j, w, f;
    unsigned long v;
    char s[16], c, d, *p;
    unsigned int sum = 0;

    for (;;)
    {
        c = *fmt++;		/* Get a char */
        if (!c)
        {
            break;    /* End of format? */
        }
        if (c != '%')
        {
            /* Pass through it if not a % sequense */
            xputc(c);
            sum++;
            continue;
        }
        f = 0;
        c = *fmt++;		/* Get first char of the sequense */
        if (c == '0')
        {
            /* Flag: '0' padded */
            f = 1;
            c = *fmt++;
        }
        else
        {
            if (c == '-')
            {
                /* Flag: left justified */
                f = 2;
                c = *fmt++;
            }
        }
        for (w = 0; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
        {
            w = w * 10 + c - '0';
        }
        if (c == 'l' || c == 'L')
        {
            /* Prefix: Size is long int */
            f |= 4;
            c = *fmt++;
        }
        if (!c)
        {
            break;    /* End of format? */
        }
        d = c;
        if (d >= 'a')
        {
            d -= 0x20;
        }
        switch (d)
        {
        /* Type is... */
        case 'S' :		/* String */
            p = va_arg(arp, char *);
            for (j = 0; p[j]; j++) ;
            while (!(f & 2) && j++ < w)
            {
                xputc(' ');
                sum++;
            }
            sum += xputs(p);
            while (j++ < w)
            {
                xputc(' ');
                sum++;
            }
            continue;
        case 'C' :		/* Character */
            xputc((char)va_arg(arp, int));
            sum++;
            continue;
        case 'B' :		/* Binary */
            r = 2;
            break;
        case 'O' :		/* Octal */
            r = 8;
            break;
        case 'D' :		/* Signed decimal */
        case 'U' :		/* Unsigned decimal */
            r = 10;
            break;
        case 'X' :		/* Hexdecimal */
            r = 16;
            break;
        default:		/* Unknown type (passthrough) */
            xputc(c);
            sum++;
            continue;
        }

        /* Get an argument and put it in numeral */
        v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
        if (d == 'D' && (v & 0x80000000))
        {
            v = 0 - v;
            f |= 8;
        }
        i = 0;
        do
        {
            d = (char)(v % r);
            v /= r;
            if (d > 9)
            {
                d += (c == 'x') ? 0x27 : 0x07;
            }
            s[i++] = d + '0';
        }
        while (v && i < sizeof(s));
        if (f & 8)
        {
            s[i++] = '-';
        }
        j = i;
        d = (f & 1) ? '0' : ' ';
        while (!(f & 2) && j++ < w)
        {
            xputc(d);
            sum++;
        }
        do
        {
            xputc(s[--i]);
            sum++;
        }
        while (i);
        while (j++ < w)
        {
            xputc(' ');
            sum++;
        }
    }
    return sum;
}

/* Put a formatted string to the default device */
/* Pointer to the format string */
/* Optional arguments */
int xprintf(const char *fmt, ...)
{
    va_list arp;
    unsigned int sum;

    va_start(arp, fmt);
    sum = xvprintf(fmt, arp);
    va_end(arp);

    return sum;
}

/* Put a formatted string to the memory */
/* Pointer to the output buffer */
/* Pointer to the format string */
/* Optional arguments */
int xsprintf(char *buff, const char *fmt, ...)
{
    unsigned int sum;
    va_list arp;

    outptr = buff;		/* Switch destination for memory */

    va_start(arp, fmt);
    sum = xvprintf(fmt, arp);
    va_end(arp);

    *outptr = 0;		/* Terminate output string with a \0 */
    outptr = 0;			/* Switch destination for device */

    return sum;
}

/* Put a formatted string to the specified device */
/* Pointer to the output function */
/* Pointer to the format string */
/* Optional arguments */
int xfprintf(void(*func)(unsigned char), const char *fmt, ...)
{
    unsigned int sum;
    va_list arp;
    void (*pf)(unsigned char);

    pf = xfunc_out;		/* Save current output device */
    xfunc_out = func;	/* Switch output to specified device */

    va_start(arp, fmt);
    sum = xvprintf(fmt, arp);
    va_end(arp);

    xfunc_out = pf;		/* Restore output device */
    return sum;
}



/*----------------------------------------------*/
/* Dump a line of binary dump                   */
/*----------------------------------------------*/
/* Pointer to the array to be dumped */
/* Heading address value */
/* Number of items to be dumped */
/* Size of the items (DF_CHAR, DF_SHORT, DF_LONG) */
void put_dump(const void *buff, unsigned long addr, int len, int width)
{
    int i;
    const unsigned char *bp;
    const unsigned short *sp;
    const unsigned long *lp;

    xprintf("%08lX ", addr);		/* address */

    switch (width)
    {
    case DW_CHAR:
        bp = buff;
        for (i = 0; i < len; i++)		/* Hexdecimal dump */
        {
            xprintf(" %02X", bp[i]);
        }
        xputc(' ');
        for (i = 0; i < len; i++)		/* ASCII dump */
        {
            xputc((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.');
        }
        break;
    case DW_SHORT:
        sp = buff;
        do 								/* Hexdecimal dump */
        {
            xprintf(" %04X", *sp++);
        }
        while (--len);
        break;
    case DW_LONG:
        lp = buff;
        do  							/* Hexdecimal dump */
        {
            xprintf(" %08LX", *lp++);
        }
        while (--len);
        break;
    }
    xputc('\n');
}

#endif /* _USE_XFUNC_OUT */



#if _USE_XFUNC_IN
unsigned char (*xfunc_in)(void);	/* Pointer to the input stream */

/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/
/* 0:End of stream, 1:A line arrived */
/* Pointer to the buffer */
/* Buffer length */
int xgets(char *buff, int len)
{
    int c, i;

    i = 0;
    while ((c = xfunc_in()) != '\r')
    {
        if (c == '\b' && i)
        {
            /* Back space? */
            i--;
            xputs("\b \b");
        }
        else if (c >= ' ' && i < len - 1)
        {
            /* Visible chars */
            buff[i++] = c;
            if (_LINE_ECHO)
            {
                xputc(c);    //display back
            }
        }
    }
    buff[i] = '\0'; /* Terminate with a \0 */
    if (_LINE_ECHO)
    {
        xputc('\n');
    }
    return 1;
}

/* 0:End of stream, 1:A line arrived */
/* Pointer to the input stream function */
/* Pointer to the buffer */
/* Buffer length */
int xfgets(unsigned char (*func)(void), char *buff, int len)
{
    unsigned char (*pf)(void);
    int n;

    pf = xfunc_in;			/* Save current input device */
    xfunc_in = func;		/* Switch input to specified device */
    n = xgets(buff, len);	/* Get a line */
    xfunc_in = pf;			/* Restore input device */

    return n;
}


/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*	"123 -5   0x3ff 0b1111 0377  w "
	    ^                           1st call returns 123 and next ptr
	       ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/
/* return 0:Failed, 1:Successful */
/* Pointer to pointer to the string */
/* Pointer to the valiable to store the value */
int xatoi(char *str, long *res)
{
    unsigned long val;
    unsigned char c, r, s = 0;

    *res = 0;
    while ((c = *str) == ' ')
    {
        str++;    /* Skip leading spaces */
    }
    if (c == '-')
    {
        /* negative? */
        s = 1;
        c = *(++str);
    }
    if (c == '0')
    {
        c = *(++str);
        switch (c)
        {
        case 'x':		/* hexdecimal */
            r = 16;
            c = *(++str);
            break;
        case 'b':		/* binary */
            r = 2;
            c = *(++str);
            break;
        default:
            if (c <= ' ')
            {
                return 1;    /* single zero */
            }
            if (c < '0' || c > '9')
            {
                return 0;    /* invalid char */
            }
            r = 8;		/* octal */
        }
    }
    else
    {
        if (c < '0' || c > '9')
        {
            return 0;    /* EOL or invalid char */
        }
        r = 10;			/* decimal */
    }
    val = 0;
    while (c > ' ')
    {
        if (c >= 'a')
        {
            c -= 0x20;
        }
        c -= '0';
        if (c >= 17)
        {
            c -= 7;
            if (c <= 9)
            {
                return 0;    /* invalid char */
            }
        }
        if (c >= r)
        {
            return 0;    /* invalid char for current radix */
        }
        val = val * r + c;
        c = *(++str);
    }
    if (s)
    {
        val = 0 - val;    /* apply sign if needed */
    }
    *res = val;
    return 1;
}
int xgetnum(void)
{
    char buf[20];
    int num;
    xgets(buf, 20);
    if (xatoi(buf, (long *)&num) == 0)
    {
        num = -1;    // failed
    }
    return num;
}
#endif /* _USE_XFUNC_IN */
