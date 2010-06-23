/*
 *  utilities.c
 *  
 *
 *  Created by Mickael Laloum on 09/06/10.
 *  Copyright 2010 Augeo Software. All rights reserved.
 *
 */

#include "utilities.h"

void printPlayer(Player *p) 
{ 
	printf("Player :\n\tname : %s\n\tposition : %d,%d\n\tsock : %d\n", 
         p->name,
         p->x,
         p->y,
         p->sock);																									
}

void printPlayerList(Player **list,int numberOfPlayers){
	int i;
	for(i=0;i<numberOfPlayers;i++){
		printPlayer(list[i]);
	}
}


void printBoxStatus(int status){
  switch(status){		
    case BoxStatusUndefined:
      printf("?");
      break;
    case BoxStatusEmpty:
      printf(" ");
      break;
    case BoxStatusWall:
      printf("*");
      break;
    case BoxStatusPlayer:
      printf("P");
      break;
    case BoxStatusObject:
      printf(".");
      break;
    case BoxStatusExit:
      printf("S");
      break;
    case -2 :
      printf("I");
      break;
    default :
      printf("E");
      break;
  }
}
void printBox(int status){
  switch(status){		
    case UNDEFINED:
      printf("?");
      break;
    case EMPTY:
      printf(" ");
      break;
    case WALL:
      printf("*");
      break;
    case PLAYER:
      printf("P");
      break;
    case OBJECT:
      printf(".");
      break;
    case EXIT :
      printf("S");
      break;
    case -2 :
      printf("I");
      break;
    default :
      printf("E");
      break;
  }
}
void printMap(int *map,int width,int height){
  
  int i,j;
  
	printf("\n\n-----------------------------------\n\n");
  
	for(i = 0; i< height; i++){
		for(j = 0; j < width; j++){
			printBox(map[width * i + j]);
			printf(" ");
		}
		printf("\n");
	}
	
	printf("\n\n-----------------------------------\n\n");
}

void printSeeMap(int *map,int width,int height){
  
  int i,j;
	printf("\n\n-----------------------------------\n\n");
  
	for(i = 0; i< height; i++){
		for(j = 0; j < width; j++){
			printBoxStatus(map[width * i + j]);
			printf(" ");
		}
		printf("\n");
	}
	
	printf("\n\n-----------------------------------\n\n");
}


void printStatusBox(int *statusBox,int width,int height){
  
  int j;
  
  
	for(j = 1 ; j >= 0; j--){
		
		
		// NORTH
		switch(statusBox[j]){		
			case UNDEFINED:
				printf("       ?\n");
				break;
			case EMPTY:
				printf("       V\n");
				break;
			case WALL:
				printf("       W\n");
				break;
			case PLAYER:
				printf("       P\n");
				break;
			case OBJECT:
				printf("       .\n");
				break;
			default :
				printf("       E\n");
				break;
		}
		
	}
	//LEFT 
  
  
  
  for(j = 1; j >= 0; j--){
    
    switch(statusBox[width*2+j]){		
      case UNDEFINED:
        printf(" ? ");
        break;
      case EMPTY:
        printf(" V ");
        break;
      case WALL:
        printf(" W ");
        break;
      case PLAYER:
        printf(" P ");
        break;
      case OBJECT:
        printf(" . ");
        break;
      default :
        printf(" E ");
        break;
    }
  }
  // CENTER
  printf(" J ");
  
  
  //Right 
	
  for(j = 0; j < 2; j++){
    switch(statusBox[width*3 +j]){		
      case UNDEFINED:
        printf(" ? ");
        break;
      case EMPTY:
        printf(" V ");
        break;
      case WALL:
        printf(" W ");
        break;
      case PLAYER:
        printf(" P ");
        break;
      case OBJECT:
        printf(" O ");
        break;
      default :
        printf(" E ");
				break;
    }
  }
  
  for(j = 0 ; j < 2; j++){
		
    // SOUTH
    switch(statusBox[width+j]){		
      case UNDEFINED:
        printf("\n       ?");
        break;
      case EMPTY:
        printf("\n       V");
        break;
      case WALL:
        printf("\n       W");
        break;
      case PLAYER:
        printf("\n       P");
        break;
      case OBJECT:
        printf("\n       O");
        break;
      default :
        printf("\n       E");
        break;
    }	
  }
	printf("\n\n");
	
  
}


void printStatuxBox2(int *statusBox,int width, int height)
{

  
  printf("       %d\n",statusBox[1]);
  printf("       %d\n",statusBox[0]);
  printf(" %d ",statusBox[width*2+1]);
  printf(" %d ",statusBox[width*2]);
  printf(" P ");
  printf(" %d \n",statusBox[width*3]);
  printf(" %d \n",statusBox[width*3+1]);
  printf("       %d\n",statusBox[width]);
  printf("       %d\n",statusBox[width+1]);
  
}
