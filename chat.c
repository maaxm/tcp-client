#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define NICK_CMD "/nick Max\n"

int main(int argc, char **argv)
{
  int opt;
  int port = 0;
  int sfd;
  int ret;
  char *nick;
  char buf[BUFFER_SIZE];
  ssize_t in, out;

  fd_set fds;

  while((opt = getopt(argc, argv, "p:n:")) != -1) {
        switch(opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'n':
            nick = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-p <port number>] [-n <nickname>]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
  }

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if(!sfd) {
    fprintf(stderr, "Socket error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  if(!port) {
    fprintf(stderr, "Error, port not specified. Usage: %s -p <port number>\n", argv[0]);
    exit(EXIT_FAILURE);
  } else if(!argv[optind]) {
    fprintf(stderr, "Error, ip not specified. Usage: %s <ip address>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  addr.sin_port = htons(port);
  inet_aton(argv[optind], &addr.sin_addr);

  ret = connect(sfd, (struct sockaddr*) & addr, sizeof(addr));

  if(ret < 0) {
    fprintf(stderr, "Failed to connect: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  write(sfd, NICK_CMD, strlen(NICK_CMD));

  for(;;) {
    FD_ZERO(&fds);
    FD_SET(sfd, &fds);
    FD_SET(STDIN_FILENO, &fds);

    select(sfd+1, &fds, NULL, NULL, NULL);

    if(FD_ISSET(sfd, &fds)) {
      in = read(sfd, buf, BUFFER_SIZE);
      if(in < 0) {
        fprintf(stderr, "Error reading the message: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      } else if(in == 0) {
        break;
      } else {
        write(STDOUT_FILENO, buf, in);
      }
    }

    if(FD_ISSET(STDIN_FILENO, &fds)) {
      in = read(STDIN_FILENO, buf, BUFFER_SIZE);
      if(in < 0) {
        fprintf(stderr, "Error reading the message: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      } else if(in == 0) {
        break;
      } else {
        write(sfd, buf, in);
      }
    }
  }
  close(sfd);
  exit(EXIT_SUCCESS);
}