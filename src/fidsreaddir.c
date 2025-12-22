/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: fids_readdir						*/
/*                                                                      */
/* 05.04.90 usage of directory subroutines of SVR3 			*/
/************************************************************************/
/* read filenames of the given directory				*/
/*                                  return number of files in directory */
/************************************************************************/
#include "fids.h"
#include "fidsext.h"

#if defined QNX4 || defined QNX6
#include <fnmatch.h>
#else
#include "fnmatch.h"
#endif

/* ------------------------------------------------------------------------- */

extern char filespec[];
extern char exclspec[];

/* ------------------------------------------------------------------------- */

static int fnmatch_filespec_list(char * speclist, char * fname)
{
  char spec[PATH_MAX];

  while (*speclist) /* Parse "path_1:path_2:..." */
  {
    int strl = 0;

    while (speclist[strl] && (speclist[strl] != ':')) strl++;
    strncpy (spec, speclist, strl), spec[strl] = '\0';

#if defined QNX4 || defined QNX6
    if (fnmatch (spec, fname, (FNM_PATHNAME)) == 0)
#else
    if (fnmatch (spec, fname, (FNM_PATHNAME|FNM_NOESCAPE)) == 0)
#endif
      return (0); /* MATCH */

    if (speclist[strl])
      speclist += strl+1;
    else
      break;
  }
  return (FNM_NOMATCH);
}

static int fnmatch_filespec(char * fname)
{
  if (fnmatch_filespec_list (exclspec, fname) == 0)
    return (FNM_NOMATCH);
  else
    return (fnmatch_filespec_list (filespec, fname));
}

/* ------------------------------------------------------------------------- */
/* check if file_name string is one word ! */
char fndelim( char *p_fn )
{
  if( p_fn )
  {
    for( ; *p_fn ; p_fn++ )
    {
      if( *p_fn <= ' ' )
        return( FILE_NAME_DELIMITER );
    }
  }
  return( '\0' );
}

/* ------------------------------------------------------------------------- */

int fids_readdir( dir_name )
char *dir_name;
{
static	BOOLEAN first = TRUE;

	register DIR *dp;
	register struct dirent *p_dirent;

	register struct FIDS *pf_head=NULL, *pf_act=NULL, *pf_tmp;
	register int num,i;

	gotoxy(1,TermLines-1);
	printf("%sreading directory... !%s%s",
		A_MESSAGE,A_NORMAL,DEL_EOL);
	fflush(stdout);

	if (!first)             /* free the old malloced storage        */
	{
		for( i=0; i < numd + numf; i++ )
			free( p_ls[i] );
		free( p_ls );
	}
	first = FALSE;

	if( ( dp=opendir( dir_name ) ) == (DIR *)0 )  /* open the directory */
	{
		error("can't open directory: ",dir_name);
		if( cmd_cdu( dir_name ) == ERR )
		    syserr("fids_readdir:open");
	}

				/* read the activ entries from the file	*/
	num = 0;
	while( ( p_dirent = readdir( dp ) ) != (struct dirent *)0 )
	{
          struct stat buf;
	  struct stat *bufp = NULL;

#if defined QNX4
          if (p_dirent->d_stat.st_status & _FILE_USED)
          {     // QNX is capable to return a stat struct
            bufp = &p_dirent->d_stat;
          }
          else
#elif defined QNX6
          {
            struct dirent_extra_stat* extra;

	    for( extra = _DEXTRA_FIRST(p_dirent);
		     _DEXTRA_VALID(extra, p_dirent);
		     extra = _DEXTRA_NEXT(extra))
            {
	      if (extra->d_type == _DTYPE_STAT)
	      {
	        bufp = &extra->d_stat;
	        break;
	      }
	    }
	  }

	  if( !bufp )
#endif
	  {
            stat( p_dirent->d_name, &buf );
            bufp = &buf;
	  }

	  if ( ( (bufp->st_mode & S_IFMT) == S_IFDIR) || fnmatch_filespec(p_dirent->d_name) == 0 )
	  {

	    /* get storage for the one entry in struct fids	*/
	    if( ( pf_tmp = (struct FIDS *)calloc( 1,sizeof( struct FIDS ))) ==
			(struct FIDS *)0 ) syserr("fids_readdir:malloc");
	    pf_tmp->f_next = (struct FIDS *)0;

	    if( num > 0 )
	    {
		pf_act->f_next = pf_tmp;
		pf_act = pf_act->f_next;
	    }
	    else
	    {
		pf_act = pf_head = pf_tmp;	/* first entry	*/
	    }
	    num++;		                  /* number of entries */
#if defined QNX4 || defined QNX6
	    pf_act->f_inode = 1;
#else
	    pf_act->f_inode = p_dirent->d_ino;
#endif
	    strncpy( pf_act->f_name, p_dirent->d_name, FIDS_NAME_LEN_MAX-1 );
            pf_act->f_st = *bufp; /* copy stat data */
	  }
	}

	closedir( dp );				/* close the file        */

/* get pointer_array depend on the number of directory entries		 */
	if( ( p_ls = (struct FIDS **) calloc( num, sizeof(p_ls) ) ) ==
		(struct FIDS **)0 ) syserr("fids_readdir:calloc");

	pf_act = pf_head;
	for( i=0; i<num; i++ )
	{
		p_ls[i] = pf_act;
		pf_act = pf_act->f_next;
	}

/* sort the subdirs on top                                              */
	numd = 0;
	for( i=0; i<num ; i++ )             /* sort the subdirs on top   */
	{
		if ( (p_ls[i]->f_st.st_mode & S_IFMT) == S_IFDIR)
		{
			pf_tmp = p_ls[numd];      /* exchange it */
			p_ls[numd] = p_ls[i];
			p_ls[i] = pf_tmp;
			numd += 1;              /* number of dirs  + 1  */
		}
	}

	numf = num - numd;
	if (actlr == 'r')
	{
		p_lsr = p_ls;
		numdr = numd;
		numfr = numf;
	}
	else
	{
		p_lsl = p_ls;
		numdl = numd;
		numfl = numf;
	}
	return 0;
}
