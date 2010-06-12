/*
** main.c
** Started on  Thu May 13 13:29:26 2010 najon
**
** Author(s):
**  - jmassot <massot.julien@gmail.com>
*/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include <glib/gi18n.h>
#include <glib/gtypes.h>
#include <glib/ghash.h>
#include "libbox.h"
#include "main.h"
struct box *currentbox;
struct box *curseurbox;
GHashTable *ht;
#define BUFFSIZE 255
char buffer[BUFFSIZE];
void Die(char *mess) { perror(mess); exit(1); }


int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in echoserver;
  ht = g_hash_table_new(g_str_hash, g_str_equal);

  currentbox    = newbox(ROUTE, 0,0,ht);
  if (argc != 4) {
    fprintf(stderr, "USAGE: %s <server_ip> <port> <pseudo>\n", argv[0]);
    exit(1);
  }
  /* Create the TCP socket */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    Die("Failed to create socket");
  }

  /* Construct the server sockaddr_in structure */
  memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
  echoserver.sin_family = AF_INET;                  /* Internet/IP */
  echoserver.sin_addr.s_addr = inet_addr(argv[1]);  /* IP address */
  echoserver.sin_port = htons(atoi(argv[2]));       /* server port */
  /* Establish connection */
  if (connect(sock,
              (struct sockaddr *) &echoserver,
              sizeof(echoserver)) < 0) {
    Die("Failed to connect with server");
  }
  send(sock,argv[3], strlen(argv[3]),0);
  //while true listen at the socket
  while (1){

    int bytes = 0;
    if ((bytes = recv(sock, buffer, BUFFSIZE-1, 0)) < 1) {
      Die("Failed to receive bytes from server");
    }

    buffer[bytes] = '\0';        /* Assure null terminated string */

    //printf("what we gonna do : \n\r %c", buffer[0]);
    WhatweGonnaDo();
    //avancer(currentbox);

    send(sock,buffer,strlen(buffer),0);

  }
}
/*
 * on n'a pas encore le protocole alors je simule la premiere lettre du msg
 *v pour voir a pour
 */
void WhatweGonnaDo(){
  unsigned int i, j,k;
  char response[9];
  fprintf(stdout, buffer);
  // si c'est une réponse a voir
  if (buffer[0] == 'P'){
    //eat until "Timeout:"
    for ( i = 0; buffer[i] != ':'; i++);
      //if( i == strlen(buffer) )
      //break;
      // }

    i++;
    /* printf("buffer ----------------------------- \n\r"); */
    /* for ( k = i; k < strlen(buffer);k++) */
    /*   printf("%c", buffer[k]); */
    /* printf("buffer ----------------------------- \n\r"); */
    //eat until North:
    for ( j = 0; j < 8 ; j++ ){
      for ( ; buffer[i] != ':'; i++){
        if( i == strlen(buffer) )
          break;
        else if (buffer[i] == ',')
          break;
      }
      /* printf("buffer ----------------------------- \n\r"); */
      /* for ( k = i; k < strlen(buffer);k++) */
      /*   printf("%c", buffer[k]); */
      /* printf("j : %d \n\r", j); */
      /* printf("buffer ----------------------------- \n\r"); */
      i++;

      switch(buffer[i]){
      case '-':
        response[j] = UNKNOWN;
        printf("response[%d] = UNKNOWN; \n\r",j);
        break;
      case'0':
        response[j] = ROUTE;
        printf("response[%d] = ROUTE; \n\r",j);
        break;
      case'1':
        response[j] = MUR;
        printf("response[%d] = MUR; \n\r",j);
        break;
      case'2':
        response[j] = PLAYER;
        printf("response[%d] = PLAYER; \n\r",j);
        break;
      case '3':
        response[j] = EXIT;
        printf("response[%d] = EXIT; \n\r",j);
        break;
      default:
        DEBUG("erreur de parsage");
        exit(1);
      }

    }

    response[j]='\0';
    nouvelle_cases(&currentbox, response);
  }


  //TODO : what we do whith this fucking buffer!
  memset(buffer,0,BUFFSIZE);
  avancer(&currentbox);
}


/*si on nous donne un resultat on rajoute les nouvelles cases dans l'arbre
 *en gros sur reponse de la commande voir
 *on manipule l'objet courent je prend des pointeurs de pointeurs pour
 *faire disparaitre au fur et a mesure la putain de variable globale avec
 *l'arbre
*/

void nouvelle_cases(struct box ** pbox, char * buff){
  int nbr_mur = 0;

  if ((*pbox)->up == NULL )
    (*pbox)->up=newbox(buff[0],(*pbox)->x, (*pbox)->y + 1, ht);
  if ((*pbox)->up->up == NULL )
    (*pbox)->up->up=newbox(buff[1],(*pbox)->x, (*pbox)->y + 2, ht);

  if ((*pbox)->down == NULL )
    (*pbox)->down=newbox(buff[2],(*pbox)->x, (*pbox)->y - 1, ht);
  if ((*pbox)->down->down == NULL )
    (*pbox)->down->down=newbox(buff[3],(*pbox)->x, (*pbox)->y - 2, ht);

  if ((*pbox)->left == NULL )
    (*pbox)->left=newbox(buff[4],(*pbox)->x - 1, (*pbox)->y, ht);
  if ((*pbox)->left->left == NULL )
    (*pbox)->left->left=newbox(buff[5],(*pbox)->x - 2, (*pbox)->y, ht);

  if ((*pbox)->right == NULL )
    (*pbox)->right=newbox(buff[6],(*pbox)->x + 1, (*pbox)->y, ht);
  if ((*pbox)->right->right == NULL )
    (*pbox)->right->right = newbox(buff[7],(*pbox)->x + 2, (*pbox)->y, ht);

}
/*
 * Sur demande change la current box
 *
 */
void update_current(struct box ** pbox){
  switch ( buffer[1] ) {
  case 'u':
    (*pbox) = (*pbox)->up;
    break;
  case 'd':
    (*pbox) = (*pbox)->down;
    break;
  case 'l':
    (*pbox) = (*pbox)->left;
    break;
  case 'r':
    (*pbox) = (*pbox)->right;
    break;
  default:
    break;

  }
}
/*
 * Cette fonction sert à laisser le choix à l'IA pour avancer
 * prend la première issue qui n'est pas un mur
 */
void avancer(struct box ** pbox){
  DEBUG("avancer dedans");
  if ( (*pbox)->up != NULL ){
    DEBUG("up not null");
    if ( !((*pbox)->up->state & MUR) ){
      strcpy(buffer, "MOVE North\0");
      (*pbox)=(*pbox)->up;
      DEBUG ("avancer :up ");
      return;
    }
  }
  else
    DEBUG("up null ");
  if ( (*pbox)->down != NULL ){
    if ( !((*pbox)->down->state & MUR) ){
      strcpy(buffer,"MOVE South\0");
      (*pbox)=(*pbox)->down;
      DEBUG ("avancer :down ");
      return;
    }
  }
  else
    DEBUG("down null ");

  if ( (*pbox)->right != NULL ){
    if ( !((*pbox)->right->state & MUR) ){
      strcpy(buffer,"MOVE East\0");
      (*pbox)=(*pbox)->right;
      DEBUG ("avancer : right\n\r");
      return;
    }
  }
  else
    DEBUG("right  null ");

  if ( (*pbox)->left != NULL ){
    if ( !((*pbox)->left->state & MUR) ){
      DEBUG ("avancer left");
      (*pbox)=(*pbox)->left;
      strcpy(buffer,"MOVE West\0");
      return;
    }
  }
  else
    DEBUG("left null ");


}


/*
 * verifie si la case est un cul de sac si c'est un cul de sac la case est definit comme un mur et
 * se rappel recursivement sur la case de la seul issue
 * ne marque pas en mur la case si le joueur est dessus
*/
void culdesacbuster(struct box * pbox){
  int issue;
  issue = 0;
  DEBUG("cul de sac buster");
  if ( pbox == NULL ){
    printf("la case est NULL");
    return;
  }

  if ( pbox != currentbox){
    pbox->state |= MUR;
    DEBUG("cul de sac detect");
    if ( pbox->left != NULL && !( pbox->left->state & MUR ) )
      culdesacbuster(pbox->left);
    else if ( pbox->right != NULL && !( pbox->right->state & MUR ) )
      culdesacbuster(pbox->right);
    else if ( pbox->down != NULL && ! ( pbox->down->state & MUR ) )
      culdesacbuster(pbox->down);
    else if ( pbox->up != NULL && !( pbox->up->state & MUR ) )
      culdesacbuster(pbox->up);
  }
}


