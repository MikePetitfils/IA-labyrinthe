#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#include "server.h"


void Die(char *mess) { perror(mess); exit(1); }
void DEBUG(char *mess) { printf("%s \n\r", mess); }

/* console log */
void log_to_console(char *msg){

  /* log to ihm*/
  if(ihm){
    send_message_to_IHM(ihm,msg);
  }else{ /* log in server */
    printf("%s \n\r",msg);
  }
}



void set(int *buffer,int val, int taille){
  int i;
  for(i=0 ; i<taille;i++){
    buffer[i]=val;
  }
}


///////////////////////////////////////////////






int main(int argc, char **argv)
{

  SOCKET sock = 0;
  int num_players = 0;
  Player **players;
  int start =  0;

  int i,j;


  /* init connection and wait players and ihm */
  if((start=initialization(&sock,&players,&num_players)) <= 0) {

    printf("The game cannot start\n");
    exit(1);
  }


  // place players on the map
  place_players(players,num_players,&current_map);

  int * map = (int *)current_map.map;

  struct timeval timeout; 	/* loop timeout */
  timeout.tv_sec = LOOP_TIMEOUT;
  timeout.tv_usec= 0;

  fd_set rdfs;		/* listen file descriptor in read mode */
  int max = sock; /* max listening file descriptor */

  int current = 0;		/* current player*/
  char buffer[BUF_SIZE];


  int next_loop = 1;

  /* loop */
  while(start){

    // display seconde left
    // timeout.tv_sec is unreliable so be carefull
    // if we are on linux timeout take seconde left but on windows don't
    printf("\n\n il reste %ld secondes avant la fin du tour \n\n",timeout.tv_sec);

    // get the current player
    Player *current_player = players[current];

    // if is a new tour
    // because sometime there is a loop when someone disconect
    if(next_loop){

      // reset loop
      next_loop = 0;

      // display a message to indicate who is the current player
      snprintf(buffer,sizeof(buffer),"c'est au tour de : %s\n",current_player->name);
      log_to_console(buffer);

      send_around_to_player(current_player);


    }

    FD_ZERO(&rdfs);		/* clear rdfs */
    FD_SET(sock,&rdfs);		/* listen for new connection */

    FD_SET(players[current]->sock,&rdfs);

    for(i = 0; i<num_players;i++){
      max = players[i]->sock > max ? players[i]->sock : max; 	/* what is the new maximum fd ? */
      FD_SET(players[i]->sock, &rdfs);	/* listen for all players */
    }

    /* wait event on  a file descriptor */
    switch(select(max + 1,&rdfs,NULL,NULL,&timeout)){

    case SELECT_ERROR :
      perror("select()");
      exit(errno);
      break;
    case SELECT_TIMEOUT :
      next_loop = 1;
      timeout.tv_sec = LOOP_TIMEOUT;
      current++;
      break;
    default:
      /* a player try to connect during the game */
      if(FD_ISSET(sock,&rdfs)){

        /* we reject new client */
        SOCKADDR_IN csin = { 0 };
        socklen_t sinsize = sizeof csin;

        SOCKET csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
        if(csock == SOCKET_ERROR){
          perror("accept()");
          continue;
        }

        if(num_players >= MAX_PLAYERS)
          write_client(csock,"REJECT\nComment:The server is full\n");
        else
          write_client(csock,"REJECT\nComment:The game has already begun\n");

        closesocket(csock);
        continue;
      }

      for(i=0;i<num_players;i++){
        /* news on socket ? */
        if(FD_ISSET(players[i]->sock,&rdfs)){
          /* one of players send data ?*/
          if(read_client(players[i]->sock,buffer) ){

            /* if current */
            if(i == current){

              /* si cmd == MOVE */
              if(strncmp(buffer,"MOVE",4) == 0){

                char direction[120];

                memset(direction,0,120);
                memcpy(direction,buffer+5,120);

                /* move the player */
                if(move_player(current_player,direction,1) > 0){
                  send_message_to_player(current_player,"OK\n");
                }else{ /* move failed */
                  send_message_to_player(current_player,"FAILED\nComment: cannot to move the player\n");
                }

                /* we move on the exit, the current user win*/
                if((1 << map[current_map.width *current_player->y + current_player->x]) &
                   (BoxStatusExit) ){

                  send_message_to_player(current_player,"You win\n");
                  send_message_to_IHM(ihm,"End of game\n");
                  start = 0;
                }
                /* si cmd == SEE */
              }else if(strncmp(buffer,"SEE",3) == 0){

                /* log info to ihm */
                log_to_console("The player looks around...\n");

                /* tableau d'observation */
                int obs_height = OBS_DEPTH * 2 + 1 ;
                int obs_width = OBS_DEPTH * 2 + 1 ;
                int obs[obs_height][obs_width];


                /* fill an array of all box status that the player has seen */
                observe_around_me(current_player,           // the curent player
                                  (int *)current_map.map, 	// the map
                                  current_map.width,				// map width
                                  current_map.height,				// map width
                                  (int *)obs,								// array of statux box
                                  OBS_DEPTH);								// depth of the sight




                /* build msg */

                char msg[2*BUF_SIZE];
                char tmp[2];

                snprintf(msg,5,"%dx%d\n",obs_height,obs_width); /* map size */

                /* for each case */
                for(i=0 ; i < obs_height ; i++){
                  for(j=0 ; j < obs_width ; j++){
                    /* cat array of box status value*/
                    snprintf(tmp,2,"%1d",obs[i][j]);
                    strcat(msg,tmp);
                  }
                  /* for each line add \n */
                  strcat(msg,"\n");
                }

                /* send the result to player */
                send_message_to_player(current_player,msg);
                send_message_to_player(current_player,"OK\n");

                /* si cmd == WAIT */
              }else if(strncmp(buffer,"WAIT",4) == 0){
                /* do nothing */
                send_message_to_player(current_player,"OK\n");


              }


              next_loop = 1; /* next_loop is a new game loop */
              timeout.tv_sec = LOOP_TIMEOUT; /* reset timeout */
              current++; /* next player */
            }else{

              send_message_to_player(players[i],"Wait your turn !\n");
            }

          }else{ /* someone disconnect from the server */

            snprintf(buffer,sizeof(BUF_SIZE),"\n%s left the server\n",players[i]->name);
            log_to_console(buffer);

            if(i == current){
              next_loop = 1;
              timeout.tv_sec = LOOP_TIMEOUT;
              current++;
            }
            /* cl&players,ose player socket */
            closesocket(players[i]->sock);
            /* free memory allocated for player */
            free(players[i]);

            /* move memory for list */
            memmove(players + i,
                    players + (i+1),
                    (num_players - i) * sizeof(Player *));

            num_players--;


          }
        }
      }//end of for


      break;
    } // end of switch

    // no more player
    if(num_players <= 0){
      end_connection(sock);
      free(players);
      break;
    }

    current = (current)%num_players;
  } // end of while
  return 0;

}
//////////////////////////////////////////////////////////////////////

int wait_IHM_connection(SOCKET sock, SOCKET *ihm){

  fd_set rdfs;	/* listen file descriptor in read mode */

  while(1){

    FD_ZERO(&rdfs);  		/* init  */
    FD_SET(sock,&rdfs);	/* add sock to listen */


    /* wait until data on sock or timeout is over */
    switch(select(sock + 1, &rdfs,NULL, NULL,NULL)){

    case SELECT_ERROR :
      perror("selec()");
      return errno;
      break;
    default :

      /* a IHM is connecting */
      if(FD_ISSET(sock,&rdfs)){

        /* new client */
        SOCKADDR_IN csin = { 0 };
        socklen_t sinsize = sizeof csin;
        char buffer[BUF_SIZE];

        SOCKET csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
        if(csock == SOCKET_ERROR){
          perror("accept()");
          continue;
        }

        /* after connecting the client sends its name */
        if(read_client(csock, buffer) == -1){
          DEBUG("disconnected ?");
          /* disconnected */
          continue;
        }


        if(strstr(buffer,"IHM") != NULL){
          *ihm = csock;
          return 1;
        }
      }

    } // end of switch
  }// end of while
}

int wait_players_connection(SOCKET sock,Player ***players,int *numberOfPlayers){

  struct timeval timeout; /* timeout for connection */
  timeout.tv_sec = CONNECTION_TIMEOUT;
  timeout.tv_usec = 0;

  fd_set rdfs;	/* listen file descriptor in read mode */

  *numberOfPlayers = 0;	/* init number of players */
  *players = malloc(MAX_PLAYERS * sizeof(Player *));

  while(1){

    if((*numberOfPlayers) >= MAX_PLAYERS)
      return 1;

    /* Attention !!!!!

       Sous Linux, la fonction select modifie timeout pour indiquer le temps restant
       mais la plupart des autres implémentations ne le font pas.

       Dans mon cas, ce n'est pas génant psuique je l'utilise que pour l'affichage
    */

    printf("Waiting for connection... (%ld s)\n",timeout.tv_sec);
    FD_ZERO(&rdfs);  		/* init  */
    FD_SET(sock,&rdfs);	/* add sock to listen */

    /* wait until data on sock or timeout is over */
    switch(select(sock + 1, &rdfs,NULL, NULL,&timeout)){

    case SELECT_ERROR :
      perror("selec()");
      return errno;
      break;
    case SELECT_TIMEOUT :
      /* if there is no enough players */
      if((*numberOfPlayers) < MIN_PLAYERS) {
        /* the game doesn't start */
        printf("There is no enough players.. (%d/%d)\n",*numberOfPlayers,MIN_PLAYERS);
        return 0;
      }

      return 1;
      break;
    default :

      /* a client is connecting */
      if(FD_ISSET(sock,&rdfs)){

        /* new client */
        SOCKADDR_IN csin = { 0 };
        socklen_t sinsize = sizeof csin;
        char buffer[BUF_SIZE];

        SOCKET csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
        if(csock == SOCKET_ERROR){
          perror("accept()");
          continue;
        }

        /* if server is full */
        if((*numberOfPlayers) >= MAX_PLAYERS){
          write_client(csock,"REJECT\nCOMMENT: The server is full.");
          continue;
        }

        /* after connecting the client sends its name */
        if(read_client(csock, buffer) == -1){
          DEBUG("disconnected ?");
          /* disconnected */
          continue;
        }


        /* create the player */
        Player *p = malloc(sizeof(Player));
        memset(p,0,sizeof(Player));
        p->sock = csock;
        strncpy(p->name, buffer, 120 - 1);


        /* add player to thelist */
        (*players)[(*numberOfPlayers)] = p;

        /* send a welcome msg to client */
        memset(buffer,0,sizeof(buffer));
        snprintf(buffer,sizeof(buffer),"Welcome %s in the server\n",p->name);
        write_client(p->sock,buffer);

        /* log */
        char msg[BUF_SIZE];
        memset(msg,0,sizeof(msg));
        snprintf(msg,sizeof(msg),"%s has joined the server.\n",p->name);
        printf("%s\n",msg);
        log_to_console(msg);

        (*numberOfPlayers)++;

      }


    }// end switch

  } // end while

}
int initialization(SOCKET *sock, Player ***players, int *numberOfPlayers) {

  int result;
  /* create the connection socket */
  *sock = init_connection();

  /* wait the IHM */
  if(wait_IHM_connection(*sock,&ihm)){


    /* load a random map */
    load_map(&current_map);

    /* wait players with a timeout*/
    result = wait_players_connection(*sock,players,numberOfPlayers);

  }else {
    result = 0;
  }

  return result;
}

/////////////////////////////////////////
//
// Socket Function Managment
//
////////

static SOCKET init_connection(void) {
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  SOCKADDR_IN sin = { 0 };

  if(sock == INVALID_SOCKET){
    perror("socket()");
    exit(errno);
  }

  /* Construct the server sockaddr_in structure */
  memset(&sin, 0, sizeof(sin));
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  //sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  sin.sin_port = htons(PORT);
  sin.sin_family = AF_INET;

  /* Enable address reuse */
  int on = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));


  if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR){
    perror("bind()");
    exit(errno);
  }

  if(listen(sock, MAX_PLAYERS) == SOCKET_ERROR){
    perror("listen()");
    exit(errno);
  }

  return sock;

}
static void end_connection(SOCKET sock)
{
  closesocket(sock);
}
static void write_client(SOCKET sock,const  char *buffer)
{
  if(send(sock, buffer, strlen(buffer), 0) < 0) {
    perror("send()");
    exit(errno);
  }
}
static int read_client(SOCKET sock, char *buffer)
{
  int n = 0;

  if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
    perror("recv()");
    /* if recv error we disonnect the client */
    n = 0;
  }

  buffer[n] = 0;

  return n;

}
static void send_message_to_IHM(SOCKET ihm, const char *buffer)
{
  if(send(ihm, buffer, strlen(buffer), 0) < 0){
    perror("send()");
    exit(errno);
  }
}
static void send_message_to_player(Player *player, const char *buffer)
{
  if(send(player->sock, buffer, strlen(buffer), 0) < 0){
    perror("send()");
    exit(errno);
  }
}


//////
//
// /Socket Function Managment
//
///////////////////////////////////







///////////////////////////////////
//
// Player Action Managment
//
//////

void around_me(Player *p,int *cases){
  int i,j;
  int x ,y;

  int numDirection = NB_DIRECTION;
  int numCase = SEE_DEPTH;

  int *currentMap = (int *)current_map.map;

  // pour chacune des direction : North,Sud,Est,Ouest
  // remplir les 2 cases
  for(i=0;i<numDirection;i++){
    for(j=0;j<numCase;j++){

      // sens par rapport à la position du joueur
      int sens = ((i % 2) == 0)? -1 : 1;
      // nombre de case (avec signe)
      int delta = (j+1) * sens;

      // si on veut renseigner Nord,Sud
      if(i < (numDirection/2)){
        y = p->y + delta;
        x = p->x;
      }
      // on veut renseigner est, ouest
      else{
        y = p->y;
        x = p->x + delta;
      }



      if(y > current_map.height || y < 0 || x < 0 || x > current_map.width)
        break;

      // renseigne la case j
      cases[(numCase*i)+j] = currentMap[current_map.width*y + x];


      // nous ne voyons pas à travers les murs et les personnes
      // csq : on rempli pas
      if((1<<cases[numCase*i+j]) & (BoxStatusWall | BoxStatusPlayer)){
        break;
      }

    }
  }

  return;
}


void send_around_to_player(Player *aPlayer){



  // create an array to store all cases aournd the current player
  int around[NB_DIRECTION][SEE_DEPTH];

  // init this array to -1
  set((int *)around,-1,NB_DIRECTION*SEE_DEPTH);

  // fill the array
  around_me(aPlayer,(int *)around);

  char msg[BUF_SIZE];
  char buffer[5][20];


  memset(buffer,0,sizeof(buffer));


  snprintf(buffer[0],20,"Timeout:%d\n",	30);
  snprintf(buffer[1],20,"North:%d,%d\n",around[0][0],around[0][1]);
  snprintf(buffer[2],20,"South:%d,%d\n",around[1][0],around[1][1]);
  snprintf(buffer[3],20,"West:%d,%d\n",	around[2][0],around[2][1]);
  snprintf(buffer[4],20,"East:%d,%d\n",	around[3][0],around[3][1]);



  memset(msg,0,sizeof(msg));


  strncpy(msg,"Play\n",20);

  int i;
  for(i=0 ; i<5; i++){

    strncat(msg,buffer[i],20);
  }
  strncat(msg,"\n",1);


  send_message_to_player(aPlayer, msg);

}

// deplace un joueur sur la carte
// renvoi vrai si le joueur s'est déplacé. faux sinon.
// j'ai fais exprès de séparer la sortie de map à le fait qu'il y ait un mur ou un joueur
// même si le resultat est le même
int move_player(Player *aPlayer, char *direction,int nb){

  int success = -1;
  int new_x = 0, new_y = 0;
  int direction_int = 0;

  char msg[1024];
  memset(msg,0,sizeof(msg));

  int * currentMap = (int *)current_map.map;

  /* verifie si la cmd est correcte */
  if(strncmp(direction,"North",5) == 0){
    direction_int =0;
  }else if(strncmp(direction,"South",5) == 0){
    direction_int =1;
  }else if(strncmp(direction,"West",4) == 0){
    direction_int =2;
  }else if(strncmp(direction,"East",4) == 0){
    direction_int =3;
  }
  else{ // error lors de la reception/envoi de la cmd
    log_to_console("FAILED\nComment: Undefined direction.\n");
    success=-1;
    return success;
  }

  /* cmd correcte, on regarde si on peut déplacer ou non le joueur */

  // sens par rapport a la case du joueur
  // - = Nord / Gauche
  // + = Sud / Droit
  int sens = (direction_int%2 == 0)? -1 : 1;

  // si on agit sur l'axe y (Nord/Sud)
  if(direction_int < 4/2){
    new_x = aPlayer->x;
    new_y = aPlayer->y + (sens * nb);
  }else{ // si on agit sur l'axe x (Gauche/Droite)
    new_x = aPlayer->x + (sens * nb);
    new_y = aPlayer->y;
  }


  // si on sort de la map
  if(new_y < 0 || new_y > current_map.height || new_x < 0 || new_x > current_map.width){
    log_to_console("FAILED\nComment: Move can't be done, position is out of the map.\n");
    success = -1;
  }
  // si il y a un mur ou un joueur
  else if((1 << currentMap[current_map.width*new_y + new_x]) & (BoxStatusWall|BoxStatusPlayer)){
    log_to_console("FAILED\nComment: Move can't be done, undefined direction.\n");
    success = -1;
  }
  // si tout va bien
  else{


    /* changer la position du joueur courrant */
    currentMap[current_map.width*aPlayer->y + aPlayer->x] = EMPTY;
    aPlayer->x = new_x;
    aPlayer->y = new_y;

    currentMap[current_map.width*aPlayer->y + aPlayer->x] = PLAYER;


    /* envoi du resultat à l'ihm */
    snprintf(msg,sizeof(msg),"MOVE %s\n",direction);
    send_message_to_IHM(ihm,msg);

    log_to_console("The player has moved\n");

    success = 1;
  }

  return success;
}



// permet remplis un tableau de donnée (obs[9][9]) dans lequel figure
// l'observation d'un joueur.
// La méthode est comme celui d'un radar sous marin ou une onde qui se propage:
// je regarde toutes les cases autour du personnage.
// Et remplies ces dernières en fonction de ce que j'ai déjà observé.
// En effet, nous pouvons observer avec une profondeur de champs passée en paramètre.
// Et nous regardons d'une profondeur de champs de 1, puis de 2, puis de 3 .. jusqu'a arrivé au paramètre de profondeur

// method récursive
// A chaque observation de profondeur n, je regarde ce que nous avons rempli lors de l'observation de profondeur n-1
// Et rempli les cases observer en fonction de la postion du mur par rapport au joueurs

// methode très compliqué si vous avez d'autre algo, je suis preneur..
// jai proposé le mien. il n'est pas parfait, j'ai essayé de l'optimiser
// je ne parcours pas ce que j'ai deja vu, je ne fais des opérations que quand c'est necessaire
void observe_around_me(Player *p,int *map, int width, int height,int *obs,int profondeur){

  // recup la profonceur
  int n = profondeur;

  // taille fixe du tableau d'observation qui va être envoyer ensuite au du joueur
  int obs_height = OBS_DEPTH * 2 + 1;
  int obs_width = OBS_DEPTH * 2 + 1;

  // postion du joueurs
  int x = p->x , y = p->y;

  // variable utiliser pour les indices de tableaux et boucles for
  int i,j;    // indice/coordonnée dans la map
  int i2,j2;  // indice/coordonnée dans le tableau d'observation obs
  int u,v;    // indice/coordonnée dans la map
  int u2,v2;  // indice/coordonnée dans le tableau d'observation obs

  // variable permettant de parcourir autour d'une case ou zone de case
  int delta_x, delta_y;  // parcourir autour de la zone deja observer
  int dx,dy;             // parcourir autour de la case a remplir

  // choses pour lesquelles le joueur ne voit pas à travers
  BoxStatus condition = BoxStatusWall|BoxStatusPlayer;


  /* condition initiale de la récursivité */

  // profondeur 0
  if(n == 0){
    // remplir dans la case du milieu de obs, la position du joueur
    obs[obs_width*4+4] = 9; // à changer
    return;

    // si on est de profondeur supérieur à 0
  }else {
    // on observe à une profondeur inférieur (-1)
    // c'est à dire qu'on renseigne/ rempli les case de profondeur inférieur (-1)
    observe_around_me(p,map,width,height,obs,profondeur-1);
  }



  /* Une fois la zone précedent remplis, nous pouvons remplir la zone courrante */

  // formation de zone autour de la zone precedent (la zone déjà observée)
  for(delta_y=-n ; delta_y<= n; delta_y++){
    for(delta_x=-n ; delta_x <= n ;delta_x ++){

      // case dans la map
      i = delta_y + y;
      j = delta_x + x;

      // si on est en dehors de la map, on passe à la case suivante
      if(i < 0 || j < 0 || i > height || j > width){
        continue;
      }

      // case dans obs
      i2 = 4 + delta_y;
      j2 = 4 + delta_x;

      // si on est en dehors de obs, on passe à la case suivante
      if(i2 < 0 || j2 < 0 || i2 > 9 || j2 > 9){
        continue;
      }

      // on ne regarde que les cases autour de la zone observée précédente
      // donc  si on n'est pas le zone precedente
      // c'est à dire que la valeur absolue de distance entre joueur et la case doit être égale n
      if(abs(delta_y) == n || abs(delta_x) == n ){

        // si aligné vertivalement ou horizontalement (Nord, Sud, Est, Ouest)
        // ne se préoccuper que de la case en dessus/dessous (Nord, Sud) ou sur le coté (Est, Ouest)
        if( j == x || i == y){



          // alignement verticale
          // partie haut
          if(j == x && i < y-1){

            // si on peut pas la voir
            if(obs[obs_width*(i2+1) + j2] & condition){
              obs[obs_width*i2 + j2] = BoxStatusUndefined;
            }else
              obs[obs_width*i2 + j2] = 1 << map[width*i + j];

          }
          // partie bas
          else if (j == x && i > y+1) {

            if(obs[obs_width*(i2-1) + j2] & condition
               || obs[obs_width*(i2-1) + j2] == BoxStatusUndefined){

              obs[obs_width*i2 + j2] = BoxStatusUndefined;
            }else
              obs[obs_width*i2 + j2] = 1 << map[width*i + j];


          }
          // alignement horizontal
          // partie gauche
          else if( i == y && j < x-1) {

            if(obs[obs_width*i2 + (j2+1)] & condition
               || obs[obs_width*i2 + (j2+1)] == BoxStatusUndefined) {
              obs[obs_width*i2 + j2] = BoxStatusUndefined;
            }else
              obs[obs_width*i2 + j2] = 1 << map[width*i + j];

          }
          // partie droite
          else if (i == y && j > x+1){

            if(obs[obs_width*i2 + (j2-1)] & condition
               || obs[obs_width*i2 + (j2-1)] == BoxStatusUndefined) {
              obs[obs_width*i2 + j2] = BoxStatusUndefined;
            }else
              obs[obs_width*i2 + j2] = 1 << map[width*i + j];

          }
          else{
            obs[obs_width*i2 + j2] = 1 << map[width*i + j];
          }

          continue;

        }


        // sinon la case n'est pas aligné
        // regarder, dans obs, chaque case autour de la case courante (celle que l'on veut remplir)
        // mais on ne regarde seulement que les cases de la zone deja observée
        for(dy=-1 ; dy <= 1 ; dy++){
          for(dx=-1; dx <= 1 ; dx++){


            // position dans obs
            u2 = i2 + dy;
            v2 = j2 + dx;

            // si les indices sortent du tableau obs
            if(u2 == -1 || v2 == -1 || u2 == 9 || v2==9)
              continue;

            // on ne parcours pas, dans tous les cas, la case que l'on veut remplir
            // puis qu'n regarde autour d'elle
            if(dx == 0 && dy == 0)
              continue;


            // partie haut gauche
            if(i < y && j < x ){

              // restreint à la zone déjà observée
              if(dx > -1 && dy > -1 && abs(u2-4)<n && abs(4-v2)<n){


                if(obs[obs_width*u2 + v2] & condition
                   || obs[obs_width*u2 + v2] == BoxStatusUndefined){

                  obs[obs_width*i2 + j2] = BoxStatusUndefined;
                  break;
                }else
                  obs[obs_width*i2 + j2] = 1 << map[width *i + j];

              }
            }
            // partie haut droite
            else if (i < y && j > x ) {

              // restreint à la zone déjà observée
              if(dx < 1 && dy > -1 && abs(u2-4)<n && abs(4-v2)<n){


                if(obs[obs_width*u2 + v2] & condition
                   || obs[obs_width*u2 + v2] == BoxStatusUndefined){
                  obs[obs_width*i2 + j2] = BoxStatusUndefined;
                  break;
                }else
                  obs[obs_width*i2 + j2] = 1 << map[width *i + j];

              }

            }
            // partie bas gauche
            else if(i > y && j < x ){

              // restreint à la zone déjà observée
              if(dx > -1 && dy < 1 && abs(u2-4)<n && abs(4-v2)<n){

                if(obs[obs_width*u2 + v2] & condition
                   || obs[obs_width*u2 + v2] == BoxStatusUndefined){
                  obs[obs_width*i2 + j2] = BoxStatusUndefined;
                  break;
                }else
                  obs[obs_width*i2 + j2] = 1 << map[width *i + j];

              }
            }
            // partie bas droite
            else if(i > y && j > x ){

              // restreint à la zone déjà observée
              if(dx < 1 && dy < 1 && abs(u2-4)<n && abs(4-v2)<n){

                if(obs[obs_width*u2 + v2] & condition
                   || obs[obs_width*u2 + v2] == BoxStatusUndefined){
                  obs[obs_width*i2 + j2] = BoxStatusUndefined;
                  break;
                }else
                  obs[obs_width*i2 + j2] = 1 << map[width *i + j];

              }
            }
          } // end of for dx


        } // end of for dy

      } // enf of if abs

    } // end of for j

  }// end of for i



  printSeeMap(obs,9,9);

}



//////
//
// /Player Action Managment
//
///////////////////////////////////


///////////////////////////////////
//
// Command Managment
//
//////


int check_cmd(char *cmd){

  DEBUG("Check cmd : ");

  char *cmds[6]={"PLAY","MOVE","SEE","AROUND","OK","REJECT"};
  char *directions[4] = {"North","South","Est","West"};
  int i;

  char *operation = strtok (cmd," ");

/*	char * str = memchr((char *)cmds,"\0");

	while(str != NULL){
        DEBUG(str);
        str = strtok(NULL,"\0");
	}*/

  DEBUG("Fin check cmd");
}

//////
//
// /Command Managment
//
///////////////////////////////////


///////////////////////////////////
//
// Map Managment
//
//////

/* Map Function Managment */
void load_map(Info_map *infoMap){

  int i,j;
  char buffer[3*BUF_SIZE];
  memset(buffer,0,sizeof(buffer));


  /* initialize random seed: */
  srand(time(NULL));

  infoMap->id = (rand() % 3) + 1;

  switch(infoMap->id){

  case 1 :
    infoMap->map = (int **)map1;
    infoMap->width = 32;
    infoMap->height = 32;
    break;
  case 2 :
    infoMap->map = (int **)map2;
    infoMap->width = 32;
    infoMap->height = 32;
    break;
  case 3 :
    infoMap->map = (int **)map3;
    infoMap->width = 32;
    infoMap->height = 32;
    break;
  default:
    infoMap->map = (int **)map;
    infoMap->width = 10;
    infoMap->height = 10;
  }


  place_exits(&current_map);

  /* création variable d'accès à la map */
  int *map = (int *) current_map.map;

  /* log to IHM loading */
  snprintf(buffer,sizeof(buffer),"Chargement de la map %d...\n",infoMap->id);
  log_to_console(buffer);

  /* add the map size to the msg */
  snprintf(buffer,sizeof(buffer),"%dx%d\n",infoMap->width,infoMap->height);

  /* build msg which represetn the map */
  char tmp[2];
  for(i=0;i<infoMap->height;i++){
    for(j=0;j<infoMap->width;j++){
      snprintf(tmp,2,"%d",map[current_map.width*i + j]);
      strncat(buffer,tmp,sizeof(tmp));
    }
    strncat(buffer,"\n",1);
  }

  send_message_to_IHM(ihm,buffer);



}

void place_exits(Info_map *currentMap){


  int i;
  int random_x, random_y;

  int *map = (int *) currentMap->map;

  /* initialize random seed: */
  srand(time(NULL));

  int num_exits = rand()%MAX_EXITS + 1;



  for(i = num_exits ; i > 0; i-- ){

    // generate a random x,y
    random_x = rand()% currentMap->width;
    random_y = rand()% currentMap->height;

    // while the box at the position x,y is a wall, player or the exit
    // generate new position
    while( ((1 << map[currentMap->width*random_y+random_x])
            & (BoxStatusWall|BoxStatusExit|BoxStatusPlayer)) ){

      random_x = rand()% currentMap->width;
      random_y = rand()% currentMap->height;
    }

    map[currentMap->width*random_y + random_x] = EXIT;

  }


}

void place_players(Player **players, int num_player,Info_map *currentMap){

  int i;
  int random_x,random_y;
  int *map = (int*) currentMap->map;
  char buffer[BUF_SIZE];
  memset(buffer,0,sizeof(buffer));

  char properties[20];
  memset(properties,0,sizeof(properties));

  /* initialize random seed: */
  srand(time(NULL));

  /* add to msg the number of player */
  snprintf(buffer,5,"%d\n",num_player);


  // for each player
  for(i=0;i<num_player;i++){


    // generate a random x,y
    random_x = rand()% currentMap->width;
    random_y = rand()% currentMap->height;

    // while the box at the position x,y is a wall, player or the exit
    // generate new position
    while( ((1 << map[currentMap->width*random_x+random_y])
            & (BoxStatusWall|BoxStatusExit|BoxStatusPlayer)) ){

      random_x = rand()% currentMap->width;
      random_y = rand()% currentMap->height;
    }

    /* set player position to the n ew position */
    players[i]->x = random_x;
    players[i]->y = random_y;

    /* fill map */
    map[currentMap->width*random_y + random_x] = PLAYER;

    /* build msg which will be send to IHM */
    // for each player
    // i:x,y\n
    snprintf(properties,20,"%d:%d,%d\n",i,players[i]->x,players[i]->y);
    strncat(buffer,properties,20);
  }

  /* send position of all player to ihm */
  send_message_to_IHM(ihm,buffer);
}
void place_objects(Object **objects, int num_object, Info_map *currentMap){

  int i;
  int random_x,random_y;
  int *map = (int*) currentMap->map;

  /* initialize random seed: */
  srand(time(NULL));

  // for each object
  for(i=0;i<num_object;i++){

    // generate a random position x,y
    random_x = rand()% currentMap->width;
    random_y = rand()% currentMap->height;

    // while the box at the position x,y is a wall, player or the exit
    // generate new position
    while( ((1 << map[currentMap->width*random_y+random_x])
            & (BoxStatusWall|BoxStatusExit|BoxStatusObject)) ){
      random_x = rand()% currentMap->width;
      random_y = rand()% currentMap->height;
    }

    objects[i]->x = random_x;
    objects[i]->y = random_y;

    map[currentMap->width*random_y+random_x] = OBJECT;

  }


}


//////
//
// /Map Managment
//
///////////////////////////////////
