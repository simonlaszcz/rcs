/*
 *                     RCS stream editor
 */
/**********************************************************************************
 *                       edits the input file according to a
 *                       script from stdin, generated by diff -n
 *                       performs keyword expansion
 **********************************************************************************
 */

/* Copyright (C) 1982, 1988, 1989 Walter Tichy
   Copyright 1990 by Paul Eggert
   Distributed under license by the Free Software Foundation, Inc.

This file is part of RCS.

RCS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

RCS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RCS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

Report problems and direct all questions to:

    rcs-bugs@cs.purdue.edu

*/


/* $Log: rcsedit.c,v $
 * Revision 5.8  1991/01/30  14:21:32  apratt
 * CI with RCS version 5
 *
 * Revision 5.7  91/01/29  17:45:46  apratt
 * Added AKP_BUGFIXES around my bug fixes
 * 
 * Revision 5.6  91/01/16  15:44:50  apratt
 * This version works passably on the ST.
 * 
 * Revision 5.5  90/12/30  05:07:35  eggert
 * checked in with -k by apratt at 91.01.10.13.15.10.
 * 
 * Revision 5.5  1990/12/30  05:07:35  eggert
 * Fix report of busy RCS files when !defined(O_CREAT) | !defined(O_EXCL).
 *
 * Revision 5.4  1990/11/01  05:03:40  eggert
 * Permit arbitrary data in comment leaders.
 *
 * Revision 5.3  1990/09/11  02:41:13  eggert
 * Tune expandline().
 *
 * Revision 5.2  1990/09/04  08:02:21  eggert
 * Count RCS lines better.  Improve incomplete line handling.
 *
 * Revision 5.1  1990/08/29  07:13:56  eggert
 * Add -kkvl.
 * Fix bug when getting revisions to files ending in incomplete lines.
 * Fix bug in comment leader expansion.
 *
 * Revision 5.0  1990/08/22  08:12:47  eggert
 * Don't require final newline.
 * Don't append "checked in with -k by " to logs,
 * so that checking in a program with -k doesn't change it.
 * Don't generate trailing white space for empty comment leader.
 * Remove compile-time limits; use malloc instead.  Add -k, -V.
 * Permit dates past 1999/12/31.  Make lock and temp files faster and safer.
 * Ansify and Posixate.  Check diff's output.
 *
 * Revision 4.8  89/05/01  15:12:35  narten
 * changed copyright header to reflect current distribution rules
 * 
 * Revision 4.7  88/11/08  13:54:14  narten
 * misplaced semicolon caused infinite loop
 * 
 * Revision 4.6  88/08/09  19:12:45  eggert
 * Shrink stdio code size; allow cc -R.
 * 
 * Revision 4.5  87/12/18  11:38:46  narten
 * Changes from the 43. version. Don't know the significance of the
 * first change involving "rewind". Also, additional "lint" cleanup.
 * (Guy Harris)
 * 
 * Revision 4.4  87/10/18  10:32:21  narten
 * Updating version numbers. Changes relative to version 1.1 actually
 * relative to 4.1
 * 
 * Revision 1.4  87/09/24  13:59:29  narten
 * Sources now pass through lint (if you ignore printf/sprintf/fprintf 
 * warnings)
 * 
 * Revision 1.3  87/09/15  16:39:39  shepler
 * added an initializatin of the variables editline and linecorr
 * this will be done each time a file is processed.
 * (there was an obscure bug where if co was used to retrieve multiple files
 *  it would dump)
 * fix attributed to  Roy Morris @FileNet Corp ...!felix!roy
 * 
 * Revision 1.2  87/03/27  14:22:17  jenkins
 * Port to suns
 * 
 * Revision 4.1  83/05/12  13:10:30  wft
 * Added new markers Id and RCSfile; added locker to Header and Id.
 * Overhauled expandline completely() (problem with $01234567890123456789@).
 * Moved trymatch() and marker table to rcskeys.c.
 * 
 * Revision 3.7  83/05/12  13:04:39  wft
 * Added retry to expandline to resume after failed match which ended in $.
 * Fixed truncation problem for $19chars followed by@@.
 * Log no longer expands full path of RCS file.
 * 
 * Revision 3.6  83/05/11  16:06:30  wft
 * added retry to expandline to resume after failed match which ended in $.
 * Fixed truncation problem for $19chars followed by@@.
 * 
 * Revision 3.5  82/12/04  13:20:56  wft
 * Added expansion of keyword Locker.
 *
 * Revision 3.4  82/12/03  12:26:54  wft
 * Added line number correction in case editing does not start at the
 * beginning of the file.
 * Changed keyword expansion to always print a space before closing KDELIM;
 * Expansion for Header shortened.
 *
 * Revision 3.3  82/11/14  14:49:30  wft
 * removed Suffix from keyword expansion. Replaced fclose with ffclose.
 * keyreplace() gets log message from delta, not from curlogmsg.
 * fixed expression overflow in while(c=putc(GETC....
 * checked nil printing.
 *
 * Revision 3.2  82/10/18  21:13:39  wft
 * I added checks for write errors during the co process, and renamed
 * expandstring() to xpandstring().
 *
 * Revision 3.1  82/10/13  15:52:55  wft
 * changed type of result of getc() from char to int.
 * made keyword expansion loop in expandline() portable to machines
 * without sign-extension.
 */


#include "rcsbase.h"

libId(editId, "$Id: rcsedit.c,v 5.8 1991/01/30 14:21:32 apratt Exp $")

static void keyreplace P((enum markers,const struct hshentry*,FILE*));


FILE *fcopy;		 /* result file descriptor			    */
const char *resultfile;  /* result file name				    */
int locker_expansion;	 /* should the locker name be appended to Id val?   */
static FILE *fedit;	 /* edit   file descriptor			    */
static const char *editfile;	 /* edit file				    */
static const char *editdir;  /* edit directory				    */
static unsigned long editline; /*fedit line counter; is always #lines+1     */
static long linecorr; /* #adds - #deletes in each edit run.		    */
               /*used to correct editline in case file is not rewound after */
               /* applying one delta                                        */

#define DIRTEMPNAMES 3
enum maker {notmade, real, effective};
struct buf dirtfname[DIRTEMPNAMES];		/* unlink these when done */
static volatile enum maker dirtfmaker[DIRTEMPNAMES];	/* if these are set */

	FILE *
initeditfiles(dir)
	const char *dir;
/* Function: Initializes resultfile and editfile with temporary filenames
 * in directory dir. Opens resultfile for reading and writing, with fcopy
 * as file descriptor. fedit is set to nil.
 */
{
	editline = 0;	/* make sure we start from the beginning*/
	linecorr = 0;
	editdir = dir;
	resultfile = makedirtemp(dir,1);
	editfile = nil;
        fedit=nil;
	errno = 0;
	return fcopy = fopen(resultfile,"w+");
}

	void
inittmpeditfiles()
{
	if (!initeditfiles(tmp()))
		efaterror(resultfile);
}


	void
arewind(f)
	FILE *f;
{
	if (fseek(f, (long)0, SEEK_SET) == EOF)
		IOerror();
}

	void
swapeditfiles(tostdout)
	int tostdout;
/* Function: swaps resultfile and editfile, assigns fedit=fcopy,
 * rewinds fedit for reading, and opens resultfile for reading and
 * writing, using fcopy. If tostdout, fcopy is set to stdout.
 */
{
	const char *tmpptr;
        fedit=fcopy;
	arewind(fedit);
        editline = 1; linecorr=0;
        tmpptr=editfile; editfile=resultfile; resultfile=tmpptr;
        if (tostdout)
                fcopy=stdout;
	else {
	    if (!resultfile)
		resultfile = makedirtemp(editdir,2);
	    errno = 0;
	    if (!(fcopy = fopen(resultfile,"w+")))
		efaterror(resultfile);
        }
}


	void
finishedit(delta)
	const struct hshentry *delta;
/* copy the rest of the edit file and close it (if it exists).
 * if delta!=nil, perform keyword substitution at the same time.
 */
{
	register FILE *fe, *fc;

	fe = fedit;
	if (fe) {
		fc = fcopy;
                if (delta!=nil) {
			while (0 < expandline(fe,fc,delta,false,(FILE*)NULL))
				;
                } else {
			fastcopy(fe,fc);
                }
		ffclose(fe);
        }
}



	static exiting void
editEndsPrematurely()
{
	fatserror("edit script ends prematurely");
}

	static exiting void
editLineNumberOverflow()
{
	fatserror("edit script refers to line past end of file");
}


	static void
copylines(upto,delta)
	register unsigned long upto;
	const struct hshentry *delta;
/* Function: copies input lines editline..upto-1 from fedit to fcopy.
 * If delta != nil, keyword expansion is done simultaneously.
 * editline is updated. Rewinds a file only if necessary.
 */
{
	register int c;
	register FILE *fe, *fc;

	if (upto < editline) {
                /* swap files */
                finishedit((struct hshentry *)nil); swapeditfiles(false);
                /* assumes edit only during last pass, from the beginning*/
        }
	fe = fedit;
	fc = fcopy;
	if (editline < upto)
	    if (delta)
		do {
			if (expandline(fe,fc,delta,false,(FILE*)NULL) <= 0)
				goto unexpected_EOF;
		} while (++editline < upto);
	    else
		do {
			do {
				c = getc(fe);
				if (c == EOF)
					goto unexpected_EOF;
				aputc(c, fc);
			} while (c != '\n');
		} while (++editline < upto);
	return;

    unexpected_EOF:
	editLineNumberOverflow();
}



	void
xpandstring(delta)
	const struct hshentry *delta;
/* Function: Reads a string terminated by SDELIM from finptr and writes it
 * to fcopy. Double SDELIM is replaced with single SDELIM.
 * Keyword expansion is performed with data from delta.
 * If foutptr is nonnull, the string is also copied unchanged to foutptr.
 */
{
	while (0 < expandline(finptr,fcopy,delta,true,foutptr))
		;
}


	void
copystring()
/* Function: copies a string terminated with a single SDELIM from finptr to
 * fcopy, replacing all double SDELIM with a single SDELIM.
 * If foutptr is nonnull, the string also copied unchanged to foutptr.
 * editline is set to (number of lines copied)+1.
 * Assumption: next character read is first string character.
 */
{	register c;
	register FILE *fin, *frew, *fcop;
	register int amidline;

	fin=finptr; frew=foutptr; fcop=fcopy;
        editline=1;
	amidline = false;
	for (;;) {
		GETC(fin,frew,c);
		switch (c) {
		    case EOF:
			unterminatedString();
			/*NOTREACHED*/
		    case '\n':
			++editline;
			++rcsline;
			amidline = false;
			break;
		    case SDELIM:
			GETC(fin,frew,c);
			if (c != SDELIM) {
				/* end of string */
				nextc = c;
				editline += amidline;
				return;
			}
			/* fall into */
		    default:
			amidline = true;
			break;
                }
		aputc(c,fcop);
        }
}




	void
editstring(delta)
	const struct hshentry *delta;
/* Function: reads an edit script from finptr and applies it to
 * file fedit; the result is written to fcopy.
 * If delta!=nil, keyword expansion is performed simultaneously.
 * If foutptr is set, the edit script is also copied verbatim to foutptr.
 * Assumes that all these files are open.
 * If running out of lines in fedit, fedit and fcopy are swapped.
 * resultfile and editfile are the names of the files that go with fcopy
 * and fedit, respectively.
 * Assumes the next input character from finptr is the first character of
 * the edit script. Resets nextc on exit.
 */
{
        int ed; /* editor command */
        register int c;
	register FILE *fin, *frew, *f;
	register unsigned long i;
	unsigned long line_lim;
	struct diffcmd dc;

        editline += linecorr; linecorr=0; /*correct line number*/
	frew = foutptr;
	fin = finptr;
	line_lim = ULONG_MAX;
	initdiffcmd(&dc);
	while (0  <=  (ed = getdiffcmd(fin,SDELIM,frew,&dc)))
		if (line_lim <= dc.line1)
			editLineNumberOverflow();
		else if (!ed) {
			copylines(dc.line1, delta);
                        /* skip over unwanted lines */
			i = dc.nlines;
			linecorr -= i;
			editline += i;
			f = fedit;
			do {
                                /*skip next line*/
				while ((c=getc(f))!='\n')
					if (c==EOF) {
					    if (i!=1)
						editLineNumberOverflow();
					    line_lim = dc.dafter;
					    break;
					}
			} while (--i);
		} else {
			copylines(dc.line1+1, delta); /*copy only; no delete*/
			i = dc.nlines;
			linecorr += i;
			f = fcopy;
			do {
                                /*copy next line from script*/
                                if (delta!=nil)
				    switch (expandline(fin,f,delta,true,frew)) {
					case 0:
					    if (i==1)
						return;
					    /* fall into */
					case -1:
					    editEndsPrematurely();
				    }
                                else {
				       for (;;) {
					    GETC(fin,frew,c);
					    if (c == EOF)
						editEndsPrematurely();
					    aputc(c, f);
					    if (c == '\n')
						break;
					    if (c==SDELIM) {
						GETC(fin,frew,c);
						if (c!=SDELIM) {
						    if (--i)
							editEndsPrematurely();
						    nextc = c;
						    return;
						}
					   }
				       }
				       ++rcsline;
				}
			} while (--i);
                }
	GETC(fin,frew,c);
	nextc = c;
}



/* The rest is for keyword expansion */



	int
expandline(in, out, delta, delimstuffed, frew)
	register FILE *in, *out;
	const struct hshentry *delta;
	int delimstuffed;
	register FILE *frew;
/* Function: Reads a line from in and writes it to out.
 * If DELIMSTUFFED is set, double SDELIM is replaced with single SDELIM.
 * Keyword expansion is performed with data from delta.
 * If FREW is set, the line is also copied unchanged to FREW.
 * DELIMSTUFFED must be set if FREW is set.
 * Yields -1 if no data is copied, 0 if an incomplete line is copied,
 * 1 if a complete line is copied.
 */
{
	register c;
	register char * tp;
	register int ds, r;
	const char *tlim;
	char keystring[keylength+2];
	static struct buf keyval;
        enum markers matchresult;

	ds = delimstuffed;
	r = -1;
	GETC(in,frew,c);
        for (;;) {
		switch (c) {
		    case EOF:
                        if(ds) {
				unterminatedString();
                        }
			return r;

		    case SDELIM:
			if (ds) {
			    GETC(in,frew,c);
			    if (c != SDELIM) {
                                /* end of string */
                                nextc=c;
				return r;
			    }
			}
			/* fall into */
		    default:
			r = 0;
			aputc(c,out);
			break;

		    case '\n':
			rcsline += ds;
			aputc(c,out);
			return 1;

		    case KDELIM:
			r = 0;
                        /* check for keyword */
                        /* first, copy a long enough string into keystring */
			tp=keystring;
			for (;;) {
			    GETC(in,frew,c);
			    if (tp < keystring+keylength)
				switch (ctab[c]) {
				    case LETTER: case Letter:
					*tp++ = c;
					continue;
				    default:
					break;
				}
			    break;
                        }
			*tp++ = c; *tp = '\0';
			matchresult = trymatch(keystring);
			if (matchresult==Nomatch) {
				tp[-1] = 0;
				aprintf(out, "%c%s", KDELIM, keystring);
				continue;   /* last c handled properly */
			}

			/* Now we have a keyword terminated with a K/VDELIM */
			if (c==VDELIM) {
			      /* try to find closing KDELIM, and replace value */
			      bufalloc(&keyval, 1);
			      tp = keyval.string;
			      tlim = tp + keyval.size;
			      for (;;) {
				      GETC(in,frew,c);
				      if (c==EOF || c=='\n' || c==KDELIM)
					break;
				      *tp++ =c;
				      if (tlim <= tp)
					  tp = bufenlarge(&keyval, &tlim);
				      if (c==SDELIM && ds) { /*skip next SDELIM */
						GETC(in,frew,c);
						if (c != SDELIM) {
							/* end of string before closing KDELIM or newline */
							*tp = 0;
							aprintf(out, "%c%s%s", KDELIM, keystring, keyval.string);
							nextc = c;
							return 0;
						}
				      }
			      }
			      if (c!=KDELIM) {
				    /* couldn't find closing KDELIM -- give up */
				    *tp = 0;
				    aprintf(out, "%c%s%s", KDELIM, keystring, keyval.string);
				    continue;   /* last c handled properly */
			      }
			}
			/* now put out the new keyword value */
			keyreplace(matchresult,delta,out);
                }
		GETC(in,frew,c);
        }
}


const char ciklog[ciklogsize] = "checked in with -k by ";

	static void
keyreplace(marker,delta,out)
	enum markers marker;
	register const struct hshentry *delta;
	register FILE *out;
/* function: outputs the keyword value(s) corresponding to marker.
 * Attributes are derived from delta.
 */
{
	register const char *sp, *cp, *date;
	register char c;
	register size_t cs, cw, ls;
	int RCSv;

	sp = Keyword[(int)marker];

	if (Expand == KEY_EXPAND) {
		aprintf(out, "%c%s%c", KDELIM, sp, KDELIM);
		return;
	}

        date= delta->date;
	RCSv = RCSversion;

	if (Expand == KEYVAL_EXPAND  ||  Expand == KEYVALLOCK_EXPAND)
		aprintf(out, "%c%s%c%c", KDELIM, sp, VDELIM,
			marker==Log && RCSv<VERSION(5)  ?  '\t'  :  ' '
		);

        switch (marker) {
        case Author:
		aputs(delta->author, out);
                break;
        case Date:
		printdate(out, date, " ");
                break;
        case Id:
	case Header:
		aprintf(out, "%s %s ",
			  marker==Id || RCSv<VERSION(4)
			? bindex(RCSfilename,SLASH)
			: getfullRCSname(),
			delta->num
		);
		printdate(out, date, " ");
		aprintf(out, " %s %s",
			delta->author,
			  RCSv==VERSION(3) && delta->lockedby ? "Locked"
			: delta->state
		);
		if (delta->lockedby!=nil)
		    if (VERSION(5) <= RCSv) {
			if (locker_expansion || Expand==KEYVALLOCK_EXPAND)
			    aprintf(out, " %s", delta->lockedby);
		    } else if (RCSv == VERSION(4))
			aprintf(out, " Locker: %s", delta->lockedby);
                break;
        case Locker:
		if (delta->lockedby)
		    if (
				locker_expansion
			||	Expand == KEYVALLOCK_EXPAND
			||	RCSv <= VERSION(4)
		    )
			aputs(delta->lockedby, out);
                break;
        case Log:
        case RCSfile:
		aputs(bindex(RCSfilename,SLASH), out);
                break;
        case Revision:
		aputs(delta->num, out);
                break;
        case Source:
		aputs(getfullRCSname(), out);
                break;
        case State:
		aputs(delta->state, out);
                break;
	default:
		break;
        }
	if (Expand == KEYVAL_EXPAND  ||  Expand == KEYVALLOCK_EXPAND) {
		afputc(' ', out);
		afputc(KDELIM, out);
	}
	if (marker == Log) {
		sp = delta->log.string;
		ls = delta->log.size;
		if (sizeof(ciklog)-1<=ls && !strncmp(sp,ciklog,sizeof(ciklog)-1))
			return;
		afputc('\n', out);
		cp = Comment.string;
		cw = cs = Comment.size;
		awrite(cp, cs, out);
		aprintf(out, "Revision %s  ", delta->num);
		printdate(out, date, "  ");
		aprintf(out, "  %s", delta->author);
		/* Do not include state: it may change and is not updated.  */
		/* Comment is the comment leader.  */
		if (VERSION(5) <= RCSv)
		    for (;  cw && (cp[cw-1]==' ' || cp[cw-1]=='\t');  --cw)
			;
		for (;;) {
		    afputc('\n', out);
		    awrite(cp, cw, out);
		    if (!ls)
			break;
		    --ls;
		    c = *sp++;
		    if (c != '\n') {
			awrite(cp+cw, cs-cw, out);
			do {
			    afputc(c,out);
			    if (!ls)
				break;
			    --ls;
			    c = *sp++;
			} while (c != '\n');
		    }
		}
	}
}


	FILE *
rcswriteopen(RCSname)
	const char *RCSname;
/*
 * Create the lock file corresponding to RCSNAME.
 * Then try to open RCSNAME for reading and yield its FILE* descriptor.
 * If all goes well, discard any previously acquired locks,
 * and set frewrite to the FILE* descriptor of the lock file,
 * which will eventually turn into the new RCS file.
 */
{
#if AKP_BUGFIXES
	int errno_hold;
#endif
	register char *tp;
	register const char *sp, *lp;
	FILE *f;
	int fdesc, r;
	struct buf *dirt;

	sp = RCSname;
	dirt = &dirtfname[frewrite != NULL];
	bufalloc(dirt, strlen(sp)+1);
	tp = dirt->string;
	if ((lp = strrchr(sp,SLASH)))
		while (sp<=lp)
			*tp++ = *sp++;
	*tp++ = RCSSEP ? RCSSEP : RCSSUF;
#if RCSSEP
	/* Insert `,' and append file name.  */
	lp = strrchr(sp, RCSSEP);
#else
	/* The file system doesn't allow `,'; use `v' as a poor substitute.  */
	lp = sp + strlen(sp) - 2;
#endif
	while (sp<=lp)
		*tp++ = *sp++;
	*tp = '\0'; /* same length as RCSname */
	tp = dirt->string;

	f = NULL;

	seteid();
	ignoreints();
#	if !open_can_creat
#		define create(f,m) creat(f, m)
#	else
#		define create(f,m) open(f, O_CREAT|O_WRONLY|o_excltrunc, m)
#		ifdef O_EXCL
#			define o_excltrunc O_EXCL
#		else
#			define o_excltrunc O_TRUNC
#		endif
#	endif
#if AKP_BUGFIXES
	errno = 0;
#endif
	if (0 <= (fdesc = create(tp, S_IRUSR|S_IRGRP|S_IROTH))) {
		dirtfmaker[0] = effective;
		errno = 0;
		f = fopen(RCSname,"r");
#if AKP_BUGFIXES
		errno_hold = errno;
#endif
		if (frewrite)
		    /* We already have a lock somewhere else.  */
		    if (f) {
			/* Discard the first acquired lock.  */
			ffclose(frewrite);  frewrite = NULL;
			if (unlink(newRCSfilename) < 0) {
			    setrid();
			    efaterror(newRCSfilename);
			}
			bufscpy(&dirtfname[0], tp);
		    } else {
			/* Prefer the first acquired lock to this one.  */
			r = close(fdesc)<0 || unlink(tp)<0;
			restoreints();
			setrid();
			if (r)
			    efaterror(tp);
#if AKP_BUGFIXES
			errno = errno_hold;
#endif
			return f;
		    }
		if (!(frewrite = fdopen(fdesc,"w"))) {
		    setrid();
		    efaterror(newRCSfilename);
		}
	}
#if !open_can_creat | !defined(O_EXCL)
	else if (errno != ENOENT) {
		/* Set errno=EEXIST if the RCS file is busy.  */
		struct stat statbuf;
		int old_errno = errno;
		errno  =  stat(tp,&statbuf)==0 ? EEXIST : old_errno;
	}
#endif
#if AKP_BUGFIXES
	errno_hold = errno;
#endif
	restoreints();
	setrid();
#if AKP_BUGFIXES
	errno = errno_hold;
#endif
	return f;
}

	void
keepdirtemp(name)
	const char *name;
/* Do not unlink name, either because it's not there any more,
 * or because it has already been unlinked.
 */
{
	register int i;
	for (i=DIRTEMPNAMES; 0<=--i; )
		if (dirtfname[i].string == name) {
			dirtfmaker[i] = notmade;
			return;
		}
	faterror("keepdirtemp");
}

	const char *
makedirtemp(name, n)
	register const char *name;
	int n;
/*
 * Have maketemp() do all the work if name==tmp.
 * Otherwise, create a unique filename in name's dir using n and name
 * and store it into the dirtfname[n].
 * Because of storage in tfnames, dirtempunlink() can unlink the file later.
 * Return a pointer to the filename created.
 */
{
	register char *tp;
	register const char *lastslash, *np;

	if (name == tmp())
		return maketemp(n);
	bufalloc(&dirtfname[n], strlen(name)+3);
	np = tp = dirtfname[n].string;
	if ((lastslash = strrchr(name,SLASH)))
		while (name<=lastslash)
			*tp++ = *name++;
	*tp++ = RCSSEP ? RCSSEP : RCSSUF;
	*tp++ = 'A'+n;
	while ((*tp++ = *name++))
	    ;
	dirtfmaker[n] = real;
	return np;
}

	void
dirtempunlink()
/* Clean up makedirtemp() files.  May be invoked by signal handler. */
{
	register int i;
	enum maker m;

	for (i = DIRTEMPNAMES;  0 <= --i;  )
	    if ((m = dirtfmaker[i]) != notmade) {
		if (m == effective)
		    seteid();
		VOID unlink(dirtfname[i].string);
		if (m == effective)
		    setrid();
		dirtfmaker[i] = notmade;
	    }
}