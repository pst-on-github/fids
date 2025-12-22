/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* file    : fids.h							*/
/*                                                                      */
/************************************** global includes *****************/
/*                                                                      */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <malloc.h>
#ifdef QNX6
#include <errno.h>
#endif

#ifndef fids_h_INCLUDED
#define fids_h_INCLUDED /* Prevent multiple includes */

/* Depending includes: */

#undef GLOBAL
#undef GLOBAL_INIT
#ifdef _fids_ /* Use GLOBAL type modifier for global variables */
#define GLOBAL
/*lint -save -e723 Suspicious use of = */
#define GLOBAL_INIT( xx ) = { xx }
/*lint -restore */
#else
#define GLOBAL extern
#define GLOBAL_INIT( xx )
#endif

/* Declarations and Definitions */

#ifdef	FILENO
#define FD_STDIN	fileno( stdin )		/* standard input */
#else
#define	FD_STDIN	0
#endif

#ifndef PATH_MAX
#define PATH_MAX        1024
#endif

#ifndef MAXNAMLEN
#define MAXNAMLEN	256
#endif

#ifndef MAIL_DIR
#define MAIL_DIR "/usr/mail"    /* directory of mail files              */
#endif

#ifdef EXT_GETENV
extern  char *getenv();
#endif

/*                                                                      */
/************************************** definitions *********************/
/*                                                                      */
#define FK_FILE ".fids.pf_keys" /* file for user defined pf_keys        */
#define FS_FILE ".fids.ss"      /* file for user saved settings         */
#define FIDS_PS1 "PS1=[fids]."  /* environment variable for Shell promt */
#define CLK_RATE 59             /* clock update interval time           */
#define MARK_CHAR '@'           /* character for file marking           */
#define DIR_CHAR  '/'           /* character in file list for dir's     */
#define MARK_DIR_CHAR '#'       /* character for directory marking      */
#define	FILE_NAME_DELIMITER   '\''
#define FLINES   4              /* number of lines for top and bottom   */
#define TERM_MIN_LINES  15      /* minimal number of screen lines       */
#define TERM_MIN_COLS   52      /* minimal number of screen cols        */
#if defined QNX4 || defined QNX6
#define CMDL_C  2048            /* number of characters in cmdl         */
#else
#define CMDL_C  5120            /* number of characters in cmdl         */
#endif
#define CMDL_N  32              /* number of saved cmd_lines            */
#define NDIR    PATH_MAX        /* number of characters for pwd_name    */
#define PFK_C   256             /* number of characters for PF Keys     */
#define PFS 32                  /* number of PF_CTRL_keys ^@ to ^_      */
#define PFK 9                   /* number of PF_keys                    */
#define PFN 8                   /* length of the PF_labels              */
/*                                                                      */
/************************************** screen definitions **************/
/*                                                                      */

#define BEL        0x07         /* Bel                                  */
#define CR         0x0d         /* Carriage Return                      */
#define ESC        0x1b         /* Escape                               */
#define DEL        0x7f         /* Delete                               */

GLOBAL char *WRAP_ON  GLOBAL_INIT( "\033[?7h" );      /* set wrap mode on          */
GLOBAL char *WRAP_OFF GLOBAL_INIT( "\033[?7l" );      /* set wrap mode off         */
GLOBAL char *CNORM    GLOBAL_INIT( "\033[?25h" );     /* cursor normal visible     */
GLOBAL char *CIVIS    GLOBAL_INIT( "\033[?25l" );     /* cursor invisible          */
GLOBAL char *KPAD_OFF GLOBAL_INIT( "\033[?1l\033>" ); /* cursor keypad mode normal */

GLOBAL char *CUF     GLOBAL_INIT( "\033[C"  ); /* Cursor forward (right) Pn columns  */
GLOBAL char *CUB     GLOBAL_INIT( "\033[D"  ); /* Cursor left            Pn colummns */
#if defined QNX4 || defined QNX6
GLOBAL char *DEL_BOL GLOBAL_INIT( "\033[C\033[1K" );
#else
GLOBAL char *DEL_BOL GLOBAL_INIT( "\033[1K" ); /* Erase to beginning of line */
#endif
GLOBAL char *DEL_EOL GLOBAL_INIT( "\033[K"  ); /* Erase to End of line       */
GLOBAL char *DEL_EOS GLOBAL_INIT( "\033[J"  ); /* Erase to end of screen     */
GLOBAL char *DEL_SCR GLOBAL_INIT( "\033[2J" ); /* Erase screen               */
GLOBAL char *SAV_CUR GLOBAL_INIT( "\0337" );   /* Save cursor                */
GLOBAL char *RST_CUR GLOBAL_INIT( "\0338" );   /* Restore cursor             */

#define gotoxy(x,y)      printf("\033[%d;%dH",(y),(x))
#define cls()            printf(DEL_SCR)
#define save_cursor()    printf(SAV_CUR)
#define restore_cursor() printf(RST_CUR)

#define GRAF_NC    '+'          /* graphic character normal cross       */
#define GRAF_NH    '-'          /* graphic character normal horizontal  */
#define GRAF_NV    '|'          /* graphic character normal vertical    */

#define XTERM_MOUSE_TRACKING_ENABLE     "\033[?1000h"
#define XTERM_MOUSE_TRACKING_DISABLE    "\033[?1000l"
#define XTERM_MOUSE_TRACKING_SAVE       "\033[?1001s"
#define XTERM_MOUSE_TRACKING_RESTORE    "\033[?1001r"

#define VT100_TERM       "vt100"    /* name for $TERM                */
#define VT100_GRAF_ON    "\033(0"   /* graphic mode on               */
#define VT100_GRAF_OFF   "\033(B"   /* graphic mode off              */
#define VT100_GRAF_H     'q'        /* grahic horizontal line        */
#define VT100_GRAF_V     'x'        /* grahic vertical line          */
#define VT100_GRAF_UL    'l'        /* grahic upper left corner      */
#define VT100_GRAF_UR    'k'        /* grahic upper right corner     */
#define VT100_GRAF_LL    'm'        /* grahic lower left  corner     */
#define VT100_GRAF_LR    'j'        /* grahic lower right corner     */
#define VT100_GRAF_HD    'w'        /* grahic horizontal down        */
#define VT100_GRAF_HU    'v'        /* grahic horizontal up          */
#define VT100_GRAF_VL    'u'        /* grahic vertical left          */
#define VT100_GRAF_VR    't'        /* grahic vertical right         */
#define VT100_GRAF_CR    'n'        /* grahic cross                  */

#define VT100_ATTR_ATR_OFF    "\033[0m"       /* all Attributes off  */
#define VT100_ATTR_REV_ON     "\033[7m"       /* Reverse on          */
#define VT100_ATTR_UL_ON      "\033[4m"       /* Underline on        */
#define VT100_ATTR_BOLD_ON    "\033[1m"       /* Bold on             */
#define VT100_ATTR_REV_UL_ON  "\033[4;7m"     /* Reverse and UL on   */
#define VT100_ATTR_BOLD_UL_ON "\033[1;4m"     /* Bold and UL on      */
#define VT100_ATTR_NULL       ""              /* nothing             */

#define ANSI_TERM       "ansi"      /* name for $TERM                */
#define ANSI_GRAF_ON    "\033[12m"  /* graphic mode on               */
#define ANSI_GRAF_OFF   "\033[10m"  /* graphic mode of               */
#define ANSI_GRAF_H     'D'         /* graphic GH horizontal line    */
#define ANSI_GRAF_V     '\63'       /* graphic GV vertical line      */
#define ANSI_GRAF_UL    'Z'         /* graphic G2 upper left corner  */
#define ANSI_GRAF_UR    '?'         /* graphic G1 upper right corner */
#define ANSI_GRAF_LL    '@'         /* graphic G3 lower left  corner */
#define ANSI_GRAF_LR    'Y'         /* graphic G4 lower right corner */
#define ANSI_GRAF_HD    'B'         /* graphic GD horizontal down    */
#define ANSI_GRAF_HU    'A'         /* graphic GU horizontal up      */
#define ANSI_GRAF_VL    '\64'       /* graphic GL vertical left      */
#define ANSI_GRAF_VR    'C'         /* graphic GR vertical right     */
#define ANSI_GRAF_CR    'E'         /* graphic GC cross              */

#ifdef _ANSI_COLOR_GREY
#define ANSI_ATTR_BASIC    "\033[1;30;47m"
#define ANSI_ATTR_REVERSE  "\033[1;33;44m"
#define ANSI_ATTR_HIGH     "\033[0;34;47m"
#define ANSI_ATTR_MESSAGE  "\033[0;37;40m"
#define ANSI_ATTR_ERROR    "\033[1;36;44m"
#define ANSI_ATTR_NULL     ""
#else
#define ANSI_ATTR_BASIC    "\033[1;37;44m"
#define ANSI_ATTR_REVERSE  "\033[0;30;47m"
#define ANSI_ATTR_HIGH     "\033[1;33;44m"
#define ANSI_ATTR_MESSAGE  "\033[0;30;46m"
#define ANSI_ATTR_ERROR    "\033[1;37;41m"
#define ANSI_ATTR_NULL     ""
#endif

#define TERM_NAME_NUM   2
#define TERM_NAME_SNUM  3
#define TERM_GRAF_NUM   11

#define GRAF_ON         ( Term_Names[ TermGrix ][1] )
#define GRAF_OFF        ( Term_Names[ TermGrix ][2] )
#define GRAF_H          ( Term_Grafic[ TermGrix ][ 0 ] )
#define GRAF_V          ( Term_Grafic[ TermGrix ][ 1 ] )
#define GRAF_UL         ( Term_Grafic[ TermGrix ][ 2 ] )
#define GRAF_UR         ( Term_Grafic[ TermGrix ][ 3 ] )
#define GRAF_LL         ( Term_Grafic[ TermGrix ][ 4 ] )
#define GRAF_LR         ( Term_Grafic[ TermGrix ][ 5 ] )
#define GRAF_HD         ( Term_Grafic[ TermGrix ][ 6 ] )
#define GRAF_HU         ( Term_Grafic[ TermGrix ][ 7 ] )
#define GRAF_VL         ( Term_Grafic[ TermGrix ][ 8 ] )
#define GRAF_VR         ( Term_Grafic[ TermGrix ][ 9 ] )
#define GRAF_CR         ( Term_Grafic[ TermGrix ][ 10 ] )


#define FIDS_TERM_NAMES { { VT100_TERM, VT100_GRAF_ON, VT100_GRAF_OFF },    \
                          { ANSI_TERM, ANSI_GRAF_ON, ANSI_GRAF_OFF }  }
#define FIDS_TERM_GRAFIC {                                                  \
              { VT100_GRAF_H, VT100_GRAF_V, VT100_GRAF_UL, VT100_GRAF_UR,   \
                VT100_GRAF_LL, VT100_GRAF_LR, VT100_GRAF_HD, VT100_GRAF_HU, \
                VT100_GRAF_VL, VT100_GRAF_VR, VT100_GRAF_CR },              \
              { ANSI_GRAF_H, ANSI_GRAF_V, ANSI_GRAF_UL, ANSI_GRAF_UR,       \
                ANSI_GRAF_LL, ANSI_GRAF_LR, ANSI_GRAF_HD, ANSI_GRAF_HU,     \
                ANSI_GRAF_VL, ANSI_GRAF_VR, ANSI_GRAF_CR }              }

#define FIDS_TERM_ATTR_STRINGS {                                                 \
              { VT100_ATTR_ATR_OFF, VT100_ATTR_REV_ON, VT100_ATTR_UL_ON,         \
                VT100_ATTR_BOLD_ON, VT100_ATTR_REV_UL_ON, VT100_ATTR_BOLD_UL_ON, \
                VT100_ATTR_NULL },                                               \
              { ANSI_ATTR_BASIC, ANSI_ATTR_REVERSE, ANSI_ATTR_HIGH,              \
                ANSI_ATTR_MESSAGE, ANSI_ATTR_ERROR, ANSI_ATTR_NULL,              \
                ANSI_ATTR_NULL }                                                 \
        }

#define TERM_ATTR_STRING_NUM   7

#define STRX_VT100_ATTR_ATR_OFF         (char*)0
#define STRX_VT100_ATTR_REV_ON          (char*)1
#define STRX_VT100_ATTR_UL_ON           (char*)2
#define STRX_VT100_ATTR_BOLD_ON         (char*)3
#define STRX_VT100_ATTR_REV_UL_ON       (char*)4
#define STRX_VT100_ATTR_BOLD_UL_ON      (char*)5
#define STRX_VT100_ATTR_NULL            (char*)6

#define STRX_ANSI_ATTR_BASIC            (char*)0
#define STRX_ANSI_ATTR_REVERSE          (char*)1
#define STRX_ANSI_ATTR_HIGH             (char*)2
#define STRX_ANSI_ATTR_MESSAGE          (char*)3
#define STRX_ANSI_ATTR_ERROR            (char*)4
#define STRX_ANSI_ATTR_NULL             (char*)5

#define TERM_ATTR_NUM   22

#define FIDS_TERM_ATTR { /* Used attributes for 'vt100' */              \
          {                                                             \
            /* A_NORMAL         */      STRX_VT100_ATTR_ATR_OFF,        \
            /* A_WORKDIR        */      STRX_VT100_ATTR_NULL,           \
            /* A_NOFMFILES      */      STRX_VT100_ATTR_REV_UL_ON,      \
            /* A_PFKEYS         */      STRX_VT100_ATTR_REV_ON,         \
            /* A_CMDLBORDER     */      STRX_VT100_ATTR_REV_UL_ON,      \
            /* A_CMDLMODE       */      STRX_VT100_ATTR_BOLD_ON,        \
            /* A_ACTUAL         */      STRX_VT100_ATTR_REV_ON,         \
            /* A_OTHER          */      STRX_VT100_ATTR_BOLD_UL_ON,     \
            /* A_DIRCHAR        */      STRX_VT100_ATTR_REV_UL_ON,      \
            /* A_MARKCHAR       */      STRX_VT100_ATTR_REV_UL_ON,      \
            /* A_ESCHELP        */      STRX_VT100_ATTR_NULL,           \
            /* A_ESCHELPKEY     */      STRX_VT100_ATTR_REV_ON,         \
            /* A_SORT           */      STRX_VT100_ATTR_NULL,           \
            /* A_SORTKEY        */      STRX_VT100_ATTR_REV_ON,         \
            /* A_SETVALUES      */      STRX_VT100_ATTR_REV_ON,         \
            /* A_MESSAGE        */      STRX_VT100_ATTR_REV_ON,         \
            /* A_FAILURE        */      STRX_VT100_ATTR_REV_ON,         \
            /* A_TITLE          */      STRX_VT100_ATTR_REV_ON,         \
            /* A_ACTFILEDATA    */      STRX_VT100_ATTR_NULL,           \
            /* A_ACTFILENAME    */      STRX_VT100_ATTR_UL_ON,          \
            /* A_SYMLINKMARK    */      STRX_VT100_ATTR_BOLD_UL_ON,     \
            /* A_SYMLINKNAME    */      STRX_VT100_ATTR_UL_ON           \
          },                                                            \
          {              /* Used attributes for 'ansi' */               \
            /* A_NORMAL         */      STRX_ANSI_ATTR_BASIC,           \
            /* A_WORKDIR        */      STRX_ANSI_ATTR_HIGH,            \
            /* A_NOFMFILES      */      STRX_ANSI_ATTR_MESSAGE,         \
            /* A_PFKEYS         */      STRX_ANSI_ATTR_MESSAGE,         \
            /* A_CMDLBORDER     */      STRX_ANSI_ATTR_BASIC,           \
            /* A_CMDLMODE       */      STRX_ANSI_ATTR_MESSAGE,         \
            /* A_ACTUAL         */      STRX_ANSI_ATTR_REVERSE,         \
            /* A_OTHER          */      STRX_ANSI_ATTR_HIGH,            \
            /* A_DIRCHAR        */      STRX_ANSI_ATTR_REVERSE,         \
            /* A_MARKCHAR       */      STRX_ANSI_ATTR_ERROR,           \
            /* A_ESCHELP        */      STRX_ANSI_ATTR_NULL,            \
            /* A_ESCHELPKEY     */      STRX_ANSI_ATTR_HIGH,            \
            /* A_SORT           */      STRX_ANSI_ATTR_NULL,            \
            /* A_SORTKEY        */      STRX_ANSI_ATTR_HIGH,            \
            /* A_SETVALUES      */      STRX_ANSI_ATTR_HIGH,            \
            /* A_MESSAGE        */      STRX_ANSI_ATTR_MESSAGE,         \
            /* A_FAILURE        */      STRX_ANSI_ATTR_ERROR,           \
            /* A_TITLE          */      STRX_ANSI_ATTR_MESSAGE,         \
            /* A_ACTFILEDATA    */      STRX_ANSI_ATTR_HIGH,            \
            /* A_ACTFILENAME    */      STRX_ANSI_ATTR_HIGH,            \
            /* A_SYMLINKMARK    */      STRX_ANSI_ATTR_MESSAGE,         \
            /* A_SYMLINKNAME    */      STRX_ANSI_ATTR_HIGH             \
          },                                                            \
        }

#define A_NORMAL        ( Term_Attr[ TermGrix ][ 0 ] )  /* Normal display (all attributes) */
#define A_WORKDIR       ( Term_Attr[ TermGrix ][ 1 ] )  /* Working directory display in the first line */
#define A_NOFMFILES     ( Term_Attr[ TermGrix ][ 2 ] )  /* n of m files display in the second line */
#define A_PFKEYS        ( Term_Attr[ TermGrix ][ 3 ] )  /* PF-keys label in the last line */
#define A_CMDLBORDER    ( Term_Attr[ TermGrix ][ 4 ] )  /* CmdLine Borders */
#define A_CMDLMODE      ( Term_Attr[ TermGrix ][ 5 ] )
#define A_ACTUAL        ( Term_Attr[ TermGrix ][ 6 ] )  /* Currently selected file (active page, if splited) */
#define A_OTHER         ( Term_Attr[ TermGrix ][ 7 ] )  /* Currently selected file (passive page, if splited) */
#define A_DIRCHAR       ( Term_Attr[ TermGrix ][ 8 ] )  /* Dirctory char */
#define A_MARKCHAR      ( Term_Attr[ TermGrix ][ 9 ] )  /* Marked file char */
#define A_ESCHELP       ( Term_Attr[ TermGrix ][ 10 ] ) /* ESC help (name) */
#define A_ESCHELPKEY    ( Term_Attr[ TermGrix ][ 11 ] ) /* ESC  help (key char) */
#define A_SORT          ( Term_Attr[ TermGrix ][ 12 ] ) /* Sort command */
#define A_SORTKEY       ( Term_Attr[ TermGrix ][ 13 ] ) /* Sort command (key char) */
#define A_SETVALUES     ( Term_Attr[ TermGrix ][ 14 ] ) /* Values from the _set command */
#define A_MESSAGE       ( Term_Attr[ TermGrix ][ 15 ] ) /* Generic messages */
#define A_FAILURE       ( Term_Attr[ TermGrix ][ 16 ] ) /* Failure messages */
#define A_TITLE         ( Term_Attr[ TermGrix ][ 17 ] ) /* Node and actual file title */
#define A_ACTFILEDATA   ( Term_Attr[ TermGrix ][ 18 ] ) /* Actual file data display (data) */
#define A_ACTFILENAME   ( Term_Attr[ TermGrix ][ 19 ] ) /* Actual file data display (name) */
#define A_SYMLINKMARK   ( Term_Attr[ TermGrix ][ 20 ] ) /* Symbolic link display */
#define A_SYMLINKNAME   ( Term_Attr[ TermGrix ][ 21 ] )

/*                                                                      */
/************************************** keyboard definitions ************/
/*                                                                      */
#define ES1 0x100               /* value to add to the escape sequence  */
#define ES2 0x200               /*      characters to get just one key  */
#define ES3 0x300
#define ES4 0x400
#ifdef CTRL
#undef CTRL
#endif
#define CTRL(c)    ( c & 037)   /* Control character                    */
#define ESC1(c)    ( c | ES1)   /* escape ?               ? = not '['   */
#define CURK(c)    ( c | ES2)   /* escape [ A,B,C,D       cursor keys   */
#define PFKY(c)    ( c | ES3)   /* escape 1 ... 9         PF-Keys       */
/*                                                                      */
/************************************** C - definitions *****************/
/*                                                                      */

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

typedef enum {FALSE,TRUE} BOOLEAN;

#ifndef ERR
#define ERR -1
#endif

#ifndef max
#define max(a,b) 		((a)<(b)? b : a)
#endif

#ifndef min
#define min(a,b) 		((a)<(b)? a : b)
#endif

/*lint -emacro(506,FIDS_NAME_LEN_MAX) Constant value Boolean */
#ifdef QNX4					/* see dirent.h	*/
#define	FIDS_NAME_LEN_MAX	( 64 > NAME_MAX ? NAME_MAX : 64 )
#else
#define	FIDS_NAME_LEN_MAX	(MAXNAMLEN>64?(MAXNAMLEN>256?256:MAXNAMLEN):64)
#endif

	struct FIDS {				/* structure Filenames  */

		struct	FIDS	*f_next;	/* pointer to next	*/
		long	f_inode;
		char	f_name[ FIDS_NAME_LEN_MAX ];
		struct	stat f_st;		/* file_status          */

	       };


/* ------------------------------------------------------------------------- */
/* Prototypes from fids.c */
/* ------------------------------------------------------------------------- */

int cmd_askexit ( void );
int cmd_exit ( void );
int cmd_ex_do ( char *com );
int cmd_ex_ret ( int rc );
int cmd_cd ( char *dir_name , int what );
int cmd_utag ( void );
int cmd_cdu ( char *dir_name );

BOOLEAN fids_list ( int iside , int iwhat );
BOOLEAN fidsnew ( int what );

int  swi_act ( void );
void error ( char *msg1 , char *msg2 );
void mesg_line ( void );
void help_page ( int mode );
void sig_catch ( void );
void sig_nocatch ( void );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidscmdmode.c */
/* ------------------------------------------------------------------------- */

int cmd_mode( int pos );
void printfspec ( void );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidserror.c */
/* ------------------------------------------------------------------------- */

void syserr ( char *msg );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidsescape.c */
/* ------------------------------------------------------------------------- */

int escape ( int c );
void PF_label ( int onoff );
int FS_save ( char *name );
int FS_load ( char *name );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidslistfile.c */
/* ------------------------------------------------------------------------- */

void listfiles ( char what );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidsreaddir.c */
/* ------------------------------------------------------------------------- */

int fids_readdir ( char *dir_name );
char fndelim( char *p_fn );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidsscreen.c */
/* ------------------------------------------------------------------------- */

void makehline ( int line , int where );
void makescreen ( void );
void makevline ( int line );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidssetact.c */
/* ------------------------------------------------------------------------- */

BOOLEAN setact ( int what );
void printSymLinkName ( void );
void printdate ( struct tm *p_tm );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidssetblock.c */
/* ------------------------------------------------------------------------- */

char cready ( void );
int get_key ( void );
int prt_str ( int num , int cis , char *s );
int push_c ( int c );
int push_s ( char *s );
void setblock ( int fd , BOOLEAN on );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidssetraw.c */
/* ------------------------------------------------------------------------- */

void setraw ( int isig );
void setraw2( int isig, int opost );
void setwinsize ( int *p_lines , int *p_cols );
void termrest ( void );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidssort.c */
/* ------------------------------------------------------------------------- */

int sortls ( int index , int number , int how );

/* ------------------------------------------------------------------------- */
/* Prototypes from fidsview.c */
/* ------------------------------------------------------------------------- */

int cmd_view ( void );

/* ------------------------------------------------------------------------- */
#undef GLOBAL
#undef GLOBAL_INIT
#endif /* fids_h_INCLUDED */
