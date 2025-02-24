# UDP Pinger

A simple implementation of a UDP-based ping utility in C, demonstrating client-server communication with simulated packet loss.

## Overview

This project implements a client-server application that simulates the functionality of the ping utility using UDP sockets. The server artificially introduces a 30% packet loss rate to demonstrate handling unreliable network conditions.

## Features

- UDP-based client-server communication
- Configurable server port
- Simulated 30% packet loss
- Round-trip time (RTT) calculations
- Statistics reporting (packet loss rate, min/max/avg RTT)
- Graceful server shutdown with SIGINT handling
- Support for multiple simultaneous clients

## Prerequisites

- GCC compiler
- Linux/Unix environment
- Make (optional, for build automation)

## Building

```bash
# Compile the server
gcc -o pingsvr pingsvr.c

# Compile the client
gcc -o pingcli pingcli.c
```

## Usage

### Server
```bash
./pingsvr <port>

# Example:
./pingsvr 8001
```

### Client
```bash
./pingcli <server_host> <server_port>

# Example:
./pingcli localhost 8001
```

## Output Format

### Server Output
```
Server listening on port 8001
Packet loss - Message dropped.
Received from 130.86.188.33:50320: PING 2
Received from 130.86.188.33:50320: PING 3
...
```

### Client Output
```
Request timed out for PING 1
Received from 130.86.188.33:8001: PONG - RTT: 3.72 ms
Received from 130.86.188.33:8001: PONG - RTT: 1.40 ms
...

Ping statistics:
Packets: Sent = 10, Received = 7, Lost = 3 (70.00% loss)
RTT statistics:
Minimum RTT: 0.51 ms, Maximum RTT: 3.72 ms, Average RTT: 1.54 ms
```

## Implementation Details

### Server (pingsvr.c)
- Listens for incoming UDP packets on specified port
- Simulates 30% packet loss using random number generation
- Responds with PONG messages for non-dropped packets
- Handles multiple clients concurrently
- Gracefully shuts down on SIGINT (Ctrl+C)

### Client (pingcli.c)
- Sends 10 PING messages to specified server
- Implements 1-second timeout for responses
- Calculates and displays RTT for successful pings
- Generates summary statistics for the session

## Error Handling

The implementation includes robust error handling for:
- Invalid command-line arguments
- Socket creation failures
- Network errors
- Timeouts
- Memory allocation failures

## Limitations

- Fixed packet loss rate (30%)
- Fixed number of ping attempts (10)
- Fixed timeout duration (1 second)
- Basic message format

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Acknowledgments

This project was created as part of the CSC/CPE138 Computer Network Fundamentals course at Sacramento State University.

README created using generative AI.
