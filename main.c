/*
** main.c
** Login : <najon@najon-desktop>
** Started on  Thu May 13 13:29:26 2010 najon
** $Id$
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
#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(1); }
void DEBUG(char *mess) { printf("%s \n\r", mess); }
int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in echoserver;
  char buffer[BUFFSIZE];
  unsigned int echolen;
  int received = 0;
  currentbox = (box*)malloc(sizeof(struct box));
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
      WhatweGonnaDo(buffer);
      avancer(buffer);
      write(sock,buffer,strlen(buffer));
    }
  }
}

void WhatweGonnaDo(char * buff){
  fprintf(stdout, buff);
  // si c'est une réponse a voir
  if (buff[0] == 'v'){
    DEBUG("new box");
    nouvelle_cases(buff);
  }
  // si c'est une réponse a avancer on repositionne le current_box
  else if (buff[0] == 'a'){
    DEBUG("update current");
    update_current(buff);

  }
//TODO : what we do whith this fucking buffer!
  memset(buff,0,BUFFSIZE);
}
//si on nous donne un resultat on rajoute les nouvelles cases dans l'arbre
//en gros sur reponse de la commande voir
void nouvelle_cases(const char * buff){
  DEBUG("nouvelle case");
  if ( buff[1] == 'm' ){
    if (currentbox->left == NULL ){
      currentbox->left = (box *)malloc(sizeof(box));
      currentbox->left->state=MUR;
      DEBUG("left : mur");
    }
  }
  else
    if ( currentbox->left == NULL ){
      DEBUG("left : box");
      currentbox->left = (box *)malloc(sizeof(box));
    }
  if ( buff[2] == 'm' ){
    if ( currentbox->right == NULL ){
      DEBUG("right : mur");
      currentbox->right = (box *)malloc(sizeof(box));
      currentbox->right->state=MUR;
    }
  }
  else
    if ( currentbox->right == NULL ){
      currentbox->right = (box *)malloc(sizeof(box));
      DEBUG("right : box");
    }
  if ( buff[3] == 'm' ){
    if ( currentbox->up == NULL ){
      DEBUG("up : mur");
      currentbox->up = (box *)malloc(sizeof(box));
      currentbox->up->state=MUR;
    }
  }
  else
    if ( currentbox->up == NULL ){
      currentbox->up = (box *)malloc(sizeof(box));
      DEBUG("lup : box");
    }
  if ( buff[4] == 'm' )
    if ( currentbox->down == NULL ){
      DEBUG("down : mur");
      currentbox->down = (box *)malloc(sizeof(box));
      currentbox->down->state=MUR;
    }
    else
      if ( currentbox->down == NULL )
      {
        currentbox->down = (box *)malloc(sizeof(box));
        DEBUG("down : box");
      }


}

void update_current(const char * buff){
  switch ( buff[1] ) {
  case 'u':
    currentbox = currentbox->up;
    break;
  case 'd':
    currentbox = currentbox->down;
    break;
  case 'l':
    currentbox = currentbox->left;
    break;
  case 'r':
    currentbox = currentbox->right;
    break;

  }
}

void avancer(char * buff){
  if ( currentbox->up != NULL ){
    if ( currentbox->up->state != MUR){
      buff = "avancer : up";
      DEBUG ("avancer :up ");
    }
  }
  else if ( currentbox->down != NULL ){
    if ( currentbox->down->state != MUR){
      buff = "avancer : down";
      DEBUG ("avancer :down ");
    }
  }
  else if ( currentbox->right != NULL ){
    if ( currentbox->right->state != MUR){
      buff = "avancer : right";
      DEBUG ("avancer : right ");
    }
  }
  else if ( currentbox->left != NULL ){
    if ( currentbox->left->state != MUR){
      DEBUG ("avancer : left ");
      buff = "avancer : left";
    }
  }
}


