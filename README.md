# Multi-Client Server in C

A robust multi-threaded client-server application written in C that demonstrates UNIX domain sockets, thread management, and concurrent client handling. The server can handle multiple simultaneous client connections using POSIX threads (pthreads), with each client running in its own dedicated thread.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Compilation](#compilation)
- [Usage](#usage)
  - [Starting the Server](#starting-the-server)
  - [Running Clients](#running-clients)
  - [Shutting Down](#shutting-down)
- [Technical Details](#technical-details)
  - [UNIX Domain Sockets](#unix-domain-sockets)
  - [Thread Management](#thread-management)
  - [Signal Handling](#signal-handling)
- [Example Session](#example-session)
- [Troubleshooting](#troubleshooting)
- [Code Structure](#code-structure)
- [Author](#author)

## Overview

This project implements a concurrent server-client system using UNIX domain sockets (SOCK_SEQPACKET). The server accepts multiple client connections simultaneously, with each client handled by a separate thread. Clients can send messages to the server, which acknowledges receipt and supports graceful shutdown via a special "Shutdown" command.

## Features

- **Multi-threaded Architecture**: Each client connection is handled by a dedicated thread
- **Concurrent Client Support**: Multiple clients can connect and communicate simultaneously
- **Thread-Safe Operations**: Mutex locks protect shared client data structures
- **Graceful Shutdown**: Special "Shutdown" command for controlled server termination
- **Signal Handling**: Robust signal handling for clean resource cleanup
- **UNIX Domain Sockets**: Uses SOCK_SEQPACKET for reliable, connection-oriented communication
- **Automatic Client Tracking**: Server maintains a list of all connected clients
- **Connection Status**: Server provides feedback on client connections and disconnections

## Architecture

The system consists of two main components:

1. **Server (`server.c`)**: 
   - Listens for incoming client connections on a UNIX domain socket
   - Creates a new thread for each connected client
   - Maintains a thread-safe array of active client connections
   - Sends acknowledgment messages back to the client that sent the message
   - Handles graceful shutdown on receiving "Shutdown" command

2. **Client (`client.c`)**:
   - Connects to the server via UNIX domain socket
   - Provides an interactive command-line interface for sending messages
   - Receives and displays server responses
   - Supports graceful disconnection

## Prerequisites

- **Operating System**: Linux or UNIX-like system (uses UNIX domain sockets)
- **Compiler**: GCC (GNU Compiler Collection) or compatible C compiler
- **Libraries**: 
  - POSIX threads library (`pthread`)
  - Standard C libraries

## Compilation

Compile both the server and client programs using GCC:

```bash
# Compile the server (requires pthread library)
gcc -o server server.c -lpthread

# Compile the client
gcc -o client client.c
```

Alternatively, compile both in one command:

```bash
gcc -o server server.c -lpthread && gcc -o client client.c
```

## Usage

### Starting the Server

1. Open a terminal and navigate to the project directory
2. Start the server:

```bash
./server
```

You should see:
```
Server is listening for connections.
```

The server will continue running and accepting connections until explicitly shut down.

### Running Clients

1. Open one or more new terminal windows
2. In each terminal, run the client:

```bash
./client
```

3. When prompted, enter messages to send to the server:

```
Enter message to send: Hello, Server!
Server: Message received

Enter message to send: Testing connection
Server: Message received
```

You can run multiple clients simultaneously to test concurrent handling.

### Shutting Down

To gracefully shut down the system:

1. In any connected client, type the special command:
```
Enter message to send: Shutdown
```

2. This will:
   - Shut down the client that sent the command
   - Terminate the server
   - Clean up all resources and socket files
   - Disconnect all other connected clients

Alternatively, you can terminate the server using signals:
- `Ctrl+C` (SIGINT)
- `kill` command with SIGTERM, SIGQUIT, or SIGABRT

## Technical Details

### UNIX Domain Sockets

- **Socket Type**: `AF_UNIX` with `SOCK_SEQPACKET`
- **Socket Path**: `/tmp/socket2`
- **Communication**: Bidirectional, connection-oriented, reliable packet delivery
- **Max Message Length**: 128 bytes (defined by `MAX_LENGTH`)

### Thread Management

- **Threading Model**: One thread per client connection
- **Thread Library**: POSIX threads (pthreads)
- **Thread Lifecycle**: Threads are detached after creation for automatic resource cleanup
- **Synchronization**: Mutex locks (`pthread_mutex_t`) protect the shared client array
- **Max Clients**: 128 (defined by `MAX_LENGTH` array size)

### Signal Handling

The server handles the following signals gracefully:
- `SIGTERM`: Termination signal
- `SIGINT`: Interrupt signal (Ctrl+C)
- `SIGQUIT`: Quit signal
- `SIGABRT`: Abort signal
- `SIGPIPE`: Broken pipe signal

All signals trigger the `cleanup()` function, which:
- Removes the socket file from `/tmp`
- Terminates the server process
- Ensures clean shutdown

## Example Session

**Terminal 1 (Server):**
```bash
$ ./server
Server is listening for connections.
Message from the client: Hello from Client 1
Message from the client: Testing concurrent connections
Client disconnected
Shutting down the server.
Quitting and cleaning up
```

**Terminal 2 (Client 1):**
```bash
$ ./client
Enter message to send: Hello from Client 1
Server: Message received

Enter message to send: Testing concurrent connections
Server: Message received

Enter message to send: Shutdown
Shutting down the client.
```

**Terminal 3 (Client 2):**
```bash
$ ./client
Enter message to send: Message from Client 2
Server: Message received

Enter message to send: Another test
Server: Message received
```

## Troubleshooting

### "Error: Failed to bind socket: Address already in use"

**Problem**: The socket file `/tmp/socket2` already exists from a previous server instance.

**Solution**:
```bash
rm /tmp/socket2
./server
```

### "Error: Unable to connect to the server"

**Problem**: The server is not running or the socket file doesn't exist.

**Solution**: 
- Ensure the server is running in another terminal
- Check that the server started successfully without errors

### Server doesn't shut down with Ctrl+C

**Problem**: Signal handling may be blocked or the server is in an uninterruptible state.

**Solution**:
```bash
# Force kill the server process
pkill -9 server

# Clean up the socket file
rm /tmp/socket2
```

### Compilation errors with pthread

**Problem**: pthread library not linked properly.

**Solution**: Ensure you include the `-lpthread` flag when compiling the server:
```bash
gcc -o server server.c -lpthread
```

## Code Structure

### server.c

**Key Components**:
- `main()`: Initializes the server, sets up signal handling, creates and binds the socket, accepts client connections
- `clientHandler()`: Thread function that handles communication with individual clients
- `cleanup()`: Signal handler for graceful shutdown and resource cleanup

**Global Variables**:
- `clients[]`: Array storing file descriptors of connected clients
- `num_clients`: Counter for active client connections
- `clients_mutex`: Mutex for thread-safe access to client array

### client.c

**Key Components**:
- `main()`: Connects to the server, handles user input, sends/receives messages

**Workflow**:
1. Create socket
2. Connect to server
3. Enter interactive loop:
   - Prompt user for input
   - Send message to server
   - Receive and display server response
   - Check for "Shutdown" command
4. Clean up and exit

## Author

**John Wright**

This project demonstrates practical application of:
- Network programming with UNIX domain sockets
- Multi-threaded programming with POSIX threads
- Synchronization primitives (mutexes)
- Signal handling in C
- Client-server architecture