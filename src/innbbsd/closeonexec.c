/*
 * $Revision: 1.2 $ *
 * 
 */
/* #include "configdata.h" */
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include "clibrary.h"

#ifndef CLX_IOCTL
#define CLX_IOCTL
#endif

#ifndef CLX_FCNTL
#define CLX_FCNTL
#endif




#if	defined(CLX_IOCTL) && !defined(IRIX)

#ifdef __linux
#ifdef REDHAT
#include <sgtty.h>
#else
#include <termios.h>
#endif
#else
/* Thor.981221: RedHat的請用這項 */
#include <sgtty.h>
#endif

/*
 * *  Mark a file close-on-exec so that it doesn't get shared with our *
 * children.  Ignore any error codes.
 */
void
closeOnExec(fd, flag)
  int fd;
  int flag;
{
  register int oerrno;

  oerrno = errno;
  (void) ioctl(fd, flag ? FIOCLEX : FIONCLEX, (char *) NULL);
  errno = oerrno;
}
#endif				/* defined(CLX_IOCTL) */




#if	defined(CLX_FCNTL)
#include <fcntl.h>


/*
 * *  Mark a file close-on-exec so that it doesn't get shared with our *
 * children.  Ignore any error codes.
 */
void
CloseOnExec(fd, flag)
  int fd;
  int flag;
{
  register int oerrno;

  oerrno = errno;
  (void) fcntl(fd, F_SETFD, flag ? 1 : 0);
  errno = oerrno;
}
#endif				/* defined(CLX_FCNTL) */
