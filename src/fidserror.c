/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: syserr							*/
/*                                                                      */
/* Ausgabe von System Error Meldungen                                   */
/************************************************************************/

#include <errno.h>
#include "fids.h"

#ifdef HAS_NO_STRERROR

extern char *sys_errlist[];
extern int sys_nerr;
char *strerror( int num )
{
  if( num > 0 && num < sys_nerr )
    return( sys_errlist[num] );
  else
    return( "???" );
}

#endif

void syserr( char *msg )
{
  gotoxy( 1, 24 );

  fprintf( stderr, "ERROR: %s (%d; %s)", msg, errno, strerror(errno) );

  cmd_exit();
  exit(1);
}
