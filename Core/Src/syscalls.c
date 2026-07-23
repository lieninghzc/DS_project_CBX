/* Includes */
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "SEGGER_RTT.h"

/* ── printf → __io_putchar → SEGGER_RTT_PutChar ── */
int __io_putchar(int ch)
{
    SEGGER_RTT_PutChar(0, ch);
    return ch;
}

/* ── newlib syscalls ── */
int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++) {
        __io_putchar(*ptr++);
    }
    return len;
}

int _close(int file)           { (void)file; return -1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return -1; }
int _isatty(int file)          { (void)file; return 1; }

int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

void *_sbrk(int incr)
{
    (void)incr;
    errno = ENOMEM;
    return (void *)-1;
}
