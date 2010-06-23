#ifndef MAP_H
#define MAP_H

#define UNDEFINED -1
#define EMPTY 0
#define WALL 1
#define PLAYER 2
#define EXIT 3
#define OBJECT 4

#define MAX_EXITS 5

enum {

BoxStatusUndefined 	= 0,
BoxStatusEmpty 			= 1 << 0,
BoxStatusWall 			= 1 << 1,
BoxStatusPlayer 		= 1 << 2,
BoxStatusExit			 	= 1 << 3,
BoxStatusObject			= 1 << 4
};

typedef unsigned int BoxStatus;



typedef struct{
	int id;
	int **map;
	int height;
	int width; 
}Info_map;

#endif
