/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: setblock							*/
/*           cready                                                     */
/*           cget                                                       */
/*           push_c                                                     */
/*           push_s                                                     */
/*           prt_str                                                    */
/*           get_key                                                    */
/************************************************************************/
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include "fids.h"

/************************************************************************/
/* set the terminal I/O blocking on and off                             */
/************************************************************************/
void setblock(fd,on)
int fd;
BOOLEAN on;
{
	static int blockf, nonblockf;
	static BOOLEAN first = TRUE;
	int flags;

	if (first)
	{
	    first = FALSE;
	    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
		syserr("fcntl");
#if defined QNX4 || defined QNX6
	    blockf = flags & ~O_NONBLOCK;     /* O_NONBLOCK off */
	    nonblockf = flags | O_NONBLOCK;   /* O_NONBLOCK on  */
#else
	    blockf = flags & ~O_NDELAY;     /* O_NDELAY off */
	    nonblockf = flags | O_NDELAY;   /* O_NDELAY on  */
#endif
	}
	if (fcntl(fd, F_SETFL, on ? blockf : nonblockf) == -1)
	    syserr("fcntl2");
}
/************************************************************************/
/* is a character ready returns the character else return 0		*/
/************************************************************************/
#define EMPTY '\0'
static char cbuf[CMDL_C] = "\0";

char cready()
{
	if( *cbuf != EMPTY )
	    return( *cbuf );

#ifdef	FIDS_TYPEAHEAD

	/* this works fine in SYSV and HP_UX,
		 but not in ULTRIX and DEC-OSF/1	*/
	setblock( FD_STDIN,FALSE );
	if( read( FD_STDIN, cbuf, 1 ) > 0 )
	{
	    cbuf[1] = EMPTY;
	    return( *cbuf );
	}
	else
	    return( EMPTY );
#else
	return( EMPTY );
#endif
}

/************************************************************************/
/* get a character if ready, else wait for the next character		*/
/************************************************************************/
static char cget( void )
{
	char c;

	if (*cbuf != EMPTY)
	{
	    c = *cbuf;
	    strcpy( cbuf, &cbuf[1] );	/* shift buffer 1 charact. down */
	    return( c );
	}
	setblock(FD_STDIN,TRUE);

	while( read( FD_STDIN, &c, 1 ) < 1 )
	{
            /* check, if we are still connected to a tty */
	    if( !isatty(FD_STDIN) )
	        syserr("stdin");
	}
	return( c );
}

/************************************************************************/
/* push a character in front of the input buffer			*/
/************************************************************************/
int push_c(c)
char c;
{
	int i,len;
	if ( (len=strlen(cbuf) ) < CMDL_C-1 )
	{
	    for( i=len+1 ; i ; i-- )
		cbuf[i] = cbuf[i-1];
	    cbuf[0] = c;
	    return(0);
	}
	return(-1);
}

/************************************************************************/
/* push a string in front of the input buffer				*/
/************************************************************************/
int push_s( char *s )
{
  int i, len_b, len_s;
  len_b = strlen( cbuf );
  len_s = strlen( s );

  if ( len_b + len_s < CMDL_C )
  {
    for (i=len_b+1 ; i ; i--)
      cbuf[i+len_s-1] = cbuf[i-1]; /*lint !e676 Possibly negative subscript (-1) in operator '[' */

    for (i=0; i < len_s ; i++)
      cbuf[i] = s[i];  /*lint !e676 Possibly negative subscript (-1) in operator '[' */

    return(0);
  }
  return(-1);
}

/************************************************************************/
/* print a string with putchar and print a '?' for char < ' '		*/
/************************************************************************/
int prt_str(int num, int cis, char *s) /* cis= number of char in string used */
                                       /* num= number of printed charcaters  */
                                       /* (-)= left justified else right jus.*/
{
	int i,len,max_l;
	len = strlen(s);
	if ( len >= cis ) max_l = cis;
		else      max_l = len;

	if ( num >= cis )                       /* right jusfified      */
	{
	    for ( i=0 ; i < num-max_l; i++ ) putchar(' ');
	}

	for ( i=0; i < max_l; i++,s++ )           /* print string         */
	{
	    if ( *s < ' ' || *s > '~' ) putchar('?');
		else putchar(*s);
	}

	if ( num >= 0 ) return(0);
	if ( (-num) >= cis )                    /* left jusfified       */
	{
	    for ( i=0 ; i < (-num)-max_l; i++ ) putchar(' ');
	}
	return(0);
}

/************************************************************************/
	char gkb[8];

#if defined(QNX4) || defined(_GNU_SOURCE)
	static sigjmp_buf ljmp_env;
#else
	static jmp_buf ljmp_env;
#endif

static void key_brk();

int get_key()
{
	int i;

	for ( i=0; i<8 ; i++ ) gkb[i] = '\0';

	gkb[0] = cget();

	alarm(0);			/* reset alarm		  */

        switch( gkb[0] )
        {
          case ESC :
            break;
	  case DEL :
            return( CTRL('_') );
          default :
            return( (int)gkb[0] );
        }

#if defined(QNX4) || defined(_GNU_SOURCE)
	if( sigsetjmp( ljmp_env, TRUE ) != 0 )
	    return( (int)( ESC1( gkb[0] ) ) );	/* interrupted	*/
#else
        if( setjmp( ljmp_env ) != 0 )
	    return( (int)( ESC1( gkb[0] ) ) );	/* interrupted	*/
#endif

	signal( SIGALRM, key_brk );	/* set signal handling          */
	alarm(1);			/* set timeout                  */

	gkb[1] = cget();		/* get next character           */
	alarm(0);			/* reset alarm                  */

	switch ( gkb[1] )
	{
	    case ESC :	/* <escape>, followed by a escape ...	*/
			push_c( ESC );		/* push it back */
			break;

	    case '[':	/* ESC [ ... */

		gkb[2]=cget();		  /* get the 3.Char of ESC seqenze */

		if ( gkb[2]>='1' && gkb[2]<='9' )
		{
		    gkb[3]=cget();     	  /* get the next Char of ESC seqenze */
		    if ( gkb[3]>='0' && gkb[3]<='9' )
		    {
			gkb[4]=cget(); 	  /* get the next Char of ESC seqenze */
			if( gkb[4] != '~' )
			    break;
			switch( (gkb[2]-'0') * 10 + (gkb[3]-'0') )
			{
			    case 11 :	return( PFKY('1') );   /* PC F1       */
			    case 17 :	return( PFKY('6') );   /* vt220 F6    */
			    case 12 :	return( PFKY('2') );   /* PC P2       */
			    case 18 :	return( PFKY('7') );   /* vt220 F7    */
			    case 13 :	return( PFKY('3') );   /* PC F3       */
			    case 19 :	return( PFKY('8') );   /* vt220 F8    */
			    case 14 :	return( PFKY('4') );   /* PC F4       */
			    case 20 :	return( PFKY('9') );   /* vt220 F9    */
			    case 15 :	return( PFKY('5') );   /* PC F5       */
			    case 21 :	return( PFKY('2') );   /* vt220 F10   */
			    case 23 :	return( ESC );         /* vt220 F11   */
			    case 24 :	return( CTRL('H') );   /* vt220 F12   */
			    case 25 :	return( PFKY('3') );   /* vt220 F13   */
			    case 26 :	return( PFKY('4') );   /* vt220 F14   */
			    case 28 :	return( ESC1('h') );   /* vt220 Help  */
			    case 29 :	return( ESC1('X') );   /* vt220 Do    */
			    case 31 :	return( PFKY('5') );   /* vt220 F17   */
			    case 32 :	return( PFKY('6') );   /* vt220 F18   */
			    case 33 :	return( PFKY('7') );   /* vt220 F19   */
			    case 34 :	return( PFKY('8') );   /* vt220 F20   */
			}
			break;
		    }

		    if( gkb[3] != '~' )
			break;
		    switch( gkb[2] )
		    {
			case '1':   return( ESC1('<') );   /* vt220 Find   */
			case '2':   return( ESC1('#') );   /* vt220 Insert */
			case '3':   return( ESC1('R') );   /* vt220 Remove */
			case '4':   return( ESC1('@') );   /* vt220 Select */
			case '5':   return( ESC1('P') );   /* vt220 Prev   */
			case '6':   return( ESC1('N') );   /* vt220 Next   */
		    }
		    break;
		}

		switch( gkb[2] )
		{
		    case 'A':   /* cursor up    */
                              return( CTRL('^') );
		    case 'B':   /* cursor down  */
                              return( 28 /*^\*/ );
		    case 'C':   /* cursor right */
		    case 'D':   /* cursor left  */
			      return( (int)gkb[2] + ES2 );

#ifdef WITH_XTERM_MOUSE
                    case 'M': /* XTERM Mouse event ESC[Mbxy */
			      gkb[3]=cget();
			      gkb[4]=cget();
			      gkb[5]=cget();

                        gkb[3] - 32 : 0 left button down
                                      1 middle button down
                                      2 right button down
                                      3 button up

                        gkb[4] - 32 : x-position (column)
                        gkb[5] - 32 : y-position (row)

#endif

#ifdef SCO
		    case 'M' :  return( PFKY('1') );
		    case 'N' :	return( PFKY('2') );
		    case 'O' :	return( PFKY('3') );
		    case 'P' :	return( PFKY('4') );
		    case 'Q' :	return( PFKY('5') );
		    case 'R' :	return( PFKY('6') );
		    case 'S' :	return( PFKY('7') );
		    case 'T' :	return( PFKY('8') );
		    case 'U' :	return( PFKY('9') );
		    case 'H' :	return( ESC1('<') );	/* ANSI - Home */
		    case 'F' :	return( ESC1('>') );	/* ANSI - End  */
		    case 'I' :	return( ESC1('P') );	/* ANSI - PgUp */
		    case 'G' :	return( ESC1('N') );	/* ANSI - PgDn */
		    case 'L' :	return( ESC1('#') );	/* ANSI - Insert */
/*		    default :	return( (int)gkb[2] + ES2 );	*/
#endif

#if defined QNX4 || defined QNX6
		    case '@' :	return( ESC1('#') );   /* qnx2 ansi - Insert -> Toggle Insert */
		    case 'P' :	return( ESC1('R') );   /* qnx2 ansi - Delete -> Remove */
		    case 'U' :	return( ESC1('N') );   /* qnx2 ansi - PgDown -> Next Cmd */
		    case 'V' :	return( ESC1('P') );   /* qnx2 ansi - PgUp   -> Prev Cmd */
		    case 'H' :	return( ESC1('<') );   /* qnx2 ansi - Home   -> Jmp BOL */
		    case 'Y' :	return( ESC1('>') );   /* qnx2 ansi - End    -> Jmp EOL */
#endif
		}
		break;

	    case 'O':
		switch( (gkb[2]=cget()) ) /* get the 3.Char of ESC seqenze*/
		{
#if defined QNX4 || defined QNX6
		    case 'P':   return( PFKY('1') );  /* qnx2 ansi - F1  */
		    case 'Q':   return( PFKY('2') );  /* qnx2 ansi - F2  */
		    case 'R':   return( PFKY('3') );  /* qnx2 ansi - F3  */
		    case 'S':   return( PFKY('4') );  /* qnx2 ansi - F4  */
		    case 'T':   return( PFKY('5') );  /* qnx2 ansi - F5  */
		    case 'U':   return( PFKY('6') );  /* qnx2 ansi - F6  */
		    case 'V':   return( PFKY('7') );  /* qnx2 ansi - F7  */
		    case 'W':   return( PFKY('8') );  /* qnx2 ansi - F8  */
		    case 'X':   return( PFKY('9') );  /* qnx2 ansi - F9  */
		    case 'Y':   return( PFKY('0') );  /* qnx2 ansi - F10 */

		    case 'p':   return( PFKY('a') );  /* qnx2 ansi - F1  */
		    case 'q':   return( PFKY('b') );  /* qnx2 ansi - F2  */
		    case 'r':   return( PFKY('c') );  /* qnx2 ansi - F3  */
		    case 's':   return( PFKY('d') );  /* qnx2 ansi - F4  */
		    case 't':   return( PFKY('e') );  /* qnx2 ansi - F5  */
		    case 'u':   return( PFKY('f') );  /* qnx2 ansi - F6  */
		    case 'v':   return( PFKY('g') );  /* qnx2 ansi - F7  */
		    case 'w':   return( PFKY('h') );  /* qnx2 ansi - F8  */
		    case 'x':   return( PFKY('i') );  /* qnx2 ansi - F9  */
#else
		    case 'M':   return( ESC1('J') );  /* Retrieve cmd */
		    case 'P':   return( PFKY('1') );  /* PF's vt100   */
		    case 'Q':   return( PFKY('2') );
		    case 'R':   return( PFKY('3') );
		    case 'S':   return( PFKY('4') );
#endif
		    default :	return( (int)gkb[2] + ES2 );
		}
                /*NOTREACHED*/
   		break;

	    default :
		break;
	}
	return( (int)( ESC1( gkb[1] ) ) );
}

/************************************************************************/
static void key_brk()
{
#if defined(QNX4) || defined(_GNU_SOURCE)
        siglongjmp( ljmp_env, -1 );
#else
	longjmp( ljmp_env, -1 );
#endif
}
