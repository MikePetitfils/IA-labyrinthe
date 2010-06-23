/*
 *  utilities.h
 *  
 *
 *  Created by Mickael Laloum on 09/06/10.
 *
 */


#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdlib.h>
#include <stdio.h>
#include "player.h"


/* affiche les infos sur un joueurs */
void printPlayer(Player *p);

/* affiche les joueurs de la liste */
void printPlayerList(Player **list,int numberOfPlayers);



/* covertie un status de case dans la map en un caractère */
void printBox(int status);

/* affiche la map */
void printMap(int *map,int width,int height);


/* convertie un status de case dans le tableau de l'action "observe" en un caractère */
void printBoxStatus(int status);

/* affiche l'obervation sous forme de map */
void printSeeMap(int *map,int width,int height);


/* affiche ce que l'utilisateur voit en chaque début de partie (2 cases Nord, 2 cases Sud, 2 cases Ouest et 2 cases Est)*/
void printStatusBox(int *statusBox,int width,int height);

void printStatuxBox2(int *statusBox,int width, int height);

#endif