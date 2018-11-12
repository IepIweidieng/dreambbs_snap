#ifndef	_HDR_H_
#define	_HDR_H_


#include <sys/types.h>


/* ----------------------------------------------------- */
/* DIR of post / mail struct : 256 bytes		 */
/* ----------------------------------------------------- */


typedef struct
{
  time_t chrono;		/* timestamp */
  int xmode;

  int xid;			/* reserved �O�d*/

  char xname[32];		/* �ɮצW�� */
  char owner[60];		/* �@�� (E-mail address) */
  char reserve[16];		/* �O�d */
  int  recommend;		/* ���ˤ峹 */
  char nick[50];		/* �ʺ� */

  char date[9];			/* [96/12/01] */
  /* Thor.990329:�S�O�`�N, date�u����ܥ�, ���@���, �H�קK y2k ���D,
                 �w�q 2000�� 00, 2001��01 */

  char title[73];		/* �D�D (TTLEN + 1) */
}          HDR;


/* gopher url �r��Gxname + owner + nick + date */

#define	GEM_URLEN	(32 + 80 + 50 + 9 - 1)


/* ----------------------------------------------------- */
/* post.xmode ���w�q					 */
/* ----------------------------------------------------- */



#define POST_READ	0x00000001	/* already read */
#define POST_MARKED	0x00000002	/* marked */
#define POST_GEM	0x00000004	/* gemed */
#define POST_CANCEL	0x00000040	/* canceled */
#define POST_DELETE	0x00000080	/* deleted */

#define	POST_INCOME	0x00000100	/* ��H�i�Ӫ� */
#define	POST_EMAIL	0x00000200	/* E-mail post �i�Ӫ� */
#define	POST_OUTGO	0x00000400	/* ����H�X�h */
#define	POST_RESTRICT	0x00000800	/* ����Ť峹�A�� manager �~��� */
#define POST_MDELETE	0x00100000	/* ���D�ί����R�� */
#define POST_LOCK	0x00200000	/* �峹��w */
#define POST_COMPLETE	0x00400000	/* �����B�z */
#define POST_MODIFY		0x00800000	/* �ק�L�F */
#define POST_CURMODIFY	0x01000000	/* ���b�ק襤 */

/* ----------------------------------------------------- */
/* mail.xmode ���w�q					 */
/* ----------------------------------------------------- */


#define MAIL_READ	0x00000001	/* already read */
#define MAIL_MARKED	0x00000002	/* marked */
#define MAIL_REPLIED	0x00000004	/* �w�g�^�L�H�F */
#define	MAIL_MULTI	0x00000008	/* mail list */
#define	MAIL_HOLD	0x00000010	/* �ۦs���Z */
#define	MAIL_NOREPLY	0x00000020	/* ���i�^�H */
#define MAIL_DELETE	0x00000080	/* �N�D�R�� */

#define	MAIL_INCOME	0x00000100	/* bbsmail �i�Ӫ� */


/* ----------------------------------------------------- */
/* gem(gopher).xmode ���w�q				 */
/* ----------------------------------------------------- */


#define	GEM_FULLPATH	0x00008000	/* ������| */

#define	GEM_RESTRICT	0x00000800	/* ����ź�ذϡA�� manager �~��� */
#define	GEM_RESERVED	0x00001000	/* ����ź�ذϡA�� sysop or board or gem �~���� */
#define	GEM_LOCK	0x00002000	/* �W����šA�� sysop �~���� */

#define	GEM_FOLDER	0x00010000	/* folder / article */
#define	GEM_BOARD	0x00020000	/* �ݪO��ذ� */
#define	GEM_GOPHER	0x00040000	/* gopher */
#define	GEM_HTTP	0x00080000	/* http */
#define	GEM_EXTEND	0x80000000	/* ������ URL */


#define	HDR_URL		(GEM_GOPHER | GEM_HTTP)
#define	HDR_LINK	0x00000400	/* link() */


#endif	/* _HDR_H_ */
