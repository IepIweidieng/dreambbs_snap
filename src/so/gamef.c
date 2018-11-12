/*-------------------------------------------------------*/
/* gamef.c	( WindTopBBS Ver 3.02 )			 */
/*-------------------------------------------------------*/
/* target : WindTop Games' API				 */
/* create : 2001/04/10				 	 */
/* update : 2001/04/10				 	 */
/*-------------------------------------------------------*/
#include <varargs.h>
#include "bbs.h"
#define getdata(x1,x2,x3,x4,x5,x6,x7) vget(x1,x2,x3,x4,x5,DOECHO)
char Bdate[20];

char *
Cdate(chrono)
  time_t *chrono;
{
  struct tm *ptime;

  ptime = localtime(chrono);
  /* Thor.990329: y2k */
  sprintf(Bdate, "%02d/%02d/%02d",
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  return Bdate;
}                 

void
pressanykey(va_alist)
  va_dcl
{
  va_list args;
  uschar buf[512], *fmt;
  int cc,ch;

  va_start(args);
  fmt = va_arg(args, char *);
  vsprintf(buf, fmt, args);
  va_end(args);
  move(b_lines, 0);
  for (fmt = buf; cc = *fmt; fmt++)
    outc(cc);
  do
  {
    ch = vkey();
  }while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  move(b_lines, 0);
  clrtoeol();
  refresh();
}

void
game_log(va_alist)
va_dcl
{
  va_list args;
  int file;
  uschar buf[200], *fmt, ff[40];
  time_t now;
  FILE *fs;

  va_start(args);
  file = va_arg(args,int);
  fmt = va_arg(args, char *);
  vsprintf(buf, fmt, args);
  va_end(args);

  switch(file)
  {
    case 1: strcpy(ff,"run/mine.log"); break;
    case 2: strcpy(ff,"run/bj.log"); break;    
  }
  fs=fopen(ff,"a+");
  now=time(0);
  fprintf(fs,"[1;33m%s [32m%s [36m%s[m\n", Cdate(&now),cuser.userid,buf);
  fclose(fs);
}

