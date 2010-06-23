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

#define BUFFSIZE 255

GHashTable *ht;

void Die(char *mess)
{
  perror(mess);
  exit(1);
}


box *explore(box *pbox);


/*
 * set the buffer to be sent to the server according to the move computed by explore
 */
void avancer(box *prev, box *next, char *buffersend){
  printf("moving to box[%d, %d]\n", next->x, next->y);
  box_print(next);
  if (prev->up == next)
    strcpy(buffersend, "MOVE North");
  else if (prev->down == next)
    strcpy(buffersend,"MOVE South");
  else if (prev->right == next)
    strcpy(buffersend,"MOVE East");
  else if (prev->left == next)
    strcpy(buffersend,"MOVE West");
  else
  {
    print_map(ht, next);
    printf("FAIL MON POTE\n");
    //exit(1);
  }
}

int startswith(const char *buffer, const char *start)
{
  int len = strlen(start);
  int i;

  for (i = 0; i < len; ++i)
    if (buffer[i] != start[i])
      return 0;
  return 1;
}

/*
 * on n'a pas encore le protocole alors je simule la premiere lettre du msg
 *v pour voir a pour
 */
box *WhatweGonnaDo(box *currentbox, const char *bufferrecv, char *buffersend){
  box         *prev;

  buffersend[0] = 0;

  if (startswith(bufferrecv, "North"))
  {
    if (box_is_unknown(currentbox->up))
      currentbox->up = newbox(box_decode_state(bufferrecv[6]),currentbox->x, currentbox->y + 1, ht);
    if (box_is_unknown(currentbox->up->up))
      currentbox->up->up = newbox(box_decode_state(bufferrecv[8]),currentbox->x, currentbox->y + 2, ht);
  }

  else if (startswith(bufferrecv, "South"))
  {
    if (box_is_unknown(currentbox->down))
      currentbox->down = newbox(box_decode_state(bufferrecv[6]),currentbox->x, currentbox->y - 1, ht);
    if (box_is_unknown(currentbox->down->down))
      currentbox->down->down = newbox(box_decode_state(bufferrecv[8]),currentbox->x, currentbox->y - 2, ht);
  }

  else if (startswith(bufferrecv, "West"))
  {
    if (box_is_unknown(currentbox->left))
      currentbox->left = newbox(box_decode_state(bufferrecv[5]),currentbox->x - 1, currentbox->y, ht);
    if (box_is_unknown(currentbox->left->left))
      currentbox->left->left = newbox(box_decode_state(bufferrecv[7]),currentbox->x - 2, currentbox->y, ht);
  }

  else if (startswith(bufferrecv, "East"))
  {
    if (box_is_unknown(currentbox->right))
      currentbox->right = newbox(box_decode_state(bufferrecv[5]),currentbox->x + 1, currentbox->y, ht);
    if (box_is_unknown(currentbox->right->right))
      currentbox->right->right = newbox(box_decode_state(bufferrecv[7]),currentbox->x + 2, currentbox->y, ht);
  }
  else if (startswith(bufferrecv, "FAILED"))
  {
    print_map(ht, currentbox);
    printf("Fail sucker... invalid move\n");
    exit(1);
  }
  //TODO: ack => the first time the server doest not send Play but Timeout (we start after each empty line as a hack)
  //if (startswith(bufferrecv, "Play") || bufferrecv[0] == 0)
  else if (bufferrecv[0] == 0)
  {

    //TODO : what we do whith this fucking buffer!
    prev = currentbox;
    currentbox = explore(currentbox);
    avancer(prev, currentbox, buffersend);
  }
  else
  {
    printf("Command : %s\n", bufferrecv);
  }
  return currentbox;
}



/**
 * - store a list of each cross
 * - find the next case not visited
 *
 */

static box *gcross[256] = { 0, };
static int  gcurrent    = 0;

//store the road we take
//used to rollback to the previous point (all box between the endpoint and the rollback point will be overriden)
static box *gposlog[50000] = { 0, };
static box *grewinddest = 0;
static int  gposcurrent = 0;


void rb_store(box *current)
{
  gposlog[gposcurrent++] = current;
}

box *rb_rewind()
{
  box *next;

  //not rewinding
  if (!grewinddest)
    return 0;
  if (gposcurrent == 0)
    return 0;

  next = gposlog[--gposcurrent];

  //rewind finish, stop rewinding
  if (next == grewinddest)
  {
    grewinddest = 0;
    //remove the cross from the list of cross, will be readded latter if it's still interesting
    gcurrent--;
  }
  return next;
}

void rb_setup_rewind(box *dest)
{
  grewinddest = dest;
}

/*
 *
 */
box *explore(box *pbox) {
  int  c    = box_count_interesting_road(pbox);
  box *next = 0;


  box_print(pbox);

  //mark current box as visited
  box_mark_visited(pbox);

  next = rb_rewind();
  if (next)
    return next;

  //store the current box (should be after rewind), we dont want to store your move when rewinding
  rb_store(pbox);

  if (box_is_exit(pbox))
  {
    printf("\n");
    printf("################################################################\n");
    printf("BIM je mange ta maman et je saute en case terminale. Kamoulox\n");
    printf("################################################################\n");
    //TODO: faire le deplacement de la fin
    exit(0);
  }

  //more than one read => store the crossing for later
  if (c > 1) {
    gcross[gcurrent++] = pbox;
  }

  next = box_get_interesting_neighboor(pbox);
  //we are in a "cul de sac" (with french accent)
  //rollback to the previous cross
  if (!next)
  {
    if (gcurrent < 1)
    {
      printf("fail 1\n");
      exit(1);
    }
    printf("rewinding:\n");
    rb_setup_rewind(gcross[gcurrent - 1]);
    //cancel the current move
    rb_rewind();
    return rb_rewind();
  }
  return next;
}

/* fgets for socket */
int sgets(char *buffer, int max, int fd)
{
  int  i = 0;
  int bytes = 0;
  char *p = buffer;

  while (i < max)
  {
    bytes = recv(fd, p, 1, 0);
    if (bytes < 1)
    {
      printf("Receive returned: %d\n", bytes);
      Die("Failed to receive bytes from server AHHH");
    }
    ++i;
    if (*p == '\n')
      break;
    p++;
  }

  //on est jamais trop sure
  *p = 0;
  buffer[max] = 0;
  return i;
}

int main(int argc, char *argv[]) {
  int    sock;
  struct sockaddr_in echoserver;
  char   bufferrecv[256];
  char   buffersend[256];
  box   *currentbox;
  box   *prevbox;

  ht     = g_hash_table_new(g_str_hash, g_str_equal);
  currentbox    = newbox(ROUTE, 0, 0, ht);
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
  if (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0)
    Die("Failed to connect with server");

  send(sock, argv[3], strlen(argv[3]), 0);
  if (!sgets(bufferrecv, 255, sock))
    Die("Failed to receive bytes from server");
  if (!sgets(bufferrecv, 255, sock))
    Die("Failed to receive bytes from server");

  //while true listen at the socket
  while (1){
    if (!sgets(bufferrecv, 255, sock))
      Die("Failed to receive bytes from server bim");
    //we ever know!
    bufferrecv[255] = 0;
    prevbox = currentbox;
    currentbox = WhatweGonnaDo(currentbox, bufferrecv, buffersend);
    if (buffersend[0])
    {
      printf("Sending: %s\n", buffersend);
      send(sock, buffersend, strlen(buffersend), 0);
      //easyier debug
      print_map(ht, prevbox);
      //sleep(1);
    }

  }
}
