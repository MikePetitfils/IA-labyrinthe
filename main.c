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
#include "main.h"
struct box *currentbox;
int last;
#define BUFFSIZE 32
char buffer[BUFFSIZE];
void Die(char *mess) { perror(mess); exit(1); }
void DEBUG(char *mess) { printf("%s \n\r", mess); }

int main(int argc, char *argv[]) {
  int sock;
  int n;
  struct sockaddr_in echoserver;
  unsigned int echolen;
  int received = 0;
  currentbox = newbox();
  last=0;
  if (argc != 3) {
    fprintf(stderr, "USAGE: %s <server_ip> <port>\n", argv[0]);
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

  //while true listen at the socket
  while (1){
    while (received < echolen) {
      int bytes = 0;
      if ((bytes = recv(sock, buffer, BUFFSIZE-1, 0)) < 1) {
        Die("Failed to receive bytes from server");
      }
      received += bytes;
      buffer[bytes] = '\0';        /* Assure null terminated string */
      printf("what we gonna do : \n\r %c", buffer[0]);
      WhatweGonnaDo();
      printf ("avancer\n\r");
      avancer(&currentbox);
      send(sock,buffer,strlen(buffer),0);
    }
  }
}
/*
 * on n'a pas encore le protocole alors je simule la premiere lettre du msg
 *v pour voir a pour
 */
void WhatweGonnaDo(){
  fprintf(stdout, buffer);
  // si c'est une r�ponse a voir
  if (buffer[0] == 'v'){

    nouvelle_cases(&currentbox);
  }
  // si c'est une r�ponse a avancer on repositionne le current_box
  else if (buffer[0] == 'a'){
    DEBUG("update current");
    update_current();
  }
  else if (buffer[0] == 'c'){
    culdesacbuster(currentbox);
  }
//TODO : what we do whith this fucking buffer!
  memset(buffer,0,BUFFSIZE);
}
/*si on nous donne un resultat on rajoute les nouvelles cases dans l'arbre
 *en gros sur reponse de la commande voir
 *on manipule l'objet courent je prend des pounteurs de pointeurs pour
 *faire disparaitre au fur et a mesure la putain de variable globale avec
 *l'arbre
*/

void nouvelle_cases(struct box ** pbox){

  if ( buffer[1] == 'm' ){
    if ((*pbox)->left == NULL ){
      (*pbox)->left=newbox();
      (*pbox)->left->state=MUR;
      DEBUG("left : mur");
    }
  }
  else
  {
    if ( ((*pbox)->left) == NULL ){
      DEBUG("left : box");
      (*pbox)->left=newbox();
      (*pbox)->left->right=(*pbox);
    }
  }

  if ( buffer[2] == 'm' ){
    if ( (*pbox)->right == NULL ){
      DEBUG("right : mur");
      (*pbox)->right=newbox();
      (*pbox)->right->state=MUR;
    }
  }else
  {
    if ( (*pbox)->right == NULL ){
      (*pbox)->right = newbox();
      (*pbox)->right->left=(*pbox);
      DEBUG("right : box");
    }
  }

  if ( buffer[3] == 'm' ){
    if ( (*pbox)->up == NULL ){
      DEBUG("up : mur");
      (*pbox)->up = newbox();
      (*pbox)->up->state=MUR;
    }
  }else
  {
    if ( (*pbox)->up == NULL ){
      (*pbox)->up = newbox();
      (*pbox)->up->down=(*pbox);
      DEBUG("up : box");
    }
  }

  if ( buffer[4] == 'm' )
    if ( (*pbox)->down == NULL ){
      DEBUG("down : mur");
      (*pbox)->down = newbox();
      (*pbox)->down->state=MUR;
    }
    else
    {
      if ( (*pbox)->down == NULL )
      {
        (*pbox)->down = newbox();
        (*pbox)->down->up=(*pbox);
        DEBUG("down : box");
      }
    }


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
    currentbox = currentbox->left;
    break;
  case 'r':
    currentbox = currentbox->right;
    break;

  }
}
/*
 * Cette fonctions sert a laiss� le chois a l'IA pour avancer
 * prend la premi�re issue qui n'est pas un mur
 */
void avancer(struct box ** pbox){
  DEBUG("avancer dedans");
  if ( (*pbox)->up != NULL ){
    DEBUG("up not null");
    if ( (*pbox)->up->state != MUR){
      strcpy(buffer, "avancer : up\n\r");
      (*pbox)=(*pbox)->up;
      DEBUG ("avancer :up ");
      return;
    }
  }
  else
    DEBUG("up null ");
  if ( (*pbox)->down != NULL ){
    if ( (*pbox)->down->state != MUR){
      strcpy(buffer,"avancer : down\n\r");
      (*pbox)=(*pbox)->down;
      DEBUG ("avancer :down ");
      return;
    }
  }
  else
    DEBUG("down null ");

  if ( (*pbox)->right != NULL ){
    if ( (*pbox)->right->state != MUR){
      strcpy(buffer,"avancer : right\n\r");
      (*pbox)=(*pbox)->right;
      DEBUG ("avancer : right\n\r");
      return;
    }
  }
  else
    DEBUG("right  null ");

  if ( (*pbox)->left != NULL ){
    if ( (*pbox)->left->state != MUR){
      DEBUG ("avancer : left\n\r");
      (*pbox)=(*pbox)->left;
      strcpy(buffer,"avancer : left\n\r");
      return;
    }
  }
  else
    DEBUG("left null ");


}

struct box * newbox(void){
  DEBUG("ds new box");
  struct box * nbox;
  nbox = (box *)malloc(sizeof(box));
  DEBUG("ds new box av assign");
  nbox->left  = NULL;
  DEBUG("ds new box apres");
  nbox->right = NULL;
  nbox->up    = NULL;
  nbox->down  = NULL;
  nbox->state = 0;
}

/*
 * verifie si la case est un cul de sac si c'est un cul de sac la case est definit comme un mur et
 * se rappel recursivement sur la case de la seul issue
 * ne marque pas en mur la case si le joueur est dessus
*/
void culdesacbuster(struct box * pbox){
  int issue;
  issue = 0;
  DEBUG("depart");
  if ( pbox == NULL ){
    printf("la case est NULL");
    return;
  }
  DEBUG("depart 2");
  if ( pbox->left == NULL || pbox->left->state != MUR )
    issue++;
  if ( pbox->right == NULL || pbox->right->state != MUR )
    issue++;
  if ( pbox->up == NULL || pbox->left->state != MUR )
    issue++;
  if ( pbox->down == NULL || pbox->left->state != MUR )
    issue++;
//there are only one issue
// comment� pour test� mais surement a cause ce ca que ca segfault
//  if ( issue < 2 && pbox != currentbox){
  if ( issue < 2){
    pbox->state=MUR;
    DEBUG("cul de sac detect");
    if ( pbox->left != NULL && pbox->left->state != MUR )
      culdesacbuster(pbox->left);
    else if ( pbox->right != NULL && pbox->right->state != MUR )
      culdesacbuster(pbox->right);
    else if ( pbox->down != NULL && pbox->down->state != MUR )
      culdesacbuster(pbox->down);
    else if ( pbox->up != NULL && pbox->up->state != MUR )
      culdesacbuster(pbox->up);
  }
}


