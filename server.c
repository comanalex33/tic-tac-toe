#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

    if (write(*firstPlayer, &code, sizeof(int)) < 0)
    {
        perror("ERROR writing to first player!\n");
        exit(2);
    }
    code = 1 - code;
    if (write(*secondPlayer, &code, sizeof(int)) < 0)
    {
        perror("ERROR writing to second player!\n");
        exit(2);
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

int emptyPosition(int line, int column, char board[][4]) {
    if(board[line - 1][column - 1] == ' ')
        return 1;
    return 0;
}

int checkWinner(char board[][4]) {
    for(int i = 0; i < 3; i++) {
        if(board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][1] != ' ') {
            if(board[i][0] == 'o')
                return 1;
            else
                return 2;
        }
        if(board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[1][i] != ' ') {
           if(board[0][i] == 'o')
                return 1;
            else
                return 2;
        }
    }
    if((board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[1][1] != ' ') ||
       (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[1][1] != ' ')) {
       if(board[1][1] == 'o')
                return 1;
            else
                return 2;
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

void printBoard(int fd, char board[][4], char message[])
{
    char line[100];
    bzero(line, 100);
    sprintf(line, "| %c | %c | %c |\n| %c | %c | %c |\n| %c | %c | %c |\n%s", 
        board[0][0], board[0][1], board[0][2],
        board[1][0], board[1][1], board[1][2],
        board[2][0], board[2][1], board[2][2],
        message);
    write(fd, line, strlen(line));
}

void applyMove(int line, int column, char board[][4], char symbol) {
    board[line - 1][column - 1] = symbol;
}

int main(int argc, char *argv[])
{
    int firstPlayerFd, secondPlayerFd, newsockfd, newsockfd2, portno;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, pid;
    char board[4][4];

    if (argc < 2)
    {
        perror("ERROR: Port not provided");
        exit(1);
    }

    initBoard(board);

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection
     */

    listen(sockfd, 0);

    /* Accept actual connection from the clients */
    printf("Waiting for players...\n");
    firstPlayerFd = connectPlayer(0);
    secondPlayerFd = connectPlayer(1);

    // Reject the rest of connections
    rejectConnections();

    // Select order
    computeOrder(&firstPlayerFd, &secondPlayerFd);
    
    while (1)
    {
        int line, column;
        if(read(firstPlayerFd, &line, sizeof(int)) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        if(read(firstPlayerFd, &column, sizeof(int)) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        if(checkPosition(line, column) == 0) {
            char message[100];
            strcpy(message, "Pozitie invalida, incearca alta!");
            write(firstPlayerFd, message, strlen(message));
            continue;
        }

        if(emptyPosition(line, column, board) == 0) {
            char message[100];
            strcpy(message, "Pozitie ocupata!");
            write(firstPlayerFd, message, strlen(message));
            continue;
        }

        applyMove(line, column, board, 'x');

        printBoard(1, board, "");
        printf("\n");

        if(checkWinner(board) == 2) {
            printf("Ai pierdut!\n");
            printBoard(firstPlayerFd, board, "Ai castigat!");
            break;
        }

        if((checkWinner(board) == 0) && (checkBoardFull(board) == 1)){
            printf("Egalitate!\n");
            printBoard(firstPlayerFd, board, "Egalitate!");
            break;
        }

        while(1) {
            printf("Linie: ");scanf("%d", &line);
            printf("Coloana: ");scanf("%d", &column);
            if(checkPosition(line, column) == 0) {
                printf("Pozitie invalida, incearca alta!\n");
            } else if(emptyPosition(line, column, board) == 0) {
                printf("Pozitie ocupata!\n");
            } else
                break;
        }

        applyMove(line, column, board, 'o');

        if(checkWinner(board) == 1) {
            printBoard(1, board, "Ai castigat!\n");
            printBoard(firstPlayerFd, board, "Ai pierdut!");
            break;
        }

        printBoard(firstPlayerFd, board, "");
    }

    close(firstPlayerFd);
    close(sockfd);

    return 0;
}