/*
 * File: pingsvr.c
 * Author: Brynn Crowley
 * Date: 02/2025
 * Description: This file contains server implementation of the UDP ping simulation tool with random packet loss.
 */
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
#define PERCENT_DROPPED_PACKETS 30

void die(const char *s);
void handle_sigint();
bool shouldDropPing();

volatile bool doExit = false;


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
  printf("Starting listening on port %d\n", port);

  char buf[BUFLEN];
  struct sockaddr_in client;
  socklen_t client_length = sizeof(client);

  while (!doExit) {
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


    if (shouldDropPing()) {
      printf("Packet loss - Message dropped.\n");
      continue;
    } else {
      printf("Received packet from %s:%d: %s \n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buf);
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


/**
* The die method calls perror then exits with EXIT_FAILURE.
* Parameters:
* - s: The string supplied to perror
*/
void die(const char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

/**
* The handle_sigint method is a signal handler for the SIGINT signal. This method sets a flag that causes the program to cleanup and exit gracefully.
*/
void handle_sigint() {
  doExit = true;
}

/**
 * The shouldDropPing() Function
 * @return if the current packet should be dropped.
 */
bool shouldDropPing() {
  return ((random() % 100) < PERCENT_DROPPED_PACKETS);
}