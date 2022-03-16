#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int readBoard(int fd)
{
   char buffer[100];
   bzero(buffer, 100);
   read(fd, buffer, 100);
   printf("%s\n", buffer);
   if (strstr(buffer, "Ai castigat") || strstr(buffer, "Ai pierdut") || strstr(buffer, "Egalitate"))
      return 0;
   return 1;
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
      printf("Eroare la citire mesaj server!\n");
      exit(1);
   }

   switch (cod)
   {
   case 0:
      printf("Conectat, se asteapta inca un jucator...\n");
      break;
   case 1:
      printf("Conectat, incepe jocul!\n");
      break;
   case 2:
      printf("Conexiune refuzata!\n");
      exit(1);
   default:
      break;
   }

   while (1)
   {
      bzero(buffer, 256);

      int position;
      printf("Linie: ");
      scanf("%d", &position);
      if (write(sockfd, &position, sizeof(int)) < 0)
      {
         perror("ERROR writing to socket");
         exit(1);
      }
      printf("Coloana: ");
      scanf("%d", &position);
      if (write(sockfd, &position, sizeof(int)) < 0)
      {
         perror("ERROR writing to socket");
         exit(1);
      }

      int final = readBoard(sockfd);
      if (final == 0)
         break;
   }

   close(sockfd);

   return 0;
}