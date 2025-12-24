/************************************************************************/
/* programm: fids                               B.Noeggerath, BeNoSoft  */
/* funktion: view							*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
/* make a view to the current file                                      */
/************************************************************************/
#include "fids.h"
#include "fidsext.h"
#include <fcntl.h>
#if defined(S_IFNAM) && defined(__QNX__)
#include <sys/mman.h>
#endif

extern BOOLEAN stat_list;
extern int     FidsCloseOnInterrupt;

#define SC '.' /* output for non-ascii-character */
               /* length of the buffer           */
#define VIEWBUF ((((TermLines * TermCols) / 1024) + 1) * 1024)

int cmd_view() /* command execution for view     */
{
    /*lint -save -e794 Conceivable use of null pointer */
    unsigned char          *buf_beg, *z, *madr = NULL;
    register unsigned char *p;
    char                    mode = 'a'; /* 'a'=ascii,'h'=hex    */
    BOOLEAN                 typeahead;
    register int            c_line, line;
    register unsigned       num;
    int                     i, j, k, new, rest, c, fd;
    long                    f_size;
    long                    offset = 0L;

    /* check if the actual entry is a directory or a regular file */
    if (((p_ls[acti]->f_st.st_mode & S_IFMT) != S_IFDIR) &&
#if defined(S_IFNAM) && defined(__QNX__)
#if defined(QNX4) && defined(_S_RSVD_2)
        !(((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFNAM) &&
          (p_ls[acti]->f_st.st_rdev == _S_RSVD_2)) &&
#elif defined(QNX6) && defined(S_INSHD)
        !(((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFNAM) &&
          (p_ls[acti]->f_st.st_rdev == S_INSHD)) &&
#endif
#endif
        ((p_ls[acti]->f_st.st_mode & S_IFMT) != S_IFREG))
    {
        /* ask the user if he really wants this */
        save_cursor();
        gotoxy(1, TermLines - 1);

        printf("Are you shure, you want to read from %s", A_MESSAGE);
        if (strlen(p_ls[acti]->f_name) > 33)
        {
            prt_str(0, 30, p_ls[acti]->f_name);
            prt_str(0, 3, "...");
        }
        else
            prt_str(0, 33, p_ls[acti]->f_name);

        printf("%s [Yes=y]:%s", A_NORMAL, DEL_EOL);

        fflush(stdout);
        if (get_key() != 'y')
        {
            restore_cursor();
            mesg_line();
            return (0);
        }
        setraw(1); /* make it interruptable ! */
    }

    fd = ERR;
    if (*(p_ls[acti]->f_name) != '\0')
    {
#if defined(S_IFNAM) && defined(__QNX__)
#if defined(QNX4) && defined(_S_RSVD_2)
        if (((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFNAM) &&
            (p_ls[acti]->f_st.st_rdev == _S_RSVD_2))
#elif defined(QNX6) && defined(S_INSHD)
        if (((p_ls[acti]->f_st.st_mode & S_IFMT) == S_IFNAM) &&
            (p_ls[acti]->f_st.st_rdev == S_INSHD))
#else
        if (0)
#endif
        { /* Shared memory in QNX */
            fd = shm_open(p_ls[acti]->f_name, O_RDONLY, 0444);
            if (fd != -1)
            {
                /* access it for reading */
                madr = (char *)mmap(0, p_ls[acti]->f_st.st_size, PROT_READ,
                                    MAP_SHARED, fd, 0);
                close(fd);
                if ((int)madr == -1)
                {
                    fd   = ERR;
                    madr = NULL;
                }
            }
        }
        else
        {
            fd = open(p_ls[acti]->f_name, 0);
        }
#else
        fd = open(p_ls[acti]->f_name, 0);
#endif
    }
    if (fd == ERR)
    {
#if defined(S_IFNAM) && defined(__QNX__)
        if (madr != NULL)
            (void)munmap(madr, p_ls[acti]->f_st.st_size);
#endif
        save_cursor();
        error("can't open file ", p_ls[acti]->f_name);
        restore_cursor();
        setraw(0); /* set terminal into raw mode   */
        return (-1);
    }
    FidsCloseOnInterrupt = fd;

    f_size = p_ls[acti]->f_st.st_size;

    if (!(buf_beg = (unsigned char *)calloc(VIEWBUF, 1)))
        syserr("fids_view_malloc");

    gotoxy(1, 1);
    printf("%sFIDS_view: ^C=quit, TAB=hex_view  , else=next         file: %s",
           A_MESSAGE, DEL_EOL);
    if (strlen(p_ls[acti]->f_name) > TermCols - 59)
    {
        prt_str(0, TermCols - 63, p_ls[acti]->f_name);
        prt_str(0, 3, "...");
    }
    else
        prt_str(-TermCols - 60, TermCols - 60, p_ls[acti]->f_name);
    printf(A_NORMAL);

    num = 0;

    for (;;)
    {
        p = buf_beg;

        if (mode == 'a')
        {
            line = c_line = 0;
            typeahead     = FALSE;
            gotoxy(25, 1);
            printf("%shex_view  %s\r\n%s", A_MESSAGE, A_NORMAL, DEL_EOL);

            for (;; p++)
            {
                if (p >= buf_beg + num) /* p in old_range */
                {
                    if (madr == NULL)
                    {
                        if ((new = read(fd, (char *)p, VIEWBUF - num)) < 1)
                            mode = 'e'; /* file_end     */
                        else
                            num = num + new;
                    }
                    else
                    {
                        new = VIEWBUF - num;
                        if (new > (f_size - offset))
                            new = f_size - offset;
                        if (new < 1)
                            mode = 'e';
                        else
                        {
                            memcpy((char *)p, &madr[offset], new);
                            num = num + new;
                        }
                    }
                }
                if (*p != '\n' && mode != 'e')
                {
                    if (!typeahead)
                    {
                        if ((*p >= ' ' && *p <= '~') || *p == '\t')
                            putchar((int)*p);
                        else
                            putchar(SC);
                        if (!(c_line % 8))
                        {
                            if (cready())
                                typeahead = TRUE;
                        }
                    }
                    if (*p == '\t')
                        c_line += 8 - c_line % 8;
                    else
                        c_line++; /* char's per line */
                    if (c_line < TermCols)
                        continue;
                }
                c_line = 0; /* char is a CR */
                line++;     /* number of lines */
                if (!typeahead && cready())
                    typeahead = TRUE;
                if (line == (TermLines - 2) || mode == 'e')
                { /* screen _ end */
                    line = 0;
                    printf(DEL_EOS);
                    if (mode == 'e')
                    {
                        gotoxy(37, 1);
                        printf("%s  END OF FILE  %s", A_MESSAGE, A_NORMAL);
                    }
                    else if (!typeahead)
                    {
                        if (!f_size)
                        {
                            stat(p_ls[acti]->f_name, &p_ls[acti]->f_st);
                            f_size = p_ls[acti]->f_st.st_size;
                        }
                        if (f_size)
                        {
                            gotoxy(46, 1);
                            printf("(%2d%%)",
                                (int)((offset + (long)(p + 1 - buf_beg)) * 100 / f_size));
                        }
                    }
                    fflush(stdout);
                    typeahead = FALSE;
                    c         = get_key(); /* wait for any charater */
                    if (c == '\t')
                    {
                        mode = 'h';
                        break;
                    }
                    if (c == CTRL('C') || c == PFKY('3') || mode == 'e')
                    {
#if defined(S_IFNAM) && defined(__QNX__)
                        if (madr != NULL)
                            (void)munmap(madr, p_ls[acti]->f_st.st_size);
#endif
                        free((char *)buf_beg);
                        close(fd);
                        FidsCloseOnInterrupt = -1;
                        setraw(0); /* set terminal into raw mode   */
                        cls();
                        makescreen();
                        PF_label(1);
                        stat_list = fids_list('?', 'c');
                        return (0);
                    }
                    z = buf_beg; /* used if it comes from mode h */
                    p++;         /* point to next char   */
                    offset += (long)(p - z);
                    for (num -= (unsigned)(p - z), i = 0; i < num; i++)
                        *z++ = *p++; /* copy rest    */
                    p = buf_beg - 1;
                }
                if (!typeahead)
                {
                    gotoxy(1, line + 2);
                    printf(DEL_EOL);
                }

            } /* for(p) */

        } /* if (mode) */
        else /* hextype                      */
        {
            typeahead = FALSE;
            gotoxy(25, 1);
            printf("%sascii_view%s\r\n%s", A_MESSAGE, A_NORMAL, DEL_EOL);

            rest = offset & 0x0f; /* offset % 16 display  */
            line = TermLines - 3;
            if (rest)
                line++;
            for (j = 0; j < line; j++)
            {
                if (!typeahead && cready())
                    typeahead = TRUE;

                if (!typeahead)
                    printf("%06lX    ", offset - rest + j * 16);

                if (!j)
                    gotoxy((rest / 2) + (rest * 2) + 11, 2);
                z = p; /* save p in z          */
                for (i = j ? 0 : rest; i < 16; i++, p++)
                {
                    if (p >= buf_beg + num) /* read next char */
                    {
                        if (madr == NULL)
                        {
                            if ((new = read(fd, (char *)p, VIEWBUF - num)) <
                                1)
                            {
                                mode = 'e'; /* file_end     */
                                break;
                            }
                            else
                                num = num + new;
                        }
                        else
                        {
                            new = VIEWBUF - num;
                            if (new > (f_size - offset))
                                new = f_size - offset;
                            if (new < 1)
                            {
                                mode = 'e';
                                break;
                            }
                            else
                            {
                                memcpy((char *)p, &madr[offset], new);
                                num = num + new;
                            }
                        }
                    }
                    if (!typeahead)
                    {
                        printf("%02X", *p);
                        if (i & 1)
                            putchar(' ');
                    }
                }
                if (!typeahead && cready())
                    typeahead = TRUE;

                if (!typeahead)
                {
                    p = z;
                    gotoxy(60, j + 2);
                    if (!j)
                        gotoxy(60 + rest, 2);
                    for (k = j ? 0 : rest; k < i; k++, p++)
                    {
                        if (*p >= ' ' && *p <= '~')
                            putchar((int)*p);
                        else
                            putchar(SC);
                    }
                    if (mode == 'e')
                        break;
                    printf("\r\n%s", DEL_EOL);
                }
            }
            printf(DEL_EOS);
            if (mode == 'e')
            {
                gotoxy(37, 1);
                printf("%s  END OF FILE  %s", A_MESSAGE, A_NORMAL);
            }
            else if (!typeahead && f_size)
            {
                gotoxy(46, 1);
                printf("(%2d%%)",
                       (int)((offset + (long)(p + 1 - buf_beg)) * 100 / f_size));
            }
            fflush(stdout);
            typeahead = FALSE;
            c         = get_key(); /* wait for any charater */

            if (c == '\t')
            {
                mode = 'a';
                continue;
            }
            if (c == CTRL('C') || c == PFKY('3') || mode == 'e')
            {
#if defined(S_IFNAM) && defined(__QNX__)
                if (madr != NULL)
                    (void)munmap(madr, p_ls[acti]->f_st.st_size);
#endif
                free((char *)buf_beg);
                close(fd);
                FidsCloseOnInterrupt = -1;
                setraw(0); /* set terminal into raw mode   */
                cls();
                makescreen();
                PF_label(1);
                stat_list = fids_list('?', 'c');
                return (0);
            }
            gotoxy(1, 2);
            printf(DEL_EOL);

            z = buf_beg;
            offset += (int)(p - z);
            for (num -= (unsigned)(p - z), i = 0; i < num; i++)
                *z++ = *p++; /* copy rest    */
        } /* if (mode) */

    } /* for(;;) */

    /*lint -restore */
}
