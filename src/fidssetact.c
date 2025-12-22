/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: setact							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/*                  '.': set the current position                       */
/*                  'A': set the current position 1 entry up            */
/*                  'B': set the current position 1 entry down          */
/*                  'C': set the current position 1 entry right         */
/*                  'D': set the current position 1 entry left          */
/*                  'F': set the current position to first file         */
/*                  'L': set the current position to last file          */
/*                  'S': set the current position special               */
#include "fids.h"
#include "fidsext.h"
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#ifdef OSF
#include <sys/mount.h>          /* "struct statfs" for DEC UNIX (OSF1) */
#define STATFS_BLOCK_SIZE	fs.f_fsize
#define STATFS_BLOCK_FREE	fs.f_bavail
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#endif

#ifdef ULTRIX
#include <sys/param.h>
#include <sys/mount.h>          /* "struct fs_data" for DEC ULTRIX     */
#define STATFS_BLOCK_SIZE	1024
#define STATFS_BLOCK_FREE	fs.fd_bfreen
#define STATFS_BLOCK_TOTAL	fs.fd_btot
#endif

#ifdef LINUX
#include <sys/vfs.h>            /* "struct statfs" */
#define STATFS_BLOCK_SIZE	fs.f_bsize
#define STATFS_BLOCK_FREE	fs.f_bavail
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#define HAS_MNTENT_H
#endif

#ifdef HP_UX
#include <sys/vfs.h>            /* "struct statfs" */
#define STATFS_BLOCK_SIZE	fs.f_bsize
#define STATFS_BLOCK_FREE	fs.f_bavail
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#define HAS_MNTENT_H
#endif

#ifdef QNX4
#include <sys/statfs.h>         /* "struct statfs" */
#define STATFS_4ARGS
#define STATFS_BLOCK_SIZE	fs.f_bsize
#define STATFS_BLOCK_FREE	fs.f_bfree
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#include <sys/fsys.h>
#endif

#ifdef QNX6
#include <sys/statvfs.h>	 /* "struct statfs" */
#define statfs statvfs
#define STATFS_BLOCK_SIZE	fs.f_bsize
#define STATFS_BLOCK_FREE	fs.f_bfree
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#endif

#ifdef SYSV3
#include <sys/statfs.h>         /* "struct statfs" */
#define STATFS_4ARGS
#define STATFS_BLOCK_SIZE	512
#define STATFS_BLOCK_FREE	fs.f_bfree
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#endif

#ifdef SUNOS
#include <sys/statfs.h>         /* "struct statfs" */
#define STATFS_4ARGS
#define STATFS_BLOCK_SIZE	512
#define STATFS_BLOCK_FREE	fs.f_bfree
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#endif

#ifdef SCO
#include <sys/statfs.h>         /* "struct statfs" */
#define STATFS_4ARGS
#define STATFS_BLOCK_SIZE	fs.f_bsize
#define STATFS_BLOCK_FREE	fs.f_bfree
#define STATFS_BLOCK_TOTAL	fs.f_blocks
#endif

#ifdef HAS_MNTENT_H
#include <mntent.h>
#endif

	char act_uid[16];               /* actual name of file uid      */
	char act_gid[16];               /* actual name of file gid      */

        char SymLinkName[PATH_MAX+1];
	void printFileSystemInfo( void );


extern  BOOLEAN err_active;

/* ------------------------------------------------------------------------- */

static void printtime( struct tm *p_tm )
{
        int nb, lt = FrightWinCols - 15;
	char bl3[4];

        if( lt >= 3 )
          nb = lt - 3;
        else
          nb = lt;
        if( nb < 0 || nb > 3 )
         nb = 1;
        strncpy( bl3, "   ", nb );
        bl3[nb] = '\0';
        printf(bl3);
          
	printdate(p_tm);
	printf(" %02d:%02d",p_tm->tm_hour,p_tm->tm_min);
        if( lt >= 3 )
	  printf(":%02d",p_tm->tm_sec);
}

void printdate(p_tm)
struct tm *p_tm;
{
	static char *month[] = {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
	};
	printf("%s,%02d %02d",month[p_tm->tm_mon]
			     ,p_tm->tm_mday,p_tm->tm_year%100);
}

/* ------------------------------------------------------------------------- */

BOOLEAN setact(what)
char what;
{
	struct passwd *pw;
	struct group *gr;
	char name[15];
	register char *p;
#if defined QNX4 || defined QNX6
    unsigned short f_mode;
#else
	ushort f_mode;
#endif
	int nact,num,row,rows,numrdl;
	char fmtstr[32];

        /* check the terminal lines and cols */
        if( TermLines < TERM_MIN_LINES || TermCols < TERM_MIN_COLS )
        {
          return( TRUE );
        }

	num = numd + numf;
	rows = FileLstRows / scrn;
	nact = acti;

	switch (what)
	{
	  case '.':  break;
	  case 'A':  if ( acti > 0   )    nact--;
	             if ( nact < scr )    listfiles('-');
		     break;
	  case 'B':  if ( acti < num-1 )  nact++;
		     if (nact >= rows*(TermLines-FLINES)+scr) listfiles('+');
		     break;
	  case 'C':  if( ( acti == rows*(TermLines-FLINES)+scr-1 ) &&
			 ( acti < num-1 ) )
		     { /* lower right corner */
                       nact++;
		       listfiles('+');
		       break;
		     }
		     if ((nact +=(TermLines-FLINES)) > num-1) nact=num-1;
		     if (nact >= rows*(TermLines-FLINES)+scr) nact=rows*(TermLines-FLINES)+scr-1;
		     break;
	  case 'D':  if( ( acti == scr ) && ( acti > 0 ) )
		     { /* upper left corner */
		       nact--;
		       listfiles('-');
		       break;
		     }
		     if ((nact -= (TermLines-FLINES)) < scr ) nact=scr;
		     break;

	  case 'F':  nact=1;
	             if ( scr )
		     {
		       acti = 1;
		       scr = 0;
		       listfiles('.');
		       return(FALSE);
		     }
		     break;
	  case 'L':  nact=num-1;
	             if ( scr < num-num%((TermLines-FLINES)*rows) )
		     {
		       acti = nact;
		       scr = num-num%((TermLines-FLINES)*rows);
		       listfiles('.');
		       return(FALSE);
		     }
		     break;
	  case 'S':  break;
	  default :  return(TRUE);
	}

	row = (acti-scr)/(TermLines-FLINES);
	if (actlr == 'r')
        {
	  if( FileLstRows == 2 )
	    row += 1;
	  else
	    row += 2;
	}
	gotoxy(row*(FileLstNameLen+1)+(FileLstRows==2?row+3:3)+row/2,(acti-scr)%(TermLines-FLINES)+3);

	if ( what == 'S' )              /* old position underlined      */
	{
		printf(A_OTHER);
		prt_str(0,FileLstNameLen,p_ls[acti]->f_name);
		printf(A_NORMAL);
		if (actlr=='l') gotoxy(2,2);
		    else        gotoxy((FileLstNameLen+1)*FileLstRows/2+3,2);
                printf("%s%3d%s",A_NOFMFILES,acti+1,A_NORMAL);
		return(TRUE);
	}

	if ( what != '.' && what != 'S' && nact != acti)
	{
		prt_str(0,FileLstNameLen,p_ls[acti]->f_name);  /* old pos normal*/
		acti = nact;                       /* set new acti      */
		if (actlr == 'r') actir = acti;
		else              actil = acti;
		row = (acti-scr)/(TermLines-FLINES);
		if (actlr == 'r')
                {
	          if( FileLstRows == 2 )
	            row += 1;
	          else
	            row += 2;
	        }
		gotoxy(row*(FileLstNameLen+1)+(FileLstRows==2?row+3:3)+row/2,(acti-scr)%(TermLines-FLINES)+3);
	}
	printf(A_ACTUAL);
	prt_str(0,FileLstNameLen,p_ls[acti]->f_name);
	printf(A_NORMAL);

	if (actlr=='l') gotoxy(2,2);
		else    gotoxy((FileLstNameLen+1)*FileLstRows/2+3,2);
        printf("%s%3d%s",A_NOFMFILES,acti+1,A_NORMAL);
	gotoxy(row*(FileLstNameLen+1)+(FileLstRows==2?row+3:3)+row/2,(acti-scr)%(TermLines-FLINES)+3);

	if (what != '.' || cready() ) return(FALSE);

	gotoxy(TermCols-(FrightWinCols-1),max(TermLines-12,8));
	printf(A_ACTFILENAME);
	prt_str(-(FrightWinCols-2),(FrightWinCols-2),p_ls[acti]->f_name);
	printf(A_NORMAL);
	gotoxy(TermCols-1,max(TermLines-12,8));
        if( strlen( p_ls[acti]->f_name ) > (FrightWinCols-2) )
	  printf("%s>%s",A_TITLE,A_NORMAL);
        else
	  printf("%s<%s",A_TITLE,A_NORMAL);
        
	p = name;
					     /* get file_status */
	*SymLinkName='\0';

#ifdef S_IFLNK
	/* get the filestatus ( symbolic link possible ) */
	if( lstat( p_ls[acti]->f_name, &p_ls[acti]->f_st ) != 0 )
        {
          stat( p_ls[acti]->f_name, &p_ls[acti]->f_st );
        }
        if( ( p_ls[acti]->f_st.st_mode & S_IFMT ) == S_IFLNK )
        {
	  sprintf(fmtstr,"%%s-> %%-%d.%ds%%s",FrightWinCols-3,FrightWinCols-3);

          /* was it a symbolic link, then get the destination name */
	  if( ( numrdl = readlink( p_ls[acti]->f_name, SymLinkName, PATH_MAX ) ) < 0 )
	  {
            gotoxy(TermCols-FrightWinCols,max(TermLines-10,10)); /* place for byte_count */
            printf(fmtstr,A_ACTFILEDATA,"  ???",A_NORMAL);
	    p_ls[acti]->f_st.st_size = -1;
	  }
	  else
	  {
            SymLinkName[numrdl] = '\0';         /* readlink in 'SCO' don't terminate the string ! */

            /* Get the status of the file where the symbolic link points to.
	     A symbolic link may point to another symbolik link ! */
	    if( lstat( SymLinkName, &p_ls[acti]->f_st ) < 0 )
	    {
              gotoxy(TermCols-FrightWinCols,max(TermLines-10,10)); /* place for byte_count */
              p_ls[acti]->f_st.st_size = -1;

              switch( errno )
              {
                case ENOTDIR :
                case ENOENT :    /* Broken link */
                            printf(fmtstr,A_FAILURE,"BROKEN LINK",A_NORMAL);
                            break;
                case EACCES :
                            printf(fmtstr,A_FAILURE,"ACCESS ERROR",A_NORMAL);
                            break;

#if ( ELOOP != ENOENT )	/* for Motorola SysV68 */
                case ELOOP :      /* Too many symbolic links */
                            printf(fmtstr,A_FAILURE,"LINK LOOP ERROR",A_NORMAL);
                            break;
#endif

                default :
                            printf(fmtstr,A_FAILURE,"LINK ERROR",A_NORMAL);
                            break;
              }
	    }
	    else
            {
              /* show the actual file name through the link.
                 This may be a symbolik link again ! */
              char *basenam = strrchr( SymLinkName, '/' );
              if( basenam == NULL )
                basenam = SymLinkName;
              else
                basenam++;
              gotoxy(TermCols-(FrightWinCols-1),max(TermLines-12,8));
              printf(A_ACTFILENAME);
	      prt_str(-(FrightWinCols-2),FrightWinCols-2,basenam);
	      printf(A_NORMAL);
            }
          }
        }
#else
        stat( p_ls[acti]->f_name, &p_ls[acti]->f_st );
#endif

	if( !err_active )
	  mesg_line();            /* overwrite the last SymLinkName */

	f_mode = p_ls[acti]->f_st.st_mode;
	gotoxy(TermCols-10,max(TermLines-11,9));        /* write file_status    */
	switch ( f_mode & S_IFMT ) {
		case S_IFDIR :  *p++ = 'd';   break;	/* directory	 */
		case S_IFBLK :  *p++ = 'b';   break;	/* block special */
		case S_IFCHR :  *p++ = 'c';   break;	/* char special	 */
		case S_IFIFO :  *p++ = 'p';   break;	/* fifo		 */
#ifdef S_IFSOCK
 		case S_IFSOCK : *p++ = 's';   break;    /* socket	 */
#endif
#ifdef S_IFLNK
		case S_IFLNK :  *p++ = 'l';   break;    /* symbolic link */
#endif
#ifdef S_IFNAM
		case S_IFNAM :  *p   = 'n';             /* Special named file */
#if defined (QNX4) && defined (_S_RSVD_2)
                  if( p_ls[acti]->f_st.st_rdev == _S_RSVD_2 )
                    *p = 'm';                           /* Shared memory in QNX4 */
#endif
#if defined (QNX6) && defined (S_INSHD)
                  if( p_ls[acti]->f_st.st_rdev == S_INSHD )
                    *p = 'm';                           /* Shared memory in QNX6 */
#endif
#if defined (QNX6) && defined (S_INMQ)
                  if( p_ls[acti]->f_st.st_rdev == S_INMQ )
                    *p = 'q';                           /* Message Queue in QNX6 */
#endif
                  p++;  break;
#endif
		case S_IFREG :  *p++ = '-';   break;    /* regular	 */
		default :       *p++ = '?';   break;    /* unkwown	 */
	}
						/* owner permissions */
	if (f_mode & S_IREAD)   *p++ = 'r'; else *p++ = '-';
	if (f_mode & S_IWRITE)  *p++ = 'w'; else *p++ = '-';
	if (f_mode & S_ISUID )    /* set user id on execution */
	{   if (f_mode & S_IEXEC ) *p++ = 's'; else *p++ = 'S'; }
	else
	{   if (f_mode & S_IEXEC ) *p++ = 'x'; else *p++ = '-'; }

						/* group permissions */
	if (f_mode & S_IREAD>>3)  *p++ = 'r'; else *p++ = '-';
	if (f_mode & S_IWRITE>>3) *p++ = 'w'; else *p++ = '-';
	if (f_mode & S_ISGID )    /* set group id on execution */
	{   if (f_mode & S_IEXEC>>3 ) *p++ = 's'; else *p++ = 'S'; }
	else
	{   if (f_mode & S_IEXEC>>3 ) *p++ = 'x'; else *p++ = '-'; }

						/* other permissions */
	if (f_mode & S_IREAD>>6)  *p++ = 'r'; else *p++ = '-';
	if (f_mode & S_IWRITE>>6) *p++ = 'w'; else *p++ = '-';
#ifndef T32
	if (f_mode & S_ISVTX )    /* save swapped text even after use */
	{   if (f_mode & S_IEXEC>>6 ) *p++ = 't'; else *p++ = 'T'; }
	else
#endif
	{   if (f_mode & S_IEXEC>>6 ) *p++ = 'x'; else *p++ = '-'; }

	*p = '\0';
	printf("%s%s%s",A_ACTFILEDATA,name,A_NORMAL);

	gotoxy(TermCols-FrightWinCols,max(TermLines-10,10));
	if ( name[0] != 'b' && name[0] != 'c' )
	{
	  if( p_ls[acti]->f_st.st_size >= 0 )
          {
#if _FILE_OFFSET_BITS == 64
	    sprintf(fmtstr,"bytes:%%s%%%dllu%%s",FrightWinCols-6);
#else
	    sprintf(fmtstr,"bytes:%%s%%%dlu%%s",FrightWinCols-6);
#endif
	    printf(fmtstr,A_ACTFILEDATA,p_ls[acti]->f_st.st_size,A_NORMAL);
	  }
	}
	else
	{
            char dev_mm[64];
	    sprintf(fmtstr,"dev: %%s%%%d.%ds%%s", FrightWinCols-5, FrightWinCols-5 );
#ifdef major
            /* major is a macro usually in /usr/include/sys/types.h */
	    sprintf(dev_mm,"%d,%d", major( p_ls[acti]->f_st.st_rdev ),
	        		    minor( p_ls[acti]->f_st.st_rdev ) );
#else
	    sprintf(dev_mm,"%d,%d", (p_ls[acti]->f_st.st_rdev >> 8) & 0xff,
			             p_ls[acti]->f_st.st_rdev & 0xff );
#endif
            printf( fmtstr, A_ACTFILEDATA,dev_mm,A_NORMAL);
	}
	gotoxy(TermCols-(FrightWinCols-4),max(TermLines-9,11));
	if ((pw = getpwuid(p_ls[acti]->f_st.st_uid)) == NULL)
		sprintf(act_uid,"%d",p_ls[acti]->f_st.st_uid);
	else    sprintf(act_uid,"%.16s",pw->pw_name);   /* file_uid     */

        printf(A_ACTFILEDATA);
	prt_str(FrightWinCols-4,FrightWinCols-4,act_uid);
        printf(A_NORMAL);

	gotoxy(TermCols-(FrightWinCols-4),max(TermLines-8,12));
	if ((gr = getgrgid(p_ls[acti]->f_st.st_gid)) == NULL)
		sprintf(act_gid,"%d",p_ls[acti]->f_st.st_gid);
	else    sprintf(act_gid,"%.16s",gr->gr_name);   /* file_gid     */
        printf(A_ACTFILEDATA);
	prt_str(FrightWinCols-4,FrightWinCols-4,act_gid);
        printf(A_NORMAL);

	if( TermLines > 17 )
        {
          gotoxy(TermCols-(FrightWinCols-6),max(TermLines-7,13));
          sprintf(fmtstr,"%%%dd",FrightWinCols-6);
          printf(A_ACTFILEDATA);
	  printf(fmtstr,p_ls[acti]->f_st.st_nlink);
          printf(A_NORMAL);
	}
	if( TermLines > 18 )
        {
	  gotoxy(TermCols-(FrightWinCols-6),max(TermLines-6,14));
          sprintf(fmtstr,"%%%dd",FrightWinCols-6);
          printf(A_ACTFILEDATA);
	  printf(fmtstr,p_ls[acti]->f_st.st_ino);
          printf(A_NORMAL);
	}

	if( TermLines <= 15 )
        {
	  gotoxy(TermCols-FrightWinCols,max(TermLines-2,13));
          printf(A_ACTFILEDATA);
	  printtime(localtime(&p_ls[acti]->f_st.st_mtime)); /* last modifi */
          printf(A_NORMAL);
        }
	if( TermLines > 15 )
        {
	  gotoxy(TermCols-FrightWinCols,max(TermLines-4,13));
          printf(A_ACTFILEDATA);
	  printtime(localtime(&p_ls[acti]->f_st.st_atime)); /* last access */
          printf(A_NORMAL);
	  gotoxy(TermCols-FrightWinCols,max(TermLines-3,14));
          printf(A_ACTFILEDATA);
	  printtime(localtime(&p_ls[acti]->f_st.st_mtime)); /* last modifi */
          printf(A_NORMAL);
        }
	if( TermLines > 16 )
        {
	  gotoxy(TermCols-FrightWinCols,max(TermLines-2,15));
          printf(A_ACTFILEDATA);
	  printtime(localtime(&p_ls[acti]->f_st.st_ctime)); /* last change */
          printf(A_NORMAL);
        }
	if( TermLines > 23 )
	{
	  gotoxy(TermCols-FrightWinCols,max(TermLines-14,6));
	  printFileSystemInfo();
	}
 	gotoxy(row*(FileLstNameLen+1)+(FileLstRows==2?row+3:3)+row/2,(acti-scr)%(TermLines-FLINES)+3);
	save_cursor();

#ifdef S_IFLNK
	/* was it a symbolic link, then do now the stat for the file,
	   so that the file will be used for the next commands */
/* the stat will get the file through the symbolic link(s),
   but the fids will show the status only through the 1. link ! */
        if( ( f_mode & S_IFMT ) == S_IFLNK )
	    stat( p_ls[acti]->f_name, &p_ls[acti]->f_st );
#endif
	fflush(stdout);
	return(TRUE);
}

void printSymLinkName( void )
{
        int len = strlen( SymLinkName );

	if( len > ( TermCols - 6 ) )
        {
          strcpy( &SymLinkName[ TermCols - 9 ], "..." );
          len = TermCols - 6;
        }
	gotoxy( TermCols -len -4, TermLines - 1 );

        printf("%s -> %s",A_SYMLINKMARK,A_NORMAL);
	printf(A_SYMLINKNAME);
	prt_str( -len, len, SymLinkName );
	printf(A_NORMAL);
}

void printFileSystemInfo( void )
{
        char *file = ".";
	FILE *mntp;
#ifdef HAS_MNTENT_H
	struct mntent *mnt;
	struct stat filestat, dirstat;
#endif
#ifdef ULTRIX
	struct fs_data fs;
#else
	struct statfs fs;
#endif
	float free=0.;
	int  freepro;
	char freec;
	char *psc;
	char fmtstr[32];

#ifdef QNX4
	char mnt_fs_dev[PATH_MAX];
	char mnt_fs_pt[PATH_MAX];
#endif

	strcpy( fmtstr, "free" );

	/* Get Filesystem status */

#ifdef STATFS_4ARGS
	if( statfs( file, &fs, sizeof(fs), 0 )  < 0 )
	  return;
#else
	if( statfs( file, &fs ) < 0 )
	  return;
#endif

#if defined QNX6
	strncpy( fmtstr, fs.f_basetype, sizeof(fmtstr)-1 );
	if (fs.f_flag & ST_RDONLY)
	  strncat( fmtstr, "-r", sizeof(fmtstr)-strlen(fmtstr)-1 );
#endif

#ifdef QNX4
	fsys_get_mount_dev( file, mnt_fs_dev );
	fsys_get_mount_pt( mnt_fs_dev, mnt_fs_pt );
	strncpy( fmtstr, mnt_fs_pt, sizeof(fmtstr)-1 );
#endif

#ifdef OSF
	strncpy( fmtstr, fs.f_mntonname, sizeof(fmtstr)-1 );
#else
#ifdef ULTRIX
	strncpy( fmtstr, fs.fd_path, sizeof(fmtstr)-1 );
#else

#ifdef HAS_MNTENT_H

	/* Get Mount info for this Filesystem */

	if( stat( file, &filestat ) < 0 )
	  return;

	if( ( mntp = setmntent(MOUNTED, "r") ) == 0 )
	  return;

	while( ( mnt = getmntent( mntp ) ) != 0 )
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

	strncpy(fmtstr,mnt->mnt_dir,sizeof(fmtstr)-1);

#endif  /* HAS_MOUNT_ENT_H      */
#endif  /* ULTRIX  */
#endif  /* OSF     */

	fmtstr[sizeof(fmtstr)-1] = '\0';
	if( strlen( fmtstr ) > FrightWinCols-10 )
	{
	  prt_str( -(FrightWinCols-9), FrightWinCols-9, "free:" );
	}
	else
	{
	  strcat(fmtstr,":");
	  printf(A_ACTFILEDATA);
	  prt_str( -(FrightWinCols-9), FrightWinCols-9, fmtstr );
	  printf(A_NORMAL);
	}

        if( STATFS_BLOCK_FREE < 0 )
          STATFS_BLOCK_FREE = 0;
        if( STATFS_BLOCK_TOTAL > 0 )
	  freepro = (int)((float)STATFS_BLOCK_FREE * 100. / (float)STATFS_BLOCK_TOTAL);
        else
	  freepro = 0;
	if( freepro > 99 )
	  freepro = 99;

	free = (float)STATFS_BLOCK_FREE * (float)STATFS_BLOCK_SIZE / 1024.;

	freec = 'K';
	if( free > 999. )
	{
	  free /= 1024.;
	  freec = 'M';
	}
	if( free > 999. )
	{
	  free /= 1024.;
	  freec = 'G';
	}

	printf(A_ACTFILEDATA);

	if( free < 10. )
	  printf("%3.1f", free );
	else
	  printf("%3.0f", free );

	printf("%s%cB=%s%2d%s%%\n", A_NORMAL, freec,
	    A_ACTFILEDATA, freepro, A_NORMAL );
	  
	return;
}
