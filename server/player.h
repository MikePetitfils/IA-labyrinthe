#ifndef PLAYER_H
#define PLAYER_H

#ifndef SOCKET_T
#define SOCKET_T
typedef int SOCKET;
#endif

#include "map.h"


typedef struct
{
   SOCKET sock; 			/* client socket */
   int 		x,y;				/* player's postion */
   int 		status;			/* player's status */
   char 	name[120];	/* player's name */
}Player;


#endif
