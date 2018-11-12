/*-------------------------------------------------------*/
/* post.c       ( NTHU CS MapleBBS Ver 2.39 )            */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines		 	 */
/* create : 95/03/29				 	 */
/* update : 2000/01/02				 	 */
/*-------------------------------------------------------*/
 

#define xlog(x)		f_cat("/tmp/b.log", x)


#include "bbs.h"

extern FWCACHE *fwshm;
extern BCACHE *bshm;
extern XZ xz[];
#ifdef HAVE_MULTI_CROSSPOST
extern LinkList *ll_head;
#endif

extern int cmpchrono();
extern int xo_delete();
extern int xo_uquery();
extern int xo_usetup();
/*extern int xo_fpath();*/          /* lkchu.981201 */

#ifdef  HAVE_DETECT_CROSSPOST
static int post_modetype;
static int checknum;
typedef struct
{
   int sum;
   int total;    
}  CHECKSUM;

CHECKSUM checksum[5];
#endif

extern int TagNum;
extern char xo_pool[];
extern char brd_bits[];

/* Thor.990113: imports for anonymous log */
extern char rusername[];


int
cmpchrono(hdr)
  HDR *hdr;
{
  return hdr->chrono == currchrono;
}
/* ----------------------------------------------------- */
/* ��} cross post ���v                                  */
/* ----------------------------------------------------- */

#ifdef  HAVE_DETECT_CROSSPOST
static int
checksum_add(title)
  char *title;
{
  int sum=0,i,end;
  int *ptr;
  ptr = (int *)title;
  end = strlen(title)/4;  
  for(i=0;i<end;i++)
  {
      sum += *ptr++;
  }
  return sum;
}

static int
checksum_put(sum,check)
  int sum;
  int check;
{
  int i;
  for(i=0;i<5;i++)
  {
    if(checksum[i].sum == sum)
    {
      if(check)
        checksum[i].total--;
      else
        checksum[i].total++;
      if(checksum[i].total > MAX_CROSS_POST)
      {
        post_modetype |= POST_STOP_PERM;
        return 1;
      }
    }
  }
  if(!check)
  {
    if(++checknum>=5)
      checknum = 0;
    checksum[checknum].sum = sum;
    checksum[checknum].total = 1;
  }
  return 0;
}

int
checksum_find(fpath,check,state)
  char *fpath;
  int check;
  int state;
{
  char buf[256];
  FILE *fp;
  char *star = "��";

  int sum,i,count=0;

  if((state & BRD_NOCNTCROSS) || HAS_PERM(PERM_ADMIN) || 
     (post_modetype & POST_STOP_PERM))
    return 0;
    
  sum = 0;
  fp = fopen(fpath,"r");
  if(fp)
  {
    for(i=0;count <= MAX_CHECKSUM;i++)
    {
      if(fgets(buf, 256, fp))
      {
        if(i>3)
        {
          if(*buf != '>' && strncmp(buf,star,2) && *buf != ':' )
          {
            sum+=checksum_add(buf);
            count++;
          }
        }
      }
      else
        break;  
    }
  }
  fclose(fp);
  return checksum_put(sum,check);
}
#endif

/* ----------------------------------------------------- */
/* ��} innbbsd ��X�H��B�s�u��H���B�z�{��		 */
/* ----------------------------------------------------- */

void
outgo_post(hdr, board)
  HDR *hdr;
  char *board;
{
  char *fpath, buf[256];

  if (board)
  {
    fpath = "innd/out.bntp";
  }
  else
  {
    board = currboard;
    fpath = "innd/cancel.bntp";
  }

  sprintf(buf, "%s\t%s\t%s\t%s\t%s\n",
    board, hdr->xname, hdr->owner, hdr->nick, hdr->title);
  f_cat(fpath, buf);
}


void
cancel_post(hdr)
  HDR *hdr;
{
  if ((hdr->xmode & POST_OUTGO) &&	/* �~��H�� */
    (hdr->chrono > ap_start - 7 * 86400))	/* 7 �Ѥ������� */
  {
    outgo_post(hdr, NULL);
  }
}


/*static inline void*/

void
move_post(hdr, board, by_bm)	/* �N hdr �q currboard �h�� board */
  HDR *hdr;
  char *board;
  int by_bm;
{
  HDR post;
  char folder[80], fpath[80];

  brd_fpath(folder, currboard, fn_dir);
  hdr_fpath(fpath, folder, hdr);

  brd_fpath(folder, board, fn_dir);
  hdr_stamp(folder, HDR_LINK | 'A', &post, fpath);
  /*unlink(fpath);*/

  /* �����ƻs trailing data */

  memcpy(post.owner, hdr->owner, TTLEN + 140);
  if (by_bm == -1)
    strcpy(post.owner,cuser.userid);
  if (by_bm == -2)
    post.xmode |= POST_MARKED;

  if (by_bm>0)
    sprintf(post.title, "%-13s%.59s", cuser.userid, hdr->title);

  rec_add(folder, &post, sizeof(post));
  if (by_bm>=0)
    cancel_post(hdr);
}


/* ----------------------------------------------------- */
/* �o��B�^���B�s��B����峹				 */
/* ----------------------------------------------------- */

#ifdef HAVE_ANONYMOUS
/* Thor.980727: lkchu patch: log anonymous post */
/* Thor.980909: gc patch: log anonymous post filename */
void
log_anonymous(fname)
  char *fname;
{
  char buf[512];
  time_t now = time(0);
  /* Thor.990113: �[�W rusername �M fromhost����Ժ� */
  sprintf(buf, "%s %-13s(%s@%s) %s %s %s\n", Etime(&now), cuser.userid, rusername, fromhost, currboard, ve_title, fname);
  f_cat(FN_ANONYMOUS_LOG, buf);
}
#endif

#ifdef	HAVE_DETECT_VIOLAWATE
int
seek_log(title,state)
  char *title;
  int state;
{
  BANMAIL *head,*tail;
  if(state & BRD_NOLOG)
    return 0;


  head = fwshm->fwcache;  
  tail = head + fwshm->number;

  while(fwshm->number && head<tail)
  {
     if(strstr(title,head->data))
       return 1;
     head++;
  }
  return 0;
}
#endif

static int
do_post(title)
  char *title;
{
  /* Thor.1105: �i�J�e�ݳ]�n curredit */
  HDR post;
  char fpath[80], folder[80], *nick, *rcpt;
  int mode,bno;
  BRD *brd;
#ifdef	HAVE_DETECT_VIOLAWATE
  int banpost;
#endif
#ifdef  HAVE_DETECT_CROSSPOST
  int crosspost;
#endif

#ifdef	HAVE_RESIST_WATER
  if(checkqt > CHECK_QUOT_MAX)
  {
    vmsg("�z�w�g��Ӧh���F�A�ФU���A�ӧa�I");
    return XO_FOOT;
  }
#endif

  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return XO_FOOT;
  }

  if (!(bbstate & STAT_POST))
  {
    vmsg("�藍�_�A���ݪO�O��Ū��");
    return XO_FOOT;
  }

  brd_fpath(fpath, currboard, "post");

  if(more(fpath,(char *)-1)==-1)
    film_out(FILM_POST, 0);

  move(20,0);
  prints("�o��峹��i %s �j�ݪO", currboard);

  if (!ve_subject(21, title, NULL))
    return XO_HEAD;

  /* ����� Internet �v���̡A�u��b�����o��峹 */
  /* Thor.990111: �S��H�X�h���ݪ�, �]�u��b�����o��峹 */

  if (!HAS_PERM(PERM_INTERNET) || (bbstate & BRD_NOTRAN))
    curredit &= ~EDIT_OUTGO;

#ifdef HAVE_ANONYMOUS
  /* Thor.980727: lkchu�s�W��[²�檺��ܩʰΦW�\��] */
  /* Thor.980909: gc patch: edit �ɰΦW����ñ�W�� */
  if (bbstate & BRD_ANONYMOUS)
  {
    if(cuser.ufo2 & UFO2_DEF_ANONY)
    {
      if(vans("�A(�p)�Q�n�ʦW��(Y/N)?[N]") == 'y')
        curredit |= EDIT_ANONYMOUS;
    }
    else if(vans("�A(�p)�Q�n�ʦW��(Y/N)?[Y]") != 'n')
        curredit |= EDIT_ANONYMOUS;
  }
#endif

  utmp_mode(M_POST);
  fpath[0] = 0;
  if (vedit(fpath, YEA) < 0)
  {
    unlink(fpath);
    vmsg("����");
    return XO_HEAD;
  }
  
  //bno = brd_bno(currboard);
  brd = bshm->bcache + currbno;
  brh_get(brd->bstamp, bno);
        
  

  /* build filename */

  brd_fpath(folder, currboard, fn_dir);
  hdr_stamp(folder, HDR_LINK | 'A', &post, fpath);
#ifdef	HAVE_DETECT_VIOLAWATE
  banpost = seek_log(ve_title,bbstate);
#endif
#ifdef	HAVE_DETECT_CROSSPOST
  crosspost = checksum_find(fpath,0,bbstate);
#endif

  /* set owner to anonymous for anonymous board */

  rcpt = cuser.userid;
  nick = cuser.username;
  mode = curredit & POST_OUTGO;
  title = ve_title;

#ifdef HAVE_ANONYMOUS
  /* Thor.980727: lkchu�s�W��[²�檺��ܩʰΦW�\��] */
  if (curredit & EDIT_ANONYMOUS)
  {
    /* nick = rcpt; */ /* lkchu: nick ���ର  userid */
    nick = "�q�q�ڬO�� ? ^o^";
    /* Thor.990113: �ȻP�{�sid�V�c */
    /* rcpt = "anonymous"; */
    rcpt = "[���i�D�A]";
    mode = 0;

/* Thor.980727: lkchu patch: log anonymous post */
/* Thor.980909: gc patch: log anonymous post filename */
    log_anonymous(post.xname);
    
  }
#endif

  post.xmode = mode;
  strcpy(post.owner, rcpt);
  strcpy(post.nick, nick);
  strcpy(post.title, title);

#ifdef  HAVE_DETECT_CROSSPOST
  if(crosspost)
  {
    move_post(&post,BRD_VIOLATELAW,-2);
    add_deny(&cuser,DENY_SEL_POST|DENY_DAYS_1|DENY_MODE_POST,0);
    deny_log_email(cuser.vmail,(cuser.userlevel & PERM_DENYSTOP) ? -1 : cuser.deny);
    bbstate &= ~STAT_POST;
    cuser.userlevel &= ~PERM_POST;
  }
#endif

#ifdef	HAVE_DETECT_VIOLAWATE   
  if(banpost)
  {
    move_post(&post, BRD_BANPOSTLOG, -1);
  } 
#endif
  
#ifdef	HAVE_OBSERVE_LIST  
  if(observeshm_find(cuser.userno))
  {
    move_post(&post, BRD_OBSERVE, -1);
  } 
#endif

  move_post(&post,BRD_LOCALPOSTS,-3);
  
  if (!rec_add(folder, &post, sizeof(HDR)))
  {
    /* if ((mode) && (!(bbstate & BRD_NOTRAN))) */
    /* Thor.990111: �w�� edit.c ���Τ@check */
	brh_add( post.chrono, post.chrono,  post.chrono);
#ifdef	HAVE_DETECT_VIOLAWATE
    if (mode && !banpost)
#else
    if (mode)
#endif    
      outgo_post(&post, currboard);


    clear();
    outs("���Q�K�X�G�i�A");

    if (bbstate & BRD_NOCOUNT)
    {
      outs("�峹���C�J�����A�q�Х]�[�C");
    }
    else
    {
      prints("�o�O�z���� %d �g�峹�C", ++cuser.numposts);
    }

    /* �^�����@�̫H�c */

    if (curredit & EDIT_BOTH)
    {
      char *msg = "�@�̵L�k���H";
#define	_MSG_OK_	"�^���ܧ@�̫H�c"

      rcpt = quote_user;
      if (strchr(rcpt, '@'))
      {
	if (bsmtp(fpath, title, rcpt, 0) >= 0)
	  msg = _MSG_OK_;
      }
      else
      {
	usr_fpath(folder, rcpt, fn_dir);
	if (hdr_stamp(folder, HDR_LINK, &post, fpath) == 0)
	{
	  strcpy(post.owner, cuser.userid);
	  strcpy(post.title, title);
	  if (!rec_add(folder, &post, sizeof(post)))
	    msg = _MSG_OK_;
	}
      }
      outs(msg);
    }
  }
  unlink(fpath);

  vmsg(NULL);
#ifdef  HAVE_DETECT_CROSSPOST  
  if(crosspost)
    board_main();
#endif

#ifdef  HAVE_COUNT_BOARD
//  if(!(strcmp(brd->brdname,"Test")))
  if(!(bbstate & BRD_NOTOTAL))
    brd->n_posts++;
#endif


#ifdef	HAVE_RESIST_WATER
  if(checkqt > CHECK_QUOT_MAX && !HAS_PERM(PERM_ADMIN))
  {
    remove_perm();
    vmsg("�z�w�g��Ӧh���F�A�ФU���A�ӧa�I");
  }
#endif

  return XO_INIT;
}


static int
do_reply(hdr)
  HDR *hdr;
{
  char *msg;
 
  curredit = 0;
  if((bbstate & BRD_NOREPLY) && !HAS_PERM(PERM_SYSOP))
    msg = "�� �^���� (M)�@�̫H�c (Q)�����H[Q] ";
  else
    msg = "�� �^���� (F)�ݪO (M)�@�̫H�c (B)�G�̬ҬO (Q)�����H[F] "; 


  switch (vans(msg))
  {
  case 'm':
    mail_reply(hdr);
    *quote_file = '\0';
    return XO_HEAD;

  case 'q':
    /*
     * Thor: �ѨM Gao �o�{�� bug.. ������ reply�峹�A�o�� Q�����A �M��h
     * Admin/Xfile�U�H�K��@�ӽs��A �A�N�|�o�{�]�X reply�峹�ɪ��ﶵ�F�C
     */
    *quote_file = '\0';
    return XO_FOOT;

  case 'b':
    if((bbstate & BRD_NOREPLY) && !HAS_PERM(PERM_SYSOP))
    {
      *quote_file = '\0';
      return XO_FOOT;
    }
    curredit = EDIT_BOTH;
    break;
    
  case 'F': case 'f':
  default:
    if((bbstate & BRD_NOREPLY) && !HAS_PERM(PERM_SYSOP))
    {
      *quote_file = '\0';
      return XO_FOOT;
    }
  }

  /*
   * Thor.1105: ���׬O��i��, �άO�n��X��, ���O�O���i�ݨ쪺,
   * �ҥH�^�H�]��������X
   */
  if (hdr->xmode & (POST_INCOME | POST_OUTGO))
    curredit |= POST_OUTGO;

  strcpy(quote_user, hdr->owner);
  strcpy(quote_nick, hdr->nick);
  return do_post(hdr->title);
}


static int
post_reply(xo)
  XO *xo;
{
  if (bbstate & STAT_POST)
  {
    HDR *hdr;

    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    if (!(hdr->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_CURMODIFY)))
    {
      if((hdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD) || bbstate & STAT_BOARD))
        return XO_NONE;
      hdr_fpath(quote_file, xo->dir, hdr);
      return do_reply(hdr);
    }
  }
  return XO_NONE;
}


/* ----------------------------------------------------- */
/* �ݪO�\���						 */
/* ----------------------------------------------------- */

#ifdef HAVE_MODERATED_BOARD
extern int XoBM();
#endif


/* ----------------------------------------------------- */


static int post_add();
static int post_body();
static int post_head();		/* Thor: �]�� XoBM �n�� */


#ifdef XZ_XPOST
static int XoXpost();		/* Thor: for XoXpost */
#endif


static int
post_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return post_head(xo);
}


static int
post_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return post_body(xo);
}


static int
post_attr(fhdr)
  HDR *fhdr;
{
  int mode, attr;

  mode = fhdr->xmode;

  if (mode & POST_CANCEL)
    return 'c';

  if (mode & POST_DELETE)
    return 'd';
  
  if (mode & POST_MDELETE)
    return 'D';
    
  if (mode & POST_LOCK)
    return 'L';  

  if(mode & POST_COMPLETE)                                                    
    return 'S';

  attr = brh_unread(fhdr->chrono) ? 0 : 0x20;
  mode &= (bbstate & STAT_BOARD) ? ~0 : ~POST_GEM;	/* Thor:�@��user�ݤ���G */
  if (mode &= (POST_MARKED | POST_GEM))
    attr |= (mode == POST_MARKED ? 'M' : (mode == POST_GEM ? 'G' : 'B'));
  else if (!attr)
  {
//    if(strcmp(fhdr->owner, cuser.userid))
      attr = 'N';
//    else
//      attr = ' ';
  }
  return attr;
}


static void
post_item(num, hdr)
  int num;
  HDR *hdr;
{

#ifdef	HAVE_DECORATE
  prints(hdr->xmode & POST_MARKED ? "%6d \033[1;36m%c\033[m" : "%6d %c", num, post_attr(hdr));
#else
  prints("%6d %c", num, post_attr(hdr));
#endif

  /* Thor.990418: HYL.bbs@Rouge.Dorm10.NCTU.edu.tw patch
                  ���M�btitle > 46�Ӧr�ɵe���|�ñ�... */
  hdr_outs(hdr, 47); 
  /* hdr_outs(hdr, 46); */
}


static int
post_body(xo)
  XO *xo;
{
  HDR *fhdr;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (bbstate & STAT_POST)
    {
      if (vans("�n�s�W��ƶܡH(Y/N) [N] ") == 'y')
      return post_add(xo);	
    }
    else
    {
      vmsg("���ݪO�|�L�峹");
    }
    return XO_QUIT;
  }

  fhdr = (HDR *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    post_item(++num, fhdr++);
  } while (num < max);

  clrtobot();
  return XO_NONE;
}


static int			/* Thor: �]�� XoBM �n�� */
post_head(xo)
  XO *xo;
{
  vs_head(currBM, xo->xyz);
  outs("\
[��]���} [��]�\\Ū [^P]�o�� [b]�Ƨѿ� [d]�R�� [V]�벼 [TAB]��ذ� [h]elp\n\
\033[30;47m  �s��    �� ��  �@  ��       ��  ��  ��  �D                                  \033[m");
  return post_body(xo);
}


static int
post_visit(xo)
  XO *xo;
{
  int ans, row, max;
  HDR *fhdr;

  ans = vans("�]�w�Ҧ��峹 (U)��Ū (V)�wŪ (Q)�����H [Q] ");
  if (ans == 'v' || ans == 'u')
  {
    brh_visit(ans = ans == 'u');

    row = xo->top;
    max = xo->max - xo->top + 3;
    if (max > b_lines)
      max = b_lines;

    fhdr = (HDR *) xo_pool;
    row = 3;

    do
    {
      move(row, 7);
      outc(post_attr(fhdr++));
    } while (++row < max);
  }
  return XO_FOOT;
}


int
getsubject(row, reply)
  int row;
  int reply;
{
  char *title;

  title = ve_title;

  if (reply)
  {
    char *str;

    str = currtitle;
    if(STR4(str) == STR4(STR_REPLY)) /* Thor.980914: ��������I��? */
    {
      strcpy(title, str);
    }
    else
    {
      sprintf(title, STR_REPLY "%s", str);
      title[TTLEN] = '\0';
    }
  }
  else
  {
    *title = '\0';
  }

  return vget(row, 0, "���D�G", title, TTLEN + 1, GCARRY);
}


static int
post_add(xo)
  XO *xo;
{
  int cmd;

  curredit = EDIT_OUTGO;
  cmd = do_post(NULL);
  return cmd;
}


int
post_cross(xo)
  XO *xo;
{
  char xboard[20], fpath[80], xfolder[80], xtitle[80], buf[80], *dir;
  HDR *hdr, xpost, xhdr ,bhdr;
  int method, rc, tag, locus, battr;
  FILE *xfp;

  if (!cuser.userlevel)
    return XO_NONE;

  /* lkchu.990428: mat patch ��ݪO�|����w�ɡA�ץ�cross post�|�_�u�����D */
  if (bbsmode == M_READA)
  {
    battr = (bshm->bcache + brd_bno(currboard))->battr;
    if (!HAS_PERM(PERM_SYSOP) && (battr & BRD_NOFORWARD))
    {
      outz("�� ���O�峹���i��K");
      return -1;
    }
  }

  // check delete or not .. by statue 2000/05/18
  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
    return XO_NONE;
  if((hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP))
  {
     vmsg("Access Deny!");
     return XO_NONE;
  }

  /* verit 021113 : �ѨM�b po �峹�M��� ctrl+u �M�ᴫ��ݪO�h��������Ƽ��D���D */
  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return XO_FOOT;
  }


  /* lkchu.981201: �����K */
  tag = AskTag("��K");

  if (tag < 0)
    return XO_FOOT;

  if (ask_board(xboard, BRD_W_BIT,
      "\n\n[1;33m�ЬD��A���ݪO�A�ۦP�峹���ŶW�L�T�O�C[m\n\n")
    && (*xboard || xo->dir[0] == 'u'))	/* �H�c���i�H��K��currboard */
  {
    if (*xboard == 0)
      strcpy(xboard, currboard);

    hdr = tag ? &xhdr : (HDR *) xo_pool + (xo->pos - xo->top);
          /* lkchu.981201: �����K */
    if(!tag && (hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP))
    {
       vmsg("���峹�T����K�I");
       return XO_HEAD;
    }
          
    method = 1;
    if ((HAS_PERM(PERM_ALLBOARD) || !strcmp(hdr->owner, cuser.userid)) &&
      (vget(2, 0, "(1)������ (2)����峹�H[1] ", buf, 3, DOECHO) != '2'))
    {
      method = 0;
    }

    if (!tag)   /* lkchu.981201: �������N���n�@�@�߰� */
    {
      if (method)
        sprintf(xtitle, "[���]%.66s", hdr->title);
      else
        strcpy(xtitle, hdr->title);

      if (!vget(2, 0, "���D�G", xtitle, TTLEN + 1, GCARRY))
        return XO_HEAD;
    }
    
    rc = vget(2, 0, "(S)�s�� (L)���� (Q)�����H[Q] ", buf, 3, LCECHO);
    if (rc != 'l' && rc != 's')
      return XO_HEAD;
      
    locus = 0;
    dir = xo->dir;

    battr = (bshm->bcache + brd_bno(xboard))->battr; 

    do	/* lkchu.981201: �����K */
    {
      if (tag)
      {
        EnumTagHdr(hdr, dir, locus++);

        if (method)
          sprintf(xtitle, "[���]%.66s", hdr->title);
        else
          strcpy(xtitle, hdr->title);
      }

      /* verit 2002.04.04 : �������� , �ˬd tag ���g�O�_�Q�R���Ψ����L */
      if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
        continue;

      /* if (rc == 'l' || rc == 's') */
      /* lkchu.981201: ������o��� rc �� 's' or 'l' */
      if(!((hdr->xmode & POST_LOCK) && !HAS_PERM(PERM_SYSOP)))
      {
        /* hdr_fpath(fpath, xo->dir, hdr); */
        xo_fpath(fpath, dir, hdr);      /* lkchu.981201 */
        brd_fpath(xfolder, xboard, fn_dir);

        if (method)
        {
  	  method = hdr_stamp(xfolder, 'A', &xpost, buf);
  	  xfp = fdopen(method, "w");

  	  strcpy(ve_title, xtitle);
  	  strcpy(buf, currboard);
  	  strcpy(currboard, xboard);

  	  ve_header(xfp);

  	  strcpy(currboard, buf);

  	  if (hdr->xname[0] == '@')
  	    sprintf(buf, "%s] �H�c", cuser.userid);
  	  else
  	    strcat(buf, "] �ݪO");
  	  fprintf(xfp, "�� ��������� [%s\n\n", buf);

  	  f_suck(xfp, fpath);
  	  /* ve_sign(xfp); */
  	  fclose(xfp);
  	  close(method);

    	  strcpy(xpost.owner, cuser.userid);
  	  /* if (rc == 's') */
  	    strcpy(xpost.nick, cuser.username);
        }
        else
        {
  	  hdr_stamp(xfolder, HDR_LINK | 'A', &xpost, fpath);
  	  memcpy(xpost.owner, hdr->owner,
  	  sizeof(xpost.owner) + sizeof(xpost.nick));
          memcpy(xpost.date, hdr->date, sizeof(xpost.date));
                                 /* lkchu.981201: �������O�d���� */
        }

        /* Thor.981205: �ɥ� method �s��ݪ��ݩ� */
        /* method = (bshm->bcache + brd_bno(xboard))->battr; */

        /* Thor.990111: �b�i�H��X�e, �ncheck user���S����X���v�O? */
        if (!HAS_PERM(PERM_INTERNET) || (/* method */ battr & BRD_NOTRAN))
          rc = 'l';

        strcpy(xpost.title, xtitle);

        if (rc == 's' && (!(battr & BRD_NOTRAN)))
  	  xpost.xmode = POST_OUTGO;

        memcpy(&bhdr,hdr,sizeof(HDR));
        strcpy(bhdr.owner,cuser.userid);
#ifdef  HAVE_DETECT_CROSSPOST        
        if(checksum_find(fpath,0,battr))
        {
          move_post(&bhdr,BRD_VIOLATELAW,-2);

          add_deny(&cuser,DENY_SEL_POST|DENY_DAYS_1|DENY_MODE_POST,0);
          deny_log_email(cuser.vmail,(cuser.userlevel & PERM_DENYSTOP) ? -1 : cuser.deny);
          bbstate &= ~STAT_POST;
          cuser.userlevel &= ~PERM_POST;

          board_main();
        }
#endif

        rec_add(xfolder, &xpost, sizeof(xpost));
#ifdef	HAVE_DETECT_VIOLAWATE        
        if (rc == 's' && (!(battr & BRD_NOTRAN)) && (!(seek_log(xpost.title,battr))))
#else
        if (rc == 's' && !(battr & BRD_NOTRAN))
#endif
  	  outgo_post(&xpost, xboard);
      }
    } while (locus < tag);
       
    /* Thor.981205: check �Q�઺�����S���C�J����? */
    if (/* method */ battr & BRD_NOCOUNT)
    {
      outs("��������A�峹���C�J�����A�q�Х]�[�C");
    }
    else
    {
      /* cuser.numposts++; */
      cuser.numposts += (tag == 0) ? 1 : tag; /* lkchu.981201: �n�� tag */
      vmsg("�������");
    }
  }
  return XO_HEAD;
}

#ifdef HAVE_MULTI_CROSSPOST
static int
post_xcross(xo)
  XO *xo;
{
  char *xboard, fpath[80], xfolder[80], buf[80], *dir;
  HDR *hdr, xpost, xhdr ;
  int tag, locus, listing;
  LinkList *wp;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  // check delete or not .. by statue 2000/05/18
  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
    return XO_NONE;

  tag = AskTag("�s����K");

  if (tag < 0)
    return XO_FOOT;

  ll_new();
  listing = brd_list(0);

  if (listing)
  {

    hdr = tag ? &xhdr : hdr;
          
    vget(2, 0, "(Y)�T�w (Q)�����H[Q] ", buf, 3, LCECHO);
    if (*buf != 'y')
      return XO_HEAD;
      
    dir = xo->dir;

    wp = ll_head;

    do
    {
      locus = 0;
      xboard = wp->data;
      do
      {
        if (tag)
        {
          EnumTagHdr(hdr, dir, locus++);
        }
        if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_CURMODIFY))
          continue;

        xo_fpath(fpath, dir, hdr);
        brd_fpath(xfolder, xboard, fn_dir);

        hdr_stamp(xfolder, HDR_LINK | 'A', &xpost, fpath);
        memcpy(xpost.owner, hdr->owner,
        sizeof(xpost.owner) + sizeof(xpost.nick));
        memcpy(xpost.date, hdr->date, sizeof(xpost.date));

        strcpy(xpost.title, hdr->title);

        rec_add(xfolder, &xpost, sizeof(xpost));
      } while (locus < tag);
    } while (wp = wp->next);
  }
  return XO_HEAD;
}

#endif


/* ----------------------------------------------------- */
/* ��Ƥ��s���Gedit / title				 */
/* ----------------------------------------------------- */


static void
post_history(xo, fhdr)
  XO *xo;
  HDR *fhdr;
{
  int prev, chrono, next, pos, top;
  char *dir;
  HDR buf;

  chrono = fhdr->chrono;
  if (!brh_unread(chrono))
    return;

  dir = xo->dir;
  pos = xo->pos;
  top = xo->top;

  if (--pos >= top)
  {
    prev = fhdr[-1].chrono;
  }
  else
  {
    if (!rec_get(dir, &buf, sizeof(HDR), pos))
      prev = buf.chrono;
    else
      prev = chrono;
  }

  pos += 2;
  if (pos < top + XO_TALL)
    next = fhdr[1].chrono;
  else
  {
    if (!rec_get(dir, &buf, sizeof(HDR), pos))
      next = buf.chrono;
    else
      next = chrono;
  }

  brh_add(prev, fhdr->chrono, next);
}


static int
post_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int cmd, xmode, pos;
  char *dir, fpath[64];
  
  char poolbuf[sizeof(HDR)*20];

  int key;

  dir = xo->dir;
  cmd = XO_NONE;

  for (;;)
  {

    pos = xo->pos;
    hdr = (HDR *) xo_pool + (pos - xo->top);
    xmode = hdr->xmode;
    if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
      break;

#ifdef	HAVE_USER_MODIFY
	if(xmode & POST_CURMODIFY)
	{
	  if(pid_find(hdr->xid))
	  {
		vmsg("���峹���b�ק襤!!");
		break;
          }
          else
          {
            xmode = (hdr->xmode &= ~POST_CURMODIFY);
            hdr->xid = 0;
            rec_put(dir, hdr, sizeof(HDR), pos);
          }
	}
#endif

    if((hdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
      break;

    hdr_fpath(fpath, dir, hdr);

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if((key = more(fpath, MSG_POST)) == -1)
      break;

    cmd = XO_LOAD;
    post_history(xo, hdr);
    strcpy(currtitle, str_ttl(hdr->title));
    
    if(key == -2)
      return XO_INIT;
    switch (xo_getch(xo, key))
    {
    case XO_BODY:
      continue;
    case Ctrl('U'):
      memcpy(poolbuf,xo_pool,sizeof(HDR)*20);
      every_U();
      memcpy(xo_pool,poolbuf,sizeof(HDR)*20);
      continue;
    case Ctrl('B'):
      every_B();
      continue;
    case 'y':
    case 'r':
      if (bbstate & STAT_POST)
      {
	strcpy(quote_file, fpath);
	if (do_reply(hdr) == XO_INIT)	/* �����\�a post �X�h�F */
	  return post_init(xo);
      }
      break;

    case 'm':
      if ((bbstate & STAT_BOARD) && !(xmode & POST_MARKED))
      {
	hdr->xmode = xmode | POST_MARKED;
	rec_put(dir, hdr, sizeof(HDR), pos);
      }
      break;
    }
    break;
  }

  return post_init(xo);
}


/* ----------------------------------------------------- */
/* ��ذ�						 */
/* ----------------------------------------------------- */


int
post_gem(xo)
  XO *xo;
{
  char fpath[32];

  strcpy(fpath, "gem/");
  strcpy(fpath + 4, xo->dir);

  /* Thor.990118: �ݪ��`�ޤ��� GEM_SYSOP */
  XoGem(fpath, "", (HAS_PERM(PERM_SYSOP|PERM_BOARD|PERM_GEM)) ? GEM_SYSOP :
    (bbstate & STAT_BOARD ? GEM_MANAGER : GEM_USER));

  return post_init(xo);
}


/* ----------------------------------------------------- */
/* �ݪO�Ƨѿ�						 */
/* ----------------------------------------------------- */


static int
post_memo(xo)
  XO *xo;
{
  char fpath[64];

  brd_fpath(fpath, currboard, FN_NOTE);
  /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
  if (more(fpath, NULL) == -1)
  {
    vmsg("���ݪO�|�L�u�Ƨѿ��v");
    return XO_FOOT;
  }

  return post_head(xo);
}

static int
post_post(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  mode = vans("�o�夽�i (D)�R�� (E)�ק� (Q)�����H[E] ");
  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, "post");
    if (mode == 'd')
    {
      unlink(fpath);
    }
    else
    {
      if(bbsothermode & OTHERSTAT_EDITING)
      {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
      }
      else
      {
        if (vedit(fpath, NA))
          vmsg(msg_cancel);
        return post_head(xo);
      }
    }
  }
  return XO_FOOT;
}

static int
post_memo_edit(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  mode = vans("�Ƨѿ� (D)�R�� (E)�ק� (Q)�����H[E] ");
  if (mode != 'q')
  {
    brd_fpath(fpath, currboard,FN_NOTE);
    if (mode == 'd')
    {
      unlink(fpath);
    }
    else
    {
      if(bbsothermode & OTHERSTAT_EDITING)
      {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
      }
      else
      {
        if (vedit(fpath, NA))
  	  vmsg(msg_cancel);
        return post_head(xo);
      }
    }
  }
  return XO_FOOT;
}


static int
post_switch(xo)
  XO *xo;
{
  int bno;
  BRD *brd;
  char bname[16];

  if (brd = ask_board(bname, BRD_R_BIT, NULL))
  {
    if (*bname && ((bno = brd - bshm->bcache) >= 0))
    {
      XoPost(bno);
      return XZ_POST;
    }
  }
  else
  {
    vmsg(err_bid);
  }
  return post_head(xo);
}


/* ----------------------------------------------------- */
/* �\��Gtag / copy / forward / download		 */
/* ----------------------------------------------------- */


int
post_tag(xo)
  XO *xo;
{
  HDR *hdr;
  int tag, pos, cur;

  pos = xo->pos;
  cur = pos - xo->top;
  hdr = (HDR *) xo_pool + cur;

#ifdef XZ_XPOST
  if (xo->key == XZ_XPOST)
    pos = hdr->xid;
#endif

  if (tag = Tagger(hdr->chrono, pos, TAG_TOGGLE))
  {
    move(3 + cur,0);
    //move(3 + cur, 8);
    //outc(tag > 0 ? '*' : ' ');
	//outs(tag > 0 ? " *" : "  ");
    post_item(xo->pos + 1,hdr);
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE; /* lkchu.981201: ���ܤU�@�� */
}


/* ----------------------------------------------------- */
/* �O�D�\��Gmark / delete				 */
/* ----------------------------------------------------- */


static int
post_mark(xo)
  XO *xo;
{
  if (bbstate & STAT_BOARD)
  {
    HDR *hdr;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
      return XO_NONE;
    hdr->xmode ^= POST_MARKED;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
//    move(3 + cur, 7);
//    outc(post_attr(hdr));
    move(3 + cur, 0);
	post_item(pos + 1,hdr);

  }
  return XO_NONE;
}

static int
post_complete(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_SYSOP|PERM_BOARD))
  {
    HDR *hdr;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    hdr->xmode ^= POST_COMPLETE;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
    move(3 + cur, 7);
    outc(post_attr(hdr));
  }
  return XO_NONE;
}

static int
post_lock(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD)
  {
    HDR *hdr;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE))
      return XO_NONE;
    hdr->xmode ^= POST_LOCK;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
    move(3 + cur, 7);
    outc(post_attr(hdr));
  }
  return XO_NONE;
}

static int
lazy_delete(hdr)
  HDR *hdr;
{
  sprintf(hdr->title, "<< ���峹�g %s �R�� >>", cuser.userid);
  if(!strcmp(hdr->owner,cuser.userid))
    hdr->xmode |= POST_DELETE;
  else 
    hdr->xmode |= POST_MDELETE;

  return 0;
}

static int
post_state(xo)
  XO *xo;
{
  HDR *hdr;
  char *dir, fpath[80];
  struct stat st;

  if (!(HAS_PERM(PERM_SYSOP)))
    return XO_NONE;

  dir = xo->dir;
  hdr = (HDR *) xo_pool + xo->pos - xo->top;
  hdr_fpath(fpath,dir,hdr);


  move(12, 0);
  clrtobot();
  outs("\nDir : ");
  outs(dir);
  outs("\nName: ");
  outs(hdr->xname);
  outs("\nFile: ");
  outs(fpath);
  if (!stat(fpath, &st))
    prints("\nTime: %s\nSize: %d\n", Ctime(&st.st_mtime), st.st_size);
  bitmsg("Flag: ", "rmg---cdIEOR--------DLSMC-------", hdr->xmode);
  prints("Xid : %d\n",hdr->xid);

  vmsg(NULL);

  return post_body(xo);
}



static int
post_delete(xo)
  XO *xo;
{
  int pos, cur, by_BM;
  HDR *fhdr;
  char buf[80],fpath[80];

#define BN_DELETED	BRD_DELETED
#define BN_JUNK		BRD_JUNK

  if (!cuser.userlevel ||
    !strcmp(currboard, BN_DELETED) ||
    !strcmp(currboard, BN_JUNK))
    return XO_NONE;

  if(cuser.userlevel & PERM_DENYPOST)
  {
    vmsg("�A���Q���v���A�L�k�R������峹�I");
    return XO_NONE;
  }

  pos = xo->pos;
  cur = pos - xo->top;
  fhdr = (HDR *) xo_pool + cur;

  if (fhdr->xmode & (POST_MARKED | POST_CANCEL | POST_DELETE | POST_MDELETE))
    return XO_NONE;

  if((fhdr->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
    return XO_NONE;

  by_BM = strcmp(fhdr->owner, cuser.userid);
  if (!(bbstate & STAT_BOARD) && by_BM)
    return XO_NONE;

  hdr_fpath(fpath,xo->dir,fhdr);
  if (vans(msg_del_ny) == 'y')
  {
    currchrono = fhdr->chrono;

    /* Thor.980911: for ���D��峹 in �걵 */
    /* if (!rec_del(xo->dir, sizeof(HDR), xo->pos, cmpchrono, lazy_delete)) */
    if (!rec_del(xo->dir, sizeof(HDR), xo->key == XZ_POST ? pos : fhdr->xid, cmpchrono, lazy_delete))
    {
      move_post(fhdr, by_BM ? BN_DELETED : BN_JUNK, by_BM);
      if (!by_BM && !(bbstate & BRD_NOCOUNT))
      {
	if (cuser.numposts > 0)
	  cuser.numposts--;
	sprintf(buf, "%s�A�z���峹� %d �g", MSG_DEL_OK, cuser.numposts);
	vmsg(buf);
#ifdef  HAVE_DETECT_CROSSPOST	
	checksum_find(fpath,1,bbstate);
#endif
      }
      lazy_delete(fhdr); /* Thor.980911: ����: �ק� xo_pool */
      move(3 + cur, 0);
      post_item(++pos, fhdr); 
    }
  }
  return XO_FOOT;

#undef	BN_DELETED
#undef	BN_JUNK
}

static int
post_undelete(xo)
  XO *xo;
{
  int pos, cur, i, len;
  HDR *fhdr;
  char buf[256],fpath[128],*ptr;
  FILE *fp;

  if (!cuser.userlevel)
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  fhdr = (HDR *) xo_pool + cur;
  hdr_fpath(fpath,xo->dir,fhdr);

  if (!(fhdr->xmode & (POST_MDELETE | POST_DELETE | POST_CANCEL)))
    return XO_NONE;

  if( !(((fhdr->xmode & POST_DELETE) && !strcmp(fhdr->owner, cuser.userid))||
      ((fhdr->xmode & (POST_MDELETE | POST_CANCEL)) && (bbstate & STAT_BOARD))||
      HAS_PERM(PERM_SYSOP)) )
    return XO_NONE;


  fp = fopen(fpath,"r");  
  if(fp)
  {
    fgets(buf, 256, fp);
    fgets(buf, 256, fp);
    buf[strlen(buf)-1] = 0;
    ptr = strchr(buf, ':');
    ptr = ptr ? ptr+2:buf;    
    strncpy(fhdr->title,ptr,60);
    if(!HAS_PERM(PERM_SYSOP))
    {
      sprintf(buf,"{%s}",cuser.userid);
      strcat(fhdr->title,buf);
    }
    fhdr->title[71] = 0;  /* verit 2002.01.23 �קK�Ϥ峹�y�� title �z�� */

    /* verit 2003.10.16 �קK�Ϥ峹�� , �X�{�m����D */
    len = strlen(fhdr->title);
    for( i=0 ; i<len ; ++i )
      if( fhdr->title[i] == '\033' )
         fhdr->title[i] = '*' ;

    fclose(fp);
    if(!strcmp(fhdr->owner,cuser.userid) && (fhdr->xmode & POST_DELETE)
       && !(bbstate & BRD_NOCOUNT))
    {
      /*
      cuser.numposts++;
      sprintf(buf, "�_��R���A�z���峹�W�� %d �g", cuser.numposts);
      vmsg(buf);*/  /* 20000724 visor: ���峹�g�ƪ� bug */
#ifdef  HAVE_DETECT_CROSSPOST      
      checksum_find(fpath,0,bbstate);    
#endif
    }
  }    
  fhdr->xmode &= (~(POST_MDELETE | POST_DELETE | POST_CANCEL));
  if (!rec_put(xo->dir, fhdr, sizeof(HDR), pos))
  {
    move(3 + cur, 0);
    post_item(++pos, fhdr);
  }
  /*return XO_LOAD;*/
  return xo->pos + 1 + XO_MOVE;
}

/* ----------------------------------------------------- */
/* �����\��Gedit / title				 */
/* ----------------------------------------------------- */


int
post_edit(xo)
  XO *xo;
{
  HDR *hdr;
  char fpath[80];
  int pos;
#ifdef	HAVE_USER_MODIFY
  int bno;
  BRD *brd;
  char buf[512],mfpath[80],mfolder[80],str[256];
  int fd;
  HDR phdr;
  time_t now;
  FILE *fp,*xfp;
	
  bno = brd_bno(currboard);
  brd = bshm->bcache + bno;


#endif

  if(bbsothermode & OTHERSTAT_EDITING)
  {
    vmsg("�A�٦��ɮ��٨S�s���@�I");
    return XO_FOOT;
  }
  pos = xo->pos;
  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  hdr_fpath(fpath, xo->dir, hdr);
#if 0
  if ((cuser.userlevel & PERM_ALLBOARD)|| ( (cuser.userlevel & PERM_VALID) \
                      && !strcmp(hdr->owner, cuser.userid)))
#endif
  if (HAS_PERM(PERM_SYSOP) && !(hdr->xmode & (POST_CANCEL|POST_DELETE)))
  {
    /*hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    hdr_fpath(fpath, xo->dir, hdr);*/
    vedit(fpath, NA); /* Thor.981020: �`�N�Qtalk�����D */
    post_head(xo);
  }
#ifdef	HAVE_USER_MODIFY
  else if((brd->battr & BRD_MODIFY) && HAS_PERM(PERM_VALID) && !strcmp(hdr->owner, cuser.userid) && !(hdr->xmode & (POST_MODIFY|POST_CANCEL|POST_DELETE|POST_LOCK|POST_MARKED|POST_MDELETE|POST_CURMODIFY)) )
  {
//    move_post(hdr,BRD_MODIFIED,-3);
        brd_fpath(mfolder, BRD_MODIFIED, FN_DIR);
        fd = hdr_stamp(mfolder, 'A', &phdr, mfpath);
        fp = fdopen(fd, "w");
	f_suck(fp, fpath);
	fclose(fp);
	close(fd);
	strcpy(phdr.owner,hdr->owner);
	strcpy(phdr.nick,hdr->nick);
	strcpy(phdr.title,hdr->title);


	rec_add(mfolder, &phdr, sizeof(HDR));

	hdr->xmode |= POST_CURMODIFY;
	hdr->xid = cutmp->pid;
	rec_put(xo->dir, hdr, sizeof(HDR), pos);

	strcpy(ve_title,hdr->title);
		
	fp = fopen(fpath,"r");

	sprintf(buf,"tmp/%s.header",cuser.userid);
	xfp = fopen(buf,"w");
	while (fgets(str, 256, fp) && *str != '\n')
	{
		fputs(str,xfp);
	}
	fputs("\n",xfp);
	fclose(xfp);

	sprintf(buf,"tmp/%s.edit",cuser.userid);
	xfp = fopen(buf,"w");
	while (fgets(str, 256, fp))
	{
		if (!strcmp(str,"--\n"))
			break;
		fputs(str,xfp);
	}
	fclose(xfp);

	sprintf(buf,"tmp/%s.footer",cuser.userid);
	xfp = fopen(buf,"w");
	fputs("--\n",xfp);
	while (fgets(str, 256, fp))
	{
		fputs(str,xfp);
	}
	fclose(xfp);

	fclose(fp);

	sprintf(buf,"tmp/%s.edit",cuser.userid);

	if(vedit(buf, NA)<0)
	{
	  sprintf(buf,"tmp/%s.header",cuser.userid);
	  unlink(buf);
	  sprintf(buf,"tmp/%s.edit",cuser.userid);
	  unlink(buf);
	  sprintf(buf,"tmp/%s.footer",cuser.userid);
	  unlink(buf);

	  rec_get(xo->dir, &phdr, sizeof(HDR), pos);
	  phdr.xmode &= ~POST_CURMODIFY;
	  phdr.xid = 0;
	  rec_put(xo->dir, &phdr, sizeof(HDR), pos);
	  vmsg("�����ק�");
	}
	else
	{
	  fp = fopen(fpath,"w");
	  sprintf(buf,"tmp/%s.header",cuser.userid);
	  f_suck(fp, buf);
	  unlink(buf);
	  sprintf(buf,"tmp/%s.edit",cuser.userid);
	  f_suck(fp, buf);
	  unlink(buf);
	  sprintf(buf,"tmp/%s.footer",cuser.userid);
	  f_suck(fp, buf);
	  unlink(buf);
	  fclose(fp);


	  time(&now);
	  sprintf(buf,"\033[1;32m�� Modify: \033[;1m<%s> \033[31;1m%s\033[m",fromhost,ctime(&now));
	  f_cat(fpath,buf);
	  rec_get(xo->dir, &phdr, sizeof(HDR), pos);

	  phdr.xmode |= POST_MODIFY;
	  phdr.xmode &= ~POST_CURMODIFY;
	  phdr.xid = 0;
	  rec_put(xo->dir, &phdr, sizeof(HDR), pos);
	  vmsg("�ק粒��");
	}


    post_init(xo);
  }
  else if(!(brd->battr & BRD_MODIFY))
  {
    vmsg("��������ק�峹!!");
  }
  else
  {
    vmsg("���峹����A�Q�ק�!!");
  }
#endif
  return XO_NONE;
}


int
post_title(xo)
  XO *xo;
{
  HDR *fhdr, mhdr;
  int pos, cur;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  pos = xo->pos;
  cur = pos - xo->top;
  fhdr = (HDR *) xo_pool + cur;
  mhdr = *fhdr;

  vget(b_lines, 0, "���D�G", mhdr.title, sizeof(mhdr.title), GCARRY);
  vget(b_lines, 0, "�@�̡G", mhdr.owner, 74 /* sizeof(mhdr.owner)*/, GCARRY);
           /* Thor.980727:lkchu patch: sizeof(mhdr.owner) = 80�|�W�L�@�� */
  vget(b_lines, 0, "����G", mhdr.date, sizeof(mhdr.date), GCARRY);
  if (vans(msg_sure_ny) == 'y' &&
    memcmp(fhdr, &mhdr, sizeof(HDR)))
  {
    *fhdr = mhdr;
    rec_put(xo->dir, fhdr, sizeof(HDR), pos);
    move(3 + cur, 0);
    post_item(++pos, fhdr);
  }
  return XO_FOOT;
}


#ifdef HAVE_TERMINATOR
static int
post_cross_terminator(xo)	/* Thor.0521: �׷��峹�j�k */
  XO *xo;
{
  char *title, buf[100],other[50];
  int mode;
  HDR *fhdr;

  fhdr = (HDR *) xo_pool + xo->pos - xo->top;
  if (fhdr->xmode & (POST_DELETE | POST_MDELETE | POST_CANCEL | POST_LOCK))
    return XO_NONE;


  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  mode = vans("�m�تḨ���١n�G 1)����D 2)��ϥΪ� 3)��L [1]�G") - '1';
  if(mode > 2 || mode < 0)
    mode =0;

  strcpy(currtitle, str_ttl(fhdr->title));

  if(mode==1)
    title = fhdr->owner;
  else if(mode == 2)
  {
    if(!vget(b_lines, 0, "��L�G", other, sizeof(other), DOECHO))
      return XO_HEAD;
    title = other;
  }
  else
    title = currtitle;
  if (!*title)
    return XO_NONE;

  if(mode==1)
    sprintf(buf, "�m�تḨ���١n�ϥΪ̡G%.40s�A�T�w�ܡHY/[N]", title);
  else if(mode ==2)
    sprintf(buf, "�m�تḨ���١n��L�G%.50s�A�T�w�ܡHY/[N]", title);
  else
    sprintf(buf, "�m�تḨ���١n���D�G%.40s�A�T�w�ܡHY/[N]", title);


  if (vans(buf) == 'y')
  {
    BRD *bhdr, *head, *tail;

    /* Thor.0616: �O�U currboard, �H�K�_�� */
    strcpy(buf, currboard);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do				/* �ܤ֦�sysop�@�� */
    {
      int fdr, fsize, xmode;
      FILE *fpw;
      char fpath[80];
      char fnew[80], fold[80];
      HDR *hdr;

      if(!strcmp(head->brdname,BRD_LOCALPOSTS))  /* LocalPosts ������ */ 
        continue;         

      /* Thor.0616:���currboard,�Hcancel post */

      strcpy(currboard, head->brdname);

      sprintf(fpath, "�m�تḨ���١n�ݪ��G%s \033[5m...\033[m", currboard);
      outz(fpath);
      refresh();

      brd_fpath(fpath, currboard, fn_dir);

      if ((fdr = open(fpath, O_RDONLY)) < 0)
	continue;

      if (!(fpw = f_new(fpath, fnew)))
      {
	close(fdr);
	continue;
      }

      fsize = 0;
      mgets(-1);
      while (hdr = mread(fdr, sizeof(HDR)))
      {
        int check_mode;
	xmode = hdr->xmode;

/*	if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE | POST_LOCK))
	  continue;*/

        if(mode==1)
          check_mode = strcmp(title, str_ttl(hdr->owner));  
        else if(mode==2)
          check_mode = !((int)strstr(hdr->owner,title)|(int)strstr(hdr->title,title));
        else      
          check_mode = strcmp(title, str_ttl(hdr->title));

	if ((xmode & (POST_MARKED | POST_CANCEL | POST_DELETE | POST_MDELETE | 
	     POST_LOCK)) || check_mode )
	{
#if 0
	  if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
	  {
	    fclose(fpw);
	    unlink(fnew);
	    close(fdr);
	    goto contWhileOuter;
	  }
	  fsize++;
#endif
	}
	else
	{
	  /* �Y���ݪO�N�s�u��H */

	  cancel_post(hdr);
          hdr->xmode |= POST_MDELETE;
          sprintf(hdr->title, "<< ���峹�g %s ���t�Υ\\��R�� >>", cuser.userid);             
	  /*hdr_fpath(fold, fpath, hdr);
	  unlink(fold);*/
	}

        if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
        {
          fclose(fpw);
          unlink(fnew);
          close(fdr);
          goto contWhileOuter;
        }
        fsize++;	
      }
      close(fdr);
      fclose(fpw);

      sprintf(fold, "%s.o", fpath);
      rename(fpath, fold);
      if (fsize)
	rename(fnew, fpath);
      else
	unlink(fnew);

  contWhileOuter:

    } while (++head < tail);

    strcpy(currboard, buf);
    post_load(xo);
  }

  return XO_FOOT;
}
#endif

int
post_ban_mail(xo)
  XO *xo;
{

  if ((bbstate & STAT_BOARD)||HAS_PERM(PERM_ALLBOARD))
  {
    post_mail();
    return post_init(xo);
  }
  else
    return XO_NONE;
}

#ifdef	HAVE_BRDTITLE_CHANGE
static int
post_brdtitle(xo)
 XO *xo;
{
  int bno;
  BRD *oldbrd, newbrd;
        
  if( !(bbstate & STAT_BOARD) ) /* �P�� visor@YZU */
    return XO_NONE;

  bno = brd_bno(currboard);
  oldbrd = bshm->bcache + bno;
  if(!(oldbrd->battr & BRD_CHANGETITLE) && !HAS_PERM(PERM_SYSOP))
    return XO_NONE;
  
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  vget(23, 0, "�ݪO�W�١G", newbrd.title+3, BTLEN - 2, GCARRY);

  if ((vans(msg_sure_ny) == 'y') &&
    memcmp(&newbrd, oldbrd, sizeof(BRD)))
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), bno);
  }
  vmsg("�]�w����");

  return XO_HEAD;
}
#endif

#ifdef	HAVE_RECOMMEND
static int
post_recommend(xo)
  XO *xo;
{

  PRH prh,buf;
  HDR *hdr;
  int pos, cur;
  BRD *brd;
  int fv;
  char fpath[80];
  char ans;
#ifdef HAVE_SONG
  ACCT acct;
#endif  

  brd = bshm->bcache + brd_bno(currboard);
  
  if (HAS_PERM(PERM_VALID) && (brd->battr & BRD_PRH) && (bbstate & STAT_POST))
  {

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK))
      return XO_NONE;

    if(!strcmp(cuser.userid,hdr->owner))
    {
      vmsg("������˦ۤv���峹�I");
      return XO_FOOT;
    }
#ifdef HAVE_SONG
    acct_load(&acct,cuser.userid);

    if(acct.request<1)
    {
      vmsg("�峹�����I�Ƥw�Χ��I");
      return XO_FOOT;
    }
#endif
              
    prh.bstamp = brd->bstamp;
    prh.chrono = hdr->chrono;

    usr_fpath(fpath, cuser.userid, FN_PRH);
    fv = open(fpath, O_RDWR | O_CREAT, 0600);

    f_exlock(fv);

    while (read(fv, &buf, sizeof(PRH)) == sizeof(PRH))
    {
      if (!memcmp(&prh, &buf, sizeof(PRH)))
      {
        f_unlock(fv);
        close(fv);
        vmsg("�A�w�g���˹L�F�I");
        return XO_FOOT;
      }
    }

    ans = vans("�z�n���˵���L�H�� ? [y/N]");
    if(ans != 'y' && ans != 'Y')
    {
      f_unlock(fv);
      close(fv);
      return XO_FOOT;
    }

    if(hdr->recommend < 99)
    {
      char tmp[128];
      hdr->recommend++;
      rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
      write(fv, &prh, sizeof(PRH));
      vmsg("���˦��\\�I");
#ifdef HAVE_SONG      
      acct_load(&acct,cuser.userid);
      acct.request -= 1;
      cuser.request = acct.request;
      acct_save(&acct);
#endif 
      sprintf(tmp,"%s  %s",hdr->xname,hdr->title);
      logitfile(FN_RECOMMEND_LOG, currboard, tmp);     
    }
    f_unlock(fv);
    close(fv);
    move(3 + cur, 0);
    post_item(pos+1, hdr);  
  }
  return XO_FOOT;
}

static int
post_cleanrecommend(xo)
  XO *xo;
{


  HDR *hdr;
  int pos, cur;
  char ans;

  if ((bbstate & STAT_BOARD) || HAS_PERM(PERM_BOARD))
  {

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    if(hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK))
      return XO_NONE;

	ans = vans("�z�n�M�����ˤ��� ? [y/N]");
	if(ans != 'y' && ans != 'Y')
	{
        return XO_FOOT;
	}
    hdr->recommend = 0;
    rec_put(xo->dir, hdr, sizeof(HDR), xo->key == XZ_POST ? pos : hdr->xid);
    move(3 + cur, 0);
	post_item(pos+1, hdr);  
  }
  return XO_FOOT;
}
#endif

static int
post_help(xo)
  XO *xo;
{
  film_out(FILM_BOARD, -1);
  return post_head(xo);
}


static int
post_spam(xo)
  XO *xo;
{
  HDR *hdr;
  char *dir, fpath[80];
  char msg[128];
  

  if (!supervisor)
    return XO_NONE;

  dir = xo->dir;
  hdr = (HDR *) xo_pool + xo->pos - xo->top;
  hdr_fpath(fpath,dir,hdr);

  sprintf(msg,"%s\n",fpath);
  f_cat(FN_SPAMPATH_LOG, msg);

  vmsg(fpath);

  return XO_FOOT;
}



KeyFunc post_cb[] =
{
  XO_INIT, post_init,
  XO_LOAD, post_load,
  XO_HEAD, post_head,
  XO_BODY, post_body,

  'B', post_ban_mail,
  'r', post_browse,
  's', post_switch,
#ifdef  HAVE_BRDTITLE_CHANGE
  'i', post_brdtitle,
#endif
  KEY_TAB, post_gem,
  'z', post_gem,
  'u', post_undelete,
  'y', post_reply,
  'd', post_delete,
  'v', post_visit,
  'S', post_state,
  'w', post_post,
  'c', post_complete,
  'q', post_spam,
  
  Ctrl('P'), post_add,
#ifdef HAVE_MULTI_CROSSPOST
  Ctrl('X'), post_xcross,
#endif
  'x', post_cross,
  Ctrl('Q'), xo_uquery,
  'I', xo_usetup,

#if 1 /* Thor.981120: �Ȯɨ���, ���~�� */
      /* lkchu.981201: �S�� 'D' �ܤ��ߺD :p */
  'D', xo_delete,
#endif

#ifdef HAVE_TERMINATOR
  'X', post_cross_terminator,
#endif

  't', post_tag,
  'l', post_lock,

  'E', post_edit,
  'T', post_title,
  'm', post_mark,

#ifdef	HAVE_RECOMMEND
  'p', post_recommend,
  'o', post_cleanrecommend,
#endif

  'R' | XO_DL, (int (*)())"bin/vote.so:vote_result",
  'V' | XO_DL, (int (*)())"bin/vote.so:XoVote",

  'b', post_memo,
  'W', post_memo_edit,

#ifdef HAVE_MODERATED_BOARD
  Ctrl('G'), XoBM,
#endif

#ifdef XZ_XPOST
  '~', XoXpost,			/* Thor: for XoXpost */
#endif

  'h', post_help
};


#ifdef XZ_XPOST
/*------------------------------------------------------------------------
  Thor.0509: �s�� �峹�j�M�Ҧ�
             �i���w�@keyword, �C�X�Ҧ�keyword�������峹�C��

  �b tmp/ �U�} xpost.{pid} �@�� folder, �t�ؤ@map�}�C, �Χ@�P��post�@map
  �O���Ӥ峹�O�b��post����B, �p���i�@ mark,gem,edit,title���\��,
  �B�����}�ɦ^�ܹ����峹�B
  <�H�W�Q�k obsolete...>

  Thor.0510:
  �إߤ峹�Q�צ�, like tin, �N�峹�� index ��J memory��,
  ���ϥ� thread, �]�� thread�n�� folder��...

  �������Mode, Title & post list

  ���Ҽ{����²�ƪ� �W�U�䲾��..

  O->O->O->...
  |  |  |
  o  o  o
  |  |  |

  index�tfield {next,text} ����int, �t�m�]�� int
  �Ĥ@�h sorted by title, ���J�ɥ� binary search
  �B MMAP only , �Ĥ@�h��� # and +

  �����ѥ���R���ʧ@, �קK�V��

  Thor.980911: �Ҽ{���ѧR�����O, ��K���D
-------------------------------------------------------------------------*/
#if 0
extern XO *xpost_xo;		/* Thor: dynamic programmin for variable dir
				 * name */
extern XO *ypost_xo;
#endif


#define	MSG_XYPOST	"[�걵�Ҧ�]���D����r:"
#define	MSG_XY_NONE	"�ŵL�@��"


typedef struct
{
  char *subject;
  int first;
  int last;
  time_t chrono;
}      Chain;			/* Thor: negative is end */



static int
chain_cmp(a, b)
  Chain *a;
  Chain *b;
{
  return a->chrono - b->chrono;
}


static int *xypostI;


/* Thor: first ypost pos in ypost_xo.key */

static int comebackPos;

/* Thor: first xpost pos in xpost_xo.key */

static char xypostKeyword[30];


/* -1 to find length, otherwise return index */


static int
XoXpost(xo)			/* Thor: call from post_cb */
  XO *xo;
{
  int *plist, *xlist, fsize, max, locus, sum, i, m, n;
  Chain *chain;
  char *fimage, *key=NULL, author[30], buf[30];
  HDR *head, *tail;
  int filter_author=0,filter_title=0,mode;
  XO *xt;

  if ((max = xo->max) <= 0) /* Thor.980911: ����: �H���U�@ */
    return XO_FOOT;

  if (xz[XZ_XPOST - XO_ZONE].xo)
  {
    vmsg("�A�w�g�ϥΤF�걵�Ҧ�");
    return XO_FOOT;
  }

  /* input condition */
  mode = vans("�� 0)�걵 1)�s�峹 2)LocalPost [0]�G") - '0';
  if(mode > 2 || mode < 0)
    mode = 0;

  if(!mode)
  {
    key = xypostKeyword;
    filter_title = vget(b_lines, 0, MSG_XYPOST, key, sizeof(xypostKeyword), GCARRY);
    str_lower(buf, key);
    key = buf;

    if (filter_author = vget(b_lines, 0, "[�걵�Ҧ�]�@�̡G", author, 30, DOECHO))
    {
      filter_author = strlen(author);
      str_lower(author, author);
    }
  }

  if(!(filter_title || filter_author || mode))
    return XO_HEAD;

  /* build index according to input condition */

  fimage = f_map(xo->dir, &fsize);

  if (fimage == (char *) -1)
  {
    vmsg("�ثe�L�k�}�ү�����");
    return XO_FOOT;
  }

  if (xlist = xypostI) /* Thor.980911: ����: �ȭ��жi�J��, ���O�O���� */
    free(xlist);

  /* allocate index memory, remember free first */

  /* Thor.990113: �Ȱ�title,author�������S���Hpost */
  max = fsize / sizeof(HDR);
  
  plist = (int *) malloc(sizeof(int) * max);
  chain = (Chain *) malloc(sizeof(Chain) * max);

  max = sum = 0;

  head = (HDR *) fimage;
  tail = (HDR *) (fimage + fsize);

  locus = -1;
  do
  {
    int left, right, mid;
    char *title;

    locus++;
    if (head->xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
      continue;			/* Thor.0701: ���L�ݤ��쪺�峹 */

    if((head->xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
      continue;

    /* check author */

    /* Thor.981109: �S�O�`�N, author�O�q�Ymatch, ���Osubstr match, �����Cload */
    if(!mode)
    {
      if (filter_author && str_ncmp(head->owner, author, filter_author))
        continue;

      /* check condition */

      title = head->title;

      if (STR4(title) == STR4(STR_REPLY)) /* Thor.980911: ���� Re: ���~ */
        title += 4;

      if (*key && !str_str(title, key))
        continue;
    }
    else if(mode == 1)
    {
      title = head->title;
      if (STR4(title) == STR4(STR_REPLY))
        continue;
    }
    else
    {
	if(strchr(head->owner,'.'))
		continue;    	
    }

    sum++;

    /* check if in table, binary check */

    left = 0;
    right = max - 1;
    for (;;)
    {
      int cmp;
      Chain *cptr;

      if (left > right)
      {
	for (i = max; i > left; i--)
	  chain[i] = chain[i - 1];

	cptr = &chain[left];
	cptr->subject = title;
	cptr->first = cptr->last = locus;
	cptr->chrono = head->chrono;
	max++;
	break;
      }

      mid = (left + right) >> 1;
      cptr = &chain[mid];
      cmp = strcmp(title, cptr->subject);

      if (!cmp)
      {
	plist[cptr->last] = locus;
	cptr->last = locus;
	break;
      }

      if (cmp < 0)
	right = mid - 1;
      else
	left = mid + 1;
    }
  } while (++head < tail);
  munmap(fimage, fsize);

  if (max <= 0)
  {
    free(chain);
    free(plist);
    vmsg(MSG_XY_NONE);
    return XO_FOOT;
  }

  if (max > 1)
    xsort(chain, max, sizeof(Chain), chain_cmp);

  xypostI = xlist = (int *) malloc(sizeof(int) * sum);

  i = locus = 0;
  do
  {
    xlist[locus++] = n = chain[i].first;
    m = chain[i].last;

    while (n != m)
    {
      xlist[locus++] = n = plist[n];
    }

  } while (++i < max);

  free(chain);
  free(plist);

  /* build XO for xpost_xo */

  if (xt = xz[XZ_XPOST - XO_ZONE].xo)
    free(xt);

  comebackPos = xo->pos;	/* Thor: record pos, future use */
  xz[XZ_XPOST - XO_ZONE].xo = xt = xo_new(xo->dir);
  xt->pos = 0;
  xt->max = sum;
  xt->xyz = xo->xyz;
  xt->key = XZ_XPOST;

  xover(XZ_XPOST);

  /* set xo->pos for new location */

  xo->pos = comebackPos;

  /* free xpost_xo */

  if (xt = xz[XZ_XPOST - XO_ZONE].xo)
  {
    free(xt);
    xz[XZ_XPOST - XO_ZONE].xo = NULL;
  }

  /* free index memory, remember check free pointer */

  if (xlist = xypostI)
  {
    free(xlist);
    xypostI = NULL;
  }

  return XO_INIT;
}


#if 0
/* Thor.980911: �@�� post_body() �Y�i*/
static int
xpost_body(xo)
  XO *xo;
{
  HDR *fhdr;
  int num, max, tail;

  max = xo->max;
#if 0
  if (max <= 0)
  { /* Thor.980911: ����: �H���U�@�� */
    vmsg(MSG_XY_NONE);
    return XO_QUIT;
  }
#endif

  fhdr = (HDR *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    post_item(++num, fhdr++);
  } while (num < max);

  clrtobot();
  return XO_NONE;
}

#endif

static int
xpost_head(xo)
  XO *xo;
{
  vs_head("�D�D��C" /* currBM */ , xo->xyz);
  outs(MSG_XYPOST);
  if (*xypostKeyword)
    outs(xypostKeyword);

  outs("\n\
\033[30;47m  �s��   �� ��  �@  ��       ��  ��  ��  �D                                   \033[m");

  /* return xpost_body(xo); */
  return post_body(xo); /* Thor.980911: �@�ΧY�i */
}


static void
xypost_pick(xo)
  XO *xo;
{
  int *xyp, fsize, pos, max, top;
  HDR *fimage, *hdr;

  fimage = (HDR *) f_map(xo->dir, &fsize);
  if (fimage == (HDR *) - 1)
    return;

  hdr = (HDR *) xo_pool;
  xyp = xypostI;

  pos = xo->pos;
  xo->top = top = (pos / XO_TALL) * XO_TALL;
  max = xo->max;
  pos = top + XO_TALL;
  if (max > pos)
    max = pos;

  do
  {
    pos = xyp[top++];
    *hdr = fimage[pos];
    hdr->xid = pos;
    hdr++;
  } while (top < max);

  munmap((void *)fimage, fsize);
}


static int
xpost_init(xo)
  XO *xo;
{
  /* load into pool */

  xypost_pick(xo);

  return xpost_head(xo);
}


static int
xpost_load(xo)
  XO *xo;
{
  /* load into pool */

  xypost_pick(xo);

  /* return xpost_body(xo); */
  return post_body(xo); /* Thor.980911: �@�ΧY�i */
}


static int
xpost_help(xo)
  XO *xo;
{
  film_out(FILM_BOARD, -1);
  return xpost_head(xo);
}


/* Thor.0509: �n�Q��k�T�� ctrl('D') */


static int
xpost_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int cmd, chrono, xmode;
  char *dir, fpath[64];

  int key;

  cmd = XO_NONE;
  dir = xo->dir;

  for (;;)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    xmode = hdr->xmode;
    if (xmode & (POST_CANCEL | POST_DELETE | POST_MDELETE))
      break;

#ifdef	HAVE_USER_MODIFY
	if(xmode & POST_CURMODIFY)
	{
		vmsg("���峹���b�ק襤!!");
		break;
	}
#endif

    if((xmode & POST_LOCK) && !(HAS_PERM(PERM_SYSOP| PERM_BOARD)||bbstate & STAT_BOARD))
      break;

    hdr_fpath(fpath, dir, hdr);

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if ((key = more(fpath, MSG_POST)) == -1)
      break;

    comebackPos = hdr->xid; 
    /* Thor.980911: �q�걵�Ҧ��^�Ӯɭn�^��ݹL�����g�峹��m */

    cmd = XO_HEAD;
    if(key == -2)
      return XO_INIT;

    chrono = hdr->chrono;
    if (brh_unread(chrono))
    {
      int prev, next, pos;
      char *dir;
      HDR buf;

      dir = xo->dir;
      pos = hdr->xid;

      if (!rec_get(dir, &buf, sizeof(HDR), pos - 1))
	prev = buf.chrono;
      else
	prev = chrono;

      if (!rec_get(dir, &buf, sizeof(HDR), pos + 1))
	next = buf.chrono;
      else
	next = chrono;

      brh_add(prev, hdr->chrono, next);
    }

    strcpy(currtitle, str_ttl(hdr->title));

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if(!key)
      key = vkey();

    switch (key)
    {
    case Ctrl('U'):                                                             
       every_U();
       continue;
    case Ctrl('B'):
       every_B();
       continue;
    
    case ']':  /* Thor.990204: ���ɷQ��]�ݫ᭱���峹 */
    case 'j':  /* Thor.990204: ���ɷQ��j�ݫ᭱���峹 */
    case ' ':
      {
        int pos = xo->pos + 1;

        /* Thor.980727: �ץ��ݹL�Y��bug */

        if (pos >= xo->max)
    	  return cmd;

        xo->pos = pos;

        if (pos >= xo->top + XO_TALL)
  	  xypost_pick(xo);

        continue;
      }

    case 'y':
    case 'r':
      if (bbstate & STAT_POST)
      {
	strcpy(quote_file, fpath);
	if (do_reply(hdr) == XO_INIT)	/* �����\�a post �X�h�F */
	  return xpost_init(xo);
      }
      break;

    case 'm': 
      if ((bbstate & STAT_BOARD) && !(xmode & POST_MARKED)) 
      { 
        hdr->xmode = xmode | POST_MARKED; 
        rec_put(dir, hdr, sizeof(HDR), hdr->xid); 
      } 
      break; 

    }
    break;
  }

  return XO_INIT;
}


KeyFunc xpost_cb[] =
{
  XO_INIT, xpost_init,
  XO_LOAD, xpost_load,
  XO_HEAD, xpost_head,
#if 0
  XO_BODY, xpost_body,
#endif
  XO_BODY, post_body, /* Thor.980911: �@�ΧY�i */

  'r', xpost_browse,
  'y', post_reply,
  't', post_tag,
  'm', post_mark,

  'd', post_delete,  /* Thor.980911: ��K���D*/

  Ctrl('P'), post_add,
  Ctrl('Q'), xo_uquery,
  'q', post_spam,
  'I', xo_usetup,
#ifdef HAVE_MULTI_CROSSPOST
  Ctrl('X'), post_xcross,
#endif
  'x', post_cross,

  'h', xpost_help
};
#endif

