/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: listfiles							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/* list files of the actual directory                  b                */
/* scr: startindex           what: + next, - previous, c begin, n new   */
/************************************************************************/
#include "fids.h"
#include "fidsext.h"

#ifdef OSF
#include <sys/mount.h>          /* "struct statfs" for DEC UNIX (OSF1) */
#include <sys/fs_types.h>       /* "mnt_names[]"                       */
#define STATFS_HAS_MNT_NAMES_OSF
#endif

#ifdef ULTRIX
#include <sys/param.h>
#include <sys/mount.h>          /* "struct fs_data" for DEC ULTRIX     */
#include <sys/fs_types.h>       /* "gt_names[]"                        */
#define STATFS_HAS_MNT_NAMES_ULTRIX
#endif

#ifdef LINUX
#define HAS_MNTENT_H
#endif

#ifdef HP_UX
#define HAS_MNTENT_H
#endif

#ifdef QNX4
#include <sys/fsys.h>
#endif

#ifdef QNX6
#include <sys/dcmd_blk.h>
#include <sys/iomsg.h>
#include <fcntl.h>
#include <share.h>
#endif

#ifdef HAS_MNTENT_H
#include <mntent.h>
#endif

	extern BOOLEAN GRAF_VT100;
	extern BOOLEAN stat_setact;

	extern char *p_cwd;
	void printMountInfo( int icols, int max_len );

void listfiles( char what )
{
	int i,x,rows;
	int num,ind,l_pwd;

        /* check the terminal lines and cols */
        if( TermLines < TERM_MIN_LINES || TermCols < TERM_MIN_COLS )
        {
          return;
        }

	rows = FileLstRows / scrn;
	num = numd + numf;

	if( acti >= num )                /* possible after cmd_execution */
	  acti = num-1;

	scr = acti-acti%((TermLines-FLINES)*rows);  /* adjust screen index */


	switch ( what )
	{
		case 'c':
		case 'b':
			mesg_line();
			if ( scrn == 1 )
			{
			    gotoxy(1,1);
			    printf( A_WORKDIR );
			    if ( (l_pwd=strlen(p_cwd)) <= (TermCols-(FrightWinCols+1)) )
				   prt_str(-(TermCols-(FrightWinCols+1)),TermCols-(FrightWinCols+1),p_cwd);
			    else { prt_str(0,3,"...");
				   prt_str(0,TermCols-(FrightWinCols+1)-3,&p_cwd[l_pwd-(TermCols-(FrightWinCols+1)-3)]); }
			    printf( A_NORMAL );
			    i = FileLstNameLen*FileLstRows/2;
			    if( i > 13 )
			      printMountInfo((FileLstNameLen+1)*FileLstRows/2+4,i);
			}
			else
			{
			    if (actlr == 'r')
			    {
				gotoxy((FileLstNameLen+1)*FileLstRows/2+2,1);
				if ( GRAF_VT100 ) { printf(GRAF_ON);
						    putchar(GRAF_V); }
				else                putchar(GRAF_NV);
				gotoxy((FileLstNameLen+1)*FileLstRows/2+2,2);
				if ( GRAF_VT100 ) { putchar(GRAF_CR);
						    printf(GRAF_OFF); }
				else                putchar(GRAF_NC);
				makevline((FileLstNameLen+1)*FileLstRows/2+2);
				gotoxy((FileLstNameLen+1)*FileLstRows/2+3,1);
			    }
			    else
			    {
				makevline(1);
				gotoxy(1,1);
			    }
			    printf( A_WORKDIR );
			    if ( (l_pwd=strlen(p_cwd)) <= (FileLstNameLen+1)*FileLstRows/2+1 )
				   prt_str(-((FileLstNameLen+1)*FileLstRows/2+1),(FileLstNameLen+1)*FileLstRows/2+1,p_cwd);
			    else { prt_str(0,3,"...");
				   prt_str(0,(FileLstNameLen+1)*FileLstRows/2-2,&p_cwd[l_pwd-((FileLstNameLen+1)*FileLstRows/2-2)]); }
			    printf( A_NORMAL );
			    i = (FileLstNameLen-(FileLstRows==2?1:0))*FileLstRows/4;
			    if( i > 13 )
			    {
			      if (actlr == 'r')
				printMountInfo((FileLstNameLen+1)*FileLstRows/2+FileLstNameLen*FileLstRows/4+5, i);
			      else
				printMountInfo(FileLstNameLen*FileLstRows/4+4, i);
			    }
			}
			break;

		case 'n':
			mesg_line();

		case '.':
			break;

		case '+': if((scr+(TermLines-FLINES)*rows) < num)
		          {
		            scr += (TermLines-FLINES)*rows;
		            acti = scr;
		          }
		          else
                          {
                            stat_setact = setact('L');
		            return;
                          }
		 	  break;

		case '-': if((scr-(TermLines-FLINES)*rows) >= 0 )
		          {
		            scr -= (TermLines-FLINES)*rows;
                            if(scr == 0)
                              acti = 1;
                            else
		              acti = scr;
		          }
			  else
                          {
                            stat_setact = setact('F');
			    return;
			  }
			  break;

		default : return;
	}

	if( actlr == 'r' )              /* set new scr r/l */
	{
	  scrr  = scr;
	  actir = acti;
	}
	else
	{
	  scrl  = scr;
	  actil = acti;
        }

	if ( scrn == 1 )                /* only 1 directory on screen   */
	{
	  if ( what != 'c' )
	  {
	    for (i=3;i<=(TermLines-2);i++)    /* clear list_window  */
	    {
	      gotoxy(TermCols-(FrightWinCols+2),i);
	      puts(DEL_BOL);
	    }
	  }
	  makevline(1);                   /* make vertical   line in 1    */
	}

	for (x=0; x<rows; x++)                  /* loop for each row    */
	{
	    for (i=0;i<(TermLines-FLINES);i++)
	    {
		if (actlr=='l')                 /* for scrn == 1 || 2   */
			gotoxy( x*(FileLstNameLen+1)+(FileLstRows==2?x+2:2)+x/2,i+3 );
		if (actlr=='r')                 /* scrn must be 2       */
			gotoxy( (FileLstRows==2?1:x+2)*(FileLstNameLen+1)+3,i+3 );

		if ( (ind = i+x*(TermLines-FLINES)+scr) < num )
		{
			if( !p_ls[ind]->f_inode )
			{
                          char m_char;
			  if( ( p_ls[ind]->f_st.st_mode & S_IFMT ) == S_IFDIR )
			    m_char = MARK_DIR_CHAR;
			  else
                            m_char = MARK_CHAR;
			  printf("%s%c%s",A_MARKCHAR,m_char,A_NORMAL);
			}
			else
			{
			  if( ( p_ls[ind]->f_st.st_mode & S_IFMT ) == S_IFDIR )
			    printf("%s%c%s",A_DIRCHAR,DIR_CHAR,A_NORMAL);
			  else
			    putchar(' ');
                        }

			if (scrn==1 || what == 'c')
			    prt_str(0,FileLstNameLen,p_ls[ind]->f_name);
			else
			    prt_str(-FileLstNameLen,FileLstNameLen,p_ls[ind]->f_name);
		}
		else
		{
			if (scrn==1 || what == 'c')
				     goto leave;        /* break break  */
			else         prt_str((FileLstNameLen+1),(FileLstNameLen+1)," ");
		} /* if */

	    } /* for (i) */

	} /* for (x) */

    leave:
    if (actlr=='l') gotoxy(5,2);
	else        gotoxy((FileLstNameLen+1)*FileLstRows/2+6,2);
    printf("%s of%3d files%s",A_NOFMFILES,num,A_NORMAL);
    return;
}

void printMountInfo( int icols, int max_len )
{
	char fmtstr[64];
	char rwopt, *p_fsname, *p_fstype, *psc;
	int i;

#ifdef STATFS_HAS_MNT_NAMES_OSF
	struct statfs fs;
#else
#ifdef STATFS_HAS_MNT_NAMES_ULTRIX
	struct fs_data fs;
#else
#ifdef HAS_MNTENT_H
	FILE *mntp;
	struct mntent *mnt;
	struct stat filestat, dirstat;
#else
#if defined QNX4 || defined QNX6
	char mnt_fs_dev[PATH_MAX];
#else
	return;
#endif
#endif
#endif
#endif
	/* First clear the last MountInfo */
	/* (this is partly copied from fidsscreen.c) */

	gotoxy(icols-1,2);
	if( GRAF_VT100 ) printf(GRAF_ON);
	for( i=icols-1; i<icols+max_len; i++ )
	{
	  if( FileLstRows == 4 && (
		i == FileLstNameLen+3   /* 17 */ ||
		i == FileLstNameLen*3+6 /* 48 */ ) )
	  {
	    if ( GRAF_VT100 )
	      putchar(GRAF_HD);
	    else
	      putchar(GRAF_NC);
	  }
	  else
	  {
	    if ( GRAF_VT100 )
	      putchar(GRAF_H);
	    else
	      putchar(GRAF_NH);
	  }
	}
	if ( GRAF_VT100 ) printf(GRAF_OFF);
	gotoxy(icols,2);

#ifdef STATFS_HAS_MNT_NAMES_OSF

	if( statfs( p_cwd, &fs ) < 0 )	/* Get Filesystem status */
	  return;

	/* check  'rw' - options */
	if( fs.f_flags & M_RDONLY )
	  rwopt = 'o';
	else
	  rwopt = 'w';

	p_fsname = fs.f_mntfromname;
	p_fstype = mnt_names[ fs.f_type ];

#endif

#ifdef STATFS_HAS_MNT_NAMES_ULTRIX

	if( statfs( p_cwd, &fs ) < 0 )	/* Get Filesystem status */
	  return;

	/* check  'rw' - options */
	if( fs.fd_flags & M_RONLY )
	  rwopt = 'o';
	else
	  rwopt = 'w';

	p_fsname = fs.fd_devname;
	p_fstype = gt_names[ fs.fd_fstype ];

#endif

#ifdef QNX4

	fsys_get_mount_dev( p_cwd, mnt_fs_dev );

	p_fsname = mnt_fs_dev;
	p_fstype = NULL;

	rwopt = 'w';

#endif

#ifdef QNX6
    // from the mig2nto lib: fsys_get_mount_dev
    {
      union
      {
        struct
        {
          struct _io_devctl       devctl;
        } ii;
        struct
        {
          struct _io_devctl_reply devctl;
          char   path[256];
        } oo;
      } msg;

      msg.ii.devctl.type = _IO_DEVCTL;
      msg.ii.devctl.combine_len = sizeof(msg.ii);
      msg.ii.devctl.dcmd = DCMD_FSYS_MOUNTED_ON;
      msg.ii.devctl.nbytes = DCMD_FSYS_MOUNTED_ON >> 16;
      msg.ii.devctl.zero = 0;
      // Whatever _connect_combine is? stoe.
      if (_connect_combine(p_cwd, 0, O_ACCMODE, SH_DENYNO, 0, _FTYPE_ANY, sizeof(msg.ii), &msg.ii, sizeof(msg.oo), &msg.oo) == -1)
        return;

      strcpy(mnt_fs_dev, msg.oo.path);
	  p_fsname = mnt_fs_dev;
	  p_fstype = NULL;
	  rwopt = '?';
    }
#endif

#ifdef HAS_MNTENT_H

	/* Get Mount info for this Filesystem */

	if( stat( p_cwd, &filestat ) < 0 )
	  return;

	if( ( mntp = setmntent(MOUNTED, "r") ) == (FILE*)0 )
	  return;

	while( ( mnt = getmntent( mntp ) ) != (struct mntent *)0 )
	{
	  if( stat( mnt->mnt_dir, &dirstat ) < 0 )
	  {
	    endmntent(mntp);
	    return;
	  }
	  if( filestat.st_dev == dirstat.st_dev )
	  {
	    endmntent( mntp );
	    break;
	  }
	}
	if( mnt == (struct mntent *)0 )
	{
	  endmntent(mntp);
	  return;
        }

	/* check  'rw' - options */
	psc = mnt->mnt_opts;
	while( ( psc = strchr( psc, 'r' ) ) != NULL )
	{
	  psc++;
	  if( *psc == '/' )
	    psc++;
	  else if( strcmp( psc, "ead/" ) == 0 )
	    psc += 4;
	  rwopt = *psc;
	  break;
	}
	if( psc == NULL )
	{
	  if( ( strncmp( mnt->mnt_opts, "default", 7 ) == 0 ) ||
	      ( strlen( mnt->mnt_opts ) < 1 ) )
	    rwopt = 'w';
	  else
	    rwopt = '?';
	}

	p_fsname = mnt->mnt_fsname;
	p_fstype = mnt->mnt_type;

#endif

	if( max_len > ( sizeof(fmtstr) -1 ) )
	  max_len = sizeof(fmtstr) -2;

	psc = strchr( p_fsname, ':' );		/* "host:path" */
	if( psc == NULL )
	{
	  psc = strchr( p_fsname, '@' );	/* "path@host" */
	}
	if( psc == NULL )	/* No NFS, check 'rw'- options */
	{
	  if( rwopt == 'w' )
	  {
	    return;	/* Do nothing if local filesystem and 'rw' */
	  }
	  strncpy( fmtstr, p_fsname, sizeof(fmtstr) -1 );
	  fmtstr[ sizeof(fmtstr) -1 ] = '\0';
	}
	else
	{ /* NFS mounted ! */
	  if( strlen(p_fsname) < max_len )
	  {
	    strcpy( fmtstr, p_fsname );
	  }
	  else
	  { /* use only the hostname */
	    if( *psc == '@' )			/* "path@host" */
	    {
	      i = min( strlen( psc+1 ), sizeof(fmtstr)-2 );
	      strncpy( fmtstr, psc+1, i );
	    }
	    else				/* "host:path" */
	    {
	      i =  min( psc - p_fsname, sizeof(fmtstr)-2 );
	      strncpy( fmtstr, p_fsname, i );
	    }
	    fmtstr[i] = ':';
	    fmtstr[i+1] = '\0';
	  }
	}

	i = strlen( fmtstr );
	if( i > max_len )	/* fsname is too long, don't use it */
	{
#ifdef QNX6
	  fmtstr[max_len-1] = '>';
	  fmtstr[max_len] = '\0';
#else
	  strcpy( fmtstr, "fstyp:" );
#ifdef QNX4
	  if( p_fstype == NULL )
	    strcat( fmtstr, "nfs" );
          else
#endif
	    strncat( fmtstr, p_fstype,
	      min( sizeof(fmtstr)-7, max_len-6 ) );
	  fmtstr[min(sizeof(fmtstr)-1,max_len)] = '\0';
#endif
	}

#ifdef QNX6
#else
	/* add the rwopt, if it's not 'rw' (and space enough) */
	if( rwopt != 'w' )
	{
	  if( ( strlen( fmtstr ) + 4 ) <= max_len )
	    sprintf( fmtstr, "%s<r%c>", fmtstr, rwopt );
	}

	if( i <= max_len )	/* fstype was not copied yet */
	{ /* add the fstype, if it's not NFS (and space enough) */
	  if( ( psc == NULL ) && p_fstype &&
		 ( (strlen( fmtstr )+2+strlen(p_fstype)) <= max_len ) )
	  {
	    sprintf( fmtstr, "%s(%s)", fmtstr, p_fstype );
	  }
	}
#endif

	printf( "%s%s%s", A_ACTFILENAME, fmtstr, A_NORMAL );

	return;
}
