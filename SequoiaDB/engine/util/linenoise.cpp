/* linenoise.c -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * You can find the latest source code at:
 *
 *   http://github.com/antirez/linenoise
 *
 * Does a number of crazy assumptions that happen to be true in 99.9999% of
 * the 2010 UNIX computers around.
 *
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html
 *
 * Todo list:
 * - Switch to gets() if $TERM is something we can't support.
 * - Filter bogus Ctrl+<char> combinations.
 * - Win32 support
 *
 * Bloat:
 * - Completion?
 * - History search like Ctrl+r in readline?
 *
 * List of escape sequences used by this program, we do everything just
 * with three sequences. In order to be so cheap we may have some
 * flickering effect with some slow terminal, but the lesser sequences
 * the more compatible.
 *
 * CHA (Cursor Horizontal Absolute)
 *    Sequence: ESC [ n G
 *    Effect: moves cursor to column n
 *
 * EL (Erase Line)
 *    Sequence: ESC [ n K
 *    Effect: if n is 0 or missing, clear from cursor to end of line
 *    Effect: if n is 1, clear from beginning of line to cursor
 *    Effect: if n is 2, clear entire line
 *
 * CUF (CUrsor Forward)
 *    Sequence: ESC [ n C
 *    Effect: moves cursor forward of n chars
 *
 * The following are used to clear the screen: ESC [ H ESC [ 2 J
 * This is actually composed of two sequences:
 *
 * cursorhome
 *    Sequence: ESC [ H
 *    Effect: moves the cursor to upper left corner
 *
 * ED2 (Clear entire screen)
 *    Sequence: ESC [ 2 J
 *    Effect: clear the whole screen
 *
 */

/* NOTICE: I extract this file from https://code.google.com/p/dart-console,
 * which is licensed under Apache license 2.0.
 * I also port it to cpp and add some additional features.
 */

#ifdef _UNICODE
#undef _UNICODE
#endif

#ifdef UNICODE
#undef UNICODE
#endif

#include "core.hpp"

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "linenoise.h"
#include "pdTrace.hpp"
#include "utilTrace.hpp"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define REDIS_NOTUSED(V) ((void) V)
#define snprintf _snprintf
#define read(x,y,z) _read(x,y,z)
#define write(x,y,z) _write(x,y,z)
#define isatty(x) _isatty(x)
#define strdup(x) _strdup(x)
#endif

#define LINENOISE_DEFAULT_HISTORY_MAX_LEN 300
#define LINENOISE_MAX_LINE 4096
static char *unsupported_term[] = {"dumb","cons25",NULL};
static linenoiseCompletionCallback *completionCallback = NULL;

#ifndef _WIN32
static struct termios orig_termios; /* in order to restore at exit */
#endif
static int rawmode = 0; /* for atexit() function to check if restore is needed*/
static int atexit_registered = 0; /* register atexit just 1 time */
int history_max_len = LINENOISE_DEFAULT_HISTORY_MAX_LEN;
int history_len = 0;
char **history = NULL;

static void setDisplayAttribute( bool enhancedDisplay );
static void linenoiseAtExit(void);

#ifdef _WIN32
#ifndef STDIN_FILENO
  #define STDIN_FILENO (_fileno(stdin))
#endif

HANDLE hOut;
HANDLE hIn;
DWORD consolemode;
static WORD oldDisplayAttribute;

PD_TRACE_DECLARE_FUNCTION ( SDB_WIN32READ, "win32read" )
static int win32read(char *c) {
    PD_TRACE_ENTRY ( SDB_WIN32READ );
    DWORD foo;
    INPUT_RECORD b;
    KEY_EVENT_RECORD e;

    while (1) {
        if (!ReadConsoleInput(hIn, &b, 1, &foo)) return 0;
        if (!foo) return 0;

        if (b.EventType == KEY_EVENT && b.Event.KeyEvent.bKeyDown) {

            e = b.Event.KeyEvent;
            *c = b.Event.KeyEvent.uChar.AsciiChar;

            //if (e.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
                /* Alt+key ignored */
            //} else
            if (e.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
                /* Ctrl+Key */
                switch (*c) {
                    case 1: // ctrl+a, move to beginning of line
                    case 2: // ctrl+b, left_arrow
                    case 3: // ctrl+c, cancel or quit
                    case 4: // ctrl+d, remove char at right of cursor
                    case 5: // ctrl+e, move to end of line
                    case 6: // ctrl+f, right_arrow
                    case 8: // ctrl+h, backspace
                    case 11: // ctrl+k, delete from current to the end of line
                    case 12: // ctrl+l, clear the screen
                    case 14: // ctrl+n, down_arrow
                    case 16: // ctrl+p, up_arrow
                    case 20: // ctrl+t, swap the char at the cursor and the one before
                    case 21: // ctrl+u, delete the whole line
                        return 1;
                    default:
                        /* Other Ctrl+KEYs ignored */
                        break;
                }
            } else {

                switch (e.wVirtualKeyCode) {

                    case VK_ESCAPE: /* ignore - send ctrl-c, will return -1 */
                        *c = 27;
                        return 1;
                    case VK_RETURN:  /* enter */
                        *c = 13;
                        return 1;
                    case VK_LEFT:   /* left */
                        *c = 2;
                        return 1;
                    case VK_RIGHT: /* right */
                        *c = 6;
                        return 1;
                    case VK_UP:   /* up */
                        *c = 16;
                        return 1;
                    case VK_DOWN:  /* down */
                        *c = 14;
                        return 1;
                    case VK_HOME:
                        *c = 1;
                        return 1;
                    case VK_END:
                        *c = 5;
                        return 1;
                    case VK_BACK:
                        *c = 8;
                        return 1;
                    case VK_DELETE:
                        *c = 127;
                        return 1;
                    default:
                        if (*c) return 1;
                }
            }
        }
    }
    PD_TRACE_EXIT ( SDB_WIN32READ );
    return -1; /* Makes compiler happy */
}
/*
#ifdef __STRICT_ANSI__
char *strdup(const char *s) {
    size_t l = strlen(s)+1;
    char *p = malloc(l);

    memcpy(p,s,l);
    return p;
}
#endif    __STRICT_ANSI__   */

#endif /*   _WIN32    */


PD_TRACE_DECLARE_FUNCTION ( SDB_ISUNSUPPTERM, "isUnsupportedTerm" )
static int isUnsupportedTerm(void) {
   PD_TRACE_ENTRY ( SDB_ISUNSUPPTERM );

#ifndef _WIN32
    char *term = getenv("TERM");
    int j;

    if (term == NULL) return 0;
    for (j = 0; unsupported_term[j]; j++)
        if (!strcasecmp(term,unsupported_term[j])) return 1;
#endif
    PD_TRACE_EXIT ( SDB_ISUNSUPPTERM );
    return 0;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_FREEHISTORY, "freeHistory" )
static void freeHistory(void) {
    PD_TRACE_ENTRY ( SDB_FREEHISTORY );
    if (history) {
        int j;

        for (j = 0; j < history_len; j++)
            free(history[j]);
        free(history);
    }
    PD_TRACE_EXIT ( SDB_FREEHISTORY );
}

PD_TRACE_DECLARE_FUNCTION ( SDB_ENABLERAWMODE, "enableRawMode" )
static int enableRawMode(int fd) {
   PD_TRACE_ENTRY ( SDB_ENABLERAWMODE );
#ifndef _WIN32
    struct termios raw;

    if (!isatty(STDIN_FILENO)) goto fatal;
    if (!atexit_registered) {
        atexit(linenoiseAtExit);
        atexit_registered = 1;
    }
    if (tcgetattr(fd,&orig_termios) == -1) goto fatal;

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;
    rawmode = 1;
#else
    REDIS_NOTUSED(fd);

    if (!atexit_registered) {
        /* Init windows console handles only once */
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut==INVALID_HANDLE_VALUE) goto fatal;

        if (!GetConsoleMode(hOut, &consolemode)) {
            CloseHandle(hOut);
            errno = ENOTTY;
            PD_TRACE_EXIT ( SDB_ENABLERAWMODE );
            return -1;
        };

        hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (hIn == INVALID_HANDLE_VALUE) {
            CloseHandle(hOut);
            errno = ENOTTY;
            PD_TRACE_EXIT ( SDB_ENABLERAWMODE );
            return -1;
        }

        GetConsoleMode(hIn, &consolemode);
        SetConsoleMode(hIn, 0);

        /* Cleanup them at exit */
        atexit(linenoiseAtExit);
        atexit_registered = 1;
    }

    rawmode = 1;
#endif
    PD_TRACE_EXIT ( SDB_ENABLERAWMODE );
    return 0;

fatal:
    errno = ENOTTY;
    PD_TRACE_EXIT ( SDB_ENABLERAWMODE );
    return -1;
}

static void disableRawMode(int fd) {
#ifdef _WIN32
    REDIS_NOTUSED(fd);
    rawmode = 0;
#else
    /* Don't even check the return value as it's too late. */
    if (rawmode && tcsetattr(fd,TCSAFLUSH,&orig_termios) != -1)
        rawmode = 0;
#endif
}

/* At exit we'll try to fix the terminal to the initial conditions. */
PD_TRACE_DECLARE_FUNCTION ( SDB_LNNOISEATEXT, "linenoiseAtExit" )
static void linenoiseAtExit(void) {
    PD_TRACE_ENTRY ( SDB_LNNOISEATEXT );
#ifdef _WIN32
    SetConsoleMode(hIn, consolemode);
    CloseHandle(hOut);
    CloseHandle(hIn);
#else
    disableRawMode(STDIN_FILENO);
#endif
    freeHistory();
    PD_TRACE_EXIT ( SDB_LNNOISEATEXT );
}

PD_TRACE_DECLARE_FUNCTION ( SDB_GETCOLUMNS, "getColumns" )
static int getColumns(void) {
    PD_TRACE_ENTRY ( SDB_GETCOLUMNS );
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO b;

    if (!GetConsoleScreenBufferInfo(hOut, &b)) return 80;
    PD_TRACE_EXIT ( SDB_GETCOLUMNS );
    return b.srWindow.Right - b.srWindow.Left;
#else
    struct winsize ws;

    if (ioctl(1, TIOCGWINSZ, &ws) == -1) return 80;
    PD_TRACE_EXIT ( SDB_GETCOLUMNS );
    return ws.ws_col;
#endif
}

PD_TRACE_DECLARE_FUNCTION ( SDB_REFRESHLINE, "refreshLine" )
static void refreshLine(int fd, const char *prompt, char *buf, size_t len, size_t pos, size_t cols) {
    PD_TRACE_ENTRY ( SDB_REFRESHLINE );
    char seq[64];
    int highlight = -1 ;
    int highlight_pos = 0 ;
    if ( pos < len )
    {
       int scanDirection = 0 ;
       if ( strchr( "}])", buf[pos] ) )
          scanDirection = -1 ;
       else if ( strchr( "{[(", buf[pos] ) )
          scanDirection = 1 ;

       if ( scanDirection )
       {
          int unmatched = scanDirection ;
          for ( int i = pos + scanDirection ; i >= 0 && i < len ;
                i += scanDirection )
          {
             if ( strchr( "}])", buf[i] ) )
                --unmatched ;
             else if ( strchr( "{[(", buf[i] ) )
                ++unmatched ;

             if ( unmatched == 0 )
             {
                highlight = 1 ;
                highlight_pos = i ;
                break ;
             }
          }
       }
    }

#ifdef _WIN32
    DWORD pl, bl, w;
    CONSOLE_SCREEN_BUFFER_INFO b;
    COORD coord;
#endif
    size_t plen = strlen(prompt);

    while((plen+pos) >= cols) {
        buf++;
        len--;
        pos--;
    }
    while (plen+len > cols) {
        len--;
    }

#ifndef _WIN32
    /* Cursor to left edge */
    snprintf(seq,64,"\x1b[0G");
    if (write(fd,seq,strlen(seq)) == -1) goto done;
    /* Write the prompt and the current buffer content */
    if (write(fd,prompt,strlen(prompt)) == -1) goto done;
    // in case no need to highlight
    if ( -1 == highlight )
    {
       if (write(fd,buf,len) == -1) goto done;
    }
    // in case need to highlight matching {}/[]/()
    else
    {
       if (write(fd,buf,highlight_pos) == -1) goto done;
       setDisplayAttribute( true ) ;
       if (write(fd,buf+highlight_pos,1) == -1) goto done;
       setDisplayAttribute( false ) ;
       if (write(fd,buf+highlight_pos+1,len-highlight_pos-1) == -1) goto done;
    }
    /* Erase to right */
    snprintf(seq,64,"\x1b[0K");
    if (write(fd,seq,strlen(seq)) == -1) goto done;
    /* Move cursor to original position. */
    snprintf(seq,64,"\x1b[0G\x1b[%dC", (int)(pos+plen));
    if (write(fd,seq,strlen(seq)) == -1) goto done;
#else

    REDIS_NOTUSED(seq);
    REDIS_NOTUSED(fd);

    /* Get buffer console info */
    if (!GetConsoleScreenBufferInfo(hOut, &b)) goto done;
    /* Erase Line */
    coord.X = 0;
    coord.Y = b.dwCursorPosition.Y;
   FillConsoleOutputCharacterA(hOut, ' ', b.dwSize.X, coord, &w);
    /*  Cursor to the left edge */
    SetConsoleCursorPosition(hOut, coord);
    /* Write the prompt and the current buffer content */
    WriteConsole(hOut, prompt, plen, &pl, NULL);

    // in case no need to highlight
    if ( -1 == highlight )
    {
       WriteConsole(hOut, buf, len, &bl, NULL);
    }
    // in case need to highlight matching {}/[]/()
    else
    {
       WriteConsole(hOut, buf, highlight_pos, &bl, NULL);
       setDisplayAttribute( true ) ;
       WriteConsole(hOut, buf + highlight_pos, 1, &bl, NULL);
       setDisplayAttribute( false ) ;
       WriteConsole(hOut, buf+highlight_pos + 1, len - highlight_pos - 1, &bl, NULL);
    }

    /* Move cursor to original position. */
    coord.X = (int)(pos+plen);
    coord.Y = b.dwCursorPosition.Y;
    SetConsoleCursorPosition(hOut, coord);
#endif
done :
    PD_TRACE_EXIT ( SDB_REFRESHLINE );
    return ;
}

static void beep() {
    fprintf(stderr, "\x7");
    fflush(stderr);
}

static void freeCompletions(linenoiseCompletions *lc) {
    size_t i;
    for (i = 0; i < lc->len; i++)
        free(lc->cvec[i]);
    if (lc->cvec != NULL)
        free(lc->cvec);
    if (lc->fill != NULL)
        free(lc->fill) ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_COMPLETELN, "completeLine" )
static int completeLine(int fd, const char *prompt, char *buf, size_t buflen, size_t *len, size_t *pos, size_t cols) {
    PD_TRACE_ENTRY ( SDB_COMPLETELN );
    linenoiseCompletions lc = { 0, 0, NULL, NULL };
    int nread, nwritten;
    char c = 0;

    completionCallback(buf,&lc);
    if (lc.len == 0) {
        beep();
    } else if ( 1 == lc.len || lc.fill ) {
        size_t clen = 0 ;
        char *newStr = NULL ;
        if ( 1 == lc.len )
        {
            newStr = lc.cvec[0] ;
            c = 32 ;
        }
        else
        {
            newStr = lc.fill ;
        }
        clen = strlen(newStr) ;
        refreshLine(fd,prompt,newStr,clen,clen,cols);
        nwritten = snprintf(buf,buflen,"%s",newStr);
        *len = *pos = nwritten;
    } else {
        size_t stop = 0;

        while(!stop) {

#ifdef _WIN32
            nread = win32read(&c);
#else
            nread = read(fd,&c,1);
#endif
            if (nread <= 0) {
                freeCompletions(&lc);
                return -1;
            }

            switch(c) {
                case 9: /* tab */
                {
                    char tmpBuf[1] = {0} ;
                    unsigned int index = 0 ;

                    printf( "\n" ) ;
                    refreshLine(fd,"",tmpBuf,0,0,1);
                    while ( index < lc.len-1 )
                    {
                       printf("%-s\t", lc.cvec[index]) ;
                       index++ ;
                    }
                    printf("%s\n", lc.cvec[index] );
                    fflush(stdout);

                    refreshLine(fd, prompt, buf, *len, *pos, cols) ;
                    break;
                }
                case 27: /* escape */
                    /* Re-show original buffer */
                    //refreshLine(fd,prompt,buf,*len,*pos,cols);
                    stop = 1;
                    break;
                default:
                    /* Update buffer and return */
                    stop = 1;
                    break;
            }
        }
    }

    freeCompletions(&lc);
    PD_TRACE_EXIT ( SDB_COMPLETELN );
    return c; /* Return last read character */
}

void linenoiseHistoryClear(void) {
    memset( history, 0, (sizeof(char*)*(history_max_len)) ) ;
    history_len = 0 ;
}

void linenoiseClearScreen(void) {

#ifdef _WIN32
    system("cls");
#else
    if (write(STDIN_FILENO,"\x1b[H\x1b[2J",7) <= 0) {
        /* nothing to do, just to avoid warning. */
    }
#endif
}

static int linenoisePrompt(int fd, char *buf, size_t buflen, const char *prompt) {
    size_t plen = strlen(prompt);
    size_t pos = 0;
    size_t len = 0;
    size_t cols = getColumns();
    int history_index = 0;
    int flag = 0 ;
#ifdef _WIN32
    DWORD foo;
#endif

    buf[0] = '\0';
    buflen--; /* Make sure there is always space for the nulterm */

    /* The latest history entry is always our current buffer, that
     * initially is just an empty string. */
    linenoiseHistoryAdd("");

#ifdef _WIN32
    if (!WriteConsole(hOut, prompt, plen, &foo, NULL)) return -1;
#else
    if (write(fd,prompt,plen) == -1) return -1;
#endif
    while(1) {
        char c;
        int nread;
        char seq[2], seq2[2];

#ifdef _WIN32
        nread = win32read(&c);
#else
        nread = read(fd,&c,1);
#endif
        if (nread <= 0) return len;

        /* Only autocomplete when the callback is set. It returns < 0 when
         * there was an error reading from fd. Otherwise it will return the
         * character that should be handled next. */
        if (c == 9 && completionCallback != NULL) {
            c = completeLine(fd,prompt,buf,buflen,&len,&pos,cols);
            /* Return on errors */
            if (c < 0) return len;
            /* Read next character when 0 */
            if (c == 0) continue;

        }

        switch(c) {
        case 13:    /* enter */
            refreshLine(fd, prompt, buf, len, len, cols);
            history_len--;
            free(history[history_len]);
            return (int)len;
        case 3:     /* ctrl-c */
            errno = len == 0 ? EAGAIN : ECANCELED ;
            return -1;
        case 127:   /* backspace */
#ifdef _WIN32
            /* delete in _WIN32*/
            /* win32read() will send 127 for DEL and 8 for BS and Ctrl-H */
            if (pos < len && len > 0) {
                memmove(buf+pos,buf+pos+1,len-pos);
                len--;
                buf[len] = '\0';
                refreshLine(fd,prompt,buf,len,pos,cols);
            }
            break;
#endif
        case 8:     /* ctrl-h */
            if (pos > 0 && len > 0) {
                memmove(buf+pos-1,buf+pos,len-pos);
                pos--;
                len--;
                buf[len] = '\0';
                refreshLine(fd,prompt,buf,len,pos,cols);
            }
            break;
        case 4:     /* ctrl-d, remove char at right of cursor */
            if (len > 1 && pos < (len-1)) {
                memmove(buf+pos,buf+pos+1,len-pos);
                len--;
                buf[len] = '\0';
                refreshLine(fd,prompt,buf,len,pos,cols);
            } else if (len == 0) {
                history_len--;
                free(history[history_len]);
                return -1;
            }
            break;
        case 20:    /* ctrl-t */
            if (pos > 0 && pos < len) {
                int aux = buf[pos-1];
                buf[pos-1] = buf[pos];
                buf[pos] = aux;
                if (pos != len-1) pos++;
                refreshLine(fd,prompt,buf,len,pos,cols);
            }
            break;
        case 2:     /* ctrl-b */
            goto left_arrow;
        case 6:     /* ctrl-f */
            goto right_arrow;
        case 16:    /* ctrl-p */
            seq[1] = 65;
            goto up_down_arrow;
        case 14:    /* ctrl-n */
            seq[1] = 66;
            goto up_down_arrow;
            break;
        case 27:    /* escape sequence */
#ifndef _WIN32
            if ( -1 == ( flag = fcntl( fd, F_GETFL ) ) )
            {
               break ;
            }
            fcntl( fd, F_SETFL, O_NONBLOCK ) ;
            if ( read(fd,seq,2) == -1 )
            {
               fcntl( fd, F_SETFL, flag ) ;
#endif
               // clear all input
               buf[0] = 0 ;
               history_len-- ;
               free(history[history_len]) ;
               return 0 ;
#ifndef _WIN32
            }
            fcntl( fd, F_SETFL, flag ) ;
#endif
            if (seq[0] == 91 && seq[1] == 68) {
left_arrow:
                /* left arrow */
                if (pos > 0) {
                    pos--;
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            } else if (seq[0] == 91 && seq[1] == 67) {
right_arrow:
                /* right arrow */
                if (pos != len) {
                    pos++;
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            } else if (seq[0] == 91 && seq[1] == 72) {
                /* home key */
                goto home_key;
            } else if (seq[0] == 91 && seq[1] == 70) {
                /* end key */
                goto end_key;
            } else if (seq[0] == 91 && (seq[1] == 65 || seq[1] == 66)) {
up_down_arrow:
                /* up and down arrow: history */
                if (history_len > 1) {
                    /* Update the current history entry before to
                     * overwrite it with tne next one. */
                    free(history[history_len-1-history_index]);
                    history[history_len-1-history_index] = strdup(buf);
                    /* Show the new entry */
                    history_index += (seq[1] == 65) ? 1 : -1;
                    if (history_index < 0) {
                        history_index = 0;
                        break;
                    } else if (history_index >= history_len) {
                        history_index = history_len-1;
                        break;
                    }
                    strncpy(buf,history[history_len-1-history_index],buflen);
                    buf[buflen] = '\0';
                    len = pos = strlen(buf);
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            } else if (seq[0] == 91 && seq[1] > 48 && seq[1] < 55) {
                /* extended escape */
                if (read(fd,seq2,2) == -1) break;
                if (seq[1] == 51 && seq2[0] == 126) {
                    /* delete */
                    if (len > 0 && pos < len) {
                        memmove(buf+pos,buf+pos+1,len-pos-1);
                        len--;
                        buf[len] = '\0';
                        refreshLine(fd,prompt,buf,len,pos,cols);
                    }
                }
            }
            break;
        default:
            if (len < buflen) {
                if (len == pos) {
                    buf[pos] = c;
                    pos++;
                    len++;
                    buf[len] = '\0';
                    if (plen+len < cols) {
                        /* Avoid a full update of the line in the
                         * trivial case. */
#ifdef _WIN32
                        if (!WriteConsole(hOut, &c, 1, &foo, NULL)) return -1;
#else
                        if (write(fd,&c,1) == -1) return -1;
#endif
                    } else {
                        refreshLine(fd,prompt,buf,len,pos,cols);
                    }
                } else {
                    memmove(buf+pos+1,buf+pos,len-pos);
                    buf[pos] = c;
                    len++;
                    pos++;
                    buf[len] = '\0';
                    refreshLine(fd,prompt,buf,len,pos,cols);
                }
            }
            break;
        case 21: /* Ctrl+u, delete the whole line. */
            buf[0] = '\0';
            pos = len = 0;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 11: /* Ctrl+k, delete from current to end of line. */
            buf[pos] = '\0';
            len = pos;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 1: /* Ctrl+a, go to the start of the line */
home_key:   pos = 0;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 5: /* ctrl+e, go to the end of the line */
end_key:    pos = len;
            refreshLine(fd,prompt,buf,len,pos,cols);
            break;
        case 12: /* ctrl+l, clear screen */
            linenoiseClearScreen();
            refreshLine(fd,prompt,buf,len,pos,cols);
        }
    }
    return len;
}

static int linenoiseRaw(char *buf, size_t buflen, const char *prompt) {
    int fd = STDIN_FILENO;
    int count;

    if (buflen == 0) {
        errno = EINVAL;
        return -1;
    }
    if (!isatty(STDIN_FILENO)) {
        if (fgets(buf, buflen, stdin) == NULL) return -1;
        count = strlen(buf);
        if (count && buf[count-1] == '\n') {
            count--;
            buf[count] = '\0';
        }
    } else {
        if (enableRawMode(fd) == -1) return -1;
        count = linenoisePrompt(fd, buf, buflen, prompt);
        disableRawMode(fd);
        printf("\n");
    }
    return count;
}

char *linenoise(const char *prompt) {
    char buf[LINENOISE_MAX_LINE];
    int count;

    if (isUnsupportedTerm()) {
        size_t len;

        printf("%s",prompt);
        fflush(stdout);
        if (fgets(buf,LINENOISE_MAX_LINE,stdin) == NULL) return NULL;
        len = strlen(buf);
        while(len && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
            len--;
            buf[len] = '\0';
        }
        return strdup(buf);
    } else {
        count = linenoiseRaw(buf,LINENOISE_MAX_LINE,prompt);
        if (count == -1) return NULL;
        return strdup(buf);
    }
}

/* Register a callback function to be called for tab-completion. */
void linenoiseSetCompletionCallback(linenoiseCompletionCallback *fn) {
    completionCallback = fn;
}

void linenoiseAddCompletion(linenoiseCompletions *lc, char *str) {
    size_t len = strlen(str);
    if ( len > lc->maxStrLen )
    {
      lc->maxStrLen = len ;
    }
    char *copy = (char*)malloc(len+1);
    memcpy(copy,str,len+1);
    lc->cvec = (char**)realloc(lc->cvec,sizeof(char*)*(lc->len+1));
    lc->cvec[lc->len++] = copy;
}

int linenoiseHistorySize()
{
   return history_len ;
}

int linenoiseHistoryMaxSize()
{
   return history_max_len ;
}

const char* linenoiseHistoryGet(int pos)
{
   if ( !history || pos >= history_len || pos < 0 )
   {
      return NULL ;
   }
   return history[pos] ;
}

/* Using a circular buffer is smarter, but a bit more complex to handle. */
int linenoiseHistoryAdd(const char *line) {
    char *linecopy;

    if (history_max_len == 0) return 0;
    if (history == NULL) {
        history = (char **)malloc(sizeof(char*)*(history_max_len));
        if (history == NULL) return 0;
        memset(history,0,(sizeof(char*)*(history_max_len)));
    }

    // find whether is the same with last
    if ( history_len > 0 && 0 == strcmp(line, history[history_len-1]) )
    {
      return 0 ;
    }

    linecopy = strdup(line);
    if (!linecopy) return 0;
    if ( history_len == history_max_len ) {
        free(history[0]);
        memmove(history,history+1,sizeof(char*)*(history_max_len-1));
        history_len--;
    }
    history[history_len] = linecopy;
    history_len++;
    return 1;
}

int linenoiseHistorySetMaxLen(int len) {
    char **newHist;

    if (len < 1) return 0;
    if (history) {
        int tocopy = history_len;

        newHist = (char **)malloc(sizeof(char*)*len);
        if (newHist == NULL) return 0;
        if (len < tocopy) tocopy = len;
        memcpy(newHist,history+(history_max_len-tocopy), sizeof(char*)*tocopy);
        free(history);
        history = newHist;
    }
    history_max_len = len;
    if (history_len > history_max_len)
        history_len = history_max_len;
    return 1;
}

/* Save the history in the specified file. On success 0 is returned
 * otherwise -1 is returned. */
int linenoiseHistorySave(const char *filename) {
#ifdef _WIN32
    FILE *fp = fopen(filename,"wb");
#else
    FILE *fp = fopen(filename,"w");
#endif
    int j;

    if (fp == NULL) return -1;
    for (j = 0; j < history_len; j++)
        fprintf(fp,"%s\n",history[j]);
    fclose(fp);
    return 0;
}

/* Load the history from the specified file. If the file does not exist
 * zero is returned and no operation is performed.
 *
 * If the file exists and the operation succeeded 0 is returned, otherwise
 * on error -1 is returned. */
int linenoiseHistoryLoad(const char *filename) {
    FILE *fp = fopen(filename,"r");
    char buf[LINENOISE_MAX_LINE];

    if (fp == NULL) return -1;

    while (fgets(buf,LINENOISE_MAX_LINE,fp) != NULL) {
        char *p;

        p = strchr(buf,'\r');
        if (!p) p = strchr(buf,'\n');
        if (p) *p = '\0';
        linenoiseHistoryAdd(buf);
    }
    fclose(fp);
    return 0;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SETDISPLAYATTRIBUTE, "setDisplayAttribute" )
static void setDisplayAttribute( bool enhancedDisplay )
{
    PD_TRACE_ENTRY ( SDB_SETDISPLAYATTRIBUTE );
#ifdef _WIN32
    if ( enhancedDisplay ) {
        CONSOLE_SCREEN_BUFFER_INFO inf;
        GetConsoleScreenBufferInfo( hOut, &inf );
        oldDisplayAttribute = inf.wAttributes;
        BYTE oldLowByte = oldDisplayAttribute & 0xFF;
        BYTE newLowByte;
        switch ( oldLowByte ) {
        case 0x07:
            //newLowByte = FOREGROUND_BLUE | FOREGROUND_INTENSITY;  // too dim
            //newLowByte = FOREGROUND_BLUE;                         // even dimmer
            newLowByte = FOREGROUND_BLUE | FOREGROUND_GREEN;        // most similar to xterm appearance
            break;
        case 0x70:
            newLowByte = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
            break;
        default:
            newLowByte = oldLowByte ^ 0xFF;     // default to inverse video
            break;
        }
        inf.wAttributes = ( inf.wAttributes & 0xFF00 ) | newLowByte;
        SetConsoleTextAttribute( hOut, inf.wAttributes );
    }
    else {
        SetConsoleTextAttribute( hOut, oldDisplayAttribute );
    }
#else
    if ( enhancedDisplay ) {
        if ( write( 1, "\x1b[1;34m", 7 ) == -1 ) return; /* bright blue (visible with both B&W bg) */
    }
    else {
        if ( write( 1, "\x1b[0m", 4 ) == -1 ) return; /* reset */
    }
#endif
   PD_TRACE_EXIT ( SDB_SETDISPLAYATTRIBUTE );
}

