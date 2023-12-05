# mysh

Mysh is a robust Unix shell designed to offer a seamless command-line interface for Unix-like operating systems. It empowers users with intuitive interactions, enabling efficient and fluid communication with their operating systems.


### General Overview

Mysh supports the following commands as builtins:

- echo
- ls
- cd
- cat
- wc
- pipes (|)
- bg (&)
- kill
- ps
- exit
- start-server
- close-server
- send
- start-client

### Getting Started

To compile:
```
make
```

To run:

```
./mysh
```

To complie and run with one command:
```
make run
```

### Manual

  - Command: start-server \<port number>
    - Initiates a background server on the provided port number. The server must be non-blocking and must allow multiple connections at the same time.
    - Report `ERROR: No port provided` if no port is provided
  - Command: close-server
    - Terminates the current server.
  - Commmand: send \<port number> \<hostname> \<message>
    - Send a message to the hostname at the port number.
  - Command: start-client \<port number> \<hostname>
    - Starts a client that can send multiple messages. The messages are taken as standard input. The client is connected once and can continue sending messages until pressing a CTRL+D or CTRL+C.
    - Report `ERROR: No port provided` if no port provided
    - Report `ERROR: No hostname provided` if no hostname provided

  The remaining commands function similarly to the Linux built-in commands. For detailed information, refer to
  ```
  man <command>
  ```