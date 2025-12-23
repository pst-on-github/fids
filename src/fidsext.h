/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* file    : fidsext.h							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/

extern struct FIDS **p_lsl, **p_lsr, **p_ls;

extern int  numdl, numdr, numd; /* number of subdirs left/right */
extern int  numfl, numfr, numf; /* number of files left right   */
extern int  scrl, scrr, scr;    /* index to dirs/files on screen*/
extern int  actil, actir, acti; /* index to dir/file actual     */
extern int  scrn;               /* number of logical screens    */
extern char actlr;              /* l: left screen, r: = right   */

extern int   TermGrix; /* index to term grafic array   */
extern char *Term_Names[TERM_NAME_NUM][TERM_NAME_SNUM];
extern char  Term_Grafic[TERM_NAME_NUM][TERM_GRAF_NUM];
extern char *Term_Attr_Str[TERM_NAME_NUM][TERM_ATTR_STRING_NUM];
extern char *Term_Attr[TERM_NAME_NUM][TERM_ATTR_NUM];
extern int   TermLines, TermCols; /* actual number of lines, cols */
extern int   FileLstRows;         /* actual number of rows for file_names */
extern int   FileLstNameLen;      /* actual number of char used for file_names */
extern int   FrightWinCols;       /* actual number of columns for the right window */

/************************************************************************/
