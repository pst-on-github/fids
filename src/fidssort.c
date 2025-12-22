/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: sortls							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/* sort in struct ls            index=startindex in ls, number=entries  */
/************************************************************************/
#include "fids.h"
#include "fidsext.h"

	extern char smodl,smodr;        /* sort modus                   */

#define DELIM '.'               /* delimitter filename fileextension    */

/************************************************************************/
static int comp( char *p_c1, char *p_c2 )            /* compare strings */
{
	int  cmp, c1,c2;

	for (;;)
	{
	    c1 = tolower( *p_c1++ );
	    c2 = tolower( *p_c2++ );

	    if ( (cmp = ( c1 - c2) ) )
		return( cmp );
	    if ( !c1 ) return(0);
	}
}

/************************************************************************/
static int sort_nam(p1,p2)                   /* funktion: sort by names */
struct FIDS **p1,**p2;
{
	return( comp( (*p1)->f_name,(*p2)->f_name) );
}

/************************************************************************/
static int sort_typ(p1,p2)               /* funktion: sort by filetypes */
struct FIDS **p1,**p2;
{
	register char *p_1,*p_2;
	register char *pc1,*pc2;
	register char *p_i1,*p_i2;
	int rc;

	pc1 = p_1 = (*p1)->f_name;
	pc2 = p_2 = (*p2)->f_name;

	for ( p_i1 = pc1; *pc1; pc1++)            /* search DELIM first */
		if ( *pc1 == DELIM ) p_i1 = pc1;

	for ( p_i2 = pc2; *pc2; pc2++)            /* search DELIM second*/
		if ( *pc2 == DELIM ) p_i2 = pc2;

	if ( p_i1 > p_1 )
	{                                       /* DELIM is in first    */
		if ( p_i2 > p_2 )
		{                               /* DELIM also in second */
		    rc = comp(p_i1,p_i2);       /* compare extension    */
		    if ( rc ) return(rc);
		    return( comp(p_1,p_2) );
		}
		return(1);                      /* DELIM in 1. not in 2.*/
	}
	else if ( p_i2 > p_2 ) return(-1);      /* DELIM is in second   */

	return( comp(p_1,p_2) );
}

/************************************************************************/
static int sort_size(p1,p2)               /* funktion: sort by filesize */
struct FIDS **p1,**p2;
{
	int rc;

	rc = (*p2)->f_st.st_size - (*p1)->f_st.st_size;
	if ( rc ) return(rc);
	return( comp( (*p1)->f_name,(*p2)->f_name) );
}

/************************************************************************/
static int sort_uid(p1,p2)                   /* funktion: sort by uid's */
struct FIDS **p1,**p2;
{
	int rc;

	rc = (*p2)->f_st.st_uid - (*p1)->f_st.st_uid;
	if ( rc ) return(rc);
	return( comp( (*p1)->f_name,(*p2)->f_name) );
}

/************************************************************************/
static int sort_gid(p1,p2)                   /* funktion: sort by gid's */
struct FIDS **p1,**p2;
{
	int rc;

	rc = (*p2)->f_st.st_gid - (*p1)->f_st.st_gid;
	if ( rc ) return(rc);
	return( comp( (*p1)->f_name,(*p2)->f_name) );
}

/************************************************************************/
static int sort_acc(p1,p2)              /* funktion: sort by access_tim */
struct FIDS **p1,**p2;
{
	int rc;

	rc = (*p2)->f_st.st_atime - (*p1)->f_st.st_atime;
	if ( rc ) return(rc);
	return( comp( (*p1)->f_name,(*p2)->f_name) );
}

/************************************************************************/
static int sort_modi(p1,p2)             /* funktion: sort by modif_time */
struct FIDS **p1,**p2;
{
	int rc;

	rc = (*p2)->f_st.st_mtime - (*p1)->f_st.st_mtime;
	if ( rc ) return(rc);
	return( comp( (*p1)->f_name,(*p2)->f_name) );
}

/************************************************************************/
static int sort_chan(p1,p2)             /* funktion: sort by chang_time */
struct FIDS **p1,**p2;
{
	int rc;

	rc = (*p2)->f_st.st_ctime - (*p1)->f_st.st_ctime;
	if ( rc ) return(rc);
	return( comp( (*p1)->f_name,(*p2)->f_name) );
}

/* ------------------------------------------------------------------------- */

int sortls(index,number,how)
int index;
int number;
int how;
{
	int (*cfkt)();

	if (how == '.')
	{
		if (actlr == 'r') how = smodr;
		      else        how = smodl;
	}

	switch( how )
	{
		case 'd':                               /* directories  */
		case 'n':       cfkt = sort_nam;
				break;
		case 't':       cfkt = sort_typ;
				break;
		case 's':       cfkt = sort_size;
				break;
		case 'u':       cfkt = sort_uid;
				break;
		case 'g':       cfkt = sort_gid;
				break;
		case 'a':       cfkt = sort_acc;
				break;
		case 'm':       cfkt = sort_modi;
				break;
		case 'c':       cfkt = sort_chan;
				break;
		default:        return(-1);
	}

	gotoxy(1,TermLines-1);
	printf("%ssorting files... !%s%s",
		A_MESSAGE,A_NORMAL,DEL_EOL);
	fflush(stdout);

	if ( how != 'd')
	{
		if (actlr == 'r') smodr = how;
		      else        smodl = how;
	}

	qsort( (char *)&p_ls[index], (unsigned)number, sizeof(struct FIDS *), cfkt );
	return(0);
}
