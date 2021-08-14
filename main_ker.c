#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <stdio.h>

void fatal() {
  write(2, "Fatal error\n", strlen("Fatal error\n"));
  exit(1);
}

int extract_message(char **buf, char **msg)
{
  char	*newbuf;
  int	i;

  *msg = 0;
  if (*buf == 0)
    return (0);
  i = 0;
  while ((*buf)[i])
  {
    if ((*buf)[i] == '\n')
    {
      newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
      if (newbuf == 0)
        fatal();
      strcpy(newbuf, *buf + i + 1);
      *msg = *buf;
      (*msg)[i + 1] = 0;
      *buf = newbuf;
      return (1);
    }
    i++;
  }
  return (0);
}

char *str_join(char *buf, char *add)
{
  char	*newbuf;
  int		len;

  if (buf == 0)
    len = 0;
  else
    len = strlen(buf);
  newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
  if (newbuf == 0)
    fatal();
  newbuf[0] = 0;
  if (buf != 0)
    strcat(newbuf, buf);
  free(buf);
  strcat(newbuf, add);
  return (newbuf);
}

typedef struct s_client {
  int fd;
  char *message;
}			t_client;
t_client  clients[66000];
int sockfd;
fd_set read_fd, write_fd, curren_fd;
int max_fd, count_client;

void add_message_to_all(int author, char *message) {
  for (int i = 0; i < count_client; ++i) {
	if (clients[i].fd != -1 && i != author) {
	  clients[i].message = str_join(clients[i].message, message);
	}
  }
}

void new_client() {
  clients[count_client].fd = accept(sockfd, NULL, NULL);
  if (clients[count_client].fd == -1) {
    if (errno == ENOMEM || errno == ENOBUFS) {
      fatal();
    } else {
	  return;
    }
  }
  clients[count_client].message = NULL;
  max_fd = clients[count_client].fd > max_fd ? clients[count_client].fd : max_fd;
  FD_SET(clients[count_client].fd, &curren_fd);
  char buf[50];
  sprintf(buf, "server: client %d just arrived\n", count_client);
  add_message_to_all(count_client, buf);
  count_client++;
}

void new_message(int id) {
  char *buffer = calloc(42, 4096);
  if (buffer == NULL)
    fatal();
  long read_byte = recv(clients[id].fd, buffer, 4096 * 42, 0);
  if (read_byte == 0) {
	FD_CLR(clients[id].fd, &curren_fd);
	close(clients[id].fd);
	clients[id].fd = -1;
	free(clients[id].message);
	clients[id].message = NULL;
	char buf[50];
	sprintf(buf, "server: client %d just left\n", id);
	add_message_to_all(id, buf);
  } else if (read_byte > 0) {
    char *message;
    char tmp[30];
    sprintf(tmp, "client %d: ", id);
    while (extract_message(&buffer, &message)) {
	  add_message_to_all(id, tmp);
	  add_message_to_all(id, message);
	  free(message);
    }
  }
  free(buffer);
}


void send_message(int id) {
  if (clients[id].message != NULL) {
    if (send(clients[id].fd, clients[id].message, strlen(clients[id].message), 0) == -1) {
     if (errno == ENOMEM)
       fatal();
    }
	free(clients[id].message);
    clients[id].message = NULL;
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
	exit(1);
  }
  struct sockaddr_in servaddr;
  // socket create and verification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    fatal();
  }
  max_fd = sockfd;
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
  servaddr.sin_port = htons(atoi(argv[1]));

  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
    fatal();
  }

  if (listen(sockfd, 10) != 0) {
    fatal();
  }
  FD_ZERO(&curren_fd);

  while (1)
  {
    read_fd = write_fd = curren_fd;
    FD_SET(sockfd, &read_fd);
    if (select(max_fd + 1, &read_fd, &write_fd, NULL, NULL) == -1) {
      fatal();
    }
    if (FD_ISSET(sockfd, &read_fd))
      new_client();
    for (int i = 0; i < count_client; ++i) {
      if (clients[i].fd != -1 && FD_ISSET(clients[i].fd, &read_fd)) {
        new_message(i);
      }
    }
    for (int i = 0; i < count_client; ++i) {
      if (clients[i].fd != -1 && FD_ISSET(clients[i].fd, &write_fd)) {
        send_message(i);
      }
    }
  }
}