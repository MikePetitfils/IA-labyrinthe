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
#include "libbox.h"
void DEBUG(char *mess) { printf("%s \n\r", mess); }
// ajoute la box dans la hash tab
inline void addboxtohtab(struct box * pbox, GHashTable *ht){
  double *key = (double *) malloc(sizeof(double));
  *key =( ( pbox->x << sizeof(int) ) | pbox->y );
  g_hash_table_insert(ht,key,pbox);

}

//retourne la box qui a les coordonnée xy NULL si aucun
inline struct box * getBoxbyXY(int x, int y, GHashTable *ht){
  double key = ( (x << sizeof(int) ) | y );
  return g_hash_table_lookup(ht, &key);
}

struct box * newbox(void){
  DEBUG("ds new box");
  struct box * nbox;
  nbox = (box *)malloc(sizeof(box));
  DEBUG("ds new box av assign");
  nbox->left  = NULL;
  DEBUG("ds new box apres");
  nbox->right = NULL;
  nbox->up    = NULL;
  nbox->down  = NULL;
  nbox->state = 0;
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
    DEBUG("chainage a gauche");
  }

  ptempbox = getBoxbyXY( (pbox->x + 1 ), pbox->y, ht);
  if (ptempbox != NULL){
    pbox->right = ptempbox;
    ptempbox->left = pbox;
    DEBUG("chainage a droite");
  }

  ptempbox = getBoxbyXY( pbox->x, ( pbox->y - 1 ), ht);
  if (ptempbox != NULL){
    pbox->down = ptempbox;
    ptempbox->up = pbox;
    DEBUG("chainage en bas");
  }

  ptempbox = getBoxbyXY( pbox->x, ( pbox->y + 1 ), ht);
  if (ptempbox != NULL){
    pbox->up = ptempbox;
    ptempbox->down = pbox;
    DEBUG("chainage en haut");
  }

}

void printbox(struct box * pbox){
  if (pbox == NULL){
    DEBUG("ERREUR : la case courante est NULL");
    return;
  }
  if (pbox->left == NULL)
    DEBUG("left : ? ");
  else if ( pbox->left->state & MUR )
    DEBUG("left : MUR ");
  else
    DEBUG("left : ROUTE ");

  if (pbox->right == NULL)
    DEBUG("right : ? ");
  else if ( pbox->right->state & MUR )
    DEBUG("right : MUR ");
  else
    DEBUG("right : ROUTE ");

  if (pbox->up == NULL)
    DEBUG("up : ? ");

  else if (pbox->up->state & MUR)
    DEBUG("up : MUR ");
  else
    DEBUG("up : ROUTE ");

  if (pbox->down == NULL)
    DEBUG("down : ? ");
  else if (pbox->down->state & MUR)
    DEBUG("down : MUR ");
  else
    DEBUG("down : ROUTE ");

  printf("coordonnees : { %d | %d } \n\r",pbox->x, pbox->y);
}


