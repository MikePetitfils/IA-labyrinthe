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

void DEBUG(char *mess) { printf("%s \n\r", mess); }
// ajoute la box dans la hash tab
inline void addboxtohtab(struct box * pbox, GHashTable *ht){
  char* key = malloc(sizeof(char)*15);
  memset(key,0,sizeof(key));
  snprintf(key,sizeof(key),"%d%d",pbox->x,pbox->y);
  g_hash_table_insert(ht,key,pbox);

}

//retourne la box qui a les coordonnée xy NULL si aucun
inline struct box * getBoxbyXY(int x, int y, GHashTable *ht){
  char key[15];
  struct box * pbox = NULL;
  memset(key,0,sizeof(key));
  snprintf(key,sizeof(key),"%d%d",x,y);
  pbox = g_hash_table_lookup(ht, &key);
  if (pbox == NULL)
    return NULL;

  if ( (pbox->x != x ) | (pbox->y != y)){
    DEBUG("ERROR HTAB");
  }
  return pbox;

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
  addboxtohtab( nbox, ht );
  if (nbox != getBoxbyXY(x,y, ht)){
    printf("error htab dans newbox");
    exit(1);
  }
  chainbox( nbox, ht);
  printbox(nbox);

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
void printbox(struct box * pbox){
  char buff[15];
  memset(buff,0, sizeof(buff));
  if (pbox == NULL){
    DEBUG("ERREUR : la case courante est NULL");
    return;
  }
  givestatebox(pbox, buff);
  printf("la case est : %s \n\r", buff);
  givestatebox(pbox->left, buff);
  printf("left : %s \n\r",  buff);
  givestatebox(pbox->right, buff);
  printf("right : %s\n\r", buff);
  givestatebox(pbox->up, buff);
  printf("up : %s\n\r", buff);
  givestatebox(pbox->down, buff);
  printf("down : %s\n\r", buff);
  printf("coordonnees : { %d | %d } \n\r",pbox->x, pbox->y);
}

void givestatebox(struct box * pbox, char* rep){
    if (  pbox == NULL   ){
      strcpy(rep, "NULL\0");
      return;
    }
  if ( pbox->state & UNKNOWN){
    strcpy(rep, "UNKNOWN\0");
    return;
  }
  if ( pbox->state & EXIT ){
    strcpy(rep, "EXIT\0");
    return;
  }
  if ( pbox->state & ROUTE ){
    strcpy(rep, "ROUTE\0");
  return;
  }

  strcpy(rep, "MUR\0");

}

