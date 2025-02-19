/*
 * File: pingcli.c
 * Author: Brynn Crowley
 * Date: 02/2025
 * Description: This file contains client implementation of the UDP ping simulation tool with random packet loss.
 */
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <float.h>

#define BUFLEN 512

void die(const char *s);
void handle_sigint();

volatile bool doExit = false;


int main(const int argc, char *argv[]) {
  //set up signal handling for SIGINT for graceful shutdown
  const struct sigaction sa = {
    .sa_handler = handle_sigint,
    .sa_flags = 0
};
  sigaction(SIGINT, &sa, NULL);


  if (argc != 3) {
    fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_fd < 0) {
    die("socket");
  }

  char *endptr;
  errno = 0;
  const uint16_t port = strtol(argv[2], &endptr, 10);
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

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
    fprintf(stderr, "Invalid IP address\n");
    exit(EXIT_FAILURE);
  }

  int packets_sent = 0;
  int packets_received = 0;
  double rtt_min = DBL_MAX;
  double rtt_max = 0;
  double rtt_total = 0;

  while (!doExit && packets_sent < 10) {
    const char* ping = "PING";

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (sendto(server_fd, ping, strlen(ping), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
      close(server_fd);
      die("sendto");
    }

    char buf[BUFLEN];
    memset(buf, 0, BUFLEN);

    //set socket timeout to 1second
    struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in response_addr;
    socklen_t response_len = sizeof(response_addr);
    memset(&response_addr, 0, sizeof(response_addr));

    ssize_t recv_len = recvfrom(server_fd, buf, BUFLEN, 0, (struct sockaddr*)&response_addr, &response_len);
    gettimeofday(&end, NULL);

    if (recv_len > 0) {
      //calculates various ping statistics and rtt times.
      packets_received++;

      long seconds = end.tv_sec - start.tv_sec;
      long microseconds = end.tv_usec - start.tv_usec;
      double elapsed = seconds + microseconds*1e-6;

      if (elapsed > rtt_max) {
        rtt_max = elapsed;
      }
      if (elapsed < rtt_min) {
        rtt_min = elapsed;
      }

      rtt_total += elapsed;

      printf("Received from %s:%hu: PONG - RTT: %.2f ms\n", inet_ntoa(response_addr.sin_addr),
           ntohs(response_addr.sin_port), elapsed * 1000);
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
      printf("Request timed out for PING %d\n", packets_sent);
    } else {
      close(server_fd);
      die("recvfrom");
    }

    packets_sent++;
    sleep(1);
  }

  printf("\nPing statistics:\n");
  printf("Packets: Sent = %d, Received = %d, Lost = %d (%.2f%% loss)\n",
         packets_sent, packets_received, packets_sent - packets_received,
         (double)(packets_sent - packets_received) / packets_sent * 100);

  if (packets_received > 0) {
    printf("RTT statistics:\n");
    printf("Minimum RTT: %.2f ms, Maximum RTT: %.2f ms, Average RTT: %.2f ms\n",
           rtt_min * 1000, rtt_max * 1000, (rtt_total / packets_received) * 1000);
  }

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