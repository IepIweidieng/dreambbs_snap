#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>


int
rec_put(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;

  if ((fd = open(fpath, O_WRONLY | O_CREAT, 0600)) < 0)
    return -1;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  lseek(fd, (off_t) (size * pos), SEEK_SET);
  write(fd, data, size);

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);

  return 0;
}
