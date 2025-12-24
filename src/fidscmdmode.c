/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* function: cmd_mode							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/* edit the command_line                                                */
/************************************************************************/

#include "fids.h"
#include "fidsext.h"
#include <signal.h>

static int cmd_m_Mnam(void);
static int cmd_m_bol(void);
static int cmd_m_cbol(void);
static int cmd_m_ceol(void);
static int cmd_m_cle(void);
static int cmd_m_cpy(void);
static int cmd_m_cri(void);
static int cmd_m_dcu(void);
static int cmd_m_del(void);
static int cmd_m_ebl(void);
static int cmd_m_eol(void);
static int cmd_m_erase(void);
static int cmd_m_exit(int rc);
static int cmd_m_hp(void);
static int cmd_m_iM(void);
static int cmd_m_ild(void);
static int cmd_m_ins(void);
static int cmd_m_insmod(void);
static int cmd_m_ird(void);
static int cmd_m_iwd(void);
static int cmd_m_lcmd(void);
static int cmd_m_lnam(void);
static int cmd_m_mov(void);
static int cmd_m_nam(void);
static int cmd_m_nwd(void);
static int cmd_m_onam(void);
static int cmd_m_pcmd(void);
static int cmd_m_rc0(void);
static int cmd_m_sht(int what);
static int cmd_m_wle(void);
static int cmd_m_wri(void);
static int cmd_m_x(void);

static int cmd_chd(void);
static int cmd_comp(void);
static int cmd_excl(void);
static int cmd_incl(void);
static int cmd_mail(void);
static int cmd_mark(void);
static int cmd_pipe(void);
static int cmd_rows(void);
static int cmd_set(void);
static int cmd_setal(void);

extern FILE *FidsF1closeOnInterrupt;
extern FILE *FidsF2closeOnInterrupt;
extern FILE *FidsPcloseOnInterrupt;

extern BOOLEAN stat_list;
extern BOOLEAN stat_setact;
extern BOOLEAN insert_mode; /* insert/overstrike toggle */

extern char     pf_cctl[PFS][PFK_C]; /* field for the pf_ctl cmds */
extern char    *p_cmdl[CMDL_N];      /* pointer to commands in table */
extern char     dir_n_l[NDIR];       /* buffer for dir_name_left     */
extern char     dir_n_r[NDIR];       /* buffer for dir_name_right    */
extern unsigned time_def;

extern BOOLEAN FS_save_flag; /* save actual settings on exit	*/
extern char    filespec[];   /* filespec for fidsreaddir     */
extern char    exclspec[];   /* exclude filespec for fidsreaddir */

static char   *p; /* pointer to actual cmd_line   */
static int     max_c;
static int     lf_index;
static BOOLEAN quit;

static char SUBST_ENV_CHAR = '$'; /* character for subst. env variable */
static char SUBST_CMD_CHAR = '%'; /* character for subst. command      */

/************************************************************************/

#define EXEC_SH  "exec "
#define ECHO_SH  "echo "
#define STDINOFF " <&-"
#define STDERR21 " 2>&1"

#define PRET_LEN 256

static int in_cmd;
static struct
{
    char *cmd_name;
    int (*cmd_fkt)(void);
    int  cmd_opt;   /* 0: pipe is not used if options given */
    int  stdin_off; /* 0: stdin is switched off (cmd_pipe)  */
    char how_ret;   /* '.' make no new f_listing (cmd_pipe) */
} cmd_table[] = {
    {"exit ",       cmd_exit,  1, 1, '.' },
    {"fids_help ",  cmd_m_hp,  1, 1, '.' },
    {"_help ",      cmd_m_hp,  1, 1, '.' },
    {"fids_set ",   cmd_set,   1, 0, 'c' },
    {"_set ",       cmd_set,   1, 0, 'c' },
    {"fids_cmp ",   cmd_comp,  1, 1, '.' },
    {"_cmp ",       cmd_comp,  1, 1, '.' },
    {"fids_tag ",   cmd_mark,  1, 0, '.' },
    {"_tag ",       cmd_mark,  1, 0, '.' },
    {"fids_rows ",  cmd_rows,  1, 0, 'c' },
    {"_rows ",      cmd_rows,  1, 0, 'c' },
    {"fids_cols ",  cmd_rows,  1, 0, 'c' },
    {"_cols ",      cmd_rows,  1, 0, 'c' },
    {"fids_incl ",  cmd_incl,  1, 0, 'b' },
    {"_incl ",      cmd_incl,  1, 0, 'b' },
    {"fids_excl ",  cmd_excl,  1, 0, 'b' },
    {"_excl ",      cmd_excl,  1, 0, 'b' },
    {"fids_alarm ", cmd_setal, 1, 1, '.' },
    {"_alarm ",     cmd_setal, 1, 1, '.' },
    {"cd ",         cmd_chd,   1, 1, '.' },
    {"mail ",       cmd_mail,  0, 1, '.' },
    {"cp ",         cmd_pipe,  1, 0, 'n' },
    {"ln ",         cmd_pipe,  1, 0, 'n' },
    {"mv ",         cmd_pipe,  1, 0, 'n' },
    {"rm ",         cmd_pipe,  0, 0, 'n' },
    {"rmdir ",      cmd_pipe,  1, 0, 'n' },
    {"mkdir ",      cmd_pipe,  1, 0, 'n' },
    {"chown ",      cmd_pipe,  1, 0, '.' },
    {"chgrp ",      cmd_pipe,  1, 0, '.' },
    {"chmod ",      cmd_pipe,  1, 0, '.' },
    {"pwd ",        cmd_pipe,  1, 1, '.' },
    {"id ",         cmd_pipe,  1, 1, '.' },
    {"date ",       cmd_pipe,  0, 1, '.' },
    {"mesg ",       cmd_pipe,  1, 1, '.' },
    {"uname ",      cmd_pipe,  1, 1, '.' },
    {"umask ",      cmd_pipe,  1, 1, '.' },
    {"touch ",      cmd_pipe,  1, 1, 'n' },
    {"",            NULL,      0, 0, '\0'}
};

/* command list ONE Character commands  */
static struct
{
    int cmd_char;
    int cmd_cchar; /* Default CTRL() or fixed bindings */
    int (*cmd_fkt)();
} cmd_mod[] = {

    /* ECS Key bindings */
    {ESC1('Q'),       CTRL('C'), cmd_m_rc0   },
    {ESC | ES1,       0,         cmd_m_rc0   },
    {ESC1('J'),       CTRL('J'), cmd_m_pcmd  },
    {ESC1('P'),       0,         cmd_m_pcmd  },
    {ESC1('K'),       CTRL('K'), cmd_m_lcmd  },
    {ESC1('N'),       0,         cmd_m_lcmd  },
    {ESC1('h'),       ESC1('?'), cmd_m_hp    },
    {ESC1('R'),       CTRL('F'), cmd_m_dcu   },
    {DEL,             CTRL('_'), cmd_m_del   },
    {ES1 | CTRL('H'), CTRL('H'), cmd_m_del   },
    {ES1 | CTRL('G'), CTRL('G'), cmd_m_Mnam  },
    {ES1 | CTRL('J'), CR,        cmd_m_x     },
    {ESC1('#'),       0,         cmd_m_insmod},
    {ES1 | CTRL('N'), CTRL('N'), cmd_m_nam   },
    {ES1 | CTRL('V'), CTRL('V'), cmd_m_onam  },
    {ES1 | CTRL('Y'), CTRL('Y'), cmd_m_lnam  },
    {ES1 | CTRL('L'), CTRL('L'), cmd_m_ild   },
    {ES1 | CTRL('R'), CTRL('R'), cmd_m_ird   },
    {ES1 | CTRL('T'), CTRL('I'), cmd_m_ebl   },
    {ES1 | CTRL('W'), CTRL('W'), cmd_m_iwd   },
    {ES1 | CTRL('U'), CTRL('U'), cmd_m_nwd   },
    {ES1 | CTRL('P'), CTRL('P'), cmd_m_cpy   },
    {ES1 | CTRL('M'), CTRL('O'), cmd_m_mov   },
    {ES1 | CTRL('F'), CURK('C'), cmd_m_cri   },
    {ES1 | CTRL('B'), CURK('D'), cmd_m_cle   },
    {ESC1('<'),       CTRL('B'), cmd_m_bol   },
    {ESC1('>'),       CTRL('E'), cmd_m_eol   },
    {ESC1('+'),       CTRL('T'), cmd_m_wri   },
    {ESC1('-'),       CTRL('Q'), cmd_m_wle   },
    {ES1 | CTRL('O'), CTRL('^'), cmd_m_ins   },
    {ES1 | CTRL('D'), 28 /*^\*/, cmd_m_dcu   },
    {ES1 | CTRL('A'), CTRL('A'), cmd_m_ceol  },
    {ES1 | CTRL('X'), CTRL('X'), cmd_m_cbol  },
    {ES1 | CTRL('Z'), CTRL('Z'), cmd_m_erase },

    {PFKY('1'),       0,         cmd_m_bol   },
    {PFKY('2'),       0,         cmd_m_eol   },
    {PFKY('3'),       0,         cmd_m_rc0   },
    {PFKY('4'),       0,         cmd_m_ild   },
    {PFKY('5'),       0,         cmd_m_nam   },
    {PFKY('6'),       0,         cmd_m_ird   },
    {PFKY('7'),       0,         cmd_m_del   },
    {PFKY('8'),       0,         cmd_m_ceol  },
    {PFKY('9'),       0,         cmd_m_erase },

    {0,               0,         NULL        }
}; /* list end     */

/************************************************************************/
static void cmdl_display(char scp) /* draw visible part of cmd_line */
{
    static int dis_c = 0; /* startindex of display of cmd_line */
    int        dlen, dnum, dadd = 0;

    dlen = p - p_cmdl[0];
    dnum = dlen - dis_c;

    if (dlen > 0 && dnum <= 0)
    {           /* position left and border left is on */
        dlen--; /* shift one more left */
        dadd++;
        scp = 'f';
    }
    else if (dlen < max_c &&
             dnum >=
                 (TermCols - 1))
    {           /* position right and border right is on */
        dlen++; /* shift one more right */
        dnum++;
        dadd--;
        scp = 'f';
    }

    if (dnum < 0 || dnum > (TermCols - 1) ||
        scp == 'f')
    { /* shift display of cmd_line */
        dis_c = dlen;
        if (dnum > (TermCols - 1))
            dis_c -= (TermCols - 1);
        dnum = dlen - dis_c;
        gotoxy(1, TermLines);
        prt_str(0, TermCols - 1, &p_cmdl[0][dis_c]);
        printf(DEL_EOL);
        scp = 'b'; /* force drawing borders */
    }
    else if (scp == 'p')
    {
        prt_str(0, TermCols - 1 - dnum, p);
        printf(DEL_EOL);
        scp = 'b'; /* force drawing borders */
    }

    if (scp == 'b')
    {
        gotoxy(2, TermLines - 1);
        printf("%s%s%s", A_CMDLMODE, insert_mode ? "[ insert ]" : "[exchange]",
               A_NORMAL);

        if (dis_c > 0)
        {
            gotoxy(1, TermLines);
            printf("%s<%s", A_CMDLBORDER, A_NORMAL);
        }
        if ((max_c - dis_c) > (TermCols - 1))
        {
            gotoxy(TermCols, TermLines);
            printf("%s>%s", A_CMDLBORDER, A_NORMAL);
        }
    }
    gotoxy(dnum + dadd + 1, TermLines);
    return;
}

/************************************************************************/
static int insert_c(char c) /* insert a char in cmd_line */
{
    int i;
    if (max_c < CMDL_C - 1)
    {
        for (i = max_c + 1; &(p_cmdl[0][i]) > p; i--)
        {
            p_cmdl[0][i] = p_cmdl[0][i - 1];
        }
        *p = c;
        max_c++;
        return (0);
    }
    return (-1);
}

/************************************************************************/
static int ins_nam(char *p_c, char cbe) /* insert string in cmdline */
{
    char *p_old, *p_new, cde = '\0';
    if (cbe)
    { /* check if name delimiter should be inserted */
        cde = fndelim(p_c);
        if (cbe == 'l')
        { /* loop is wanted */
            cbe = cde;
            cde = ' ';
        }
        else
        {
            cbe = cde;
            cde = '\0';
        }
    }

    p_old = p;
    if (p > p_cmdl[0])
    {
        if (isalnum(*(p - 1)) || *(p - 1) == '_' || *(p - 1) == cbe)
        {
            if (!insert_c(' '))
                p++;
        }
    }
    if (cbe)
    {
        if (!insert_c(cbe))
            p++;
    }
    for (; *p_c; p_c++)
    {
        if (!insert_c(*p_c))
        {
            p++;
        }
    }
    if (cbe)
    {
        if (!insert_c(cbe))
        {
            p++;
            if (cde)
            {
                if (!insert_c(cde))
                    p++;
            }
        }
    }
    p_new = p;
    p     = p_old;
    cmdl_display('p');
    p = p_new;
    cmdl_display('.');
    return (0);
}

/************************************************************************/
static int cmd_m_cri(void) /* move cursor right in cmd_line */
{
    if (*p)
    {
        p++;
        printf(CUF);
    }
    cmdl_display('r');
    return (0);
}

/************************************************************************/
static int cmd_m_wri(void) /* move cursor word right in cmd_line */
{
    while (*p && *p != ' ')
    {
        p++;
        printf(CUF);
    }
    while (*p && *p == ' ')
    {
        p++;
        printf(CUF);
    }
    cmdl_display('r');
    return (0);
}

/************************************************************************/
static int cmd_m_cle(void) /* move cursor left in cmd_line */
{
    if (p > p_cmdl[0])
    {
        p--;
        printf(CUB);
    }
    cmdl_display('l');
    return (0);
}

/************************************************************************/
static int cmd_m_wle(void) /* move cursor word left in cmd_line */
{
    if (p > p_cmdl[0] && *p != ' ')
    {
        p--;
        printf(CUB);
    }
    while (p > p_cmdl[0] && *p == ' ')
    {
        p--;
        printf(CUB);
    }
    while (p > p_cmdl[0] && *p != ' ')
    {
        p--;
        printf(CUB);
    }
    if (p > p_cmdl[0])
    {
        p++;
        printf(CUF);
    }
    cmdl_display('l');
    return (0);
}

/************************************************************************/
static int cmd_m_ebl(void) /* set cursor to end/begin line */
{
    if (p == p_cmdl[0] + max_c)
        cmd_m_bol();
    else
        cmd_m_eol();
    return (0);
}

/************************************************************************/
static int cmd_m_bol(void) /* set cursor to  begin of line */
{
    p = p_cmdl[0]; /* set pointer to begin of line */
    cmdl_display('l');
    return (0);
}

/************************************************************************/
static int cmd_m_eol(void) /* set cursor to end    of line */
{
    p = p_cmdl[0] + max_c; /* set pointer to end of line   */
    cmdl_display('r');
    return (0);
}

/************************************************************************/
static int cmd_m_dcu(void) /* delete the actual position   */
{
    if (*p)
    {
        max_c--;
        strcpy(p, p + 1);
        cmdl_display('p');
    }
    return (0);
}

/************************************************************************/
static int cmd_m_del(void) /* delete one character left    */
{
    if (p > p_cmdl[0])
    {
        p--;
        max_c--;
        strcpy(p, p + 1);
        printf(CUB);
        cmdl_display('p');
    }
    return (0);
}

/************************************************************************/
static int cmd_m_ceol(void) /* delete to end of line        */
{
    max_c = p - p_cmdl[0];
    *p    = '\0';
    cmdl_display('p');
    return (0);
}

/************************************************************************/
static int cmd_m_cbol(void) /* delete to begin of line      */
{
    if (p > p_cmdl[0])
    {
        strcpy(p_cmdl[0], p);
        max_c -= p - p_cmdl[0];
        p = p_cmdl[0];
        cmdl_display('f'); /* force printout of cmd_line	*/
    }
    return (0);
}

/************************************************************************/
static int cmd_m_erase(void) /* delete the command line      */
{
    max_c = 0;         /* number of characters         */
    p     = p_cmdl[0]; /* pointer to actual cmdline    */
    *p    = 0;         /* reset first entry            */
    cmdl_display('f'); /* force printout of cmd_line	*/
    return (0);
}

/************************************************************************/
static int cmd_m_ins(void) /* insert a blank in cmd_line   */
{
    if (*p) /* insert only if not on right end of input line*/
    {
        if (!insert_c(' '))
        {
            cmdl_display('p');
        }
    }
    return (0);
}

/************************************************************************/
static int cmd_m_insmod(void) /* toggle insert/overstrike     */
{
    insert_mode = !insert_mode;
    cmdl_display('p'); /* toggle the mode indication	*/
    return (0);
}

/************************************************************************/
static int cmd_m_nam(void) /* get actual file_name(s) in cmdline */
{
    int i;

    if (!p_ls[acti]->f_inode) /* if inode == 0 */
    {
        for (i = 0; i < numd + numf; i++)
        {
            if (!p_ls[i]->f_inode)
                ins_nam(p_ls[i]->f_name, 'l');
        }
        return (0);
    }
    ins_nam(p_ls[acti]->f_name, '1');
    return (0);
}

/************************************************************************/
static int cmd_m_onam(void) /* get other file_name in cmdline */
{
    if (scrn > 1)
    {
        if (actlr == 'l')
        {
#ifdef INSERT_OTHER_FILENAME_ALL_MARKED
            int i;
            if (!p_lsr[actir]->f_inode) /* if inode == 0 */
            {
                for (i = 0; i < numdr + numfr; i++)
                {
                    if (!p_lsr[i]->f_inode)
                        ins_nam(p_lsr[i]->f_name, 'l');
                }
                return (0);
            }
#endif
            ins_nam(p_lsr[actir]->f_name, '1');
        }
        else
        {
#ifdef INSERT_OTHER_FILENAME_ALL_MARKED
            int i;
            if (!p_lsl[actil]->f_inode) /* if inode == 0 */
            {
                for (i = 0; i < numdl + numfl; i++)
                {
                    if (!p_lsl[i]->f_inode)
                        ins_nam(p_lsl[i]->f_name, 'l');
                }
                return (0);
            }
#endif
            ins_nam(p_lsl[actil]->f_name, '1');
        }
    }
    return (0);
}

/************************************************************************/
extern char SymLinkName[];
static int  cmd_m_lnam(void) /* get symbolic link name in cmdline */
{
#ifdef S_IFLNK
    ins_nam(SymLinkName, '1');
#endif
    return (0);
}

/*****************************************************************************/
static int cmd_m_Mnam(void) /* get actual file_name or '@' in cmdline */
{
    if (!p_ls[acti]->f_inode) /* if inode == 0 */
    {
        cmd_m_iM();
        return (0);
    }
    ins_nam(p_ls[acti]->f_name, '1');

    return (0);
}

/************************************************************************/
static int cmd_m_iM(void) /* get the MARK character in cmdl. */
{
    static char mark_str[3] = {MARK_CHAR, /* ' ', */ '\0'};
    ins_nam(mark_str, '\0');
    return (0);
}

/************************************************************************/
static int cmd_m_iwd(void) /* get directory active in cmdline */
{
    if (actlr == 'r')
        return (cmd_m_ird());
    else
        return (cmd_m_ild());
}

/************************************************************************/
static int cmd_m_nwd(void) /* get directory not active side   */
{
    if (actlr == 'l')
        return (cmd_m_ird());
    else
        return (cmd_m_ild());
}

/************************************************************************/
static int cmd_m_ild(void) /* get directory left in   cmdline */
{
    return (ins_nam(dir_n_l, '1'));
}

/************************************************************************/
static int cmd_m_ird(void) /* get directory right in  cmdline */
{
    if (scrn == 1)
        return (-1);
    return (ins_nam(dir_n_r, '1'));
}

/************************************************************************/
static int cmd_m_cpy(void) /* copy actual file to other side  */
{
    cmd_m_erase();        /* erase the cmd_line           */
    ins_nam("cp ", '\0'); /* insert the command           */
    if (!p_ls[acti]->f_inode)
        ins_nam("@ ", '\0'); /* if marked */
    else
        ins_nam(p_ls[acti]->f_name, '1');
    return (cmd_m_nwd()); /* insert the not working dir   */
}

/************************************************************************/
static int cmd_m_mov(void) /* move actual file to other side  */
{
    cmd_m_erase();        /* erase the cmd_line           */
    ins_nam("mv ", '\0'); /* insert the command           */
    if (!p_ls[acti]->f_inode)
        ins_nam("@ ", '\0'); /* if marked */
    else
        ins_nam(p_ls[acti]->f_name, '1');
    return (cmd_m_nwd()); /* insert the not working dir   */
}

/************************************************************************/
static int cmd_m_pcmd() /* get prev. command in actual cmd_line  */
{
    lf_index++;
    if (lf_index == CMDL_N || *p_cmdl[lf_index] == 0)
        lf_index = 1; /* wrap around  */

    if (*p_cmdl[lf_index] == 0)
    {
        lf_index = 0; /* no saved cmd */
        return (0);
    }
    p = p_cmdl[0];
    strcpy(p, p_cmdl[lf_index]); /* store to actual line */
    for (max_c = 0; *p; max_c++, p++)
        ;

    p = p_cmdl[0];
    cmdl_display('f'); /* force printout of cmd_line	*/
    cmd_m_eol();       /* move cursor to end of line	*/
    return (0);
}

/************************************************************************/
static int cmd_m_lcmd() /* get last command in actual cmd_line  */
{
    lf_index--;
    if (lf_index < 1)
    {
        lf_index = CMDL_N - 1; /* wrap around  */
                               /* search last one */
        while (lf_index > 1 && *p_cmdl[lf_index] == 0)
            lf_index--;
    }
    if (*p_cmdl[lf_index] == 0)
    {
        lf_index = 0; /* no saved cmd */
        return (0);
    }
    p = p_cmdl[0];
    strcpy(p, p_cmdl[lf_index]); /* store to actual line */
    for (max_c = 0; *p; max_c++, p++)
        ;

    p = p_cmdl[0];
    cmdl_display('f'); /* force printout of cmd_line	*/
    cmd_m_eol();       /* move cursor to end of line	*/
    return (0);
}

/************************************************************************/
static int cmd_m_sht(what) /* shift history table  */
char       what;           /* 't' for just testing */
{
    int i;
    /* delete following blanks      */
    for (p = p_cmdl[0] + max_c - 1; p >= p_cmdl[0]; p--)
    {
        if (*p == ' ')
        {
            *p = '\0';
            max_c--;
        }
        else
            break;
    }
    /* delete leading   blanks      */
    for (p = p_cmdl[0]; *p == ' '; max_c--)
        strcpy(p, p + 1);

    if (*p_cmdl[0]) /* is there a command   */
    {
        if (what == 't')
            return (0);
        /* check, if the last command was the same */
        if (strcmp(p_cmdl[0], p_cmdl[1]) == 0)
            return (0);
        p = p_cmdl[CMDL_N - 1];          /* shift        */
        for (i = CMDL_N - 1; i > 0; i--) /* history      */
            p_cmdl[i] = p_cmdl[i - 1];   /* table        */
        p_cmdl[0] = p;
        return (0);
    }
    return (-1);
}

/************************************************************************/
static int cmd_m_hp(void) /* show the help_pages          */
{
    help_page('c');
    cmdl_display('b');
    return (0);
}

/************************************************************************/
/*ARGSUSED*/
static void sub_err(int sig_num) /* command substitution execution error */
{
    gotoxy(1, TermLines - 1);
    printf("%s     TIMEOUT in Command substitution : COMMAND BROKEN     %s%s\n",
           A_FAILURE, A_NORMAL, DEL_EOL);
    fflush(stdout);
    cmd_askexit();
}

/************************************************************************/
static char *sub_cmd(char *p_s) /* command substitution execution via pipe */
{

    FILE       *fp;
    static char p_ret[PRET_LEN];
    char       *pp, *p_i;
    int         i, rc;

    *p_ret = '\0';
    if ((pp = malloc((unsigned)strlen(p_cmdl[0]) + 2 + strlen(EXEC_SH) +
                     strlen(STDINOFF) + strlen(STDERR21))) == NULL)
        syserr("fids:malloc");

    gotoxy(1, TermLines - 1);
    printf("%ssubstituting...%s%s", A_MESSAGE, A_NORMAL, DEL_EOL);
    fflush(stdout);

    strcpy(pp, EXEC_SH);
    for (p_i = pp + strlen(EXEC_SH); *p_s && *p_s != ' ';)
    {
        *p_i++ = *p_s++;
    }
    *p_i = '\0';
    strcat(pp, STDINOFF); /* switch off std_in    */
    strcat(pp, STDERR21); /*switch stderr to the same as stdout(=pipe)*/
    strcat(pp, p_s);      /* rest of command      */

    sig_nocatch();
    if (!(fp = popen(pp, "r")))
        syserr("popen failed");
    for (p_i = p_ret, i = sizeof(p_ret) - 1; (*p_i = fgetc(fp)) != (char)EOF;)
    {
        if (*p_i < ' ')
            *p_i = ' '; /* if CR etc. make it to blank  */
        if (i)
        {
            p_i++;
            i--;
        } /* read always the hole pipe    */
    }
    *p_i-- = '\0';
    while (p_i > p_ret && *p_i == ' ')
        *p_i-- = '\0';
    rc = pclose(fp);
    free(pp);
    sig_catch();

    mesg_line(); /* restore the mesg_line */
    stat_setact = FALSE;

    if (!rc && *p_ret && *p_ret != (char)EOF)
        return (p_ret);
    else
        return (NULL);
}

/************************************************************************/
static int cmd_m_x(void) /* go to execute the command            */
{
    int     i, m, rc, max_old;
    char   *p_c, *p_e, tmp_c;
    BOOLEAN mark_flag;
    BOOLEAN err_flag;

    if (cmd_m_sht('t') == 0) /* is there a command   */
    {

#ifdef _FDRNFS
        if (scrn == 2) /* two screens? */
        {
            p_lsr[0]->f_st.st_ctime = 0; /* Force dir reread cause nfs on QNX */
            p_lsl[0]->f_st.st_ctime = 0;
        }
        else
            p_ls[0]->f_st.st_ctime = 0; /* Force dir reread cause nfs on QNX */
#endif

        /********** P A R A M E T E R   S U B S T I T U T I O N
         * *****************/

        mark_flag = FALSE;

        for (p = p_cmdl[0]; *p; p++) /* search for $???? */
        {
            if ((*p == SUBST_ENV_CHAR) &&
                ((p > p_cmdl[0]) && (*(p - 1) != '\\') &&
                 (*(p - 1) != '\"')))
            { /* search end of VAR_Name */
                for (p_e = p + 1; (*p_e >= '0' && *p_e <= '9') ||
                                  (*p_e >= 'A' && *p_e <= 'Z') ||
                                  (*p_e >= 'a' && *p_e <= 'z') || (*p_e == '_');
                     p_e++)
                    ;

                tmp_c = *p_e;              /* save next character  */
                *p_e  = '\0';              /* set end of string    */
                if ((p_c = getenv(p + 1))) /* is there these env_var */
                {                          /* then substitute $name  */
                    mark_flag = TRUE;      /* and set the found flag */

                    *p_e = tmp_c;   /* restore next character */
                    strcpy(p, p_e); /* delete the $VAR_NAME   */
                    max_c -= (p_e - p);

                    for (; *p_c; p_c++)
                        if (!insert_c(*p_c)) /* insert value */
                            p++;

                    p--;
                    continue;
                }
                *p_e = tmp_c; /* restore next character */
                              /* don't substitute $name */
            }
        }

        if (mark_flag)
        {
            p = p_cmdl[0];     /* set pointer to begin of line */
            cmdl_display('f'); /* force printout of cmd_line	*/
            cmd_m_eol();       /* move cursor to end of line	*/
            return (0);
        }

        /**********  end  P A R A M E T E R   S U B S T I T U T I O N
         * ***********/

        /**************** C O M M A N D       S U B S T I T U T I O N
         * ***********/

        err_flag = mark_flag = FALSE;

        for (p = p_cmdl[0]; *p; p++) /* search for `???` */
        {
            if ((*p == SUBST_CMD_CHAR) &&
                ((p > p_cmdl[0]) && (*(p - 1) != '\\') &&
                 (*(p - 1) != '\"')))
            { /* search end of CMD_Name */
                for (p_e = p + 1; *p_e && *p_e != SUBST_CMD_CHAR; p_e++)
                    ;

                tmp_c = *p_e; /* save next character  */
                *p_e  = '\0'; /* set end of string    */

                if ((signal(SIGALRM, sub_err)) == SIG_ERR)
                    syserr("signal");
                alarm(60); /* set timeout for non_return_commands! */

                p_c = sub_cmd(p + 1); /* is there these command */

                alarm(0);

                if (p_c)              /* is there these command */
                {                     /* then substitute $name  */
                    mark_flag = TRUE; /* and set the found flag */

                    if (tmp_c)
                        strcpy(p, ++p_e); /* delete the CMD_NAME    */
                    else
                        *p = '\0';

                    max_c -= (p_e - p);
                    for (; *p_c; p_c++)
                        if (!insert_c(*p_c)) /* insert value */
                            p++;

                    p--;
                    continue;
                }
                /* don't substitute $name */
                err_flag = TRUE;
                if (tmp_c)
                {
                    *p_e = tmp_c; /* restore next character */
                    p    = p_e;
                }
            }
        }

        if (err_flag)
        {
            cmd_m_sht('.'); /* shift history table  */
            error("Error in ", "Command substitution");
            return (cmd_m_exit(-1));
        }

        if (mark_flag)
        {
            p = p_cmdl[0];     /* set pointer to begin of line */
            cmdl_display('f'); /* force printout of cmd_line	*/
            cmd_m_eol();       /* move cursor to end of line	*/
            return (0);
        }

        /**********  end  C O M M A N D     S U B S T I T U T I O N
         * ************/

        cmd_m_sht('.'); /* shift history table  */

        mark_flag = FALSE;
        p         = p_cmdl[1];

        /* check, if at least one file is marked */
        for (i = 0; i < numd + numf; i++)
        {
            if (!p_ls[i]->f_inode) /* this file is marked  */
            {
                mark_flag = TRUE;
                break;
            }
        }

        if (mark_flag)
        {
            mark_flag = FALSE;
            for (i = 0; *p; i++, p++) /* search MARK in cmdl  */
            {
                if (*p == MARK_CHAR)
                {
                    mark_flag = TRUE;
                    break;
                }
            }
        }

        if (!mark_flag)
        {
            /* is command an intern command  */
            for (i = 0; *cmd_table[i].cmd_name; i++)
            {
                m = strlen(cmd_table[i].cmd_name);
                if (strlen(p_cmdl[1]) < m)
                    m--; /* (# - ' ') */
                if (!strncmp(p_cmdl[1], cmd_table[i].cmd_name,
                             m))
                { /* command is an intern command */
                    in_cmd = i;
                    if (cmd_table[i].cmd_opt)
                        return (cmd_m_exit((*cmd_table[i].cmd_fkt)()));

                    for (i = m;; i++)
                    {
                        switch (p_cmdl[1][i])
                        {
                            case ' ':
                                continue;
                            case '-':
                                cmd_ex_do(p_cmdl[1]);
                                quit = TRUE;
                                return (0);
                            default:
                                return (cmd_m_exit((*cmd_table[in_cmd].cmd_fkt)()));
                        }
                    }
                }
            }

            /**************** L O C A L E S       E N V I R O N M E N T
             * *************/

            for (p = p_cmdl[1] + 1; *p; p++) /* search for 'name=value' */
            {
                /* search end of VAR_Name */
                if ((*p >= '0' && *p <= '9') || (*p >= 'A' && *p <= 'Z') ||
                    (*p >= 'a' && *p <= 'z') || (*p == '_'))
                    continue;

                if (*p == '=')
                {
                    char *penv;
                    if ((penv = malloc((unsigned)(strlen(p_cmdl[1]) + 1))) ==
                        NULL)
                        syserr("fids:malloc");
                    strcpy(penv, p_cmdl[1]);
                    if (putenv(penv))
                    {
                        error("Error in ", "Variable assignment");
                        return (cmd_m_exit(-1));
                    }
                    stat_list = fidsnew('c');
                    return (cmd_m_exit(0));
                }
                else
                    break;
            }

            cmd_ex_do(p_cmdl[1]); /* execute cmd not MARK */
            quit = TRUE;          /* set leave flag       */
            return (0);
        }

        setblock(0, TRUE); /* set terminal to blocking mode*/
        termrest();        /* set terminal normal  */
        PF_label(0);
        cls();
        gotoxy(1, TermLines);
        rc = 0;
        /* search all marked files */
        max_old = max_c; /* save length of line  */
        for (i = 0; i < numd + numf; i++)
        {
            if (!p_ls[i]->f_inode) /* this is marked  */
            {
                char cbe = fndelim(p_ls[i]->f_name);
                strcpy(p_cmdl[0], p_cmdl[1]); /*copy actual line */
                max_c = max_old;              /* get right linelength */
                p     = p_cmdl[0];
                for (; *p; p++) /* search for MARK */
                {
                    if (*p == MARK_CHAR)
                    {
                        strcpy(p, p + 1); /* delete the MARK */
                        max_c--;
                        if (cbe)
                            if (!insert_c(cbe))
                            {
                                p++;
                            }
                        for (p_c = p_ls[i]->f_name; *p_c; p_c++)
                            if (!insert_c(*p_c)) /* insert fname */
                            {
                                p++;
                            }
                        if (cbe)
                            if (!insert_c(cbe))
                            {
                                p++;
                            }
                    }
                } /* for (m) */
                prt_str(0, CMDL_C, p_cmdl[0]);
                printf("\n");
                fflush(stdout); /* send it out          */
                sig_nocatch();
                rc = system(p_cmdl[0]); /* execute the command  */
                sig_catch();

            } /* if(p_ls[i]) */
        } /* for (i) */

        cmd_ex_ret(rc);
        quit = TRUE; /* set leave flag       */
        return (0);

    } /* if (*p) */
    return (cmd_m_exit(0));
}

/************************************************************************/
static int cmd_m_rc0(void) /* leave the command_mode with rc=0     */
{
    cmd_m_sht('.');
    return (cmd_m_exit(0));
}
/************************************************************************/
static int cmd_m_exit(rc) /* leave the command_mode right  */
int        rc;
{
    PF_label(1);
    restore_cursor();
    if (rc == 0)
        mesg_line(); /* restore the mesg_line */
    quit = TRUE;     /* set leave flag        */
    return (rc);
}

/* ------------------------------------------------------------------------- */
static int cmd_chd(void) /* execute intern command chd */
{
    char *p_c;

    restore_cursor();
    if (max_c == 2) /* only 'cd' ? */
    {
        if (!(p_c = getenv("HOME")))
            p_c = "$HOME";
        return (cmd_cd(p_c, 'b'));
    }
    return (cmd_cd(&p_cmdl[1][3], 'f'));
}

/* ------------------------------------------------------------------------- */
static int cmd_mail(void) /* intern command 'mail' */
{
    struct stat m_stat;

    restore_cursor();

    if (max_c == 4) /* command without arguments? */
    {
        char *m_path;
        char  mail_path[NDIR];

        if (!(m_path = getenv("MAIL")))
        {
            sprintf(mail_path, "%s/%s", MAIL_DIR, getlogin());
            m_path = mail_path;
        }

        if ((stat(m_path, &m_stat) == ERR) || (m_stat.st_size == 0))
        {
            error("No mail.", "");
            return 1;
        }
    }

    return (cmd_ex_do(p_cmdl[1]));
}

/* ------------------------------------------------------------------------- */
static int cmd_rows(void) /* toggle FileLstRows */
{
    switch (FileLstRows)
    {
        case 4:
            FileLstRows = 2;
            break;
        case 2:
        default:
            FileLstRows = 4;
            break;
    }

    stat_list = fidsnew('c');

    return (0);
}

/************************************************************************/
static int cmd_set(void) /* set internal variables */
{
#define FIDS_SET_CMD_alarm "alarm" /* cmd for setting time_def       */
#define FIDS_SET_CMD_rows  "rows"  /* cmd for setting FileLstRows    */
#define FIDS_SET_CMD_ssoe  "ssoe"  /* cmd for setting FS_save_flag   */
#define FIDS_SET_CMD_csev  "csev"  /* cmd for setting SUBST_ENV_CHAR */
#define FIDS_SET_CMD_cscmd "cscmd" /* cmd for setting SUBST_CMD_CHAR */

    int   rc = -1;
    char *pp, *p1, *p2 = NULL, old_val[128];

    old_val[0] = '\0';

    pp = strchr(p_cmdl[1], ' '); /* jump after the 'fids_set' */
    if (pp == NULL)
    {
        /* print all settings */
        error("", ""); /* set the error flag ! */
        printf("Actual values : '%s'= %s%d%s , '%s'= %s%d%s , '%s'= %s%c%s , "
               "'%s'= %s%c%s , '%s'= %s%c%s%s",
               FIDS_SET_CMD_alarm, A_SETVALUES, time_def, A_NORMAL,
               FIDS_SET_CMD_rows, A_SETVALUES, FileLstRows, A_NORMAL,
               FIDS_SET_CMD_ssoe, A_SETVALUES, FS_save_flag ? 'y' : 'n',
               A_NORMAL, FIDS_SET_CMD_csev, A_SETVALUES, SUBST_ENV_CHAR,
               A_NORMAL, FIDS_SET_CMD_cscmd, A_SETVALUES, SUBST_CMD_CHAR,
               A_NORMAL, DEL_EOL);
        fflush(stdout);
        return (-1); /* if rc == -1 : the printed line keeps visible */
    }
    while (*pp == ' ') /* skip blanks */
        pp++;
    if (strlen(pp) < 1)
        return (-1);
    p1 = pp; /* p1 = start of 1. token  */

    pp = strchr(p1, '='); /* check if there is a '=' */
    if (pp != NULL)
        *pp = ' ';

    pp = strchr(p1, ' '); /* jump after the 1. token */
    if (pp != NULL)
    {
        *pp++ = '\0'; /* terminate the 1. token  */
        while (*pp == ' ')
            pp++; /* skip blanks */
        if (strlen(pp) > 0)
            p2 = pp; /* p2 = start of 2. token  */
    }

    /* check if the first token is a valid keyword */

    if (strcmp(p1, FIDS_SET_CMD_alarm) == 0)
    {
        if (p2 == NULL)
            sprintf(old_val, "%d", time_def);
        else
        {
            int i;
            if (sscanf(p2, "%d", &i) > 0)
            {
                if (i >= 2 && i <= CLK_RATE)
                {
                    time_def = i;
                    rc       = 0;
                }
            }
        }
    }
    else if (strcmp(p1, FIDS_SET_CMD_rows) == 0)
    {
        if (p2 == NULL)
            sprintf(old_val, "%d", FileLstRows);
        else
        {
            int i;
            if (sscanf(p2, "%d", &i) > 0)
            {
                if (i == 2)
                    FileLstRows = 2;
                else if (i == 4)
                    FileLstRows = 4;
                rc = 1;
            }
        }
    }
    else if (strcmp(p1, FIDS_SET_CMD_ssoe) == 0)
    {
        if (p2 == NULL)
            sprintf(old_val, "%c", FS_save_flag ? 'y' : 'n');
        else
        {
            FS_save_flag = (*p2 == 'n') ? FALSE : TRUE;
            rc           = 0;
        }
    }
    else if (strcmp(p1, FIDS_SET_CMD_csev) ==
             0) /* char for subst. env variable */
    {
        if (p2 == NULL)
            sprintf(old_val, "%c", SUBST_ENV_CHAR);
        else
        {
            SUBST_ENV_CHAR = *p2;
            rc             = 0;
        }
    }
    else if (strcmp(p1, FIDS_SET_CMD_cscmd) ==
             0) /* char for subst. command */
    {
        if (p2 == NULL)
            sprintf(old_val, "%c", SUBST_CMD_CHAR);
        else
        {
            SUBST_CMD_CHAR = *p2;
            rc             = 0;
        }
    }
    else
    { /* take it as an environment variable */
        if (p2 == NULL)
        {
            pp = getenv(p1);
            if (pp != NULL)
                strncpy(old_val, getenv(p1), sizeof(old_val) - 1);
        }
        else
        {
            char *penv;
            if ((penv = malloc((unsigned)(strlen(p1) + strlen(p2) + 2))) ==
                NULL)
                syserr("fids:malloc");
            sprintf(penv, "%s=%s", p1, p2);
            if (putenv(penv))
            {
                error("Error in ", "Environment Variable assignment");
                return (-1);
            }
            rc = 1;
        }
    }

    if (p2 == NULL)
    {
        error("", ""); /* set the error flag ! */
        printf("Actual value for '%s' = %s%s%s%s", p1, A_SETVALUES, old_val,
               A_NORMAL, DEL_EOL);
        fflush(stdout);
        return (-1);
    }

    if (rc == 1)
        stat_list = fidsnew('c');

    return (rc);
}

/************************************************************************/
static int cmd_setal(void) /* set update interval time */
{
    gotoxy(1, TermLines - 1);
    printf("Update interval time = xx sec  < CURSOR UP/DOWN: change,  else go "
           "back >%s",
           DEL_EOL);

    for (;;)
    {
        gotoxy(24, TermLines - 1);
        printf("%2d", time_def);
        fflush(stdout);

        switch (get_key())
        {
            case CURK('A'):
            case CTRL('^'):
                if (time_def < CLK_RATE)
                    time_def++;
                break;
            case CURK('B'):
            case 28 /*^\*/:
                if (time_def > 2)
                    time_def--;
                break;
            case CTRL('E'):
            case CTRL('F'):
                time_def = CLK_RATE;
                break;
            case CTRL('X'):
            case CTRL('L'):
                time_def = 2;
                break;
            default:
                restore_cursor();
                mesg_line();
                return (0);
        }
    }
}

/* ------------------------------------------------------------------------- */
static int cmd_comp(void) /* compare 2 directories */
{
    int         i, c;
    char       *pp, *dir_other;
    FILE       *fp1, *fp2;
    struct stat stbuf;

    if (scrn == 1)
        return (-1);

    if (actlr == 'l')
        dir_other = dir_n_r;
    else
        dir_other = dir_n_l;

    if ((pp = malloc((unsigned)PATH_MAX + PATH_MAX)) == NULL)
        syserr("fids:malloc");

    gotoxy(1, TermLines - 1);
    printf("%sComparing all files...%s%s", A_MESSAGE, A_NORMAL, DEL_EOL);
    fflush(stdout);

    /* first untag all files */
    cmd_utag();
    swi_act();
    cmd_utag();
    swi_act();

    /* loop now for all files in the current directory */

    sig_nocatch();
    for (i = numd; i < numd + numf; i++)
    {
        /* check, if this file exists also in the other dir */

        sprintf(pp, "%s/%s", dir_other, p_ls[i]->f_name);
        if (stat(pp, &stbuf) == ERR)
        {
            p_ls[i]->f_inode = 0; /* mark the file as not equal */
            continue;
        }

        /* the file exists also in the other directory */

        if (((stbuf.st_mode & S_IFMT) != S_IFREG) ||
            ((p_ls[i]->f_st.st_mode & S_IFMT) != S_IFREG))
        {
            /* no regular file : compare some entries of the stat structures !
             */
            if ((stbuf.st_mode != p_ls[i]->f_st.st_mode) ||
                (stbuf.st_rdev != p_ls[i]->f_st.st_rdev) ||
                (stbuf.st_size != p_ls[i]->f_st.st_size))
                p_ls[i]->f_inode = 0; /* mark the file as not equal */
        }
        else
        { /* both are regular files : compare them ! */
            if (stbuf.st_size != p_ls[i]->f_st.st_size)
            {
                p_ls[i]->f_inode = 0; /* mark the file as not equal */
            }
            else
            {
                setraw(1); /* make it interruptable ! */
                if (!(fp1 = fopen(p_ls[i]->f_name, "r")))
                {
                    error("Error: Can't open file: ", p_ls[i]->f_name);
                    break;
                }
                FidsF1closeOnInterrupt = fp1;
                if (!(fp2 = fopen(pp, "r")))
                {
                    error("Error: Can't open file: ", p_ls[i]->f_name);
                    FidsF1closeOnInterrupt = NULL;
                    fclose(fp1);
                    break;
                }
                FidsF2closeOnInterrupt = fp2;

                while ((c = fgetc(fp1)) != EOF)
                {
                    if (c != fgetc(fp2))
                    {
                        p_ls[i]->f_inode = 0; /* mark the file as not equal */
                        break;
                    }
                }
                FidsF1closeOnInterrupt = NULL;
                FidsF2closeOnInterrupt = NULL;
                fclose(fp1);
                fclose(fp2);
            }
        }
    }
    free(pp);
    sig_catch();
    setraw(0); /* set terminal into raw mode */

    listfiles('.');
    stat_setact = FALSE;
    return (0);
}

/************************************************************************/
static int cmd_mark(void) /* mark all files found with exec. cmd */
{
    int   i, len;
    FILE *fp;
    char *pp, *p_i, *p_n, p_ret[PRET_LEN];

    *p_ret = '\0';
    p_i    = strchr(p_cmdl[1], ' '); /* jump after the 'fids_tag' */
    if (p_i == NULL)
        return (-1);
    while (*p_i == ' ')
        p_i++;
    if (strlen(p_i) < 1)
        return (-1);

    if ((pp = malloc((unsigned)strlen(p_i) + strlen(ECHO_SH) + 2 +
                     strlen(EXEC_SH) + strlen(STDINOFF) + strlen(STDERR21))) ==
        NULL)
        syserr("fids:malloc");
    *pp = '\0';
    len = 0;
    strcpy(pp, EXEC_SH);

    /* search if the first token has '*' or '?' */
    for (p_n = p_i; *p_n && *p_n != ' '; p_n++)
    {
        if (*p_n == '*' ||
            *p_n == '?')
        { /* the first command must be the 'echo' */
            strcat(pp, ECHO_SH);
            len = 1;
            p_i--; /* start with the ' ' character */
            break;
        }
    }

    p_n = pp + strlen(pp);
    if (len == 0)
    { /* insert the the first token as the command */
        for (; *p_i && *p_i != ' ';)
        {
            *p_n++ = *p_i++;
        }
        *p_n = '\0';
        strcat(pp, STDINOFF); /* switch off std_in    */
    }
    strcat(pp, STDERR21); /*switch stderr to the same as stdout(=pipe)*/
    strcat(pp, p_i);      /* rest of command      */

    gotoxy(1, TermLines - 1);
    printf("%sexecuting... %s%s", A_MESSAGE, A_NORMAL, DEL_EOL);
    fflush(stdout);

    /* first untag all files */
    cmd_utag();

    sig_nocatch();

    if (!(fp = popen(pp, "r")))
        syserr("popen failed");
    FidsPcloseOnInterrupt = fp;
    setraw(1); /* make it interruptable ! */
    p_i = p_ret;
    len = sizeof(p_ret) - 1;
    while ((*p_i = fgetc(fp)) != (char)EOF)
    {
        if (*p_i < ' ')
            *p_i = ' '; /* if CR etc. make it to blank  */

        if (*p_i == ' ')
        {
            *p_i = '\0';
            p_n  = p_ret;
            while (*p_n && *p_n <= ' ')
            {
                continue; /* skip to the start of the name */
            }
            if (*p_n)
            {
                /* loop now for all files in the current directory */

                for (i = 0; i < numd + numf; i++)
                {
                    if (!strcmp(p_n, p_ls[i]->f_name))
                    {
                        p_ls[i]->f_inode = 0; /* mark the file */
                        break;
                    }
                }
            }
            p_i = p_ret; /* set the read pointer again to the start */
            len = sizeof(p_ret) - 1;
        }
        else
        {
            if (len)
            {
                p_i++;
                len--;
            } /* read always the hole pipe    */
        }
    }
    pclose(fp);
    FidsPcloseOnInterrupt = NULL;
    free(pp);
    sig_catch();
    setraw(0); /* set terminal into raw mode */

    listfiles('.');
    stat_setact = FALSE;
    return (0);
}

/************************************************************************/
void printfspec()
{
    char allfspec[(PATH_MAX) + (PATH_MAX)];

    strcpy(allfspec, filespec);
    strcat(allfspec, " ! ");
    strcat(allfspec, exclspec);
    if (strlen(allfspec) > (TermCols - 4))
        allfspec[TermCols - 4] = '\0';
    gotoxy(TermCols - (unsigned)strlen(allfspec) - 3, TermLines - 1);
    printf(" %s ", allfspec);
}

/************************************************************************/
static int cmd_incl(void) /* set fspec to cmdl. */
{
    char *pp;

    pp = strchr(p_cmdl[1], ' '); /* jump after the 'fids_xxx' */
    if (pp == NULL)
        strcpy(filespec, "");
    else
    {
        while (*pp == ' ')
            pp++;

        if (strlen(pp) < 1)
            strcpy(filespec, "");
        else
            strcpy(filespec, pp);
    }

    switch (cmd_table[in_cmd].how_ret)
    {
        case '.':
            stat_setact = setact('.');
            break;
        case 'b':
            stat_list = fidsnew('b');
            break;
        case 'n':
            stat_list = fidsnew('n'); /* new if file '.' */
            break;                    /* has been changed*/
        default:
            stat_list = fidsnew('c');
            break;
    }
    return (0);
}

/************************************************************************/
static int cmd_excl(void) /* set fspec exclude to cmdl. */
{
    char *pp;

    pp = strchr(p_cmdl[1], ' '); /* jump after the 'fids_xxx' */
    if (pp == NULL)
        strcpy(exclspec, "");
    else
    {
        while (*pp == ' ')
            pp++;

        if (strlen(pp) < 1)
            strcpy(exclspec, "");
        else
            strcpy(exclspec, pp);
    }

    switch (cmd_table[in_cmd].how_ret)
    {
        case '.':
            stat_setact = setact('.');
            break;
        case 'b':
            stat_list = fidsnew('b');
            break;
        case 'n':
            stat_list = fidsnew('n'); /* new if file '.' */
            break;                    /* has been changed*/
        default:
            stat_list = fidsnew('c');
            break;
    }
    return (0);
}

/************************************************************************/
static int cmd_pipe(void) /* intern sh command execution via pipe */
{
    FILE *fp;
    char *pp, p_ret[PRET_LEN];
    int   i;

    *p_ret = '\0';
    if ((pp = malloc((unsigned)strlen(p_cmdl[1]) + 2 + strlen(EXEC_SH) +
                     strlen(STDINOFF) + strlen(STDERR21))) == NULL)
        syserr("fids:malloc");

    strcpy(pp, p_cmdl[1]);
    if (!cmd_table[in_cmd].stdin_off)
        strcat(pp, STDINOFF); /* switch off std_in    */
    strcat(pp, STDERR21);     /*switch stderr to the same as stdout(=pipe)*/

    gotoxy(1, TermLines - 1);
    printf("%sexecuting...%s%s", A_MESSAGE, A_NORMAL, DEL_EOL);
    fflush(stdout);

    sig_nocatch();
    if (!(fp = popen(pp, "r")))
        syserr("popen failed");
    FidsPcloseOnInterrupt = fp;
    setraw(1); /* make it interruptable ! */
    for (p = p_ret, i = sizeof(p_ret) - 1;
         (*p = fgetc(fp)) != (char)EOF;) /*lint !e789 Assigning address of auto
                                            (p_ret) to static (p) */
    {
        if (*p < ' ')
            *p = ' '; /* if CR etc. make it to blank  */
        if (i)
        {
            p++;
            i--;
        } /* read always the hole pipe    */
    }
    *p = '\0';
    pclose(fp);
    FidsPcloseOnInterrupt = NULL;
    sig_catch();
    setraw(0); /* set terminal into raw mode */
    free(pp);

    restore_cursor();

    if (*p_ret && *p_ret != (char)EOF)
    {
        stat_setact = setact('.');  /* update the actual file parms */
        stat_list   = fidsnew('n'); /* read new if file '.' changed */
        error("", p_ret);
        return (1);
    }

    switch (cmd_table[in_cmd].how_ret)
    {
        case '.':
            stat_setact = setact('.');
            break;
        case 'n':
            stat_list = fidsnew('n'); /* new if file '.' */
            break;                    /* has been changed*/
        default:
            stat_list = fidsnew('c');
            break;
    }
    return (0);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
int cmd_mode(pos) /* command input mode           */
int pos;          /* pos !=0 for set cursor to pos*/
{                 /*         and edit old line    */
    int c, i, rc = 0;

    quit     = FALSE;
    lf_index = max_c = 0; /* set start indexes            */

    save_cursor();
    gotoxy(1, TermLines - 1);
    makehline(TermLines - 1, 'l'); /* restore the bot -1  */
    printfspec();

    printf(CNORM); /* set cursor visible           */

    p = p_cmdl[0]; /* pointer to actual cmdline    */
    if (pos > 0)   /* is there a given position to edit */
    {
        for (; *p; max_c++, p++) /* calculate length of cmd_line */
            ;

        p = p_cmdl[0];
    }
    else
        *p = '\0'; /* start for a new cmd_line     */

    cmdl_display('f'); /* force printout of cmd_line	*/

    if (pos > 0)
    {
        pos--;
        p = p_cmdl[0] + pos;
        cmdl_display('.');
    }

    for (;;)
    {
        fflush(stdout); /* send it all out              */
        if (quit)
            return (rc); /* leave the cmd_mode           */

        c = get_key();
        /* next character isn't cmd     */
        if (c >= ' ' && c < DEL)
        {
            if (*p) /* position isn't end of line   */
            {
                if (insert_mode)
                    cmd_m_ins(); /* insert ' ' into input buffer */
                *p = c;          /* save character       */
                if (max_c > (TermCols - 1))
                {
                    cmdl_display('p');
                    p++;
                    cmdl_display('.');
                }
                else
                {
                    p++;
                    putchar(c); /* put char on screen   */
                }
            }
            else if (max_c < CMDL_C - 1)
            {             /* position is end of line      */
                *p++ = c; /* save character       */
                *p   = 0; /* reset next entry     */
                max_c++;  /* one character added  */
                if (max_c > (TermCols - 1))
                    cmdl_display('r');
                else
                    putchar(c); /* put char on screen   */
            }
            continue; /* go to next loop run  */
        }

        if (c < ' ') /* is it a <CTRL> Command ?     */
        {
            if (*pf_cctl[c]) /* is this user defined ?       */
            {
                push_s(pf_cctl[c]); /* push the str to input buffer */
                continue;
            }
        }

        /* is the key a command_key     */
        for (i = 0; cmd_mod[i].cmd_char; i++)
        {
            if ((c == cmd_mod[i].cmd_char) || (c == cmd_mod[i].cmd_cchar))
            {
                rc = (cmd_mod[i].cmd_fkt)(); /* call funct.  */
                c  = '\0';
                break;
            }
        } /* for(i) */

    } /* for (;;) */
}
