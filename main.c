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

#include <glib/ghash.h>
#include <glib/gi18n.h>
#include <glib/gtypes.h>
#include "libbox.h"

#define BUFFSIZE 255
#define MAX_POS  50000

typedef struct {
  //store each crossing
  box       *crossing[MAX_POS];
  int        crossing_current;

  //store the road made
  box       *history[MAX_POS];
  int        history_current;

  //if rewind is active, this is the destination
  box       *rewind_destination;

  box       *current_box;
  box       *previous_box;

  //store the board in a hashtable
  GHashTable *ht;

} Map;


void win()
{
  printf("\n");
  printf("################################################################\n");
  printf("BIM je mange ta maman et je saute en case terminale.\n");
  printf("################################################################\n");
  //TODO: faire le deplacement de la fin
  //return 0;
  exit(0);
}

void die(char *mess)
{
  perror(mess);
  exit(1);
}


/*
 * set the buffer to be sent to the server according to the move computed by explore
 */
void avancer(Map *map, char *buffersend){

  printf("moving from [%d, %d] to[%d, %d]\n",
         map->previous_box->x, map->previous_box->y,
         map->current_box->x, map->current_box->y);
  //box_print(map->current_box);
  if (map->previous_box->up == map->current_box)
    strcpy(buffersend, "MOVE North");
  else if (map->previous_box->down == map->current_box)
    strcpy(buffersend,"MOVE South");
  else if (map->previous_box->right == map->current_box)
    strcpy(buffersend,"MOVE East");
  else if (map->previous_box->left == map->current_box)
    strcpy(buffersend,"MOVE West");
  else
  {
    //print_map(ht, map->current_box);
    printf("FAIL MON POTE, this destination is impossible to reach (%d, %d) -> (%d, %d)\n",
           map->previous_box->x, map->previous_box->y, map->current_box->x, map->current_box->y);
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



void rb_store(Map *map)
{
  map->history[map->history_current++] = map->current_box;
  if (map->history_current > MAX_POS)
  {
    printf("map is too big\b");
    exit(1);
  }
}

/* return the next map to go, 0 if nothing to rewind */
box *rb_rewind(Map *map)
{
  box *next;

  //not rewinding
  if (!map->rewind_destination)
    return 0;
  if (map->history_current <= 0)
  {
    printf("nothing to rewind!!!\n");
    return 0;
  }

  next = map->history[--map->history_current];

  //rewind finish, stop rewinding
  if (next == map->rewind_destination)
  {
    map->rewind_destination = 0;
    //remove the cross from the list of cross, will be readded latter if it's still interesting
    map->crossing_current--;
  }
  return next;
}

void rb_setup_rewind(Map *map, box *dest)
{
  map->rewind_destination = dest;
}

/*
 * return the next box to explore
 */
box *explore(Map *map) {
  int  c    = box_count_interesting_road(map->current_box);
  box *next = 0;


  //box_print(map->current_box);

  //mark current box as visited
  box_mark_visited(map->current_box);

  next = rb_rewind(map);
  if (next)
    return next;

  //store the current box (should be after rewind), we dont want to store your move when rewinding
  rb_store(map);

  if (box_is_exit(map->current_box))
  {
    win();
  }

  //more than one read => store the crossing for later
  if (c >= 1) {
    map->crossing[map->crossing_current++] = map->current_box;
    if (map->crossing_current > MAX_POS){
      printf("map too big\n");
      exit(1);
    }
  }

  next = box_get_interesting_neighboor(map->current_box);
  //we are in a "cul de sac" (with french accent)
  //rollback to the previous cross
  if (!next)
  {
    if (map->crossing_current < 1)
    {
      printf("no more crossing: there is no solution\n");
      exit(1);
    }
    printf("rewinding:\n");
    rb_setup_rewind(map, map->crossing[map->crossing_current - 1]);
    //cancel the current move
    //rb_rewind();
    map->history_current--;
    return rb_rewind(map);
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
      die("Failed to receive bytes from server AHHH");
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

/*
 * on n'a pas encore le protocole alors je simule la premiere lettre du msg
 *v pour voir a pour
 */
void WhatweGonnaDo(Map *map, const char *bufferrecv, char *buffersend) {
  buffersend[0] = 0;

  if (startswith(bufferrecv, "North"))
  {
    if (box_is_unknown(map->current_box->up))
      map->current_box->up = newbox(box_decode_state(bufferrecv[6]),map->current_box->x, map->current_box->y + 1, map->ht);
    if (box_is_unknown(map->current_box->up->up))
      map->current_box->up->up = newbox(box_decode_state(bufferrecv[8]),map->current_box->x, map->current_box->y + 2, map->ht);
  }

  else if (startswith(bufferrecv, "South"))
  {
    if (box_is_unknown(map->current_box->down))
      map->current_box->down = newbox(box_decode_state(bufferrecv[6]),map->current_box->x, map->current_box->y - 1, map->ht);
    if (box_is_unknown(map->current_box->down->down))
      map->current_box->down->down = newbox(box_decode_state(bufferrecv[8]),map->current_box->x, map->current_box->y - 2, map->ht);
  }

  else if (startswith(bufferrecv, "West"))
  {
    if (box_is_unknown(map->current_box->left))
      map->current_box->left = newbox(box_decode_state(bufferrecv[5]),map->current_box->x - 1, map->current_box->y, map->ht);
    if (box_is_unknown(map->current_box->left->left))
      map->current_box->left->left = newbox(box_decode_state(bufferrecv[7]),map->current_box->x - 2, map->current_box->y, map->ht);
  }

  else if (startswith(bufferrecv, "East"))
  {
    if (box_is_unknown(map->current_box->right))
      map->current_box->right = newbox(box_decode_state(bufferrecv[5]),map->current_box->x + 1, map->current_box->y, map->ht);
    if (box_is_unknown(map->current_box->right->right))
      map->current_box->right->right = newbox(box_decode_state(bufferrecv[7]),map->current_box->x + 2, map->current_box->y, map->ht);
  }
  else if (startswith(bufferrecv, "FAILED"))
  {
    //print_map(map->ht, map->current_box);
    printf("Fail sucker... invalid move\n");
    exit(1);
  }
  else if (startswith(bufferrecv, "You win"))
  {
    printf("Command : %s\n", bufferrecv);
    win();
  }
  //TODO: ack => the first time the server doest not send Play but Timeout (we start after each empty line as a hack)
  //if (startswith(bufferrecv, "Play") || bufferrecv[0] == 0)
  else if (bufferrecv[0] == 0)
  {

    //TODO : what we do whith this fucking buffer!
    map->previous_box = map->current_box;
    map->current_box = explore(map);
    if (!map->current_box)
    {
      printf("error: no destination... trying again!!\n");
      map->current_box = map->previous_box;
    }

    avancer(map, buffersend);
  }
  else
  {
    printf("Command : %s\n", bufferrecv);
  }
}


int main(int argc, char *argv[]) {
  int                   sock;
  struct sockaddr_in    echoserver;
  char                  bufferrecv[256];
  char                  buffersend[256];
  Map                   mymap;
  Map                  *map = &mymap;

  if (argc != 4) {
    fprintf(stderr, "USAGE: %s <server_ip> <port> <pseudo>\n", argv[0]);
    exit(1);
  }

  memset(map, 0, sizeof(Map));
  map->ht          = g_hash_table_new(g_str_hash, g_str_equal);
  map->current_box = newbox(ROUTE, 0, 0, map->ht);

  /* the TCP socket */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    die("Failed to create socket");
  }

  /* Construct the server sockaddr_in structure */
  memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
  echoserver.sin_family = AF_INET;                  /* Internet/IP */
  echoserver.sin_addr.s_addr = inet_addr(argv[1]);  /* IP address */
  echoserver.sin_port = htons(atoi(argv[2]));       /* server port */
  /* Establish connection */
  if (connect(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0)
    die("Failed to connect with server");

  //send name
  send(sock, argv[3], strlen(argv[3]), 0);
  //fuck header
  if (!sgets(bufferrecv, 255, sock))
    die("Failed to receive bytes from server");
  //fuck header
  if (!sgets(bufferrecv, 255, sock))
    die("Failed to receive bytes from server");

  while (1) {
    if (!sgets(bufferrecv, 255, sock))
      die("Failed to receive bytes from server bim");
    WhatweGonnaDo(map, bufferrecv, buffersend);
    if (buffersend[0])
    {
      printf("Sending: %s\n", buffersend);
      send(sock, buffersend, strlen(buffersend), 0);
      //easyier debug
      printf("history: %4d, crossing: %4d\n", map->history_current, map->crossing_current);
      print_map(map->ht, map->previous_box);
      //sleep(1);
    }
  }

  printf("toto");
}
