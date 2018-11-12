/*-------------------------------------------------------*/
/* km.c         ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : KongMing Chess routines                      */
/* create : 01/02/08                                     */
/* update : 01/05/09                                     */
/* author : einstein@bbs.tnfsh.tn.edu.tw                 */
/*          itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#define LOG_KM      /* �O�_���ѰO�����Ъ��\�� */
#define RETRACT_CHESS   /* �O�_���Ѯ��ѥ\�� */


#if 0
�ѽL�b etc/game/km �榡�p�U�G

�Ĥ@����`�@���X�L�ѽL(�T���)�A�q�ĤT��}�l�h�O�@�L�@�L�����СC
#123�V�@�t��
0 0 2 2 2 0 0
0 0 2 2 2 0 0           TILE_NOUSE 0 ��ܤ��ಾ�ʪ���l
2 2 2 2 2 2 2           TILE_BLANK 1 ��ܪŮ�
2 2 2 1 2 2 2           TILE_CHESS 2 ��ܴѤl
2 2 2 2 2 2 2
0 0 2 2 2 0 0
0 0 2 2 2 0 0
#endif


enum
{
  KM_XPOS = 5,
  KM_YPOS = 5,
  MAX_X = 7,            /* �_�ơA�ק� MAX_X MAX_Y �n�۹�ק� etc/km */
  MAX_Y = 7,            /* �_�� */

  TILE_NOUSE = 0,       /* ���ಾ�ʪ���l */
  TILE_BLANK = 1,       /* �Ů� */
  TILE_CHESS = 2        /* �Ѥl */
};


static int board[MAX_X][MAX_Y];
static int cx, cy;          /* �ثe���Ҧb */
static char piece[4][3] = {"�@", "��", "��", "��"};
static char title[20];      /* ���ЦW�� */

#ifdef RETRACT_CHESS
/* ���ѨB�Ƥ��i��W�L�ѽL�j�p */
static int route[MAX_X * MAX_Y][4]; /* �O�� (fx, fy) -> (tx, ty) */
static int step;
#endif


static void
out_song()
{
  /* �Цۤv�g���w���y�l�A�Ϊ̥κq���]�i�H :p */
  uschar *msg[8] = {
    "�A�ӱj�F�A�N�O�o�ˡI",
    "�A���i��Q��o�@�B",
    "�o�u�O�ӯ��_�F�A�ǧJ",
    "�ڤ����D�ӻ��Ǥ���F",
    "�o�@�ۯu�O�ѤH�ⵧ�r",
    "�ӨتA�A�F�A�o�ˤ]��",
    "�֧����F�I�[�o�[�o�I",
    "�n�����Эn�i�D������"
  };
  move(21, 0);
  clrtoeol();
  prints("\033[1;3%dm%s\033[m", time(0) % 7, msg[time(0) % 8]);
}


static void
show_board()
{
  int i, j;

  vs_bar("�թ���");
  move(2, KM_YPOS + MAX_Y - 6);     /* �m����ܴ��ЦW�� */
  outs(title);

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      move(KM_XPOS + i, KM_YPOS + j * 2);
      outs(piece[board[i][j]]);
    }
  }

  move(3, 40);
  outs("�Y��Ц����ʿ��~���{�H");
  move(4,40);
  outs("�ȮɱN������V����������Y�i");
  move(6, 40);
  outs("��������            ��V��");
  move(8, 40);
  outs("[Enter] or [Space]  ���/�Ͽ��");
  move(10, 40);
  outs("Q/q                 ���}");
  move(12, 40);
  outs("h                   Ū�����нd��");

#ifdef RETRACT_CHESS
  move(14, 40);
  outs("r                   ����");
#endif

  move(16, 40);
  outs("��                  �Ŧ�");
  move(17, 40);
  outs("��                  �Ѥl");
  move(18, 40);
  outs("��                  ���");

  out_song();
  move(KM_XPOS + MAX_X / 2, KM_YPOS + MAX_Y / 2 * 2);
}


static inline int
read_board()
{
  int i, j, count;
  int NUM_TABLE;
  FILE *fp;
  char buf[40], ans[4];

  if (!(fp = fopen("game/km/km", "r")))
    return 0;

  fgets(buf, 4, fp);
  NUM_TABLE = atoi(buf);    /* game/km/km �Ĥ@��O�����м� */

  sprintf(buf, "�п�ܽs�� [1-%d]�A[0] �H���X�D�A�Ϋ� [Q] ���}�G", NUM_TABLE);
  vget(b_lines, 0, buf, ans, 4, DOECHO);
  if (ans[0] == 'q' || ans[0] == 'Q')
  {
    fclose(fp);
    return 0;
  }
  i = atoi(ans) - 1;
  if (i < 0 || i >= NUM_TABLE)
    i = time(0) % NUM_TABLE;

  fseek(fp, 4 + i * (2 * MAX_X * MAX_Y + 14), SEEK_SET);
  /* 4: �Ĥ@�檺�T��ƴ��мƥ�\n  14: \n#999���ЦW��\n */

  fscanf(fp, "%s", &title);     /* ���ЦW�� */

  count = 0;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      fscanf(fp, "%d", &board[i][j]);
      if (board[i][j] & TILE_CHESS)
      {
        count++;
      }
    }
  }
  fclose(fp);
  return count;
}


static inline int
valid_pos(x, y)
  int x, y;
{
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y ||
    board[x][y] == TILE_NOUSE)  /* TILE_NOUSE = 0 ����� & operation */
  {
    return 0;
  }
  return 1;
}


static void
get_pos(x, y)
  int *x, *y;
{
  char ch;
  while (1)
  {
    ch = vkey();
    if (ch == KEY_UP && valid_pos(cx - 1, cy))
    {
      move(KM_XPOS + (cx - 1), KM_YPOS + cy * 2);
      cx -= 1;
    }
    else if (ch == KEY_DOWN && valid_pos(cx + 1, cy))
    {
      move(KM_XPOS + (cx + 1), KM_YPOS + cy * 2);
      cx += 1;
    }
    else if (ch == KEY_LEFT && valid_pos(cx, cy - 1))
    {
      move(KM_XPOS + cx, KM_YPOS + (cy - 1) * 2);
      cy -= 1;
    }
    else if (ch == KEY_RIGHT && valid_pos(cx, cy + 1))
    {
      move(KM_XPOS + cx, KM_YPOS + (cy + 1) * 2);
      cy += 1;
    }
    else if (ch == 'h')
    {
      more("game/km/km.hlp", NULL);
      show_board();
      move(KM_XPOS + cx, KM_YPOS + cy * 2);
    }
    else if (ch == 'q' || ch == 'Q')
    {
      vmsg("�����F��");
      *x = -1;
      break;
    }
    else if (ch == '\n' || ch == ' ')
    {
      *x = cx;
      *y = cy;
      break;
    }
#ifdef RETRACT_CHESS
    else if (ch == 'r')
    {
      *x = -2;
      break;
    }
#endif
  }
}


static inline void
jump(fx, fy, tx, ty)
  int fx, fy, tx, ty;       /* From (fx, fy) To (tx, ty) */
{
  out_song();

  board[fx][fy] = TILE_BLANK;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[1]);

  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_BLANK;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[1]);

  board[tx][ty] = TILE_CHESS;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[2]);
  move(KM_XPOS + tx, KM_YPOS + ty * 2);

#ifdef RETRACT_CHESS
  route[step][0] = fx;
  route[step][1] = fy;
  route[step][2] = tx;
  route[step][3] = ty;
  step++;
#endif
}


#ifdef RETRACT_CHESS
static inline void
retract()
{
  int fx, fy, tx, ty;

  out_song();

  step--;
  ty = route[step][3];
  tx = route[step][2];
  fy = route[step][1];
  fx = route[step][0];

  board[tx][ty] = TILE_BLANK;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[1]);

  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_CHESS;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[2]);

  board[fx][fy] = TILE_CHESS;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[2]);
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  cx = fx;
  cy = fy;
}
#endif


static inline int
check(fx, fy, tx, ty)
  int fx, fy, tx, ty;
{
  if ((board[(fx + tx) / 2][(fy + ty) / 2] & TILE_CHESS) &&
    ((abs(fx - tx) == 2 && fy == ty) || (fx == tx && abs(fy - ty) == 2)))
  {
    return 1;
  }
  return 0;
}


static inline int
live()
{
  int dir[4][2] = {1, 0, -1, 0, 0, 1, 0, -1};
  int i, j, k, nx, ny, nx2, ny2;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      for (k = 0; k < 4; k++)
      {
        nx = i + dir[k][0];
        ny = j + dir[k][1];
        nx2 = nx + dir[k][0];
        ny2 = ny + dir[k][1];
        if (valid_pos(nx2, ny2) && (board[i][j] & TILE_CHESS) &&
          (board[nx][ny] & TILE_CHESS) && (board[nx2][ny2] & TILE_BLANK))
        {
          return 1;
        }
      }
    }
  }
  return 0;
}


#ifdef LOG_KM
static void
log_km(fp)
  FILE *fp;
{
  int i, j;

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      fprintf(fp, "%s", piece[board[i][j]]);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
}
#endif


int
main_km()
{
  int fx, fy, tx, ty, count;

#ifdef LOG_KM
  char fpath[64];
  char buf[80];
  FILE *fp;
  time_t now;
#endif

  if (!(count = read_board()))
    return 0;

#ifdef LOG_KM
  time(&now);
  usr_fpath(fpath, cuser.userid, "km.log");
  fp = fopen(fpath, "w");
  fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid, cuser.username);
  fprintf(fp, "���D: �թ����� %s �}�ѹL�{\n�ɶ�: %s\n", title, ctime(&now));
  fprintf(fp, "%s\n\n", title);
  log_km(fp);
#endif

#ifdef RETRACT_CHESS
  step = 0;
#endif

  show_board();
  cx = MAX_X / 2;
  cy = MAX_Y / 2;

  while (1)
  {
    if (count == 1 && board[MAX_X / 2][MAX_Y / 2] & TILE_CHESS)
    {           /* �̫�@�l�n�b������ */
      vmsg("���߱z���\\�F");

#ifdef LOG_KM


      fclose(fp);
      if (vans("�z�O�_�n�⧹�������ЫO�s�b�H�c���H[Y/n] ") != 'n')
      {
   	HDR fhdr;
    	char folder[64];

    	usr_fpath(folder, cuser.userid, fn_dir);
    	hdr_stamp(folder, HDR_LINK, &fhdr, fpath);
    	sprintf(fhdr.title, "�թ����� %s �}�ѹL�{", title);
    	strcpy(fhdr.owner, cuser.userid);
    	rec_add(folder, &fhdr, sizeof(fhdr));
      }
      sprintf(buf, "%s �թ����� %s �}�ѹL�{", cuser.userid,title);
      keeplog(fpath, BRD_SPECIAL, buf, 1);
//      unlink(fpath);
#endif
      return 0;
    }
    if (!live())
    {
      vmsg("�V�|...�S�ѤF...@@");
#ifdef	LOG_KM      
      fclose(fp);
      unlink(fpath);
#endif
      return 0;
    }

    while (1)       /* �Ĥ@�� */
    {
      get_pos(&fx, &fy);
      if (fx == -1)
      {
#ifdef	LOG_KM
        fclose(fp);
        unlink(fpath);
#endif        
        return 0;
      }
#ifdef RETRACT_CHESS
      else if (fx == -2)
      {
      	  if (step) /* �@�B���٨S���A���஬�� */
      	  {
            retract();
            count++;
#ifdef LOG_KM
            fprintf(fp, "���ѡA�^��W�@�B\n");
            log_km(fp);
#endif
          }
          continue;
      }
#endif      
      
      if (!(board[fx][fy] & TILE_CHESS))
      {
        continue;
      }
      else      /* ��l */
      {
        outs(piece[3]);
        move(KM_XPOS + fx, KM_YPOS + fy * 2);
        break;
      }
    }

    while (1)       /* �ĤG�� */
    {
      get_pos(&tx, &ty);
      if (tx == -1)
      {
        return 0;
      }
#ifdef RETRACT_CHESS
      else if (tx == -2)
      {
        continue; /* �n������l�~�஬�� */
      }
#endif
      
      if (fx == tx && fy == ty) /* ����l */
      {
        outs(piece[2]);
        move(KM_XPOS + tx, KM_YPOS + ty * 2);
        break;
      }
      else if (!(board[tx][ty] & TILE_BLANK) || !check(fx, fy, tx, ty))
      {     /* ������a�褣��� */
        continue;
      }
      else      /* ����Ӧa�� */
      {
        jump(fx, fy, tx, ty);
        count--;
#ifdef LOG_KM
        log_km(fp);
#endif
        break;
      }
    }
  }
}
