#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int sockfd;

/* 
    Connect player function
    - code : a specifier used to identify the state of the player
      0 - first player
      1 - second player
      2 - rejected player, already 2 players connected 
*/
int connectPlayer(int code)
{
    int cli_addr, clilen;
    clilen = sizeof(cli_addr);

    int playerFd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (playerFd < 0)
    {
        perror("ERROR on accept");
        exit(1);
    }
    else
    {
        printf("Player connected!\n");
        if(write(playerFd, &code, sizeof(int)) < 0) {
            perror("ERROR writing to client");
            exit(2);
        }
    }

    return playerFd;
}

/*  
    Reject player connections. 
    This creates a process that runs parallel to the rest of the code 
    This process will accept connections and close them immediately
*/
void rejectConnections()
{
    int playerFd, cli_addr, clilen;
    clilen = sizeof(cli_addr);
    int parallelProcess = fork();
    if (parallelProcess == 0)
    {
        while (playerFd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen))
        {
            int cod = 2;
            if(write(playerFd, &cod, sizeof(int)) < 0) {
                perror("ERROR writing to client");
                exit(2);
            }
        }
    }
}

/*
    Establish the order of the players. The order will be determined randomly
*/
void computeOrder(int *firstPlayer, int *secondPlayer)
{
    srand(time(NULL));
    int code = rand() % 2;

    int statusWrite = write(*firstPlayer, &code, sizeof(int));
    if (statusWrite <= 0)
    {
        if(statusWrite < 0) {
            perror("ERROR writing to first player!\n");
            exit(2);
        } else {
            printf("One of the players left the game\n");
            shutdown(*secondPlayer, SHUT_RDWR);
            close(*secondPlayer);
        }
    }
    code = 1 - code;
    statusWrite = write(*secondPlayer, &code, sizeof(int));
    if (statusWrite <= 0)
    {
        if(statusWrite < 0) {
            perror("ERROR writing to second player!\n");
            exit(2);
        } else {
            printf("One of the players left the game\n");
            shutdown(*firstPlayer, SHUT_RDWR);
            close(*firstPlayer);
        }
    }
    if (code == 0)
    {
        int aux = *firstPlayer;
        *firstPlayer = *secondPlayer;
        *secondPlayer = aux;
    }
}

void initBoard(char board[][4])
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = ' ';
}

int checkPosition(int line, int column) {
    if(line < 1 || line > 3)
        return 0;
    if(column < 1 || column > 3)
        return 0;
    return 1;
}

char checkWinner(char board[][4])
{
    for (int i = 0; i < 3; i++)
    {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][1] != ' ')
        {
            return board[i][0];
        }
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[1][i] != ' ')
        {
            return board[0][i];
        }
    }
    if ((board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[1][1] != ' ') ||
        (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[1][1] != ' '))
    {
        return board[1][1];
    }
    return 0;
}

int checkBoardFull(char board[][4]){
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(board[i][j] == ' ') return 0;
        }
    }

    return 1;
}

/*
    Function used to send the board to the client
    At the beggining will send a code
      0 - the game is not finished
      1 - this player is the winner
      2 - this player is the losser
      3 - equality
      4 - invalid/ occupied position
*/
void sendBoard(int currentPlayer, int otherPlayer, char board[][4], int code)
{
    int statusWrite = write(currentPlayer, &code, sizeof(int));
    if(statusWrite <= 0) {
        if(statusWrite < 0) {
            perror("ERROR writing to client");
            exit(2);
        } else {
            printf("One of the players left the game\n");
            shutdown(otherPlayer, SHUT_RDWR);
            close(otherPlayer);
        }
    }

    char line[100];
    bzero(line, 100);
    sprintf(line, "| %c | %c | %c |\n| %c | %c | %c |\n| %c | %c | %c |\n",
            board[0][0], board[0][1], board[0][2],
            board[1][0], board[1][1], board[1][2],
            board[2][0], board[2][1], board[2][2]);
    statusWrite = write(currentPlayer, line, strlen(line));
    if(statusWrite <= 0) {
        if(statusWrite < 0) {
            perror("ERROR writing to client");
            exit(2);
        } else {
            printf("One of the players left the game\n");
            shutdown(otherPlayer, SHUT_RDWR);
            close(otherPlayer);
        }
    }
}

void writeError(int currentPlayer, int otherPlayer, char message[]) {
    int code = 4;
    int statusWrite = write(currentPlayer, &code, sizeof(int));
    if(statusWrite <= 0) {
        if(statusWrite < 0) {
            perror("ERROR writing to client");
            exit(2);
        } else {
            printf("One of the players left the game\n");
            shutdown(otherPlayer, SHUT_RDWR);
            close(otherPlayer);
            exit(1);
        }
    }
    statusWrite = write(currentPlayer, message, strlen(message));
    if(statusWrite <= 0) {
        if(statusWrite < 0) {
            perror("ERROR writing to client");
            exit(2);
        } else {
            printf("One of the players left the game\n");
            shutdown(otherPlayer, SHUT_RDWR);
            close(otherPlayer);
            exit(1);
        }
    }
}

/*
    This is used to make current player's move
    Steps:
     - Read symbol from the player
     - Check if this position is valid
     - Apply move
*/
void makeMove(int currentPlayer,  int otherPlayer, char board[][4], char symbol) {
    int line, column;

    while (1)
    {
        int statusRead = read(currentPlayer, &line, sizeof(int));
        if (statusRead <= 0)
        {
            if(statusRead < 0) {
                perror("ERROR reading from socket");
                exit(1);
            } else {
                printf("One of the players left the game\n");
                shutdown(otherPlayer, SHUT_RDWR);
                close(otherPlayer);
                exit(1);
            }
        }
        statusRead = read(currentPlayer, &column, sizeof(int));
        if (statusRead <= 0)
        {
            if(statusRead < 0) {
                perror("ERROR reading from socket");
                exit(1);
            } else {
                printf("One of the players left the game\n");
                shutdown(otherPlayer, SHUT_RDWR);
                close(otherPlayer);
                exit(1);
            }
        }

        if (checkPosition(line, column) == 0)
        {
            writeError(currentPlayer, otherPlayer, "Invalid position, try another!\n");
            continue;
        }
        if (board[line - 1][column - 1] != ' ')
        {
            writeError(currentPlayer, otherPlayer, "Occupied position!\n");
            continue;
        }
        break;
    }

    board[line - 1][column - 1] = symbol;
}

/*
    This is used to handle current player's move
    Steps:
     - Apply move using previous function 
     - Check if this is the final of the game
     - Send board to the other player
*/
void handlePlayerMove(int currentPlayer, int otherPlayer, char board[][4], char symbol)
{
    makeMove(currentPlayer, otherPlayer, board, symbol);

    if (checkWinner(board) == symbol)
    {
        sendBoard(otherPlayer, currentPlayer, board, 2);
        sendBoard(currentPlayer, otherPlayer, board, 1);
        close(currentPlayer);
        close(otherPlayer);
        close(sockfd);
        exit(0);
    }
    else if(checkWinner(board) == 0 && checkBoardFull(board) == 1) {
        sendBoard(otherPlayer, currentPlayer, board, 3);
        sendBoard(currentPlayer, otherPlayer, board, 3);
        close(currentPlayer);
        close(otherPlayer);
        close(sockfd);
        exit(0);
    } 
    else
        sendBoard(otherPlayer, currentPlayer, board, 0);
}

int main(int argc, char *argv[])
{
    int firstPlayerFd, secondPlayerFd, portno;
    struct sockaddr_in serv_addr;
    char board[4][4];

    if (argc < 2)
    {
        perror("ERROR: Port not provided");
        exit(1);
    }

    initBoard(board);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    // Initialize socket structure
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Bind the host address
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    // Start listening for players
    listen(sockfd, 0);

    // Connect players
    printf("Waiting for players...\n");
    firstPlayerFd = connectPlayer(0);
    secondPlayerFd = connectPlayer(1);

    // Reject the rest of connections
    rejectConnections();

    // Select order
    computeOrder(&firstPlayerFd, &secondPlayerFd);

    while (1)
    {
        handlePlayerMove(firstPlayerFd, secondPlayerFd, board, 'x');    // Handle first player move
        handlePlayerMove(secondPlayerFd, firstPlayerFd, board, 'o');    // Handle second player move
    }

    return 0;
}