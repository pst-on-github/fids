/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: makescreen							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/* make the screen layout                                               */
/************************************************************************/
#include "fids.h"
#include "fidsext.h"
#include <grp.h>
#include <pwd.h>
#include <sys/utsname.h>

#ifdef QNX4
#include <sys/param.h>
#endif
#include <netdb.h>

extern BOOLEAN GRAF_VT100;

static struct utsname utsbuf;
char                  flogin[25];

void makescreen()
{
    struct group  *gr;
    struct passwd *pw;
    static BOOLEAN first = TRUE;
    static char    term[25], uid[25], gid[25], inet[25];
    char          *dis, fmtstr[32];

    /* check the terminal lines and cols */
    if (TermLines < TERM_MIN_LINES || TermCols < TERM_MIN_COLS)
    {
        if (TermLines > max(TermLines / 2, 1))
        {
            gotoxy(max(TermCols / 2 - 11, 1), max(TermLines / 2, 1) + 1);
            printf("   [ min. %d x %d ]", TERM_MIN_COLS, TERM_MIN_LINES);
        }
        gotoxy(max(TermCols / 2 - 11, 1), max(TermLines / 2, 1));
        printf("%sFIDS term too small !!%s", A_FAILURE, A_NORMAL);
        fflush(stdout);
        return;
    }

    /* calculate the max. number of characters for
                               printing the name of one file */
    FileLstNameLen =
        (TermCols - 24 /* min. RightWinCols =16 plus borders */) / FileLstRows;
    FrightWinCols = TermCols - (FileLstNameLen * FileLstRows + 4 + FileLstRows);

    if (first)
    {
        char *gl = NULL;
        first    = FALSE;
        if (uname(&utsbuf) == ERR)
            syserr("uname");

        sprintf(term, "%.24s", ttyname(0));
        gl = getlogin();
        sprintf(flogin, "%.24s", gl ? gl : "<-?->");

        if ((pw = getpwuid(getuid())) == NULL)
        {
            sprintf(uid, "%24d", (int)getuid());
        }
        else
        {
            sprintf(uid, "%.24s", pw->pw_name);
        }

        if ((gr = getgrgid((int)getgid())) == NULL)
        {
            sprintf(gid, "%24d", (int)getgid());
        }
        else
        {
            sprintf(gid, "%.24s", gr->gr_name);
        }

        {
            struct hostent *hp;
#ifdef QNX4
            char host[MAXHOSTNAMELEN + 1];

            if (gethostname(host, MAXHOSTNAMELEN) == -1)
                host[0] = '\0';

            if ((hp = gethostbyname(host)) == 0)
#else
            if ((hp = gethostbyname(utsbuf.nodename)) == 0)
#endif
            {
                strcpy(inet, "?");
            }
            else
            {
                unsigned char *p_inet;

                p_inet = (unsigned char *)hp->h_addr;
                sprintf(inet, "%d.%d.%d.%d", *p_inet, *(p_inet + 1),
                        *(p_inet + 2), *(p_inet + 3));
            }
        }
    }

    makevline(TermCols - (FrightWinCols + 1)); /* make vertical   line in 63 */
    makevline(TermCols);                       /* make vertical   line in 80   */

    makehline(2, 'u'); /* make horizontal line in 2      */

    gotoxy(TermCols - (FrightWinCols + 1), max(TermLines - 13, 7));
    if (GRAF_VT100)
    {
        printf(GRAF_ON);
        putchar(GRAF_VR);
    }
    else
        putchar(GRAF_NC);
    gotoxy(TermCols, max(TermLines - 13, 7));
    if (GRAF_VT100)
    {
        putchar(GRAF_VL);
        printf(GRAF_OFF);
    }
    else
        putchar(GRAF_NC);

    gotoxy(TermCols - (FrightWinCols - 1), 1);
    printf("fids ");
    if ((strlen(VERSION) + 13) > FrightWinCols)
        sprintf(fmtstr, "%%-%1d.%1ds", FrightWinCols - 10, FrightWinCols - 10);
    else
        sprintf(fmtstr, "%%-%1d.%1ds by", FrightWinCols - 13,
                FrightWinCols - 13);
    printf(fmtstr, VERSION);
    printf(" BeNo");

    gotoxy(TermCols - FrightWinCols, 3);
    if (strlen(inet) < FrightWinCols - 3)
        sprintf(fmtstr, "net:%%%2d.%2ds", FrightWinCols - 4, FrightWinCols - 4);
    else
        sprintf(fmtstr, " %%%2d.%2ds", FrightWinCols - 1, FrightWinCols - 1);
    printf(fmtstr, inet);

    gotoxy(TermCols - FrightWinCols, 4);
    printf("date:");
    gotoxy(TermCols - FrightWinCols, 5);
    printf("time:");

#ifdef QNX4
    gotoxy(TermCols - FrightWinCols, 6);
    sprintf(fmtstr, "phot:%%%2d.%2ds", FrightWinCols - 5, FrightWinCols - 5);
    printf(fmtstr, (dis = getenv("PHOTON")) ? dis : "<-?->");
#else
    gotoxy(TermCols - FrightWinCols, 6);
    sprintf(fmtstr, "disp:%%%2d.%2ds", FrightWinCols - 5, FrightWinCols - 5);
    printf(fmtstr, (dis = getenv("DISPLAY")) ? dis : "<-?->");
#endif

    if (TermLines > 20)
    {
        gotoxy(TermCols - FrightWinCols, 7);
        sprintf(fmtstr, "login:%%%2d.%2ds", FrightWinCols - 6,
                FrightWinCols - 6);
        printf(fmtstr, flogin);
    }
    if (TermLines > 21)
    {
        gotoxy(TermCols - FrightWinCols, 8);
        sprintf(fmtstr, "uid:%%%2d.%2ds", FrightWinCols - 4, FrightWinCols - 4);
        printf(fmtstr, uid);
    }
    if (TermLines > 22)
    {
        gotoxy(TermCols - FrightWinCols, 9);
        sprintf(fmtstr, "gid:%%%2d.%2ds", FrightWinCols - 4, FrightWinCols - 4);
        printf(fmtstr, gid);
    }
    if (TermLines > 23)
    {
        gotoxy(TermCols - FrightWinCols, max(TermLines - 14, 6));
        /*	  printf("free:");	*/
    }
    if (TermLines > 24)
    {
        int ii;
        gotoxy(TermCols - FrightWinCols, 10);
        sprintf(fmtstr, "term:%%%2d.%2ds", FrightWinCols - 5,
                FrightWinCols - 5);
        ii = strlen(term) - (FrightWinCols - 5);
        if (ii < 0)
            ii = 0;
        printf(fmtstr, &term[ii]);
    }
    if (TermLines > 25)
    {
        gotoxy(TermCols - FrightWinCols, 11);
        printf("system:");
        prt_str(FrightWinCols - 7, FrightWinCols - 7, utsbuf.sysname);
    }
    if (TermLines > 26)
    {
        gotoxy(TermCols - FrightWinCols, 12);
        printf("version:");
        prt_str(FrightWinCols - 8, FrightWinCols - 8, utsbuf.version);
    }
    if (TermLines > 27)
    {
        gotoxy(TermCols - FrightWinCols, 13);
        printf("release:");
        prt_str(FrightWinCols - 8, FrightWinCols - 8, utsbuf.release);
    }
    if (TermLines > 28)
    {
        gotoxy(TermCols - FrightWinCols, 14);
        if (FrightWinCols < 17 && strlen(utsbuf.machine) > 8)
        {
            printf("machin:");
            prt_str(FrightWinCols - 7, FrightWinCols - 7, utsbuf.machine);
        }
        else
        {
            printf("machine:");
            prt_str(FrightWinCols - 8, FrightWinCols - 8, utsbuf.machine);
        }
    }

        gotoxy(TermCols - FrightWinCols, max(TermLines - 13, 7));
        sprintf(fmtstr, "%%s  %%-*.*s%%s");
        printf(fmtstr, A_TITLE, FrightWinCols - 2, FrightWinCols - 2, "actual file:", A_NORMAL);
    gotoxy(TermCols - FrightWinCols, max(TermLines - 12, 8));
    printf("%s>%s", A_TITLE, A_NORMAL);
    gotoxy(TermCols - 1, max(TermLines - 12, 8));
    printf("%s<%s", A_TITLE, A_NORMAL);

    gotoxy(TermCols - FrightWinCols, max(TermLines - 11, 9));
    printf("mode:");
    gotoxy(TermCols - FrightWinCols, max(TermLines - 10, 10));
    printf("bytes:");
    gotoxy(TermCols - FrightWinCols, max(TermLines - 9, 11));
    printf("uid:");
    gotoxy(TermCols - FrightWinCols, max(TermLines - 8, 12));
    printf("gid:");
    if (TermLines > 17)
    {
        gotoxy(TermCols - FrightWinCols, max(TermLines - 7, 13));
        printf("links:");
    }
    if (TermLines > 18)
    {
        gotoxy(TermCols - FrightWinCols, max(TermLines - 6, 14));
        printf("inode:");
    }
    if (TermLines > 19)
    {
        gotoxy(TermCols - FrightWinCols, max(TermLines - 5, 15));
        printf("acc-modi-chan-t:");
    }
}
/*                                                                      */
/************************************************************************/
/* write a horizontal line to the terminal         line=vertical line   */
/*                                                                      */
/************************************************************************/
void makehline(line, where) int line;
char where;
{
    int i;
    if (GRAF_VT100)
        printf(GRAF_ON);
    gotoxy(1, line);
    printf(A_NORMAL);
    for (i = 1; i <= TermCols; i++)
    {
        if (i == 1)
        {
            if (GRAF_VT100)
            {
                if (where == 'u')
                    putchar(GRAF_UL);
                else
                    putchar(GRAF_LL);
            }
            else
                putchar(GRAF_NC);
        }
        else if ((FileLstRows == 4 &&
                  (i == FileLstNameLen + 3 /* 17 */ ||
                   i == FileLstNameLen * 2 + 4 /* 32 */ ||
                   i == FileLstNameLen * 3 + 6 /* 48 */ ||
                   i == FileLstNameLen * 4 + 7 /* 63 */)) ||
                 (FileLstRows == 2 &&
                  (i == FileLstNameLen + 3 /* 32 */ ||
                   i == FileLstNameLen * 2 + 5 /* 63 */)))
        {
            if (GRAF_VT100)
            {
                if (where == 'u')
                    putchar(GRAF_HD);
                else
                    putchar(GRAF_HU);
            }
            else
                putchar(GRAF_NC);
        }
        else if (i == TermCols)
        {
            if (GRAF_VT100)
            {
                if (where == 'u')
                    putchar(GRAF_UR);
                else
                    putchar(GRAF_LR);
            }
            else
                putchar(GRAF_NC);
        }
        else
        {
            if (GRAF_VT100)
                putchar(GRAF_H);
            else
                putchar(GRAF_NH);
        }
    }
    if (GRAF_VT100)
        printf(GRAF_OFF);

    if (line == 2 && where == 'u' && utsbuf.nodename[0] != '\0')
    {
        gotoxy(TermCols - FrightWinCols, 2);
        printf("%snode: ", A_TITLE);
#ifdef QNX4
        {
            char host[MAXHOSTNAMELEN + 1];
            char node[MAXHOSTNAMELEN * 2];

            if (gethostname(host, MAXHOSTNAMELEN) == -1)
                sprintf(host, "?");

            sprintf(node, "%s (%s)", utsbuf.nodename, host);
            prt_str(FrightWinCols - 6, FrightWinCols - 6, node);
        }
#else
        prt_str(FrightWinCols - 6, FrightWinCols - 6, utsbuf.nodename);
#endif
        printf(A_NORMAL);
    }
}
/*                                                                      */
/************************************************************************/
/* write a vertical   line to the terminal         line=horizontal line */
/*                                                                      */
/************************************************************************/
void makevline(line) int line;
{
    int i;
    gotoxy(line, 3);
    if (GRAF_VT100)
        printf(GRAF_ON);
    for (i = 3; i <= (TermLines - 2); i++)
    {
        gotoxy(line, i);
        if (GRAF_VT100)
            putchar(GRAF_V);
        else
            putchar(GRAF_NV);
    }
    if (GRAF_VT100)
        printf(GRAF_OFF);
}
