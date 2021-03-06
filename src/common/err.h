#ifndef _ERR_
#define _ERR_

/* wypisuje informacje o blednym zakonczeniu funkcji systemowej
i konczy dzialanie */
extern void syserr(const char *fmt, ...) __attribute__ ((noreturn));

/* wypisuje informacje o bledzie i konczy dzialanie */
extern void fatal(const char *fmt, ...) __attribute__ ((noreturn));

#endif