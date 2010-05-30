/*
** main.h
** Login : <najon@najon-desktop>
** Started on  Thu May 13 17:04:44 2010 najon
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

#ifndef   	MAIN_H_
# define   	MAIN_H_
void WhatweGonnaDo();


#define MUR      0x01
#define PELLE    0x02
#define MINE     0x04
#define COLLET   0x08
#define PLAYER   0x10

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

}box;
void culdesacbuster(struct box *);
void nouvelle_cases(struct box **);
void update_current(struct box **);
struct box * newbox(void);
void avancer(struct box **);
void printbox(struct box *);
inline void addtohtab(struct box *);
inline struct box * getBoxbyXY(int, int);
#endif 	    /* !MAIN_H_ */
