#include "innbbsconf.h"
#include "daemon.h"

/* char *AccessFile=ACCESSFILE; */
#define INNBBSDPORT1 "1904"
#define INNBBSDPORT2 "1234"
#define INNBBSDPATH1  ".innbbsd1"
#define INNBBSDPATH2  ".innbbsd2"


int
pmain(port)
  char *port;
{
  if (port == NULL)
  {
    register int rel;
    /* installbbstalkd(); */
    fprintf(stderr, "Trying to listen in port %s\n", INNBBSDPORT1);
    rel = open_listen(INNBBSDPORT1, "tcp", NULL);

#ifdef DEBUG
    printf("port fd %d allocated\n", rel);
#endif

    if (rel < 0)
    {
      fprintf(stderr, "Trying to listen in port %s\n", INNBBSDPORT2);
      return open_listen(INNBBSDPORT2, "tcp", NULL);
    }
    return rel;
  }
  else
  {

#ifdef DEBUG
    printf("start to allocate port\n");
#endif

    return open_listen(port, "tcp", NULL);
  }
}


int
p_unix_main(path)
  char *path;
{
  register int rel;

  if (path == NULL)
  {
    /* installbbstalkd(); */
    fprintf(stderr, "Trying to listen in port %s\n", INNBBSDPATH1);
    rel = open_unix_listen(INNBBSDPATH1, "tcp", NULL);

#ifdef DEBUG
    printf("port fd %d allocated\n", rel);
#endif

    if (rel < 0)
    {
      fprintf(stderr, "Trying to listen in port %s\n", INNBBSDPATH2);
      return open_listen(INNBBSDPATH2, "tcp", NULL);
    }
    return rel;
  }
  else
  {

#ifdef DEBUG
    printf("start to allocate path %s\n", path);
#endif

    rel = unixclient(path, "tcp");
    if (rel < 0)
      unlink(path);
    else
      close(rel);
    return open_unix_listen(path, "tcp", NULL);
  }
}
