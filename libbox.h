/*
** libbox.h
** Login : <najon@najon-desktop>
** Started on  Sun May 30 03:10:47 2010 najon
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


#ifndef   	LIBBOX_H_
# define   	LIBBOX_H_

#define MUR      0x001
#define VISITED  0x002
#define PLAYER   0x010
#define EXIT     0x020
#define UNKNOWN  0x040
#define ROUTE    0x080

#define UP       1
#define DOWN     2
#define RIGHT    3
#define LEFT     4


typedef struct box
{
  struct box * left;
  struct box * right;
  struct box * up;
  struct box * down;
  unsigned char state;
  int x;
  int y;

} box;

void print_map(GHashTable *ht, box *current);

struct box * newbox(char, int , int , GHashTable *);
void printbox(struct box *);
char * givestatebox(struct box *, char* );

inline  box *addboxtohtab(struct box *, GHashTable *);
inline struct box * getBoxbyXY(int, int, GHashTable *);
void chainbox(struct box *, GHashTable *);
void DEBUG(char *);


int box_decode_state(char c);

int box_is_unknown(box *pbox);
int box_is_wall(box *pbox);
int box_is_visited(box *pbox);
int box_is_interesting(box *pbox);
int box_is_exit(box *pbox);
int box_count_interesting_road(box *pbox);
void box_mark_visited(box *pbox);
box *box_get_interesting_neighboor(box *pbox);
void box_print(box *box);



#endif 	    /* !LIBBOX_H_ */
