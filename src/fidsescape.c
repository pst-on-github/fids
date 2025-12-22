/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: escape							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/* execution for escape commands                                        */
/************************************************************************/
#include "fids.h"
#include "fidsext.h"
/************************************************************************/

extern  char pf_ctl[PFS][PFK_C];       /* field for the pf_ctl commands */
extern	char pf_cctl[PFS][PFK_C];      /* field for the pf_ctl for Cmode */
extern	char pf[PFK][PFK_C];           /* field for the pf_key commands */
extern	char pf_s[PFK][PFK_C];         /* field for the shift pf_key commands */
extern	char pf_nam[PFK][PFN+1];       /* field for the pf_command names*/

	extern char dir_n_l[];
	extern char dir_n_r[];
	extern char dir_n_s[];
	extern char *p_cwd;
	extern char *p_cmdl[];
	extern char act_uid[];
	extern char act_gid[];

	extern char smodl;
	extern char smodr;
	extern char flogin[];
	extern unsigned time_def;
        extern char filespec[];
        extern char exclspec[];

static	char pf_path[NDIR];             /* buffer for FK_FILE path name */

#define	LINE_LEN ( CMDL_C > 2048 ? CMDL_C : 2048 )

/* ------------------------------------------------------------------------- */
static int PF_load( char *l_path )      /* load PF_keys from the file l_path */
{
        int n,how;
	char buff[LINE_LEN],*z,*p1,*pn, rem_char=';';
        FILE *fp;

	how = 0;
	if ( !(*l_path) )                               /* given name ? */
	{
	    l_path = FK_FILE;           /* take the default name        */
	    how = 1;
	}
	if ( (fp = fopen(l_path,"r") ) == NULL )
	{
	    if (how)
	      error("can't open file : ",l_path);
            else
              *l_path = '\0';
	    return(-1);
	}

        /* read the first line, this should be always the headline */
        if ( fgets( buff, LINE_LEN-1, fp ) == NULL )
	{
          fclose(fp);
	  if (how) error("unexpected end of file ! : ",l_path);
          else *l_path = '\0';
	  return(-1);
        }
        if( buff[0] == '#' )
          rem_char=buff[0];


	while ( fgets( buff, LINE_LEN-1, fp ) != NULL )
	{
          z=buff;
          if( *z == rem_char )
            continue;                   /* this is a remark line */

          p1 = pn = NULL;

          switch( *z )
          {
            case '.' :

              z+=2;

              if( *z >= '1' && *z <= '9' && *(z+1) == '=' )
              {
                switch( *(z-1) )
                {
                  case 'C' :	/* ".Cn=" with n 1..9  for PF_command_strings */

                    p1 = pn = pf[(*z)-'1'];
                    for( z+=2, n=PFK_C-1 ;
                      *z!='\n' && *z!='\t' && *z!=rem_char && n>0 ;
                        z++, p1++, n-- )
                    {
                      *p1 = *z;
                      if ( *z == '^' && *(z+1) > '@' )
                      {
                        *p1 = *(++z) & 037;
                        /* avoid recursivity */
                        if( ( *p1==ESC ) && ( isdigit( *(z+1) ) ) )
                        { p1--; z++; }
                      }
                    }
                    *p1-- = '\0';
                    break;

                  case 'c' :	/* ".cn=" with n 1..9  for shift PF_command_strings */

                    p1 = pn = pf_s[(*z)-'1'];
                    for( z+=2, n=PFK_C-1 ;
                      *z!='\n' && *z!='\t' && *z!=rem_char && n>0 ;
                        z++, p1++, n-- )
                    {
                      *p1 = *z;
                      if ( *z == '^' && *(z+1) > '@' )
                      {
                        *p1 = *(++z) & 037;
                        /* avoid recursivity */
                        if( ( *p1==ESC ) && ( isdigit( *(z+1) ) ) )
                        { p1--; z++; }
                      }
                    }
                    *p1-- = '\0';
                    break;

                  case 'L' :	/* ".Ln=" with n 1..9  for PF_label_strings */

                    p1 = pn = pf_nam[(*z)-'1'];
                    for( z+=2, n=PFN ;
                      *z!='\n' && *z!='\t' && *z!=rem_char && n>0 ;
                        z++, p1++, n-- )
                    {
                      *p1 = *z;
                    }
                    *p1-- = '\0';
                    break;

                  case 'H' :	/* ".Hn=" with n 1..9  for HISTORY strings */

                    p1 = pn = p_cmdl[(*z)-'0'];
                    for( z+=2, n=CMDL_C ;
                      isprint(*z) && n>0 ; z++, p1++, n-- )
                    {
                      *p1 = *z;
                    }
                    *p1-- = '\0';
                    break;
                }
              }
              else if( *z >= '@' && *z <= '_' && *(z+1) == '=' )
              {
                switch( *(z-1) )
                {
                  /* ".^*=" with *= @ to _  for <^*>_command_strings DispMode */

                  case '^' :

                    p1 = pn = pf_ctl[(*z)-'@'];
                    for( z+=2, n=PFK_C-1 ;
                      *z!='\n' && *z!='\t' && *z!=rem_char && n>0 ;
                        z++, p1++, n-- )
                    {
                      *p1 = *z;
                      if ( *z == '^' && *(z+1) > '@' )
                      {
                        *p1 = *(++z) & 037;
                      }
                    }
                    *p1-- = '\0';
                    break;


                  /* ".D*=" with *= @ to _  for D? saved directory names */

                  case 'D' :

                    p1 = pn = NULL;
                    switch( *z )
                    {
                      case 'I' : p1 = filespec;       /* incl. filespec */
                                 break;
                      case 'E' : p1 = exclspec;       /* excl. filespec */
                                 break;
                      case 'L' : p1 = dir_n_l;         /* dir_name_left */
                                 break;
                      case 'R' : p1 = dir_n_r;        /* dir_name_right */
                                 scrn = 2;
                                 actlr = 'l';    /* .DA should follow ! */
                                 break;
                      case 'S' : p1 = dir_n_s;        /* dir_name_saved */
                                 break;
                    }
                    if( p1 != NULL )
                    { /* do the directory name copy */
                      for( z+=2, n=NDIR;
                        isprint(*z) && n>0 ; z++, p1++, n-- )
                      {
                        *p1 = *z;
                      }
                      *p1-- = '\0';
                      break;
                    }
                    break;


                  /* ".S*=" with *= @ to _  for N? saved numbers */

                  case 'S' :

                    p1 = pn = NULL;
                    switch( *z )
                    {
                      case 'A' :
                        actlr = *(z+2);
                        if( actlr != 'l' && actlr != 'r' )
                          actlr = 'l';
                        break;

                      case 'L' :
                        smodl = *(z+2);              /* sort modus left */
                        break;

                      case 'R' :
                        smodr = *(z+2);             /* sort modus right */
                        break;
                    }
                    break;


                  /* ".N*=" with *= @ to _  for N? saved numbers */

                  case 'N' :

                    p1 = pn = NULL;
                    switch( *z )
                    {
                      case 'R' :                      /* number of rows */
                        FileLstRows = atoi(z+2);
                        FileLstRows = max( FileLstRows, 2 );
                        if( FileLstRows > 2 )
                          FileLstRows = 4;
                        break;

                      case 'T' :                     /* alarm time      */
                        time_def = atoi(z+2);
                        time_def = min( max( time_def, 2 ), CLK_RATE );
                        break;

                    }
                    break;
                }
              }
              break;


            case '_' :

              z+=2;

              if( *z >= '@' && *z <= '_' && *(z+1) == '=' )
              {
                switch( *(z-1) )
                {
                  /* "_^*=" with *= @ to _  for <^*>_command_strings CmdMode */

                  case '^' :

                    p1 = pn = pf_cctl[(*z)-'@'];
                    for( z+=2, n=PFK_C-1 ;
                      *z!='\n' && *z!='\t' && *z!=rem_char && n>0 ;
                        z++, p1++, n-- )
                    {
                      *p1 = *z;
                      if ( *z == '^' && *(z+1) > '@' )
                      {
                        *p1 = *(++z) & 037;
                      }
                    }
                    *p1-- = '\0';
                }
              }
              break;

          }
          /* delete blanks from the end of line */
          while( p1 > pn && *p1 == ' ' )
            *p1-- = '\0';
	}

        fclose(fp);
        PF_label(1);

	if( *pf_ctl[0] )		/* is .^@ user defined ? 	*/
	    push_s(pf_ctl[0]);		/* push the str to input buffer */
	*pf_ctl[0] = '\0';		/* do the .^@ only once here !	*/

	return(0);
}

/* ------------------------------------------------------------------------- */

int FS_save( char *name )                            /* save actual settings */
{
        int n, err=0;
	char buff[LINE_LEN], *z;
        FILE *fp;

	z = getenv( "HOME" );
	if( z == NULL )
	  return(-1);

	sprintf( pf_path, "%s/%s_%s", z, FS_FILE, name!=NULL?name:flogin );

	if( ( fp = fopen( pf_path, "w" ) ) == NULL )
	  return(-1);

        /* write the first line, this is the headline */

        if ( fputs( "# === FIDS SAVED SETTINGS ===\n", fp ) < 0 )
          err++;

	sprintf( buff, ".DL=%s\n", dir_n_l );          /* dir_name_left */
        if ( fputs( buff, fp ) < 0 )
          err++;

        sprintf( buff, ".SL=%c\n", smodl );            /* sort modus    */
        if ( fputs( buff, fp ) < 0 )
          err++;

        if( scrn == 2 )
        {
          sprintf( buff, ".DR=%s\n", dir_n_r );       /* dir_name_right */
          if ( fputs( buff, fp ) < 0 )
            err++;

          sprintf( buff, ".SR=%c\n", smodr );         /* sort modus     */
          if ( fputs( buff, fp ) < 0 )
            err++;

          sprintf( buff, ".SA=%c\n", actlr );         /* actual side    */
          if ( fputs( buff, fp ) < 0 )
            err++;
        }

        sprintf( buff, ".DS=%s\n", dir_n_s );         /* dir_name_saved */
        if ( fputs( buff, fp ) < 0 )
          err++;

        sprintf( buff, ".DI=%s\n", filespec );        /* incl. filespec */
        if ( fputs( buff, fp ) < 0 )
          err++;

        sprintf( buff, ".DE=%s\n", exclspec );        /* excl. filespec */
        if ( fputs( buff, fp ) < 0 )
          err++;

        sprintf( buff, ".NR=%d\n", FileLstRows );     /* number of rows */
        if ( fputs( buff, fp ) < 0 )
          err++;

        sprintf( buff, ".NT=%d\n", time_def );       /* alarm time      */
        if ( fputs( buff, fp ) < 0 )
          err++;

	for( n=1; n < 10; n++ )
	{
	  sprintf( buff, ".H%d=%s\n", n, p_cmdl[n] );
          if ( fputs( buff, fp ) < 0 )
            err++;
	}

        fclose(fp);
	if( err != 0 )
	{
	  unlink( pf_path );
	  return(-1);
        }

	return(0);
}

/* ------------------------------------------------------------------------- */

int FS_load( char *name )               /* restore previously saved settings */
{
	char *z;

	z = getenv( "HOME" );
	if( z == NULL )
	  return(-1);

	sprintf( pf_path, "%s/%s_%s", z, FS_FILE, name!=NULL?name:flogin );

	return( PF_load( pf_path ) );
}

/* ------------------------------------------------------------------------- */

int escape(c)                           /* escape functions             */
int c;
{
	int pos,help,p_end;
	char *z1,*z2,sbe[2];

	save_cursor();
	help = 0;
	if( c == ESC )			/* one single Escape Key */
	{
	    help = 1;

	    gotoxy(1,TermLines-1);
	    printf(" %sh%s%selppage%s  %sl%s%soad pf_keys%s",
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL,
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL );
	    printf("  %s dir:%s %ss%s%save%s %sr%s%sestore%s",
		   A_ESCHELPKEY,A_NORMAL, /* dirs */
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL,   /*save*/
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL ); /*restore*/
	    printf("  %s change file:%s %sn%s%same%s %su%s%sid%s %sg%s%sid%s %sm%s%sode%s %s?%s%s",
		   A_ESCHELPKEY,A_NORMAL, /*change file*/
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL, /*name*/
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL, /*uid*/
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL, /*gid*/
		   A_ESCHELPKEY,A_NORMAL,A_ESCHELP,A_NORMAL, /*mode */
		   A_ESCHELPKEY,A_NORMAL,DEL_EOL);
	    fflush(stdout);
	    c = get_key();		/* get the next key */
	}

	pos = 0;
	switch (c)
	{
	    case 'q':	cmd_askexit();
	    		return(0);

	    case 'h':   restore_cursor();
			help_page('d');
	    		return(0);
			
	    case 'l':
		    sprintf(pf_path,"./%s",FK_FILE);
		    if ( PF_load(pf_path) )  /* search first in the cwd */
		    {
					     /* search then in $HOME    */
			if ( (z1=getenv("HOME") ) )
			{
				sprintf(pf_path,"%s/%s",z1,FK_FILE);
				if ( !PF_load(pf_path) )
				{
				    restore_cursor();
				    if ( help ) mesg_line();
				    return(0);
				}
			}
					/* search FK_FILE down the PATH */
			if ( (z1=getenv("PATH") ) )
			{
			    for ( p_end=1; p_end ; z1 = ++z2 )
			    {
				z2 = strchr(z1,':');
				if ( z2 == z1 ) continue;
				if ( z2 == NULL )
				{ z2 = z1 + strlen(z1); p_end = 0; }
				strncpy(pf_path,z1,z2-z1);
				pf_path[z2-z1] = '/';
				pf_path[z2-z1+1] = '\0';
				strcat(pf_path,FK_FILE);
				if ( !PF_load(pf_path) ) break;
			    }
			}
		    }
		    restore_cursor();
		    if ( help ) mesg_line();
		    return(0);

	    case 'p': /* print pf_key path in cmd_line ( e.g. to edit it ) */

                    push_s(pf_path);
                    return(0);


	    case 'r':
			restore_cursor();
			cmd_cd(dir_n_s,'b');            /* restore dir  */
#ifdef hds
			escape('l');		        /* load PF_keys */
#endif
			return(0);

	    case 's':
			strcpy(dir_n_s,p_cwd);  /* save actual dir_name */
			restore_cursor();
			if ( help ) mesg_line();
			return(0);

	    /* file change functions */
	    case 'n':
                        sbe[0] = fndelim( p_ls[acti]->f_name );
                        sbe[1] = '\0';
                        if( *sbe )
			  sprintf(p_cmdl[0],"mv %c%s%c %c", *sbe, p_ls[acti]->f_name, *sbe, *sbe);
                        else
			  sprintf(p_cmdl[0],"mv %s ", p_ls[acti]->f_name);
			pos = strlen(p_cmdl[0]) + 1;
			strcat(p_cmdl[0],p_ls[acti]->f_name);
                        if( *sbe )
			{
			  z1 = p_cmdl[0] + pos -1;
			  while( *z1 ) { if( *z1 <= ' ' ) { *z1 = '_'; } z1++; }
			  *z1 = *sbe;
 			}
			break;
	    case 'u':
	    case 'g':
	    case 'm':
                        sbe[0] = fndelim( p_ls[acti]->f_name );
                        sbe[1] = '\0';
			if      ( c == 'u')
			    sprintf(p_cmdl[0],"chown %s ",act_uid);
			else if ( c == 'g' )
			    sprintf(p_cmdl[0],"chgrp %s ",act_gid);
			else
			    sprintf(p_cmdl[0],"chmod %04o ",
					   p_ls[acti]->f_st.st_mode & 07777);
                        if( *sbe )
			  strcat(p_cmdl[0],sbe);
			strcat(p_cmdl[0],p_ls[acti]->f_name);
                        if( *sbe )
			  strcat(p_cmdl[0],sbe);
			pos = 7;
			break;
	    default :
			restore_cursor();
			if ( help ) mesg_line();
			return( c + ES1 );
	}
	restore_cursor();
	cmd_mode(pos);          /* goto cmd_mode to edit and execute    */
	return(0);
}

/************************************************************************/
void PF_label( int onoff )
{
  int i, len, shift, offset;
  char fmtstr[32];

  if ( onoff )
  {
    len = ( TermCols-8 ) / 9;
    gotoxy(1,TermLines);                /* PF_display in bottom line */

    offset = TermCols - ( len * 9 + 8 );
    shift = (offset + 1)/3;
    if( offset == 4 || offset > 5 )
    {
      putchar(' ');
    }

    sprintf(fmtstr,"%%1d%%s%%-%d.%ds%%s", len,len );
    printf(&fmtstr[3],A_PFKEYS,pf_nam[0],A_NORMAL);
    for(i=1; i<9; i++)
    {
      if( shift > 0 && ( i==3 || i== 6 ) )
        prt_str( -shift,shift, " " );

      printf(fmtstr,i+1,A_PFKEYS,pf_nam[i],A_NORMAL);
    }
    printf(DEL_EOL);
  }
}
