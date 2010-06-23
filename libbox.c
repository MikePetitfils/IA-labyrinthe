/*
** libbox.c
** Login : <najon@najon-desktop>
** Started on  Sun May 30 02:58:36 2010 najon
** $Id$
**
** Author(s):
**  - najon <>
**
** Copyright (C) 2010 najon
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <string.h>
#include "libbox.h"

void DEBUG(char *mess)
{
  printf("%s\n", mess);
}

static int gmaxx = 0;
static int gmaxy = 0;
static int gminx = 0;
static int gminy = 0;

// ajoute la box dans la hash tab
inline box *addboxtohtab(struct box * pbox, GHashTable *ht){
  char* key = malloc(sizeof(char)*16);

  memset(key,0, 16);
  snprintf(key, 16,"%.4d%.4d",pbox->x, pbox->y);
  box *b = getBoxbyXY(pbox->x, pbox->y, ht);
  //do add a box twice to the hashmap
  if (b)
  {
    if (b->x != pbox->x ||
        b->y != pbox->y)
      printf("add to ht FAIL: %d, %d     %d, %d\n", b->x, b->y, pbox->x,pbox->y);
      printf("reusing: %d, %d    --   %d, %d\n", b->x, b->y, pbox->x,pbox->y);
    b->state = pbox->state;
    return b;
  }
  g_hash_table_insert(ht,key,pbox);
  return  pbox;
}

//retourne la box qui a les coordonnée xy NULL si aucun
inline struct box * getBoxbyXY(int x, int y, GHashTable *ht){
  char key[16];
  struct box * pbox = NULL;
  memset(key, 0, 16);
  snprintf(key, 16, "%.4d%.4d", x, y);
  pbox = g_hash_table_lookup(ht, &key);
  if (pbox == NULL)
    return NULL;

  if ( (pbox->x != x ) | (pbox->y != y)){
    printf("ERROR HTAB: %d, %d    --   %d, %d\n", x, y, pbox->x,pbox->y);
    //DEBUG("ERROR HTAB");
  }
  return pbox;

}

int box_decode_state(char c)
{
  switch(c)
  {
  case '0':
    return ROUTE;
  case '1':
    return MUR;
  case '2':
    return PLAYER;
  case '3':
    return EXIT;
  }
  return UNKNOWN;
}

char box_get_state(box *pbox)
{
  if (!pbox)
    return ' ';
  if (pbox->state & UNKNOWN)
    return 'U';
  if (pbox->state & EXIT)
    return '#';
  if (pbox->state & VISITED)
    return '*';
  if (pbox->state & ROUTE)
    return '.';
  if (pbox->state & MUR)
    return '1';
  return ' ';
}

void box_verify(box *pbox, GHashTable *ht)
{
  box *test;

  if (!pbox)
    return;
  if (pbox->up != getBoxbyXY(pbox->x, pbox->y + 1, ht))
    printf("fucked up box[UP]: %d, %d\n", pbox->x, pbox->y);
  if (pbox->down != getBoxbyXY(pbox->x, pbox->y - 1, ht))
    printf("fucked up box[DO]: %d, %d\n", pbox->x, pbox->y);
  if (pbox->left != getBoxbyXY(pbox->x - 1, pbox->y, ht))
    printf("fucked up box[LE]: %d, %d\n", pbox->x, pbox->y);
  if (pbox->right != getBoxbyXY(pbox->x + 1, pbox->y, ht))
    printf("fucked up box[RI]: %d, %d\n", pbox->x, pbox->y);
}

void print_map(GHashTable *ht, box *current)
{
  int i = 0;
  int j = 0;

  printf("Map size: x[%d, %d] y[%d, %d]\n", gminx, gmaxx, gminy, gmaxy);
  //for (i = gminy; i < gmaxy; ++i)
  for (i = gmaxy - 1; i >= gminy; --i)
  {
    for (j = gminx; j < gmaxx; ++j)
    {
      box_verify(getBoxbyXY(j, i, ht), ht);
      if (current && current->x == j && current->y == i)
        printf("X");
      else
        printf("%c", box_get_state(getBoxbyXY(j, i, ht)));
    }
    printf("\n");
  }
}

int box_is_unknown(box *pbox)
{
  if (!pbox)
    return 1;
  if (pbox->state & UNKNOWN)
    return 1;
  return 0;
}

struct box * newbox(char state, int x, int y, GHashTable *ht){
  //DEBUG("ds new box");
  struct box * nbox;
  nbox = (box *)malloc(sizeof(box));
  //DEBUG("ds new box av assign");
  nbox->left  = NULL;
  //DEBUG("ds new box apres");
  nbox->right = NULL;
  nbox->up    = NULL;
  nbox->down  = NULL;
  nbox->state = state;
  nbox->x     = x;
  nbox->y     = y;

  if (x > gmaxx)
    gmaxx = x;
  if (x < gminx)
    gminx = x;
  if (y > gmaxy)
    gmaxy = y;
  if (y < gminy)
    gminy = y;

  //printf("newbox[%d, %d]: %d\n", x, y, state);
  nbox = addboxtohtab( nbox, ht );
  if (nbox != getBoxbyXY(x,y, ht)){
    printf("error htab dans newbox");
    exit(1);
  }
  chainbox( nbox, ht);
  //box_print(nbox);
  return nbox;
}

//chaine la box avec les cases autours
void chainbox(struct box * pbox, GHashTable *ht){
  struct box * ptempbox = NULL;

  //si il y a une case a x - 1 on fait le chainage a gauche
  ptempbox = getBoxbyXY( (pbox->x - 1 ), pbox->y, ht);
  if (ptempbox != NULL){
    pbox->left = ptempbox;
    ptempbox->right = pbox;
  }
  ptempbox = getBoxbyXY( (pbox->x + 1 ), pbox->y, ht);
  if (ptempbox != NULL){
    pbox->right = ptempbox;
    ptempbox->left = pbox;
  }
  ptempbox = getBoxbyXY( pbox->x, ( pbox->y - 1 ), ht);
  if (ptempbox != NULL){
    pbox->down = ptempbox;
    ptempbox->up = pbox;
  }
  ptempbox = getBoxbyXY( pbox->x, ( pbox->y + 1 ), ht);
  if (ptempbox != NULL){
    pbox->up = ptempbox;
    ptempbox->down = pbox;
  }
  //  printbox(pbox);
}
void box_print(box *pbox){
  char buff[15];

  memset(buff,0, sizeof(buff));
  if (pbox == NULL){
    DEBUG("ERREUR : la case courante est NULL");
    return;
  }
  printf("Box[%d, %d] : %s (Up:%s, Down:%s, Left: %s, Right:%s)\n", pbox->x, pbox->y,
         givestatebox(pbox       , buff),
         givestatebox(pbox->up   , buff),
         givestatebox(pbox->down , buff),
         givestatebox(pbox->left , buff),
         givestatebox(pbox->right, buff));
}

char *givestatebox(struct box * pbox, char* rep){
  int i = 0;

  if (pbox == NULL){
    strcpy(rep, "NULL");
    return rep;
  }
  if (pbox->state & UNKNOWN)
    rep[i++] = 'U';
  if (pbox->state & EXIT)
    rep[i++] = 'E';
  if (pbox->state & ROUTE)
    rep[i++] = 'R';
  if (pbox->state & VISITED)
    rep[i++] = 'V';
  if (pbox->state & MUR)
    rep[i++] = 'M';
  rep[i] = 0;
  return rep;
}

int box_is_wall(box *pbox)
{
  if (pbox->state & MUR)
    return 1;
  return 0;
}

int box_is_visited(box *pbox)
{
  if (pbox->state & VISITED)
    return 1;
  return 0;
}

//return true is this is an unexplorated case
int box_is_interesting(box *pbox)
{
  if (!pbox)
  {
    printf("error: box is NULL\n");
    //on feinte, on va dire que c'est encore une case inconnu (au pire ca fait pas de mal)
    return 1;
  }
  if (box_is_wall(pbox))
    return 0;
  if (box_is_visited(pbox))
    return 0;
  return 1;
}


int box_is_exit(box *pbox)
{
  if (pbox->state & EXIT)
    return 1;
  return 0;
}

// return the number of road starting from this box
int box_count_interesting_road(box *pbox)
{
  int ret = 0;
  ret += box_is_interesting(pbox->left);
  ret += box_is_interesting(pbox->right);
  ret += box_is_interesting(pbox->up);
  ret += box_is_interesting(pbox->down);
  return ret;
}

void box_mark_visited(box *pbox)
{
  pbox->state |= VISITED;
}

/* try to find exit first
 * if no exit found, try to found a box not explorated yet
 */
box *box_get_interesting_neighboor(box *pbox)
{
  if (box_is_exit(pbox->left))
    return pbox->left;
  if (box_is_exit(pbox->right))
    return pbox->right;
  if (box_is_exit(pbox->up))
    return pbox->up;
  if (box_is_exit(pbox->down))
    return pbox->down;

  if (box_is_interesting(pbox->left))
    return pbox->left;
  if (box_is_interesting(pbox->right))
    return pbox->right;
  if (box_is_interesting(pbox->up))
    return pbox->up;
  if (box_is_interesting(pbox->down))
    return pbox->down;
  return 0;
}
