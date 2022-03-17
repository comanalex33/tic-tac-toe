# tic-tac-toe

There is created a multiplayer tic-tac-toe game using C sockets. To the server can connect only 2 players and the rest of the players which are trying to connect to their game will be rejected.

### General description

The server have the following functionalities:
  * Handle initial connection and decide who will start
  * Keep the board in the memory
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

At this step, server will read the moves of the players, will process them and will send back specific messages.

The server will send every time a code (integer) and a message (string). The message can be an informing message or the board.

Code explained:
  * 0 - The game will continue, it's not finished
  * 1 - You won
  * 2 - You lost
  * 3 - The game ended in a draw

The client will read server's messages and will make specific actions in every case.