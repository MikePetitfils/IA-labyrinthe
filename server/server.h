#ifndef SERVER_H
#define SERVER_H


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h> /* timeval , time*/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */


#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SELECT_ERROR -1
#define SELECT_TIMEOUT	0

/* redefine usual socket struct */ 

#ifndef SOCKET_T
#define SOCKET_T
typedef int SOCKET;
#endif

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#define closesocket(s) close(s)

/* configure server */
#define PORT	3333
#define MAX_PLAYERS	1
#define MIN_PLAYERS 1

#define LOOP_TIMEOUT	30
#define CONNECTION_TIMEOUT 10;

#define BUF_SIZE	1024

#define NB_DIRECTION 4
#define OBS_DEPTH 4
#define SEE_DEPTH 2

#include "utilities.h"
#include "object.h"
#include "player.h"
#include "map.c"


SOCKET ihm = 0;
Info_map current_map;


/*
 * Socket Function Managment *
                             */

static SOCKET init_connection(void);
static void end_connection(SOCKET sock);
static void write_client(SOCKET sock,const  char *buffer);
static int	read_client(SOCKET sock, char *buffer);
static void send_message_to_IHM(SOCKET ihm, const char *buffer);
static void send_message_to_player(Player *player, const char *buffer);

/*
 * Map Function Managment *
                          */

/* Charge une map aléatoirement et envoi la map au IHM*/ 
void load_map(Info_map *infoMap);
/* Place aléatoirement un nombre aléatoire de sortie (1 à MAX_EXITS)*/
void place_exits(Info_map *currentMap);
/* Place aléatoirement tous les joueurs */
void place_players(Player **players, int num_player, Info_map *currentMap);
/* Place aléatoirement tous les objets */
void place_objects(Object **objects, int num_object, Info_map *currentMap);


/*
 * Game's actions * 
                  */

/* attend indéfiniement qu'un IHM se connect */
int wait_IHM_connection(SOCKET sock, SOCKET *ihm);


/* attend pendant un certains temps qu'un minimum de joueurs se connectent 
   renvoi 1 si un minimum de joueurs se sont connectés dans le temps imparti
   renvoi -1 sinon. Dans ce cas la partie ne peut commencer */
int wait_players_connection(SOCKET sock,Player ***players,int *num_players);

/* initialise la partie : connexion, chargement de la map.
    return -1 si la partie ne peux pas commencer (ex: pas assez de joueurs) */
int initialization(SOCKET *sock, Player ***players, int *numberOfPlayers);

/* envoi 2 cases (pour chaque direction Nord, Soud ,Est et Ouest) autour du joueur
   Cette fonction est appelé à chaque tour  */
void send_around_to_player(Player *aPlayer);

/* Effectue un déplacement du joueur dans la map 
   renvoi 1 si le déplacement est réussi
   sinon -1 */
int move_player(Player *aPlayer,char * direction,int nb);

/* observer autour du joueur avec un champs de profondeur 
   rempli un tableau obs de taille fixe 9x9 puisque le champs de profondeur max est 4
   height : 9 = 4 (Nord) + 1 (position du joueur) + 4 (Sud)
   height : 9 = 4 (Est) + 1 (position du joueur) + 4 (Ouest)
 */
void observe_around_me(Player *p,int *map, int width, int height,int *obs,int profondeur);


/*
  regarde 2 case autour du joueurs afin de lui indiquer en chaque débu de tour
  ce qu'il y a devant,derriere et sur les cotés
 */
void around_me(Player *p,int *cases);



/* 
 * Cmd Management *
                  */

/* verifie la validiter de la cmd reçu */
int check_cmd(char *cmd); // pas fini de l'implémenter

/* construit la cmd */
void build_cmd(); // pas du tout implémenter

#endif
