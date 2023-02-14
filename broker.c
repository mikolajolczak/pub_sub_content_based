#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT 2137
#define ADDRESS "192.168.56.103"

// header values
#define PUBLISH 1
#define SUBSCRIBE 2
#define UNSUBSCRIBE 3
// type values
#define TEMPERATURE 1
#define HUMIDITY 2
#define SOIL_MOISTURE 3

// condition values
#define EQUAL 1
#define LESS_THAN 2
#define GREATER_THAN 3
#define LESS_THAN_OR_EQUAL 4
#define GREATER_THAN_OR_EQUAL 5

#define EMPTY 0

#define ANY 0

struct message {
  unsigned short header : 2;
  unsigned short type : 2;
  unsigned short condition : 3;
  unsigned short location : 1;
  uint8_t node_id;
  unsigned short message;
};

struct node {
  struct sockaddr_in clientaddr;
  unsigned short type : 2;
  unsigned short condition : 3;
  unsigned short condition_value;
  unsigned short location : 1;
  uint8_t node_id;
  struct node *next;
};

void handle_message(int sock, struct message *msg, struct node **head,
                    struct sockaddr_in *clientaddr) {
  switch (msg->header) {
  case PUBLISH:
    printf("Dostalismy Publisha!\n");
    printf("Type: %d\n", msg->type);
    printf("Location: %d\n", msg->location);
    printf("ID: %d\n", msg->node_id);
    printf("Message: %d\n", msg->message);
    publish(sock, msg, head);
    break;
  case SUBSCRIBE:
    printf("Dostalismy Subscribe!\n");
    printf("Type: %d\n", msg->type);
    printf("Location: %d\n", msg->location);
    printf("Condition: %d\n", msg->condition);
    printf("ID: %d\n", msg->node_id);
    printf("Message: %d\n", msg->message);
    subscribe(*msg, head, *clientaddr);
    break;
  case UNSUBSCRIBE:
    printf("Dostalismy Unsubscribe!\n");
    printf("Type: %d\n", msg->type);
    printf("Location: %d\n", msg->location);
    printf("Condition: %d\n", msg->condition);
    printf("ID: %d\n", msg->node_id);
    printf("Message: %d\n", msg->message);
    unsubscribe(*msg, head, *clientaddr);
    break;
  }
}

void publish(int sock, struct message *msg, struct node **head) {
  struct node *ptr = (*head);
  msg->condition = EMPTY;
  while (ptr != NULL) {
    if (ptr->location == ANY || ptr->location == msg->location) {
      if (ptr->type == msg->type) {
        switch (ptr->condition) {
        case ANY:
          sendto(sock, msg, sizeof(*msg), EMPTY,
                 (const struct sockaddr *)&ptr->clientaddr,
                 sizeof(ptr->clientaddr));
          break;
        case EQUAL:
          if (msg->message == ptr->condition_value) {
            sendto(sock, msg, sizeof(*msg), EMPTY,
                   (const struct sockaddr *)&ptr->clientaddr,
                   sizeof(ptr->clientaddr));
          }
          break;
        case LESS_THAN:
          if (msg->message < ptr->condition_value) {
            sendto(sock, msg, sizeof(*msg), EMPTY,
                   (const struct sockaddr *)&ptr->clientaddr,
                   sizeof(ptr->clientaddr));
          }
          break;
        case GREATER_THAN:
          if (msg->message > ptr->condition_value) {
            struct sockaddr_in cos = ptr->clientaddr;
            struct message masaz = *msg;
            sendto(sock, &masaz, sizeof(masaz), EMPTY,
                   (const struct sockaddr *)&cos, sizeof(cos));
          }
          break;
        case LESS_THAN_OR_EQUAL:
          if (msg->message <= ptr->condition_value) {
            sendto(sock, msg, sizeof(*msg), EMPTY,
                   (const struct sockaddr *)&ptr->clientaddr,
                   sizeof(ptr->clientaddr));
          }
          break;
        case GREATER_THAN_OR_EQUAL:
          if (msg->message >= ptr->condition_value) {
            sendto(sock, msg, sizeof(*msg), EMPTY,
                   (const struct sockaddr *)&ptr->clientaddr,
                   sizeof(ptr->clientaddr));
          }
          break;
        }
      }
    }
    ptr = ptr->next;
  }
  ptr = NULL;
  free(ptr);
}

void subscribe(struct message msg, struct node **head,
               struct sockaddr_in clientaddr) {
  int notcopy = 1;
  struct node *temp = (struct node *)malloc(sizeof(struct node));
  temp->clientaddr = clientaddr;
  temp->condition = msg.condition;
  temp->condition_value = msg.message;
  temp->type = msg.type;
  temp->location = msg.location;
  temp->node_id = msg.node_id;
  if (*head == NULL) {
    (*head) = temp;
    (*head)->next = NULL;
    notcopy = 0;
  } else {
    struct node *ptr = (*head);
    while (ptr->next != NULL) {
      if (ptr->clientaddr.sin_addr.s_addr == clientaddr.sin_addr.s_addr) {
        if (ptr->type == msg.type) {
          if (ptr->condition == msg.condition) {
            if (ptr->condition_value == msg.message) {
              if (ptr->location == msg.location) {
                notcopy = 0;
              }
            }
          }
        }
      }
      ptr = ptr->next;
    }
    if (ptr->clientaddr.sin_addr.s_addr == clientaddr.sin_addr.s_addr) {
      if (ptr->type == msg.type) {
        if (ptr->condition == msg.condition) {
          if (ptr->condition_value == msg.message) {
            if (ptr->location == msg.location) {
              notcopy = 0;
            }
          }
        }
      }
    }
    if (notcopy) {
      ptr->next = temp;
      temp->next = NULL;
    }
  }
}

void unsubscribe(struct message msg, struct node **head,
                 struct sockaddr_in clientaddr) {
  struct node *ptr = (*head);
  struct node *temp;
  if ((*head)->clientaddr.sin_addr.s_addr == clientaddr.sin_addr.s_addr) {
    if ((*head)->type == msg.type) {
      if ((*head)->condition == msg.condition) {
        if ((*head)->condition_value == msg.message) {
          if ((*head)->location == msg.location) {
            temp = (*head);
            (*head) = (*head)->next;
            free(temp);
          }
        }
      }
    }
  }
  while (ptr->next != NULL) {
    if (ptr->clientaddr.sin_addr.s_addr == clientaddr.sin_addr.s_addr) {
      if (ptr->type == msg.type) {
        if (ptr->condition == msg.condition) {
          if (ptr->condition_value == msg.message) {
            if ((*head)->location == msg.location) {
              temp = ptr->next;
              ptr->next = ptr->next->next;
              free(temp);
              break;
            }
          }
        }
      }
    }
    ptr = ptr->next;
  }
}

void display_list(struct node **head) {
  struct node *ptr = (*head);
  while (ptr != NULL) {
    printf("%d\n", ptr->type);
    ptr = ptr->next;
  }
  free(ptr);
}

int main(void) {
  int sock;
  struct sockaddr_in servaddr, clientaddr;
  struct message msg;
  memset(&msg, 0, sizeof(msg));
  int len = sizeof(struct sockaddr_in);
  fd_set readfds;
  struct node *head = NULL;
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    printf("Nie udalo sie stworzyc socketu\n");
    return -1;
  }
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);
  if (bind(sock, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    printf("Nie udalo sie powiazac socketu\n");
    return -1;
  }
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 5000;
  while (1) {
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    if (select(sock + 1, &readfds, NULL, NULL, &tv) >= 0) {
      if (FD_ISSET(sock, &readfds)) {
        memset(&msg, 0, sizeof(msg));
        recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&clientaddr,
                 &len);
        handle_message(sock, &msg, &head, &clientaddr);
      }
    }
  }
  return 0;
}