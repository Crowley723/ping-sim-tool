#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>

#define BUFLEN 512

void die(const char *s);
void handle_sigint(int sig);
bool shouldDropPing();

volatile bool sigint = false;


int main(const int argc, char *argv[]) {
  const struct sigaction sa = {
    .sa_handler = handle_sigint,
    .sa_flags = 0
};
  sigaction(SIGINT, &sa, NULL);

  srandom(time(NULL));

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_fd < 0) {
    die("socket");
  }

  char *endptr;
  errno = 0;
  const uint16_t port = strtol(argv[1], &endptr, 10);
  if (errno != 0) {
    die("Invalid port number");
  }
  if (*endptr != '\0') {
    fprintf(stderr, "Invalid characters in port number\n");
    exit(EXIT_FAILURE);
  }
  if (port <= 0 || port > 65535) {
    fprintf(stderr, "Port number must be between 1 and 65535\n");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(port),
    .sin_addr.s_addr = htonl(INADDR_ANY),
  };


  if (bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    close(server_fd);
    die("bind");
  }

  char buf[BUFLEN];
  struct sockaddr_in client;
  socklen_t client_length = sizeof(client);

  while (!sigint) {
    printf("Waiting for data...\n");
    fflush(stdout);
    memset(buf, 0, BUFLEN);

    const ssize_t recv_len = recvfrom(server_fd, buf, BUFLEN, 0, (struct sockaddr *) &client, &client_length);

    if (recv_len < 0) {
      if (errno == EINTR) {
        continue;
      }

      close(server_fd);
      die("recvfrom()");
    }

    printf("Received packet from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    printf("Data: %s\n", buf);

    if (shouldDropPing()) {
      printf("Packet loss sucks, huh?\n");
      continue;
    }

    const char* pong = "PONG";

    if (sendto(server_fd, pong, strlen(pong), 0, (struct sockaddr *) &client, client_length) < 0) {
      close(server_fd);
      die("sendto");
    }
  }

  printf("\nExiting...\n");
  close(server_fd);
  return EXIT_SUCCESS;
}


void die(const char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

void handle_sigint(int sig) {
  sigint = true;
}

bool shouldDropPing() {
  return ((random() % 100) < 30);
}