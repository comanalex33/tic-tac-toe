#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

// Read the message from the server
int readMessage(int fd)
{
   int code;
   if(read(fd, &code, sizeof(int)) < 0) {
      perror("Error reading from server\n");
      exit(1);
   }

   int size;
   if(read(fd, &size, sizeof(int)) < 0) {
      perror("Error reading from server\n");
      exit(1);
   }

   char buffer[100];
   bzero(buffer, 100);
   if(read(fd, buffer, size) < 0) {
      perror("Error reading from server\n");
      exit(1);
   }
   
   printf("\n%s", buffer);
   switch (code)
   {
   case 1:
      printf("You won!\n");
      break;
   case 2:
      printf("You lost!\n");
      break;
   case 3:
      printf("Draw\n");
      break;
   default:
      break;
   }
   if (code && code != 4)
      return 0;
   return 1;
}

// Read a number from stdin. Read until a number is entered
int readNumber(char message[]) {
   int number;
   char line[20];
   printf("%s", message);
   while(fgets(line, 20, stdin) != 0)
      if(sscanf(line, "%d", &number) != 0)
         return number;
      else {
         printf("It's not a number, keep tryng!\n");
         printf("%s", message);
      }
}

// Write position to server
void writePosition(int sockfd) {
   int position;
   position = readNumber("Line: ");
   if (write(sockfd, &position, sizeof(int)) < 0)
   {
      perror("ERROR writing to socket");
      exit(1);
   }
   position = readNumber("Column: ");
   if (write(sockfd, &position, sizeof(int)) < 0)
   {
      perror("ERROR writing to socket");
      exit(1);
   }
}

int main(int argc, char *argv[])
{
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   char buffer[256];

   if (argc < 3)
   {
      fprintf(stderr, "usage %s hostname port\n", argv[0]);
      exit(0);
   }

   portno = atoi(argv[2]);

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0)
   {
      perror("ERROR opening socket");
      exit(1);
   }

   server = gethostbyname(argv[1]);

   if (server == NULL)
   {
      fprintf(stderr, "ERROR, no such host\n");
      exit(0);
   }

   bzero((char *)&serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
   {
      perror("ERROR connecting");
      exit(1);
   }

   // Get connection status
   int cod;
   if (read(sockfd, &cod, sizeof(int)) < 0)
   {
      printf("Error reading from server!\n");
      exit(1);
   }
   switch (cod)
   {
   case 0:
      printf("Connected, waiting for one more player...\n");
      break;
   case 1:
      printf("Connected, the game will start!\n");
      break;
   case 2:
      printf("Connection refused!\n");
      exit(1);
   default:
      break;
   }

   // Read order
   if (read(sockfd, &cod, sizeof(int)) < 0)
   {
      printf("Error reading from server!\n");
      exit(1);
   }
   switch (cod)
   {
   case 0:
      printf("You start, your symbol is 'x'\n");
      break;
   case 1:
      printf("The other player starts, your symbol is 'o'\n");
   default:
      break;
   }

   //Start play
   while (1)
   {
      bzero(buffer, 256);
      if (cod == 0)
      {
         writePosition(sockfd);

         if(readMessage(sockfd) == 0)
            break;
      } else {
         if(readMessage(sockfd) == 0)
            break;

         writePosition(sockfd);
      }
   }

   close(sockfd);

   return 0;
}