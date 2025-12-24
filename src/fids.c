/************************************************************************/
/* programm fids                                B.Noeggerath, BeNoSoft  */
/* funktion main                                Rel. 2.9.87  12.05.2005 */
/*                                                                      */
/*       Fullscreen Interactive Directory Shell for UNIX                */
/*       ===============================================                */
/*                                                                      */
/*  Permission to use, copy, modify, and distribute this software and   */
/*  its documentation  for  any  purpose  and  without  fee is hereby   */
/*  granted, provided that the above copyright notice appear  in  all   */
/*  copies and that both  that  copyright  notice  and  this  permis-   */
/*  sion  notice appear in supporting  documentation,  and  that  the   */
/*  name of Bernhard Noeggerath  not be used in advertising or publi-   */
/*  city pertaining to distribution  of the software without  specif-   */
/*  ic, written prior permission.                                       */
/*                                                                      */
/*                                                                      */
/* History:                                                             */
/*                                                                      */
/* Rel 2.8 27.10.1988	printf ersetzt fuer char < ' '.			*/
/*			file_names intern > 14 character		*/
/* Rel 2.9 11.11.1988	.fids.pf_keys:	User_definitions		*/
/*					for special CTRL_cmd's :	*/
/*                           .^A=e ^N^Mx   # Cmd_sequence for <CTRL>A   */
/*                           .^P=cp ^N^U   # Cmd_sequence for <CTRL>P   */
/*                           .^O=mv ^N^U   # Cmd_sequence for <CTRL>O   */
/*           retrieve buffer_depth changed from 16 to 32                */
/*           option -g       don't use the grafic alter_char_set        */
/* R.2.9ev cmd_mode        cd $name : substitute with env_var value	*/
/*         cmd_exec        execute ./cmd_name to take the current dir	*/
/* 2.9.02 cmd_view        correktur fuer character_count if \t found	*/
/* 2.9.03 readdir         changed to fids_readdir for SVR3		*/
/* 2.9.04 cmd_ex_ret      if exitcode != 0 print it in "Hit any..."	*/
/* 2.9.05 setuid          setuid to euid to allow s-bit setting		*/
/*        f_clock         >You have mail< only, if size > 0		*/
/* 2.9.06 fidsview()      typeahead eingebaut               		*/
/* 2.9.07 `command`       substitute commands in command mode, 05.12.89 */
/* 2.9.08 name=value      putenv for environment variable assignment	*/
/* 2.9.09 setraw          TCSETAF -> TCSETA (don't flush input buffer	*/
/*									*/
/* 04.04.90  2.9.40 fidsreaddir     usage of directory subroutines SVR3	*/
/*           			    ( dirent.h instead of dir.h )	*/
/* 28.04.92  2.9.41	port to SCO-Unix				*/
/* 08.08.92		Changes for SCO-UNIX (ansi) by HDS		*/
/* 2.9.42               port to ULTRIX by MB            	*/
/*    "      fids.c     bug-fix in cmd_split (no of files + 1)		*/
/* 2.9.43 fidssetblock.c  get_key(): <esc>, <esc> sequence behaviour	*/
/* 2.9.44 fidssetblock.c  get_key(): <esc>, <esc> sequence behaviour	*/
/* 2.9.50  05.05.94	collect all changes together, BeNo		*/
/*           		in ULTRIX the read() is continued after a signal, */
/*			so i have to use the setjmp() / longjmp() !	*/
/* 2.9.52  16.05.94	Intern ESC commands, quit with user question,	*/
/*			.fids.pf_keys " .^@ = exec after loading	*/
/* 2.9.53  26.05.94	if a directory is not readable, goto ..		*/
/* 2.9.54  31.05.94	exit fids on signal SIGQUIT			*/
/* 2.9.55  06.06.94	signal handling					*/
/* 2.9.56  08.06.94	signal handling	DEC alpha			*/
/* 2.9.57  08.07.94	some bugs found with purify !			*/
/* 2.9.58  21.07.94	get the terminal window size with ioctl 	*/
/* 2.9.59  04.10.94	use macros major(),minor() if available 	*/
/* 2.9.60  20.10.94	changes for disp symbolik links (Broken Links)  */
/* 2.9.61  25.10.94	calculations window size dependend              */
/* 2.9.62  02.02.95	bug fix in symbolic links to symbolic links     */
/* 2.9.63  16.02.95	bug fix in cmd_cdu if we are up to "/"          */
/*                      mark (tag) now also for directories             */
/* 2.9.64  23.02.95	bug fix in print ...dirname                     */
/*                      cmd_cd may not get the cwd                      */
/* 2.9.65  13.03.95	possible error with readlink in SCO fixed       */
/*                      cmd_line edit with INS-Mode (by HDS)            */
/* 2.9.66  18.10.95	change in fids_cmp "sum" for 'ULTRIX'           */
/* 2.9.67  28.06.96	changes in delete file if a symbolik link       */
/*                      .fids.pf_keys: if 1.Char=='#' then # is remark  */
/*                      New cmd fids_tag                                */
/*                      substitute commands now with !command           */
/* 2.9.68  17.07.96	^F and ^E for include/exclude filespec (by HDS) */
/* 2.9.69  05.08.96	^F,^E changed to fids_incl,fids_excl   (by HDS) */
/* 2.9.70  13.08.96	Bug Fix : use CMDL_C in array cmd_line[][] !!!  */
/*                      fids_cmp: don't call 'sum' for a special file ! */
/* 2.9.71  28.05.97	New cmd 'fids_rows' 4 or 2 and new 'fids_set'   */
/* 2.9.72  05.06.97     New help pages with <ESC>?                      */
/* 2.9.73  16.06.97     <DEL> is intern converted to ^_ and may so      */
/*                      be set in the .fids.pf_keys file                */
/* 2.9.74  25.06.97     cmd_view add cmd's via pipe interruptable       */
/* 2.9.75  04.07.97     cmd_line add cmd's : jump cursor ESC'+' ESC'-'  */
/* 2.9.76  23.07.97     ins file_name other side, ins symbolic link nam */
/* 2.9.77  15.01.98     Port to QNX4/stoe                               */
/*                      Fehler behoben: String cwd wenn #cols < 80      */
/* 2.9.78  18.03.98     horizontal scrolling of the cmd line added      */
/* 2.9.79  25.03.98     options added and load/store actual settings    */
/* 2.9.80  11.04.98     filesystem info's added                         */
/* 2.9.81  10.08.98     stoe: no err on reload saved settings director. */
/* 2.9.81b 16.12.99     bugfix Y2k: printdate() now with tm_year%100    */
/* 2.9.82  07.11.00     stoe: node & ip addr display (QNX), Y2K READY   */
/* 2.9.83  26.08.02     stoe: special named file indication (IFNAM,n)   */
/* 2.9.84  23.11.03     auto filename delimiter ' for names with ' 's   */
/* 2.9.85  25.11.03     new united fids sources                         */
/* 2.9.86  18.05.04     new fids_cmp (don't use sum !)                  */
/* 2.9.87  12.05.05     filesize > 2GB for Linux 2.4                    */
/************************************************************************/
/*                                                                      */

#define RELEASE "2.9.87"

#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <time.h>
#ifdef QNX4
#include <sys/wait.h>
#endif
#define _fids_
#include "fids.h"

static int cmd_atag(void);
static int cmd_attag(void);
static int cmd_cm(void);
static int cmd_copy(void);
static int cmd_cr(void);
static int cmd_del(void);
static int cmd_exec(void);

static int cmd_fpage(void);
static int cmd_helpp(void);
static int cmd_inam(void);
static int cmd_lpage(void);
static int cmd_move(void);
static int cmd_onam(void);
static int cmd_pdn(void);
static int cmd_pup(void);
static int cmd_rel(void);
static int cmd_retrl(void);
static int cmd_retrp(void);
static int cmd_sort(void);
static int cmd_split(void);
static int cmd_switch(void);
static int cmd_tag(void);

static void cmd_winch(int sig_num);
static void cmd_sig(int sig_num);

/*                                                                      */
/************************************** global declarationen ************/
/*                                                                      */

extern char GEN_SYS[];
extern char GEN_DAT[];
extern char GEN_UID[];

extern char gkb[8]; /* get_key_buffer for get_key() */

struct FIDS **p_lsl, **p_lsr, **p_ls;

int          numdl, numdr, numd;                               /* number of subdirs left/right */
int          numfl, numfr, numf;                               /* number of files left/right   */
int          scrl, scrr, scr;                                  /* index to dirs/files on screen*/
int          actil, actir, acti;                               /* index to dir/file actual     */
int          scrn  = 1;                                        /* number of logical screens    */
char         actlr = 'l';                                      /* l: left , r: right           */
char         smodl = 'n', smodr;                               /* sort modus                   */
static char *term;                                             /* terminal type from $TERM     */
int          TermLines = 24, TermCols = 80;                    /* actual number of lines, cols */
int          FileLstRows                               = 4;    /* actual number of rows for file_names */
int          FileLstNameLen                            = 14;   /* actual number of char used for file_names */
int          FrightWinCols                             = 16;   /* actual number of columns for the right window */
BOOLEAN      GRAF_VT100                                = TRUE; /* switch for vt100 Graphicmode */
int          TermGrix                                  = 0;    /* default = 'vt100' grafic char */
char        *Term_Names[TERM_NAME_NUM][TERM_NAME_SNUM] = FIDS_TERM_NAMES;
char         Term_Grafic[TERM_NAME_NUM][TERM_GRAF_NUM] = FIDS_TERM_GRAFIC;
char        *Term_Attr_Str[TERM_NAME_NUM][TERM_ATTR_STRING_NUM] =
    FIDS_TERM_ATTR_STRINGS;
char *Term_Attr[TERM_NAME_NUM][TERM_ATTR_NUM] = FIDS_TERM_ATTR;

char        dir_n_l[NDIR];           /* buffer for dir_name_left     */
char        dir_n_r[NDIR];           /* buffer for dir_name_right    */
char        dir_n_s[NDIR];           /* buffer for dir_name_saved    */
static char dir_f_n[NDIR] = "..";    /* buffer for dir_file_name     */
char       *p_cwd         = dir_n_l; /* pointer to current work dir  */

char pf_ctl[PFS][PFK_C];   /* field for the pf_ctl commands */
char pf_cctl[PFS][PFK_C];  /* field for the pf_ctl for Cmode */
char pf[PFK][PFK_C];       /* field for the pf_key commands */
char pf_s[PFK][PFK_C];     /* field for the shift pf_key commands */
char pf_nam[PFK][PFN + 1]; /* field for the pf_command names*/

static char  cmd_line[CMDL_N][CMDL_C]; /* command history table        */
char        *p_cmdl[CMDL_N];           /* pointer to commands in table */
static char *ps1;                      /* pointer to environment PS1   */
static char  scan_mail = 'y';          /* if 'y' scan mail on alarm    */

static char *FS_save_name = NULL; /* if == NULL use uid as name   */
BOOLEAN      FS_save_flag = TRUE; /* save actual settings on exit	*/

char filespec[PATH_MAX] = "*"; /* filespec for fidsreaddir     */
char exclspec[PATH_MAX] = "";  /* exclude filespec for fidsreaddir */

static BOOLEAN stat_fidsnew;
BOOLEAN        stat_list, stat_setact;
static BOOLEAN err_flag    = FALSE;
BOOLEAN        err_active  = FALSE; /* error message active (stoe) */
BOOLEAN        insert_mode = TRUE;  /* insert/overstrike toggle, (HDS) */

static unsigned time_out;
unsigned        time_def = CLK_RATE;

static struct cmd_liste
{
    int cmd_char;
    int cmd_cchar; /* Default CTRL() or fixed bindings */
    int (*cmd_fkt)(void);
} cmd[] = {
    /* ECS Key bindings */

    {ESC1('Q'), CTRL('C'), cmd_askexit},
    {ESC1('/'), CR,        cmd_cr     },
    {ESC1('#'), '&',       cmd_split  },
    {ESC1('_'), 0,         cmd_cm     },
    {ESC1('J'), CTRL('J'), cmd_retrp  },
    {ESC1('K'), CTRL('K'), cmd_retrl  },
    {ESC1('P'), 0,         cmd_pup    },
    {ESC1('N'), 0,         cmd_pdn    },
    {ESC1('<'), CTRL('F'), cmd_fpage  },
    {ESC1('>'), CTRL('L'), cmd_lpage  },
    {ESC1('S'), CTRL('T'), cmd_sort   },
    {ESC1('I'), CTRL('I'), cmd_switch },
    {ESC1('*'), CTRL('U'), cmd_attag  },
    {ESC1('V'), CTRL('W'), cmd_view   },
    {ESC1('R'), CTRL('D'), cmd_del    },
    {DEL,       CTRL('_'), cmd_del    },
    {ESC1('@'), ' ',       cmd_tag    },
    {ESC1('T'), 0,         cmd_atag   },
    {ESC1('U'), 0,         cmd_utag   },
    {ESC1('X'), CTRL('Z'), cmd_exec   },
    {ESC1('H'), 0,         cmd_helpp  },
    {ESC1('.'), CTRL('N'), cmd_inam   },
    {ESC1('C'), CTRL('P'), cmd_copy   },
    {ESC1('M'), CTRL('O'), cmd_move   },
    {ESC1('O'), CTRL('V'), cmd_onam   },
    {ESC1('!'), CTRL('G'), cmd_rel    },
    {ESC1('q'), 0,         cmd_askexit},
    {ESC1('?'), '?',       cmd_helpp  },

    {0,         0,         NULL       }
}; /* list end     */

int   FidsCloseOnInterrupt   = -1;
FILE *FidsF1closeOnInterrupt = NULL;
FILE *FidsF2closeOnInterrupt = NULL;
FILE *FidsPcloseOnInterrupt  = NULL;
#if defined(QNX4) || defined(_GNU_SOURCE)
static sigjmp_buf ljmp_env;
#else
static jmp_buf ljmp_env;
#endif

#ifdef _UATOABORT
#define ABORT_CHAR 'a'
#else
#define ABORT_CHAR 'q'
#endif

/* ------------------------------------------------------------------------- */
/*ARGSUSED*/
static void f_clock(int sig_num) /* the clock interrupt function */
{
    struct stat stdir;
    struct tm  *p_tm;
    time_t      tloc;

    static char *day[] = {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"};

    if (!stat_setact)
        stat_setact = setact('.');

    if (time_out > 1)
    {
        stat_setact = FALSE;
        stat_list   = fidsnew('n');

        if (scan_mail == 'y')
        {
            char *m_path;
            char  mail_path[NDIR];

            if (!(m_path = getenv("MAIL")))
            {
                sprintf(mail_path, "%s/%s", MAIL_DIR, getlogin());
                m_path = mail_path;
            }

            if (stat(m_path, &stdir) != ERR)
            {
                if (stdir.st_size)
                {
                    error("You have mail !", "");
                    scan_mail = 'n';
                }
            }
        }
    }

    if (time(&tloc) == (time_t)ERR)
        syserr("time");

    p_tm = localtime(&tloc);

    save_cursor();
    gotoxy(TermCols - 9, 4);
    printdate(p_tm);

    gotoxy(TermCols - 9, 5);
    printf("%s %02d:%02d", day[p_tm->tm_wday], p_tm->tm_hour, p_tm->tm_min);

    restore_cursor();

#if defined(QNX4) || defined(_GNU_SOURCE)
    siglongjmp(ljmp_env, -1);
#else
    longjmp(ljmp_env, -1);
#endif
}

/*                                                                      */
/************************************************************************/

int main(int argc, char *argv[])
{
    /************************************** declarationen main **************/
    /*                                                                      */
    struct passwd *pw;
    int            i, c, opt_sl;
    char          *z1;
    /*                                                                      */
    /************************************** start main **********************/
    /*                                                                      */
    if (!isatty(FD_STDIN))
    {
        fprintf(stderr, "%s: ERROR = stdin not a tty !\n", argv[0]);
        exit(-1);
    }

    if (argc >= 2)
    {
        if ((argv[1][0] == '?') || !strncmp(argv[1], "-?", 2) ||
            !strncmp(argv[1], "-he", 3) || !strcmp(argv[1], "help"))
        {
            char llsswe[] = "load last saved settings with extension";
            fprintf(
                stderr,
                "Usage: fids [-] [ -l name ] [ -e VAR ] [-s] [-g] [ dir ]\n\n");
            fprintf(stderr, "\t-        : %s=uid\n", llsswe);
            fprintf(stderr, "\t-l name  : %s=name\n", llsswe);
            fprintf(stderr, "\t-e VAR   : %s from env. $VAR\n", llsswe);
            fprintf(stderr,
                    "\t-s       : don't save the actual settings on exit\n");
            fprintf(stderr,
                    "\t-g       : don't use the graphic vt100 symbols\n");
            fprintf(stderr, "\tdir      : do 'cd dir first'\n\n");
            fprintf(stderr, "\tIf ![-s] : On exit the actual user settings are "
                            "stored with\n");
            fprintf(stderr,
                    "\t           \"$HOME/%s_'extension'\". (default "
                    "extension=uid)\n",
                    FS_FILE);
            exit(-1);
        }
    }

    if (setuid(geteuid()))
        syserr("fids.setuid");

    opt_sl = 0;
    while (argc >= 2 && argv[1][0] == '-')
    {
        switch (argv[1][1])
        {
            case 'g':
            case 'G':
                GRAF_VT100 = FALSE;
                break;

            case 's':
            case 'S':
                FS_save_flag = FALSE;
                break;

            case 'e': /* Environment variable name given */
            case 'E': /* take profile name from this */
                if (argc >= 3)
                {
                    FS_save_name = getenv(argv[2]);
                    --argc;
                    ++argv;
                }
                opt_sl = 1;
                break;

            case 'l':
            case 'L':
                if (argc >= 3)
                {
                    FS_save_name = argv[2];
                    --argc;
                    ++argv;
                }
                opt_sl = 1;
                break;

            case '\0':
                opt_sl = 1;
                break;
        }
        --argc;
        ++argv;
    }

    if (argc == 2)
    {
        /* try to take this as a directory name */
        (void)(chdir(argv[1]));
    }

    setwinsize(&TermLines, &TermCols);

    if (!(term = getenv("TERM")))
        term = Term_Names[0][0]; /* default = 1. entry */

    /* search the terminal name in the array */
    for (i = 0; i < TERM_NAME_NUM; i++)
    {
        if (!strcmp(term, Term_Names[i][0]))
        {
            TermGrix = i;
            break;
        }
    }

#if defined(_ANSI_COLOR) || defined(_ANSI_COLOR_GREY)
    {
        int TermAttrGrix = 0;

        /* search if the $TERM incl. the name from the array as substr */
        for (i = 0; i < TERM_NAME_NUM; i++)
        {
            z1 = term;
            c  = strlen(Term_Names[i][0]);
            while (*z1)
            {
                if (!strncmp(z1, Term_Names[i][0], c))
                {
                    TermAttrGrix = i;
                    break;
                }
                z1++;
            }
        }

        /* initialize the attribute string array */
        for (i = 0; i < TERM_ATTR_NUM; i++)
        {
            c = (int)Term_Attr[TermAttrGrix][i];
            if (c < 0 || c >= TERM_ATTR_STRING_NUM)
                syserr("fids.setupTermAttr");
            Term_Attr[TermGrix][i] = Term_Attr_Str[TermAttrGrix][c];
        }
    }

#else /* take the 'vt100' attributes */

    /* initialize the attribute string array */
    for (i = 0; i < TERM_ATTR_NUM; i++)
    {
        c = (int)Term_Attr[0][i];
        if (c < 0 || c >= TERM_ATTR_STRING_NUM)
            syserr("fids.setupTermAttr");
        Term_Attr[TermGrix][i] = Term_Attr_Str[0][c];
    }
#endif

    if ((z1 = getenv("PS1")) == NULL)
        z1 = "$ ";
    if ((ps1 = malloc((unsigned)strlen(z1) + strlen(FIDS_PS1) + 1)) == NULL)
        syserr("fids:malloc");
    strcpy(ps1, FIDS_PS1);
    strcat(ps1, z1);
    if (putenv(ps1))
        syserr("fids.putenv");

    setraw(0); /* set terminal into raw mode   */

#if defined(QNX4) || defined(_GNU_SOURCE)
    sigsetjmp(ljmp_env, TRUE); /* set here for the longjmp	*/
#else
    (void)(setjmp(ljmp_env)); /* set here for the longjmp	*/
#endif
    sig_catch(); /* set the signal catching	*/

    err_flag = FALSE;
    cls();
    gotoxy(1, TermLines / 2 - 4);
    printf("  F I D S   %s, Release: %s\r\n\n", VERSION, RELEASE);
    printf("\tby\r\n");
    printf("\tBernhard Noeggerath\r\n\n\n\n\n");
    printf("  Copyright reserved (1983 - 2003)\n");

    sleep(1);

    for (i = 0; i < CMDL_N; i++) /* initialize the cmd_pointers  */
        p_cmdl[i] = cmd_line[i];

    if (getcwd(p_cwd, NDIR) == NULL) /* get current work dir */
    {
        /* can't get name of current work directory ! */

        if ((pw = getpwuid(geteuid())) == NULL) /* get euid */
            syserr("getcwd");
        else
        { /* use the Home directory of euid to chdir ! */
            strncpy(p_cwd, pw->pw_dir, NDIR);
            p_cwd[NDIR - 1] = '\0';
            if (cmd_cdu(p_cwd) == ERR)
                syserr("getcwd");
            error("", ""); /* set the error flag ! */
        }
    }
    strcpy(dir_n_s, p_cwd); /* save actual dir_name */

    makescreen(); /*     make the screen new */

    /* check the terminal lines and cols, makescreen gives the message !*/
    if ((TermLines < TERM_MIN_LINES) || (TermCols < TERM_MIN_COLS))
    {
        cmd_exit();
    }

    stat_list = fidsnew('b'); /* read directory and make fids */
    escape('l');              /* search and load PF_key-file  */

    if (opt_sl == 1)
    {
        char dir_ntr[NDIR];
        int  FileLstRowsTmp, FileLstRowsStart = FileLstRows;

        FS_load(FS_save_name); /* load the last saved setting */

        /* don't do the .^@ commands from the PF_key-file ! */
        while (cready()) /* flush the cmd buffer (.^@)  */
            c = get_key();

        FileLstRowsTmp = FileLstRows; /* save the reloaded value */
        FileLstRows    = FileLstRowsStart;

        if (scrn == 2)
        { /* the loaded settings want to use 2 screens */
            /* get first the left side and then do the right side ! */
            char actlr_tmp = actlr; /* save the reloaded 'r'-values */
            char smodr_tmp = smodr;

            strcpy(dir_ntr, dir_n_l);
            actlr = 'l';
            scrn  = 1;
            cmd_cd(dir_ntr, 'b');

            strcpy(dir_ntr, dir_n_r);
            cmd_split();
            smodr = smodr_tmp;
            cmd_cd(dir_ntr, 'b');

            if (actlr_tmp == 'l')
                cmd_switch();
        }
        else
        {
            strcpy(dir_ntr, dir_n_l);
            actlr = 'l';
            scrn  = 1;
            cmd_cd(dir_ntr, 'b');
        }

        if (FileLstRows != FileLstRowsTmp)
        {
            FileLstRows = FileLstRowsTmp; /* set number of rows to loaded */
            stat_list   = fidsnew('c');   /* read directory and make fids */
        }
    }

    if (err_flag == TRUE)
        error("Can't use current directory : DIRECTORY CHANGED !!!", "");

    for (;;)
    {
        printf(CIVIS);
        fflush(stdout);

        /* check the terminal lines and cols */
        if (TermLines < TERM_MIN_LINES || TermCols < TERM_MIN_COLS)
            pause(); /* wait for a proper signal */

        if (!cready())
        {
            if (!stat_list)
                stat_list = fids_list('.', 'b');
            if (!stat_fidsnew)
                stat_list = fidsnew('.');
        }

#if defined(QNX4) || defined(_GNU_SOURCE)
        if (sigsetjmp(ljmp_env, TRUE) != 0) /* set here for the longjmp	*/
            continue;
#else
        if (setjmp(ljmp_env) != 0) /* set here for the longjmp	*/
            continue;
#endif
        signal(SIGALRM, f_clock); /* set signal handling          */
        if (!cready())            /* if no key pressed on terminal*/
        {
            if (!stat_setact)
                time_out = 1;
            else
                time_out = time_def;
            alarm(time_out);
        }

        fflush(stdout);
        c = get_key(); /* wait for next input character*/

        if (err_flag == TRUE)
        {
            err_flag  = FALSE;
            scan_mail = 'y';
            mesg_line();
        }

        if (c > ES1) /* is it a special ESC sequence */
        {
            if (c < ES2) /* escape ?      ? = not '[' && ! 'O'   */
            {
                i = c - ES1;
                if (i >= '1' && i <= '9')
                {                        /* execute pf_keys      */
                    push_s(pf[i - '1']); /* push to input buffer */
                    continue;

                } /* end c = '1..9' */

                if ((i >= 'a' && i <= 'z') || i == ESC)
                { /* ESC 'lowercase'
                   */
                    if ((c = escape(i)) == 0)
                        continue; /* 0 only, if in escape() o.k.	*/
                }
            }
            else if (c < ES3) /* escape [ A,B,C,D       cursor keys   */
            {
                stat_setact = setact(c - ES2);
                continue;
            }
            else if (c < ES4) /* PF Keys    */
            {
                i = c - ES3;
                if (i >= '1' && i <= '9')
                {                        /* execute pf_keys      */
                    push_s(pf[i - '1']); /* push to input buffer */
                    continue;

                } /* end c = '1..9' */
                else if (i >= 'a' && i <= 'i')
                {                          /* execute shift pf_keys */
                    push_s(pf_s[i - 'a']); /* push to input buffer */
                    continue;

                } /* end c = 'a..i' */
                continue;
            }
        }

        if (c > 0 && c < ' ') /* is it a <CTRL> Command ?     */
        {
            if (*pf_ctl[c]) /* is this user defined ?       */
            {
                push_s(pf_ctl[c]); /* push the str to input buffer */
                continue;
            }
        }

        switch (c) /* check cursor up/down */
        {
            case CTRL('^'): /* cursor up    */
                stat_setact = setact('A');
                continue;
            case 28 /*^\*/: /* cursor down  */
                stat_setact = setact('B');
                continue;
            default:
                break;
        }

        for (i = 0; cmd[i].cmd_char; i++)
        {
            if ((c == cmd[i].cmd_char) || (c == cmd[i].cmd_cchar))
            {
                (cmd[i].cmd_fkt)(); /* call funct.  */
                goto next;
            }
        } /* for(i) */

        if (c >= ' ' && c < DEL) /* if normal character, then    */
        {
            push_c(c);   /* push to input buffer */
            cmd_mode(0); /* goto command_mode    */
        }

    next:
        continue;

    } /* for(;;) */

    /*NOTREACHED*/

    return 0;

} /* main */

/* ------------------------------------------------------------------------- */
static int cmd_sort(void) /* command execution for sort */
{
    char smod_old;

    if (actlr == 'r')
        smod_old = smodr;
    else
        smod_old = smodl;

    save_cursor();
    gotoxy(1, TermLines - 1);
    printf(" %ssort files?%s  %sn%s%same%s, %st%s%s=[.*]%s, %ss%s%size%s, "
           "%su%s%sid%s, %sg%s%sid%s",
           A_SORT, A_NORMAL, A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL, /* name */
           A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL,                   /* type */
           A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL,                   /* size */
           A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL,                   /* uid */
           A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL);                  /* gid */
    printf(",  %sdate%s:  %sa%s%scc%s, %sm%s%sodi%s, %sc%s%shan%s  ? %c%s%s",
           A_SORT, A_NORMAL,                      /*date*/
           A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL, /* acc */
           A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL, /* modi */
           A_SORTKEY, A_NORMAL, A_SORT, A_NORMAL, /* chan */
           smod_old, DEL_EOL, CUB);
    fflush(stdout);
    restore_cursor();

    if ((sortls(numd, numf, get_key()))) /* sort files   */
    {
        mesg_line();
        return (0);
    }
    listfiles('b');
    stat_setact = FALSE;

    return (0);
}

/************************************************************************/
static int cmd_fpage(void) /* set cursor to first position */
{
    stat_setact = setact('F');
    return (0);
}

/************************************************************************/
static int cmd_lpage(void) /* set cursor to end of filelist*/
{
    stat_setact = setact('L');
    return (0);
}

/************************************************************************/
static int cmd_inam(void) /* command insert name */
{
    push_c(ES1 | CTRL('N')); /* push 'insert act name' to input buffer */
    return (cmd_mode(0));
}

/************************************************************************/
static int cmd_onam(void) /* command insert name other */
{
    push_c(ES1 | CTRL('V')); /* push 'insert oth name' to input buffer */
    return (cmd_mode(0));
}

/************************************************************************/
static int cmd_copy(void) /* command insert copy function */
{
    push_c(ES1 | CTRL('P')); /* push 'copy file' to input buffer */
    return (cmd_mode(0));
}

/************************************************************************/
static int cmd_move(void) /* command insert move function */
{
    push_c(ES1 | CTRL('M')); /* push 'move file' to input buffer */
    return (cmd_mode(0));
}

/************************************************************************/
static int cmd_retrp(void) /* command retrieve previous    */
{
    push_c(ES1 | CTRL('J')); /* push 'prev_cmd' to input buffer */
    return (cmd_mode(0));
}

/************************************************************************/
static int cmd_retrl(void) /* command retrieve last    */
{
    push_c(ES1 | CTRL('K')); /* push 'prev_last' to input buffer */
    return (cmd_mode(0));
}

/************************************************************************/
static int cmd_cr(void) /* command execution for CR     */
{
    /* is actual entry a directory then try to chdir        */

    if ((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFDIR)
    {
        return (cmd_cd(p_ls[acti]->f_name, 'b'));
    }

    return (cmd_mode(0));
}

/************************************************************************/
static int cmd_cm(void) /* go into command mode	        */
{
    return (cmd_mode(0));
}

/************************************************************************/
int   cmd_cdu(dir_name) /* command change directory up  */
char *dir_name;
{
    struct stat stdir;
    char       *p;

    while ((chdir(dir_name) == ERR) || (stat(dir_name, &stdir) == ERR) ||
           (access(".", 04) == ERR))
    {
        /* delete the lowest directory name and try again */
        if ((p = strrchr(dir_name, '/')) == NULL)
            return (-1);
        else if (p > dir_name)
            *p = '\0';
        else
            *(p + 1) = '\0'; /* we have "/" now ! */
    }

    return (0);
}

/************************************************************************/
int   cmd_cd(dir_name, what) /* command change directory     */
char *dir_name;              /* what = 'f' for find subdir   */
char  what;
{
    struct stat stdir;
    int         i;

    if (!strcmp(dir_name, ".")) /* refresh current dir  */
    {
        if (scrn == 2) /* set to read new */
        {
            p_lsr[0]->f_st.st_ctime = 0;
            p_lsl[0]->f_st.st_ctime = 0;
        }
        else
            p_ls[0]->f_st.st_ctime = 0;

        setwinsize(&TermLines, &TermCols);
        stat_list = fidsnew('c');
        return (0);
    }

    if (stat(dir_name, &stdir) == ERR)
    {
        save_cursor();
        error("can't get stat of directory: ", dir_name);
        restore_cursor();
        return (-1);
    }

    if (chdir(dir_name) == ERR)
    {
        save_cursor();
        error("can't change to directory: ", dir_name);
        restore_cursor();
        return (-1);
    }

    if (access(".", 04) == ERR)
    {
        save_cursor();
        chdir(p_cwd); /* go back */
        error("can't read directory: ", dir_name);
        restore_cursor();
        getcwd(p_cwd, NDIR); /* get cwd_nam  */
        if (cmd_cdu(p_cwd) == ERR)
            cmd_exit();
        if (what != 's')
            stat_list = fidsnew('b'); /* s=switch cwd */
        return (-1);
    }

    strcpy(dir_f_n, "..");
    if (what == 'f' && !strcmp(dir_name, "..") &&
        strlen(p_cwd) > 1)
    { /* get name of subdir */
        for (i = strlen(p_cwd) - 1; i > 0 && p_cwd[i] != '/'; i--)
            ;

        strcpy(dir_f_n, &p_cwd[i + 1]);
    }

    if (getcwd(p_cwd, NDIR) == NULL) /* get cwd_nam  */
    {
        strcpy(p_cwd, ".");
    }

    if (what != 's')
        stat_list = fidsnew('b'); /* s=switch cwd */

    return (0);
}

/************************************************************************/
static int cmd_exec(void) /* use actual file as command   */
{
    /* is actual entry a directory then try to chdir        */

    if ((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFDIR)
    {
        return (cmd_cd(p_ls[acti]->f_name, 'f'));
    }

    /* be shure to take the command from the current dir    */
    push_s("\r");            /* push string to input buffer  */
    push_c(ES1 | CTRL('N')); /* push 'insert act name' to input buffer */
    push_s("./");
    return (cmd_mode(0));
}

/************************************************************************/
int   cmd_ex_do(com) /* sh command execution         */
char *com;
{
    setblock(0, TRUE); /* set terminal to blocking mode*/
    termrest();        /* reset terminal parameters    */
    PF_label(0);       /* clear the PF_labels if extra */
    cls();
    gotoxy(1, TermLines);
    printf("%s\n", com);
    fflush(stdout); /* send it out          */

    sig_nocatch();
    cmd_ex_ret(system(com)); /* execute the command  */
    sig_catch();

    return (0);
}

/* ------------------------------------------------------------------------- */

static void continue_or_abort(void)
{
#if defined QNX4 || defined QNX6
    printf(" ==> %sHit any key to restart fids! ('%c' aborts)%s%s\n", A_MESSAGE,
           ABORT_CHAR, A_NORMAL, DEL_EOL);
#else
    printf(" ==> %sHit any key to restart fids! ('%c' aborts)%s%s", A_MESSAGE,
           ABORT_CHAR, A_NORMAL, DEL_EOL);
#endif

    fflush(stdout); /* send it out */

    if (tolower(get_key()) == tolower(ABORT_CHAR)) /* wait for any character */
        cmd_exit();
}

/* ------------------------------------------------------------------------- */
#if defined QNX4 || defined QNX6
/* ------------------------------------------------------------------------- */

/************************************************************************/
int cmd_ex_ret(rc) /* return after sh command exec */
int rc;
{
    unsigned char es = WEXITSTATUS(rc);

    if (rc == -1)
        printf(" ==> %sError: %d%s <==", A_MESSAGE, errno, A_NORMAL);
    else if (es & 0x80)
        printf(" ==> %sSignal: %d%s <==", A_MESSAGE, es & 0x7f, A_NORMAL);
    else if (es != 0)
        printf(" ==> %sExit: %d%s <==", A_MESSAGE, es, A_NORMAL);

    setraw2(0, 1); /* set terminal into raw mode, but w/ output processig */
    continue_or_abort();
    err_flag = FALSE;

    setraw(0); /* set terminal into raw mode   */
    stat_list = fidsnew('c');

    return -1;
}

/* ------------------------------------------------------------------------- */
#else

/************************************************************************/
int cmd_ex_ret(rc) /* return after sh command exec */
int rc;
{
#define lowbyte(x)  ((x) & 0377)
/*lint -emacro(702,highbyte) Shift right of signed quantity (int) */
#define highbyte(x) lowbyte((x) >> 8)

    putchar('\n'); /* BeNo, 09.05.1994 */
    setraw(0);     /* set terminal into raw mode   */
    gotoxy(1, TermLines);
    if (lowbyte(rc) == 0)
    {
        if ((rc = highbyte(rc)) != 0)
            printf(" ==> %sLast Exitcode: %d%s <== ", A_MESSAGE, rc, A_NORMAL);
    }
    else
    {
        if ((rc = rc & 0177) != 0)
            printf(" ==> %sSignal #%d%s <== ", A_MESSAGE, rc, A_NORMAL);
    }

    continue_or_abort();

    err_flag = FALSE;

    setwinsize(&TermLines, &TermCols);

    stat_list = fidsnew('c');
    return (-1);
}
/* ------------------------------------------------------------------------- */
#endif /*QNX4 || QNX6*/
/* ------------------------------------------------------------------------- */

/************************************************************************/
static int cmd_switch(void) /* switch to the other side     */
{
    if (scrn == 1)
        return (-1);

    setact('S'); /* set old position special     */
    swi_act();
    cmd_cd(p_cwd, 's'); /* change to inactiv directory  */
    stat_setact = setact('.');
    return (0);
}

/* ------------------------------------------------------------------------- */
int swi_act(void) /* switch the actual parameters */
{
    if (actlr == 'l')
    {
        actlr = 'r'; /* actual = right side          */
        p_ls  = p_lsr;
        numd  = numdr;
        numf  = numfr;
        scr   = scrr;
        acti  = actir;
        p_cwd = dir_n_r;
    }
    else
    {
        actlr = 'l'; /* actual = left side           */
        p_ls  = p_lsl;
        numd  = numdl;
        numf  = numfl;
        scr   = scrl;
        acti  = actil;
        p_cwd = dir_n_l;
    }
    return 0;
}

/************************************************************************/
static int cmd_split(void) /* split the screen to 2 dir's  */
{                          /* if 2dirs then unsplit        */
    int   i, j, l_pwd;
    char *p1, *p2;

    if (scrn == 1) /* activ = 'l' if scrn == 1     */
    {
        scrn  = 2; /* set mode to 2 screens        */
        p_lsl = p_ls;
        /* duplicate all entries        */
        p_lsr = (struct FIDS **)calloc((unsigned)(numd + numf), sizeof(p1));
        if (!p_lsr)
            syserr("fids:calloc");

        for (i = 0; i < numd + numf; i++)
        {
            if ((p_lsr[i] = (struct FIDS *)malloc(sizeof(struct FIDS))) == NULL)
                syserr("fids:malloc");

            p1 = (char *)p_lsl[i];
            p2 = (char *)p_lsr[i];

            for (j = 0; j < sizeof(struct FIDS); j++, p1++, p2++)
                *p2 = *p1;
        }

        numdr = numdl = numd;
        numfr = numfl = numf;
        scrr = scrl = scr;
        actir = actil = acti;
        smodr         = smodl;

        strcpy(dir_n_r, dir_n_l); /* copy cwd_name        */

        makehline(2, 'u'); /* make horizontal line in 2    */
        actlr = 'r';
        listfiles('b');
        actlr = 'l';
        listfiles('b');
        cmd_switch();
    }
    else
    {
        if (actlr == 'l') /* free the inactiv side */
        {
            for (i = 0; i < numdr + numfr; i++)
                free(p_lsr[i]);
            free(p_lsr);
        }
        else
        {
            for (i = 0; i < numdl + numfl; i++)
                free(p_lsl[i]);
            free(p_lsl);
            strcpy(dir_n_l, dir_n_r); /* copy cwd_name*/
            smodl = smodr;
        }

        makehline(2, 'u'); /* make horizontal line in 2    */
        scrn  = 1;         /* set mode to 1 screen         */
        actlr = 'l';
        p_cwd = dir_n_l;
        listfiles('b');
        stat_setact = setact('.');
    }
    return (0);
}

/************************************************************************/
static int cmd_pup(void) /* move screen page up          */
{
    listfiles('-');
    stat_setact = FALSE;
    return (0);
}

/************************************************************************/
static int cmd_pdn(void) /* move screen page down        */
{
    listfiles('+');
    stat_setact = FALSE;
    return (0);
}

/************************************************************************/
static int cmd_tag(void) /* tag the actual file */
{
    char m_char;

    if (!stat_setact)
        stat_setact = setact('.');

    if (p_ls[acti]->f_inode) /* if != 0 */
    {
        p_ls[acti]->f_inode = 0; /* mark the actual file */

        if ((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFDIR)
            m_char = MARK_DIR_CHAR;
        else
            m_char = MARK_CHAR;

        printf("%s%s%c%s", CUB, A_MARKCHAR, m_char, A_NORMAL);
    }
    else
    {
        p_ls[acti]->f_inode = 1; /* set mark off */

        if ((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFDIR)
            printf("%s%s%c%s", CUB, A_DIRCHAR, DIR_CHAR, A_NORMAL);
        else
            printf("%s%c", CUB, ' ');
    }

    stat_setact = setact('B'); /* next entry   */

    return 0;
}

/************************************************************************/
static int cmd_atag() /* tag all files */
{
    int ii;

    if ((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFDIR)
    {
        for (ii = 0; ii < numd; ii++)
            p_ls[ii]->f_inode = 0; /* mark the directory */
    }
    else
    {
        for (ii = numd; ii < numd + numf; ii++)
            p_ls[ii]->f_inode = 0; /* mark the file */
    }

    listfiles('.');
    stat_setact = FALSE;

    return 0;
}

/************************************************************************/
int cmd_utag() /* untag all files   */
{
    int i;

    for (i = 0; i < numd + numf; i++)
        p_ls[i]->f_inode = 1; /* unmark the file */
    listfiles('.');
    stat_setact = FALSE;
    return (0);
}

/************************************************************************/
static int cmd_attag() /* toggle tags for all files   */
{
    int  i;
    long am;

    if (p_ls[acti]->f_inode) /* actual file is not marked */
        am = 0;              /* do mark all */
    else
        am = 1; /* do unmark all */

    if ((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFDIR)
    {
        for (i = 0; i < numd; i++)
            p_ls[i]->f_inode = am;
    }
    else
    {
        for (i = numd; i < numd + numf; i++)
            p_ls[i]->f_inode = am;
    }
    listfiles('.');
    stat_setact = FALSE;
    return (0);
}

/************************************************************************/
static int cmd_del(void) /* delete the actual file       */
{
    struct stat stdir;

    if (!stat_setact)
        stat_setact = setact('.');
    save_cursor();
    gotoxy(1, TermLines - 1);

#ifdef S_IFLNK
    /* get the filestatus ( symbolic link possible ) */
    lstat(p_ls[acti]->f_name, &stdir);
#endif

    if (!p_ls[acti]->f_inode) /* if inode == 0 */
    {
        printf("Remove all marked files? [Yes=y]:%s", DEL_EOL);
        fflush(stdout);
        if (get_key() == 'y')
        {
            restore_cursor();
            push_s("rm -rf @\r"); /* push string to input buffer  */
            return (cmd_mode(0));
        }
    }
    else if (((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFDIR)
#ifdef S_IFLNK
             && ((stdir.st_mode & S_IFMT) != S_IFLNK)
#endif
    )
    {
        printf("Remove directory? %s", A_MESSAGE);
        prt_str(0, TermCols - 28, p_ls[acti]->f_name);
        printf("%s [Yes=y]:%s", A_NORMAL, DEL_EOL);
        fflush(stdout);
        if (get_key() == 'y')
        {
            restore_cursor();
            push_s("\r");            /* push string to input buffer  */
            push_c(ES1 | CTRL('N')); /* push 'ins name' to input buffer */
            push_s("rmdir ");
            return (cmd_mode(0));
        }
    }
    else
    {
        printf("Remove? %s", A_MESSAGE);
        if (strlen(p_ls[acti]->f_name) > TermCols - 18)
        {
            prt_str(0, TermCols - 21, p_ls[acti]->f_name);
            prt_str(0, 3, "...");
        }
        else
            prt_str(0, TermCols - 18, p_ls[acti]->f_name);

        printf("%s [Yes=y]:%s", A_NORMAL, DEL_EOL);

        fflush(stdout);
        if (get_key() == 'y')
        {
            if (unlink(p_ls[acti]->f_name) == ERR)
            {
                error("can't remove : ", p_ls[acti]->f_name);
                restore_cursor();
                return (-1);
            }
            restore_cursor();

#ifdef _IRSDEL
            push_s("cd .\r"); /* push string to input buffer  */
            return (cmd_mode(0));
#else
            *(p_ls[acti]->f_name) = '\0';
            prt_str(FileLstNameLen, FileLstNameLen, " ");
            stat_setact = setact('B'); /* next entry   */
            mesg_line();
            return (0);
#endif
        }
    }
    restore_cursor();
    mesg_line();
    return (-1);
}

/************************************************************************/
void sig_catch()
{
    int i;

#ifdef QNX4
#define NSIG _SIGMAX
#endif

    for (i = 1; i < NSIG; i++) /* catch all possible signals */
    {
        switch (i)
        {
            case SIGSEGV: /* should not be ignored */
            case SIGKILL: /* kill (cannot be ignored) */
#ifdef SIGSTOP
            case SIGSTOP:
#endif
                continue;

#ifdef SIGWINCH
                /* this signal tells that the window lines and rows are changed ! */
            case SIGWINCH:
                (void)signal(SIGWINCH, cmd_winch);
                break;
#endif

#ifdef SIGCHLD
            case SIGCHLD:
#else
            case SIGCLD: /* death of a child */
#endif
                (void)signal(i, SIG_DFL);
                break;

            default:
                (void)signal(i, cmd_sig);
                break;
        }
    }
}

/************************************************************************/
void sig_nocatch()
{
    int i;

    for (i = 1; i < NSIG; i++)
    {
        switch (i)
        {
                /* these signals are still catched for exit ok. */
            case SIGHUP:  /* hangup */
            case SIGINT:  /* interrupt (rubout) */
            case SIGQUIT: /* quit (ASCII FS) */
            case SIGABRT: /* used by abort */
            case SIGTERM: /* software termination signal */
            case SIGKILL: /* kill (cannot be ignored) */

#ifdef SIGCHLD
            case SIGCHLD:
#else
            case SIGCLD: /* death of a child */
#endif
#ifdef SIGSTOP
            case SIGSTOP:
#endif
                break;

            default:
                (void)signal(i, SIG_DFL);
                break;
        }
    }
}

/************************************************************************/
static void cmd_sig(sig_num) /* we got an interrupt signal   */
    int sig_num;
{
    alarm(0); /* clear the time_out	*/
    fflush(stdout);
    cls();
    gotoxy(1, TermLines);
    fflush(stdout);

    switch (sig_num)
    {
        case SIGINT: /* interrupt (rubout) */

            if (FidsCloseOnInterrupt != -1)
            {
                close(FidsCloseOnInterrupt);
                FidsCloseOnInterrupt = -1;
            }
            if (FidsF1closeOnInterrupt != NULL)
            {
                fclose(FidsF1closeOnInterrupt);
                FidsF1closeOnInterrupt = NULL;
            }
            if (FidsF2closeOnInterrupt != NULL)
            {
                fclose(FidsF2closeOnInterrupt);
                FidsF2closeOnInterrupt = NULL;
            }
            if (FidsPcloseOnInterrupt != NULL)
            {
                pclose(FidsPcloseOnInterrupt);
                FidsPcloseOnInterrupt = NULL;
            }
            printf(" ==> %sInterrupted%s <== ", A_MESSAGE, A_NORMAL);

            continue_or_abort();

            err_flag = FALSE;

            (void)(signal(sig_num, cmd_sig)); /* set it again */
            stat_list = fidsnew('c');
            setraw(0); /* set terminal into raw mode   */
#if defined(QNX4) || defined(_GNU_SOURCE)
            siglongjmp(ljmp_env, -1);
#else
            longjmp(ljmp_env, -1);
#endif
            break;

        default:
            fprintf(stderr, " ==> %sSignal #%d%s <== : Tschuess!", A_FAILURE,
                    sig_num, A_NORMAL);
            cmd_exit();
    }
    return;
}

/************************************************************************/
static void cmd_winch(sig_num) /* we got a SIGWINCH signal	*/
    int sig_num;
{
    cmd_cd(".", 'c'); /* refresh the screen */

    if ((signal(sig_num, cmd_winch)) == SIG_ERR) /* set it again */
        syserr("signal");

    /* check the terminal lines and cols */
    if ((TermLines >= TERM_MIN_LINES) && (TermCols >= TERM_MIN_COLS))
    {
        stat_setact = FALSE;
        f_clock(sig_num);
    }
    return;
}

/************************************************************************/
int cmd_askexit(void) /* ask the user if he want's to exit */
{
    cls();
    gotoxy(1, TermLines);

    continue_or_abort();

    err_flag  = FALSE;
    stat_list = fidsnew('c');
    return (-1);
}

/* ------------------------------------------------------------------------- */
int cmd_exit(void) /* command execution for exit */
{
    setblock(0, TRUE); /* set terminal to blocking mode */
    termrest();
    PF_label(0);
    if (FS_save_flag)
        FS_save(FS_save_name);
    gotoxy(1, TermLines);
    printf("\n");
    exit(0);

    /*NOTREACHED*/

    return 0;
}

/************************************************************************/
extern char SymLinkName[];

void mesg_line(void) /* print the mesg_line : bot -1 */
{
    save_cursor();
    gotoxy(1, TermLines - 1);

    makehline(TermLines - 1, 'l');
    err_active = FALSE;

    if (isprint(*SymLinkName))
    {
        printSymLinkName();
    }
    else
    {
        gotoxy(FileLstNameLen * 3 + 7, TermLines - 1);
        printfspec();
    }

    restore_cursor();
    fflush(stdout);
}

/************************************************************************/
static int cmd_helpp(void) /* print help pages             */
{
    help_page('d');
    return (0);
}
/************************************************************************/
#include "fids.hlp"
void help_page(mode) /* print help pages             */
    char mode;
{
    int i, page;
    if (TermLines < 24 || TermCols < (84 - FileLstRows))
        return;

    if (mode == 'c')
        page = FIDS_HELPPAGE_START_CM;
    else
        page = FIDS_HELPPAGE_START_DM;

    while (page < FIDS_HELPPAGE_NUM)
    {
        for (i = 3; i <= (TermLines - 2); i++) /* clear list_window    */
        {
            gotoxy(TermCols - (FrightWinCols + 2), i);
            puts(DEL_BOL);
        }
        makevline(1);      /* make vertical   line in 1    */
        makehline(2, 'u'); /* reset line in 2      */

        /* print headline in line 2 */
        gotoxy(2, 2);
        printf("%s%s%s", A_MESSAGE, fids_hlpp[page][0], A_NORMAL);

        for (i = 1; i <= FIDS_HELPPAGE_LINES; i++)
        {
            gotoxy(2, 2 + i);
            printf("%s", fids_hlpp[page][i]);
        }

        fflush(stdout);    /* send it out          */
        switch (get_key()) /* wait for any character       */
        {
            case 'Q':
            case 'q':
            case CR:
            case CTRL('C'):
            case ESC1('Q'):
            case ES1 | ESC:
                page = -1;
                break;
            default:
                break;
        }
        if (page < 0)
            break;

        page++;
        if (page >= FIDS_HELPPAGE_NUM)
            page = 0;
    }

    err_flag = FALSE;

    setwinsize(&TermLines, &TermCols);

    makehline(2, 'u'); /* reset line in 2      */
    stat_list = fids_list('?', 'b');
    return;
}

/************************************************************************/
void  error(msg1, msg2) char *msg1; /* cut msg1 to max. 64 char.            */
char *msg2;                         /* cut msg2 if it is to large           */
{
    gotoxy(1, TermLines - 1);
    printf("%s%.64s%c", A_FAILURE, msg1, BEL);
    prt_str(0, TermCols - 4 - strlen(msg1), msg2);
    if ((strlen(msg1) + strlen(msg2)) > (TermCols - 4))
        prt_str(0, 3, "...");
    printf("%s%s", A_NORMAL, DEL_EOL);
    fflush(stdout);
    err_flag   = TRUE;
    err_active = TRUE;
}

/************************************************************************/
static int cmd_rel(void) /* print copyright and release  */
{
    int i;
    if (TermLines < 22 || TermCols < 78)
        return (0);

    for (i = 3; i <= (TermLines - 2); i++) /* clear list_window    */
    {
        gotoxy(TermCols - (FrightWinCols + 2), i);
        puts(DEL_BOL);
    }
    makevline(1); /* make vertical   line in 1    */
    gotoxy(1, TermLines / 2 - 8);
    printf("\t%sF%sullscreen %sI%snteractive %sD%sirectory %sS%shell\r\n",
           A_TITLE, A_NORMAL, A_TITLE, A_NORMAL, A_TITLE, A_NORMAL, A_TITLE,
           A_NORMAL);
    printf("\t======================================\r\n");
    printf("\tVersion: %s, Release: %s,    date: %s\r\n\n", VERSION, RELEASE,
           GEN_DAT);
#define GEN_SYS_UID_MAX_NUM 40
    if (strlen(GEN_SYS) > GEN_SYS_UID_MAX_NUM)
    {
        for (i = GEN_SYS_UID_MAX_NUM; i >= 0 && GEN_SYS[i] != ' '; i--)
            ;

        if (i == 0)
            i = GEN_SYS_UID_MAX_NUM;
        GEN_SYS[i] = '\0';
    }
    if (strlen(GEN_UID) > GEN_SYS_UID_MAX_NUM)
    {
        for (i = GEN_SYS_UID_MAX_NUM; i >= 0 && GEN_UID[i] != ' '; i--)
            ;

        if (i == 0)
            i = GEN_SYS_UID_MAX_NUM;
        GEN_UID[i] = '\0';
    }
    printf("\tCompiled on : %s\r\n", GEN_SYS);
    printf("\t\t by : %s\r\n\n\n", GEN_UID);
    printf("\tCopyright : Bernhard Noeggerath (1983-2005)\r\n\n");
    printf("\tBeNoSoft Ltd.\r\n");
    printf("\tDepartment: S\r\n\n");
    get_key();
    stat_list = fids_list('?', 'b');
    return (0);
}

/************************************************************************/
BOOLEAN fidsnew(what) /* read directory and make fids */
char    what;
{
    int i;

    struct stat stdir;
    static char list_side, list_pwd;

    switch (what)
    {
        case 'c':
            cls(); /* read new and make screen */
            makescreen();
            /* check the terminal lines and cols */
            if (TermLines < TERM_MIN_LINES || TermCols < TERM_MIN_COLS)
            {
                return (TRUE);
            }
            PF_label(1);
            list_side = actlr;

        case 'n': /* read new if dir changed */
            if (what == 'n')
                list_side = ' ';
            list_pwd = what;
            switch (scrn)
            {
                case 2:
                    if (what == 'c')
                        list_side = 'b';
                    swi_act(); /* jmp to not activ side */
                    if (chdir(p_cwd) == ERR)
                    {
                        error("can't change to directory: ", p_cwd);
                        if (cmd_cdu(p_cwd) == ERR)
                            cmd_exit();
                        p_ls[0]->f_st.st_ctime = 0;
                        list_pwd               = 'b';
                    }
                    if (stat(p_cwd, &stdir) == ERR)
                    {
                        error("can't get status of directory: ", p_cwd);
                        if (cmd_cdu(p_cwd) == ERR)
                            cmd_exit();
                        p_ls[0]->f_st.st_ctime = 0;
                        list_pwd               = 'b';
                    }
                    if (stdir.st_ctime > p_ls[0]->f_st.st_ctime)
                    {
                        fids_readdir(p_cwd);
                        sortls(0, numd, 'd');    /* sort subdirs */
                        sortls(numd, numf, '.'); /* sort files   */
                        if (what == 'n')
                            list_side = actlr;
                    }
                    swi_act(); /* switch back           */
                    if (chdir(p_cwd) == ERR)
                    {
                        error("can't change to directory: ", p_cwd);
                        if (cmd_cdu(p_cwd) == ERR)
                            cmd_exit();
                        p_ls[0]->f_st.st_ctime = 0;
                        list_pwd               = 'b';
                    }

                case 1:
                    if (stat(p_cwd, &stdir) == ERR)
                    {
                        error("can't get status of directory: ", p_cwd);
                        if (cmd_cdu(p_cwd) == ERR)
                            cmd_exit();
                        p_ls[0]->f_st.st_ctime = 0;
                        list_pwd               = 'b';
                    }
                    if (stdir.st_ctime > p_ls[0]->f_st.st_ctime)
                    {
                        fids_readdir(p_cwd);
                        sortls(0, numd, 'd');    /* sort subdirs */
                        sortls(numd, numf, '.'); /* sort files   */
                        if (what == 'n')
                        {
                            if (list_side == ' ')
                                list_side = actlr;
                            else
                                list_side = 'b';
                        }
                    }

                default:
                    if (list_side == ' ')
                        return (TRUE);

            } /* switch(scrn) */
            break;

        case 'b': /* read actual dir new  */
            if (actlr == 'r')
            {
                actir = acti;
                scrr  = scr;
            }
            else
            {
                actil = acti;
                scrl  = scr;
            }

            if (stat(p_cwd, &stdir) == ERR)
            {
                error("can't get status of directory: ", p_cwd);
                if (cmd_cdu(p_cwd) == ERR)
                    cmd_exit();
                p_ls[0]->f_st.st_ctime = 0;
            }
            fids_readdir(p_cwd);
            sortls(0, numd, 'd');    /* sort subdirs */
            sortls(numd, numf, '.'); /* sort files   */
            list_side = actlr;
            list_pwd  = what;
            scr       = 0;
            acti      = 1;
            for (i = 0; i < numd && i < (TermLines - FLINES) * FileLstRows / scrn;
                 i++)
            { /* search the dirname */
                if (!strcmp(p_ls[i]->f_name, dir_f_n))
                {
                    acti = i;
                    break;
                }
            }
            break;

        default:
            break;

    } /* switch(what) */

    stat_fidsnew = TRUE;
    return (fids_list(list_side, list_pwd));
}

/************************************************************************/
BOOLEAN fids_list(iside, iwhat) /* iside: l,r,b,'.',?,          */
char    iside, iwhat;           /* '.' do the interrupted list  */
{
    static char side, what;
    int         c;

    switch (iside)
    {
        case '?': /* calculate the side(s) to list */
            if (scrn == 2)
                side = 'b';
            else
                side = 'l';
            what = iwhat;
            break;
        case 'l': /* list only the left side      */
        case 'r': /* list only the right side     */
        case 'b': /* list both sides of screen    */
            side = iside;
            what = iwhat;
            break;
        case '.':
            if (side != 'l' && side != 'r' && side != 'b')
                error("fids.c: fids_list: old side not l,r,b", "");
            break;
        default:
            break;
    }

    if (cready()) /* is a character available?    */
    {
        c = get_key();
        push_s(gkb);        /* push it back again !!!       */
        switch (c)          /* if the key is one of these   */
        {                   /* then leave here already !!!  */
            case ESC1('Q'): /*   "exit"     */
            case ESC1('J'): /*   retrieve   */
            case ESC1('V'): /*   browse     */
            case ESC1('X'): /*   execute    */
            case ESC1('/'): /*   "<CR>"     */
                return (FALSE);
            default:
                if ((c > ES3 && c < ES4) /* PF's */
                    || (c > ' ' && c < DEL && c != '&'))
                    return (FALSE); /* Split*/
        }
    }

    stat_setact = FALSE;

    if (side == actlr) /* list only activ side */
    {
        listfiles(what); /* list activ side      */
        return (TRUE);
    }
    swi_act();
    listfiles(what); /* list inactiv side    */
    setact('S');     /* set inactiv side old position */
    swi_act();
    if (side == 'b')
        listfiles(what); /* list activ side      */
    return (TRUE);
}
