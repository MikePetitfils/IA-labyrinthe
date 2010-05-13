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
void WhatweGonnaDo(const char * );


#define UNKNOWN 255
#define MUR      NULL
typedef struct
   {
  struct box * left;
  struct box * right;
  struct box * up;
  struct box * down;

   } box;


void nouvelle_cases(const char * );
#endif 	    /* !MAIN_H_ */
