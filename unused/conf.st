/* example RCS compile-time configuration */

	/* $Id: conf.st,v 1.5 91/01/30 12:04:02 apratt Exp $ */

/*
 * This is an example RCS compile-time configuration.
 * If you can't get conf.sh to work as described in the Makefile,
 * copy this file to conf.h and edit conf.h by hand.
 */

#define exitmain(n) return n /* how to exit from main() */
#if !MAKEDEPEND
#	include <stdio.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <limits.h>
#	include <stdlib.h>
#	include <string.h>
#	include <unistd.h>
	/* #include <vfork.h> does not work.  */
#endif /* !MAKEDEPEND */
#define has_tmpnam 1	/* does tmpnam() work? (if not, mktemp is used) */
#define has_sys_dir_h 1 /* Does #include <sys/dir.h> work?  */
#define has_sys_param_h 1 /* Does #include <sys/param.h> work?  */
#define has_sys_wait_h 0 /* Does #include <sys/wait.h> work?  */
/* #define const */ /* The 'const' keyword works.  */
/* #define volatile */ /* The 'volatile' keyword works.  */
/* typedef int gid_t; */ /* Standard headers define gid_t.  */
typedef u_short mode_t; /* Standard headers define mode_t.  */
/* typedef int pid_t; */ /* Standard headers define pid_t.  */
/* typedef int sig_atomic_t; */ /* Standard headers define sig_atomic_t.  */
/* typedef int size_t; */ /* Standard headers define size_t.  */
/* typedef long time_t; */ /* Standard headers define time_t.  */
/* typedef int uid_t; */ /* Standard headers define uid_t.  */
#define has_prototypes 1 /* Do function prototypes work?  */
#if has_prototypes
#	define P(params) params
#	if !MAKEDEPEND
#		include <stdarg.h>
#	endif
#	define vararg_start(ap,p) va_start(ap,p)
#else
#	define P(params) ()
#	if !MAKEDEPEND
#		include <varargs.h>
#	endif
#	define vararg_start(ap,p) va_start(ap)
#endif
#define has_getuid 0 /* Does getuid() work?  */
#define declare_getpwuid struct passwd *getpwuid P((int));
#define has_rename 1 /* Does rename() work?  */
#define bad_rename 1 /* Does rename(A,B) fail if B exists?  */
#define VOID (void) /* 'VOID e;' discards the value of an expression 'e'.  */
#define signal_type void /* type returned by signal handlers */
#define sig_zaps_handler 0 /* Must a signal handler reinvoke signal()?  */
#define has_seteuid 0 /* Does seteuid() obey Posix 1003.1-1990?  */
#define has_sigaction 0 /* Does struct sigaction work?  */
#define has_sigblock 0 /* Does sigblock() work?  */
#define has_sys_siglist 0 /* Does sys_siglist[] work?  */
#define exit_type void /* type returned by exit() */
#define underscore_exit_type void /* type returned by _exit() */
typedef size_t fread_type; /* type returned by fread() and fwrite() */
typedef void *malloc_type; /* type returned by malloc() */
#define free_type void /* type returned by free() */
typedef size_t strlen_type; /* type returned by strlen() */
#define has_getcwd 1 /* Does getcwd() work?  */
#define has_getwd 1 /* Does getwd() work?  */
#define has_vfork 0 /* Does vfork() work?  */
#define has_vfprintf 1 /* Does vfprintf() work?  */
#define CO "co" /* name of 'co' program */
#define COMPAT2 0 /* Are version 2 files supported?  */
#define DATEFORM "%.2d.%.2d.%.2d.%.2d.%.2d.%.2d" /* e.g. 01.01.01.01.01.01 */
#define DIFF "diff" /* name of 'diff' program */
#define DIFF_FLAGS , "-n" /* Make diff output suitable for RCS.  */
#define DIFF_L 1 /* Does diff -L work? */
#define EXECRCS execv /* variant of execv() to use on subprograms */
#define MERGE "merge" /* name of 'merge' program */
#define RCSDIR "RCS\\" /* subdirectory for RCS files */
#define SLASH '\\' /* path name separator */
#define TMPDIR "/tmp/" /* default directory for temporary files */
#define DIFF_PATH_HARDWIRED 1 /* Is DIFF absolute, not relative?  */
#define ROOTPATH(p) ((p)[0]==SLASH)
#define RCSSEP ',' /* separator for RCSSUF */
#define SENDMAIL "/bin/mail" /* how to send mail */

/* AKP: added switches to control things I needed to make this work on	  */
/* a mostly-ANSI, partly-POSIX, but not-UNIX, system. Leave them all zero */
/* to get the distribution code for the features they control.		  */

/* HEAD_REV is added so you can say "rcs -nRELEASE:head *.c" to do the    */
/* same thing that rcsfreeze does: apply the name to the head revision of */
/* each named file.  'head' is special-cased in the function lookupsym,	  */
/* and applies anywhere a symbolic revision spec can be used.		  */

#define DONT_USE_SIGNALS 1	/* 1=just ignore signal handling */
#define DONT_USE_FORK 1		/* 1=use system() instead */
#define DONT_USE_MAIL 1		/* 1=ask the user to tell lock holder */
#define ANSI_INCLUDE_FILES 1	/* 1=don't redeclare ANSI stuff in headers */
#define USE_AKP_PAIRS 1		/* 1=use 8.3 filename pair code */
#define HEAD_REV 1		/* 1=allow "head" as revision name */
#define terrible_rename 1	/* 1=rename(A,B) fails if A is read-only */
#define bad_unlink 1		/* 1=unlink(A) fails if A is read-only */
#define AKP_MODES 1		/* 1=define WORKMODE per gcc lib, not unix */
#define AKP_BUGFIXES 1		/* 1=enable misc AKP bug fixes */

/* these defines are for Atari TOS, where system() just returns the exit code. */
#if DONT_USE_FORK
#ifdef WIFEXITED
#  undef WIFEXITED
#endif
#ifdef WEXITSTATUS
#  undef WEXITSTATUS
#endif

#define WIFEXITED(stat_val) ((stat_val) >= 0)
#define WEXITSTATUS(stat_val) (stat_val)

/* end if DONT_USE_FORK */
#endif

#if !ANSI_INCLUDE_FILES
#if 0 /* These agree with <stdio.h>.  */
	int fprintf P((FILE*,const char*,...));
	int printf P((const char*,...));
#	if has_vfprintf
		int vfprintf P((FILE*,const char*,...));
#	else
		void _doprnt P((const char*,...));
#	endif
#endif
#if 0
char *sprintf P((char*,const char*,...));
int chmod P((const char*,mode_t));
int fcntl P((int,int,...));
int open P((const char*,int,...));
mode_t umask P((mode_t));
pid_t wait P((int*));
#endif
#ifndef O_CREAT
	int creat P((const char*,mode_t));
#endif
#if has_seteuid
	int setegid P((gid_t));
	int seteuid P((uid_t));
#endif

/* end if !ANSI_INCLUDE_FILES */
#endif
