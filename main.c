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
box currentbox;
#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(1); }

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in echoserver;
  char buffer[BUFFSIZE];
  unsigned int echolen;
  int received = 0;
  currentbox->left = UNKNOWN;
  currentbox->up   = UNKNOWN;
  currentbox->right= UNKNOWN;
  currentbox->down = UNKNOWN;
  if (argc != 4) {
    fprintf(stderr, "USAGE: TCPecho <server_ip> <word> <port>\n");
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
  echoserver.sin_port = htons(atoi(argv[3]));       /* server port */
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
      WhatweGonnaDo(buffer);
    }
  }
}

void WhatweGonnaDo(const char * buff){
  fprintf(stdout, buff);
  box *currentbox;

  nouvelle_case(buff);
//TODO : what we do whith this fucking buffer!

}
//si on nous donne un resultat on rajoute les nouvelles cases dans l'arbre
//en gros sur reponse de la commande voir
box *nouvelle_case(const char * buff){

}
