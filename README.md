# tic-tac-toe

This is a multiplayer tic-tac-toe game using C sockets. Only 2 players can connect to the server, the rest will be rejected is they are trying to connect to the game.

### How to play

Compile code:
  * server: `gcc -o server server.c`
  * client: `gcc -o client client.c`

Start game:
  * server
    * `./server port_number`
    * Example: `./server 8000`
  * client
    * `./client server_hostname/server_ip server_port`
    * If you are trying this locally, server ip is 127.0.0.1 (localhost)
    * Example: `./client 127.0.0.1 8000`

### General description

The server has the following functionalities:
  * Handle initial connection and decide who will start
  * Store the board
  * Handle player moves
  * Send the board and other messages to the clients

The client will only read the messages from the server, will execute specific actions and will send his coordinates.

### Message transfer

The server will start and will be waiting for players.

##### First step - connection

Server's messages:
  * 0 - Connected, waiting for one more player - this will be send to the first player
  * 1 - Connected, the game will start - this will be send to the second player
  * 2 - Connection refused - this will be send to all of the players that are trying to connect to their game

The client will read these messages and will wait for the next step.

##### Second step - order of the players

Server's messages:
  * 0 - You will start the game - this will be send to the starting player
  * 1 - The other player will start - will be send to the other player

The client will read these messages and then the game starts

##### Final step - game

In this step, the server will read the moves of the players, process them and send back specific messages.

The server will always send a code (integer) and a message (string). The message can be an information or the board.

Code explained:
  * 0 - The game will continue, it's not finished
  * 1 - You won
  * 2 - You lost
  * 3 - The game ended in a draw
  * 4 - The requestet position is invalid/ occupied

The client will read the server's messages and take specific actions in every case.