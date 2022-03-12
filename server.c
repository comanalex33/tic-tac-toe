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
    }

    return 0;
}