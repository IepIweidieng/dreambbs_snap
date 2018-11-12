/*-------------------------------------------------------*/
/* config.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : site-configurable settings		 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/

#ifndef	_CONFIG_H_
#define	_CONFIG_H_

#undef	TREAT
#undef	TRANUFO

/* ----------------------------------------------------- */
/* �w�q BBS ���W��}					 */
/* ------------------------------------------------------*/

#define BOARDNAME       "�����j�� ������"		/* ���寸�W */
#define NICKNAME	"������"			/* ����²�� */
#define MYHOSTNAME	"bbs.yzu.edu.tw"		/* ������} */
#define SYSOPNICK	"�������D"			/* SYSOP �ʺ� */
#define BBSHOME		"/home/bbs"			/* BBS ���a */
#define BBSUID		9999
#define BBSGID		99
#define TAG_VALID       "[WindTopBBS]To "                 /* �����{�Ҩ�token */
#define SYSOPNAME	"visor"				/* �S�ů��� */
#define	HIDEDN_SRC	"bbs.yzu.edu.tw"		/* ���èӷ���m */
#define BBSNAME		"[WindTopBBS Ver.20031002]"	/* �^�寸�W */
#define	BBSIP		"140.138.2.235"			/* bbs �� ip */

/* ----------------------------------------------------- */
/* �H BBS ���W�Ҧ������X�W				 */
/* ----------------------------------------------------- */

#ifndef	MAXBOARD
#define MAXBOARD	(1024)		/* �̤j�}�O�Ӽ� */
#endif

#ifndef	MAXACTIVE
#define MAXACTIVE	(2500)		/* �̦h�P�ɤW���H�� (ex.1400)*/
					/* �Y�n�W�L 1024 �Эק� kernel */
					/* options         SHMMAXPGS=? */ 
#endif

#define MAXFIREWALL	(300)		/* ����׫H�C��W�� */

#define MAXOFILEWALL	(500)		/* �ݪ��׫H�C��W�� */

#define PAL_MAX		(512)		/* �n�ͦW��W�� */

#define PAL_ALMR	(450)		/* ĵ�i */

#define	BANMSG_MAX	(32)		/* �ڦ��T���W��W�� */

/* ----------------------------------------------------- */
/* guest���W�r                             by Jerics     */
/* ------------------------------------------------------*/

#define GUESTNAME	(1)                               /* guest�W�٪��ƶq */
#define GUEST_NAMES	GUEST_1
#define GUEST_1         NICKNAME"�X��"

#define CHATROOMNAME	"�������y"

/* ----------------------------------------------------- */
/* �N�����j�Ǫ� mail server �w�� define                  */
/* ----------------------------------------------------- */

#define YZUMAIL84	"alumni.yzu.edu.tw"
#define	YZUMAIL85	"alumni.yzu.edu.tw"
#define	YZUMAIL86	"alumni.yzu.edu.tw"
#define YZUMAIL87	"alumni.yzu.edu.tw"
#define YZUMAIL88	"bach.yzu.edu.tw"
#define	YZUMAIL89	"chopin.yzu.edu.tw"
#define	YZUMAIL90	"haydn.yzu.edu.tw"
#define YZUMAIL91	"mozart.yzu.edu.tw"
#define	DEFAULTSERVER	"mail.yzu.edu.tw"

#if 0
#define	YZUMAIL		{"84",YZUMAIL84,\
			 "85",YZUMAIL85,\
			 "86",YZUMAIL86,\
			 "87",YZUMAIL87,\
			 "88",YZUMAIL88,\
			 "89",YZUMAIL89,\
			 "90",YZUMAIL90,\
			 "91",YZUMAIL91,\
			 NULL,YZUMAIL84}
#endif
#define YZUMAIL         {NULL,"pop3.yzu.edu.tw"}

/* ----------------------------------------------------- */
/* �պA�W��						 */
/* ------------------------------------------------------*/

#define	HAVE_WEBBBS		/* only for WindTop */

#undef	HAVE_PIP_FIGHT		/* �p����� */ /* �|�����u �ܦh bug */
#define	HAVE_PIP_FIGHT1

#define	EMAIL_JUSTIFY		/* �o�X InterNet Email �����{�ҫH�� */

#define	HAVE_BRDTITLE_CHANGE	/* ���D�ק睊�W */

#define	BLACK_SU		/* �t�� debug �� */

#define	HAVE_OBSERVE_LIST	/* �t���[��W�� */

#undef	NEWUSER_LIMIT		/* �s��W�����T�ѭ��� */

#undef	HAVE_REPORT		/* �t�ΰl�ܳ��i */

#define	HAVE_RECOMMEND		/* ���ˤ峹 */
#ifdef	HAVE_RECOMMEND
#endif

#define	HAVE_MMAP		/* �ĥ� mmap(): memory mapped I/O */
				/* �b SunOS�BFreeBSD �W�i�H�[�t 30 �� */
				/* Linux �� mmap() �����D�A�мȮɤ��n�� */

#ifdef	HAVE_MMAP
#include <sys/mman.h>
#ifdef MAP_FILE		/* 44BSD defines this & requires it to mmap files */
#  define BBS_MAP	(MAP_SHARED | MAP_FILE)
#else
#  define BBS_MAP	(MAP_SHARED)
#endif
#endif

#define	HAVE_INPUT_TOOLS	/* �Ÿ���J�u�� */

#define	HAVE_BANMSG		/* �ڦ��T���\��C�� */

#define	HAVE_DETECT_CROSSPOST	/* cross-post �۰ʰ��� */

#define	HAVE_DETECT_VIOLAWATE	/* �H�k���� */

#define	HAVE_RESIST_WATER	/* ������\�� */   

#define	HAVE_CLASSTABLE		/* �ӤH�\�Ҫ� */

#ifdef	HAVE_CLASSTABLE
#define	HAVE_CLASSTABLEALERT	/* �Ҫ�ɨ�q�� */
#endif

#undef	HAVE_CHANGE_SKIN	/* ���� skin *//* �|���������n�}�� */

#define	HAVE_SEM		/* �ϥ� Semaphores */

#define	HAVE_MULTI_CROSSPOST	/* �s����K */

#define	HAVE_BM_CHECK		/* ���D�T�{ */

#define	HAVE_RAND_INCOME	/* �üƶi���e�� */

#define	HAVE_MAILGEM		/* �ϥΫH���ذ� */

#define	HAVE_FAVORITE		/* �ϥΧڪ��̷R */

#define	HAVE_PROFESS		/* �ϥαM�~�Q�װ� */

#define	HAVE_USER_MODIFY	/* �ϥΪ̭ק�峹 */

#define	HAVE_INFO		/* �ϥήդ褽�i�� */

#define	HAVE_SHOWNUMMSG		/* ��ܤ��y�Ӽ� */

#define	HAVE_STUDENT		/* �ϥξǥͤ��i�� */

#undef	HAVE_MAXRESERVE		/* �����O�d�W����m *//* �|�����D  �ФŶ}�� */

#define	HAVE_MAILUNDELETE	/* �_��R���H�c�����H�� */

#define HAVE_WRITE		/* �զW�� */

#define	HAVE_SONG		/* �����I�q�t�� */

#undef	HAVE_SONG_TO_CAMERA	/* �O�_�I�q��ʺA�ݪ� */

#define	HAVE_MAIL_FIX		/* �ץ� mail title ���\�� *//* ps:�p�ߤ��n�ö} */

#define	HAVE_MODERATED_BOARD	/* ���ѯ��K�� */

#ifdef	HAVE_MODERATED_BOARD
#define HAVE_WATER_LIST		/* ���Ѥ���\�� */

#ifdef	HAVE_WATER_LIST
#define	HAVE_SYSOP_WATERLIST	/* SYSOP ����W�� */
#endif
#endif	

#undef	HAVE_GAME_QKMJ		/* ���� QKMJ �C�� */

#define	HAVE_BOARD_PAL		/* ���ѪO�ͥ\�� */

#define HAVE_ANONYMOUS		/* ���� anonymous �O */

#define	HAVE_ORIGIN		/* ��� author �Ӧۦ�B */

#define HAVE_MIME_TRANSFER      /* ���� MIME �ѽX */

#define HAVE_SIGNED_MAIL	/* Thor.990409: �~�e�H��[ñ�W */

#ifdef HAVE_SIGNED_MAIL
#define PRIVATE_KEY_PERIOD	0	/* Thor.990409: ����key, auto gen */
#endif

#define SHOW_USER_IN_TEXT       /* �b��󤤦۰���� User ���W�r */

#ifndef	SHOW_USER_IN_TEXT
#define outx	outs
#endif

#define HAVE_BOARD_SEE		/* PERM_ALLBOARD �i�H�ݨ�����ݪO */

#define	HAVE_PERSON_DATA	/* ���ѥͤ鵥��L��� */

#define	HAVE_TERMINATOR		/* �׵��峹�j�k --- �تḨ���� */

#define HAVE_XYPOST		/* �峹�걵�Ҧ� */

#define	LOGIN_NOTIFY            /* �t�Ψ�M���� */

#define	AUTHOR_EXTRACTION	/* �M��P�@�@�̤峹 */

#define EVERY_Z			/* ctrl-z Everywhere (���F�g�峹) */

#define EVERY_BIFF		/* Thor.980805: �l�t��B�ӫ��a */

#define BMW_TIME		/* Thor.980911: ���T�ɶ��O�� */

#define	HAVE_COUNT_BOARD	/* visor.20030816: �ݪO��T�έp */

#undef	MODE_STAT               /* �[��βέp user ���ͺA, �H�����g���w */

#define	HAVE_DECLARE		/* �ϬY��title����� */

#define	HAVE_CHK_MAILSIZE	/* visor.20031111: �H�c�W�L�j�p�A�W����H�c */

#define	HAVE_ALOHA              /* lkchu.981201: �n�ͤW���q�� */

#define	HAVE_DECORATE           /* �аO�M�W�� */

#define MULTI_MAIL		/* �s�ձH�H�\�� */ /* Thor.981016: �������B�H */

#define	HAVE_REGISTER_FORM	/* ���U��{�� */

#ifdef	HAVE_REGISTER_FORM
#undef	HAVE_SIMPLE_RFORM	/* ²�Ƶ��U�� */
#endif

#define	JUSTIFY_PERIODICAL	/* �w�������{�� */

#define	LOG_ADMIN		/* Thor.990405: lkchu patch: ��������v�� */

#define LOG_BMW                 /* lkchu.981201: ���T�O���B�z */

#define	LOG_TALK		/* lkchu.981201: ��ѰO���B�z */

#define	LOG_CHAT		/* ��ѫǬ��� */

#define	LOG_BRD_USIES		/* lkchu.981201: �O���ݪO�\Ū���p�i�Ѳέp */

#define HAVE_ETC_HOSTS          /* lkchu.981201: �ѷ� etc/hosts �� database */

#define HAVE_CHANGE_FROM        /* lkchu.981201: ^F �Ȯɧ��G�m */

#define COLOR_HEADER		/* lkchu.981201: �ܴ��m����Y */

#undef	EMAIL_PAGE		/* lkchu.990429: �q�l�l��ǩI */

#define	FRIEND_FIRST		/* lkchu.990512: �n�͸m�e */

#undef	HAVE_SMTP_SERVER	/* bmtad�O���H�{��, ���O�H�H�{��
				�H�H�{���O�bbbsd��, mail.c��bsmtp
				�nrelay���� statue.00725 */
#ifdef	HAVE_SMTP_SERVER
#define	SMTP_SERVER		{"mail.yzu.edu.tw",NULL}
#endif

#undef	HAVE_FORCE_BOARD	/* �j�� user login �ɭ�Ū���Y���i�ݪO: */	
#ifdef	HAVE_FORCE_BOARD
#define	FORCE_BOARD		BRD_ANNOUNCE /* statue.000725 */
#endif

#define	HAVE_BBSNET		/* ���� BBSNET */

#define HAVE_COLOR_VMSG		/* �m�⪺VMSG */


/* ----------------------------------------------------- */
/* ��L�t�ΤW���Ѽ�					 */
/* ----------------------------------------------------- */

#ifdef  HAVE_PIP_FIGHT1
#define	PIP_MAX		(16)
#endif

#define LOGINATTEMPTS	(3)		/* �̤j�i�����~���� */

#define LOGINASNEW	(1)		/* �ĥΤW���ӽбb����� */

#define	MAX_REGIST	(3)		/* �̤j���U�� */

#define	MAX_RESERVE	(16)		/* ���ȫO�d��m */

#define MAX_MEMORANDUM	(200)		/* �̤j�Ƨѿ��Ӽ� */

#define MAX_CONTACT	(200)		/* �̤j�p���W��Ӽ� */

#define	BMW_MAX		(128)		/* ���T�W�� */

#define	BMW_PER_USER	(9)		/* �C��ϥΪ̼��T�W�� */

#define	BMW_EXPIRE	(60)		/* ���T�B�z�ɶ� */

#define BMW_MAX_SIZE	(100)		/* ���T�W�� (K) */

#define	MAX_BBSMAIL	(10000)		/* bbsmail ���H�W�� (��) */

#define	MAX_VALIDMAIL	(500)		/* �{�� user ���H�W�� (��) */

#define	MAX_NOVALIDMAIL	(100)		/* ���{�� user ���H�W�� (��) */

#define MAIL_SIZE_NO	(100)		/* ���{�� user ���H�W�� (K) */

#define MAIL_SIZE	(1000)		/* �̤j�H�c�W�� (K) */

#define	MAX_LIST	(6)		/* �s�զW��̤j�� */

#define MAX_MAIL_SIZE	(10240)		/* �`�H�c�e�q */

                       /* Thor.981011: �H�Wfor bbs�D�{����, �W�L�h���� */
                       /* �O�o�P�B�ק� etc/mail.over */
                       /* Thor.981014: bquota �ΨӲM�L���ɮ�/�H��, �ɶ��p�U */
                       /* �O�o�P�B�ק� etc/justified �q����H�� etc/approved */

#define MARK_DUE        (180)		/* �аO�O�s���H�� (��) */
#define MAIL_DUE        (60)		/* �@��H�� (��) */
#define FILE_DUE        (30)		/* ��L�ɮ� (��) */

#define	MAXGUEST	(1024)		/* �̦h���X�� guest�H */
					/* statue change 128 -> 1024 for test*/

#define	MAX_ALOHA	(64)		/* �W���q���ޤJ�� */

#define MAXSIGLINES	(6)		/* ñ�W�ɤޤJ�̤j��� */

#define MAXQUERYLINES	(19)		/* ��� Query/Plan �T���̤j��� */

#define	MAX_CHOICES	(32)		/* �벼�ɳ̦h�� 32 �ؿ�� */

#define MAX_BOOKS	(16)		/* �̤j���W�U�� */

#define	TAG_MAX		(256)		/* TagList ���Ҽƥؤ��W�� */

#define NBRD_MAX	(30)		/* �̤j�s�p�� */

#define NBRD_DAYS	(14)		/* �s�p�Ѽ� */

#define NBRD_MAX_CANCEL	(30)		/* �̤j�o���D�H�� */

#define	MAXPAGES	(256)		/* more.c ���峹���ƤW�� (lines/22),�w�]:128 */

#define MAXLASTCMD	(8)		/* line input buffer */

#define	MOVIE_MAX	(180)		/* �ʵe�i�� */

#define	MOVIE_SIZE	(108*1024)	/* �ʵe cache size */

#ifdef	HAVE_RECOMMEND
#define	MIN_RECOMMEND	(1)		/* �̤p���ˤ峹��ܼƦr */
#endif

#ifdef	HAVE_DETECT_CROSSPOST
#define MAX_CHECKSUM	(6)		/* crosspost checksum ��� */

#define MAX_CROSS_POST	(3)		/* cross post �̤j�ƶq */
#endif

#ifdef	HAVE_OBSERVE_LIST	/* �t���[��W�� */
#define	MAXOBSERVELIST	(32)
#define	BRD_OBSERVE		"ObserveList"
#endif

#define MOVIE_LINES	(20)		/* �ʵe�̦h�� 10 �C */

#define	CHECK_PERIOD	(86400 * 14)	/* ��z�H�c/�n�ͦW�檺�g�� */

#define	VALID_PERIOD	(86400 * 180)	/* �����{�Ҧ��Ĵ� */
                     /* Thor.981014: �O�o�P�B�ק� etc/re-reg */

#define CHECK_BM_TIME	(86400 * 14)	/* ���D�T�{�ɶ� */

#define IDLE_TIMEOUT	(30)		/* �o�b�L�[�۰�ñ�h */

#ifdef	HAVE_RESIST_WATER
#define	CHECK_QUOT	(3)		/* �峹����̤p��� */

#define CHECK_QUOT_MAX	(3)		/* �峹����s��g�� */	
#endif

#define	BANMAIL_EXPIRE	(30)		/* �׫H�C���s (��) */

#define	BLK_SIZ		4096		/* disk I/O block size */

#define CNA_MAX         20              /* lkchu.981201: �Y�ɷs�D�W�� */

#define	BBSNETMAX	10		/* BBSNET �̤j�s�u�ƶq */

/* ----------------------------------------------------- */
/* chat.c & xchatd.c ���ĥΪ� port �� protocol		 */
/* ------------------------------------------------------*/

#define	BBTP_PORT	4884

#define CHAT_PORT	3838
#define CHAT_SECURE			/* �w������ѫ� */

#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"


/* ----------------------------------------------------- */
/* �t�ΰѼ�						 */
/* ----------------------------------------------------- */

#define BRDSHM_KEY	(2997)
#define UTMPSHM_KEY	(1998)
#define FILMSHM_KEY	(2999)
#define FWSHM_KEY	(3999)
#define FWOSHM_KEY	(5000)
#define COUNT_KEY	(4000)

#ifdef	HAVE_OBSERVE_LIST	/* �t���[��W�� */
#define	OBSERVE_KEY	(6000)
#endif

#define	SPEAK_MAX	(50)
#define	CONDITION_MAX	(100)
#define	PARTY_MAX	(150)



#define	BSEM_KEY	2000	/* semaphore key */
#define	BSEM_FLG	0600	/* semaphore mode */
#define BSEM_ENTER      -1	/* enter semaphore */
#define BSEM_LEAVE      1	/* leave semaphore */
#define BSEM_RESET	0	/* reset semaphore */


/* ----------------------------------------------------- */
/* �ӽбb���ɭn�D�ӽЪ̯u����				 */
/* ----------------------------------------------------- */

#define REALINFO

#ifdef	REALINFO
#undef	POST_REALNAMES		/* �K���ɪ��W�u��m�W */
#undef	MAIL_REALNAMES		/* �H�����H��ɪ��W�u��m�W */
#undef	QUERY_REALNAMES		/* �Q Query �� User �i���u��m�W */
#endif

/* ----------------------------------------------------- */
/* �n�ͦW���C���w					 */
/* ----------------------------------------------------- */

#define         COLOR_PAL       "\033[1;32m"
#define         COLOR_BAD       "\033[1;31m"
#define         COLOR_CLOAK     "\033[1;35m"
#define         COLOR_BOTH      "\033[1;37m"
#define         COLOR_OPAL      "\033[1;33m"
#define		COLOR_BOARDPAL	"\033[36m"

#endif				/* _CONFIG_H_ */

