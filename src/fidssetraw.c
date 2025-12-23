/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: setraw							*/
/*           setwinsize                                                 */
/*           termrest                                                   */
/*                                                                      */
/************************************************************************/
/* set the terminal into raw mode                                       */
/************************************************************************/
#include <sys/ioctl.h>

#ifdef QNX6
#include <termios.h>
#define termio termios
#else
#include <termio.h>
#endif
#include "fids.h"
#include "fidsext.h"

#ifdef QNX6
static struct termios tbufsave;
#else
static struct termio tbufsave;
#endif

void setraw2(int isig, int opost)
{
#if defined QNX6
    struct termios tbuf;
#else
    struct termio tbuf;
#endif
    static BOOLEAN first = TRUE;

#if defined QNX6
    if (tcgetattr(FD_STDIN, &tbuf) == ERR)
#else
    if (ioctl(FD_STDIN, TCGETA, &tbuf) == ERR)
#endif
        syserr("ioctl get termio");
    if (first)
    {
        first    = FALSE;
        tbufsave = tbuf; /* save the old structure */
    }
/* INLCR : map (not map) NL to CR                               */
/* ICRNL : map (not map) CR to NL                               */
/* IUCLC : map (not map) upper case to lower case on input      */
/* ISTRIP: strip (not)   input characters to seven bits         */
/*	tbuf.c_iflag &= ~(INLCR|ICRNL|IUCLC|ISTRIP|BRKINT);	*/
#if defined QNX4 || defined QNX6
    tbuf.c_iflag &= ~(INLCR | ICRNL | ISTRIP);
#else
    tbuf.c_iflag &= ~(INLCR | ICRNL | IUCLC | ISTRIP);
#endif
#ifdef hds
    tbuf.c_iflag &= ~(IXON | IXOFF); /* ^S / ^Q available for application */
#endif
    tbuf.c_oflag &= ~OPOST;
    if (opost == 1)
        tbuf.c_oflag |= OPOST;
    tbuf.c_lflag &= ~(ICANON | ISIG | ECHO);
    if (isig == 1)
        tbuf.c_lflag |= ISIG;
    tbuf.c_cc[4] = 1; /* MIN */
    tbuf.c_cc[5] = 1; /* TIME */
#if defined QNX6
    if (tcsetattr(FD_STDIN, TCSADRAIN, &tbuf) == ERR)
#else
    if (ioctl(FD_STDIN, TCSETA, &tbuf) == ERR)
#endif
        syserr("ioctl set termio");

    printf(KPAD_OFF); /* cursor keypad mode normal	*/
    printf(A_NORMAL); /* all Attributes off		*/
    printf(GRAF_OFF); /* graphic mode off		*/
    if (opost == 0)
        printf(WRAP_OFF); /* set wrap mode off            */
    printf(CIVIS);        /* set cursor invisible	        */

    if (opost == 0)
        setwinsize(&TermLines, &TermCols);
}

void setraw(int isig) { setraw2(isig, 0); }

/* set the previously saved terminal flags back                         */
void termrest()
{
    /* switch the terminal settings back to normal operation	*/
    printf(KPAD_OFF); /* cursor keypad mode normal	*/
    printf(A_NORMAL); /* all Attributes off		*/
    printf(GRAF_OFF); /* graphic mode off		*/
    printf(WRAP_ON);  /* set wrap mode off            */
    printf(CNORM);    /* set cursor normal visible	*/
    fflush(stdout);

#if defined QNX6
    if (tcsetattr(FD_STDIN, TCSADRAIN, &tbufsave) == ERR)
#else
    if (ioctl(FD_STDIN, TCSETAF, &tbufsave) == ERR)
#endif
    {
        fprintf(stderr, "ERROR : fids_termrest");
    }
}

/* get the actual terminal window size */

void setwinsize(p_lines, p_cols) int *p_lines, *p_cols;
{
    int t_lines = *p_lines, t_cols = *p_cols;

#if defined QNX6
    if (tcgetsize(FD_STDIN, &t_lines, &t_cols) == ERR)
    {
        t_lines = *p_lines; /* Restore rows */
        t_cols  = *p_cols;  /* Restore columns */
    }

#elif defined SCO /* by HDS */
    {
        FILE       *fd;
        static char X11_tput[] =
            "eval `/usr/bin/X11/resize -u`;tput lines;tput cols";
        static char Unix_tput[] = "tput lines;tput cols";
        char       *tput_cmd    = getenv("DISPLAY") ? X11_tput : Unix_tput;

        fd = popen(tput_cmd, "r");
        if (fd)
        {
            fscanf(fd, "%d", &t_lines);
            fscanf(fd, "%d", &t_cols);
            pclose(fd);
        }
    }
#elif defined TIOCGWINSZ
    struct winsize win_size;

    if (ioctl(FD_STDIN, TIOCGWINSZ, &win_size) != ERR)
    {
        t_lines = (int)win_size.ws_row; /* Rows, in characters    */
        t_cols  = (int)win_size.ws_col; /* Columns, in characters */
    }
#endif

    if (t_lines > 0 && t_lines < 1000)
        *p_lines = t_lines;

    if (t_cols > 0 && t_cols < 1000)
        *p_cols = t_cols;

    return;
}
