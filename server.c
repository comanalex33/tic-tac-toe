#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

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

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, newsockfd2, portno, clilen;
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
    clilen = sizeof(cli_addr);

    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    // newsockfd2 = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        exit(1);
    }
    
    while (1)
    {
        int line, column;
        if(read(newsockfd, &line, sizeof(int)) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        if(read(newsockfd, &column, sizeof(int)) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        if(checkPosition(line, column) == 0) {
            char message[100];
            strcpy(message, "Pozitie invalida, incearca alta!");
            write(newsockfd, message, strlen(message));
            continue;
        }

        if(emptyPosition(line, column, board) == 0) {
            char message[100];
            strcpy(message, "Pozitie ocupata!");
            write(newsockfd, message, strlen(message));
            continue;
        }
    }

    return 0;
}