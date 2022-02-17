/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>

void construire_message(char * message,char motif, int lg)
{
	int i;
	for(i=0;i<lg;i++) message[i]=motif;
}

void socket_S_UDP(int port, char * HostName, int nb_message, int lg_message )
{
	int k;
	int socket_local = socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in addr_distant;
	struct hostent * hp = gethostbyname(HostName);
	char * message = calloc(lg_message,sizeof(char));
	int lg_addr_dest=sizeof(addr_distant);

	if (socket_local==-1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}
	
	memset((char *)& addr_distant,0,sizeof(addr_distant));
	addr_distant.sin_family = AF_INET;
	addr_distant.sin_port = port;
	if (hp==NULL)
	{
		printf("Erreur lors du gethostbyname\n");
		exit(1);
	}
	memcpy((char*)&(addr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);

	for (k=0;k<nb_message;k++)
	{
		construire_message(message,'a'+k%26,lg_message);
		if (sendto(socket_local,message,lg_message,0,(struct sockaddr *)&addr_distant,lg_addr_dest))
		{
			printf("Erreur lors de l'envoie de message\n");
			exit(1);
		}
	}

	close(socket_local);
}

void socket_S_TCP(int port, char * HostName, int nb_message, int lg_message)
{
	int k;
	int socket_local = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in addr_distant;
	struct hostent * hp = gethostbyname(HostName);
	char * message = calloc(lg_message,sizeof(char));
	int lg_addr_dest=sizeof(addr_distant);

	if (socket_local==-1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}

	memset((char *)& addr_distant,0,sizeof(addr_distant));
	addr_distant.sin_family = AF_INET;
	addr_distant.sin_port = port;
	if (hp==NULL)
	{
		printf("Erreur lors du gethostbyname\n");
		exit(1);
	}
	memcpy((char*)&(addr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);

	if (connect(socket_local,(struct sockaddr *)&addr_distant,lg_addr_dest)==-1)
	{
		printf("Erreur lors du connect\n");
		exit(1);
	}

	for (k=0;k<nb_message;k++)
	{
		construire_message(message,'a'+k%26,lg_message);
		if (write(socket_local,message,lg_message)==-1)
		{
			printf("Erreur lors de l'envoie de message\n");
			exit(1);
		}
	}

	close(socket_local);
}


void afficher_message(char *message, int lg) 
{
	int i;
	printf("message construit : ");
	for (i=0;i<lg;i++) printf("%c", message[i]); printf("\n");
}


void socket_P_UDP(int port, int lg_message)
{
	int socket_local = socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in addr_local;
	int lg_addr_local=sizeof(addr_local);
	char * message = calloc(lg_message,sizeof(char));
	struct sockaddr * addr_source = calloc(1,sizeof(struct sockaddr));
	int lg_effective;
	int lg_addr_source;

	if (socket_local==-1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}

	memset((char *)& addr_local,0,sizeof(addr_local));
	addr_local.sin_family = AF_INET;
	addr_local.sin_port = port;
	addr_local.sin_addr.s_addr = INADDR_ANY;

	if (bind(socket_local,(struct sockaddr *) &addr_local, lg_addr_local) == -1)
	{
		printf("Erreur lors du bind\n");
		exit(1);
	}

	while(1)
	{
		lg_effective = recvfrom(socket_local,message,lg_message,0,addr_source,&lg_addr_source);
		if (lg_effective==-1)
		{
			printf("Erreur recvfrom");
			exit(1);
		}
		afficher_message(message,lg_effective);
	}
	
	close(socket_local);
}

void socket_P_TCP(int port, int lg_message)
{
	int socket_local = socket(AF_INET,SOCK_STREAM,0);
	int socket_bis;
	struct sockaddr_in addr_local;
	int lg_addr_local=sizeof(addr_local);
	char * message = calloc(lg_message,sizeof(char));
	struct sockaddr * addr_source = calloc(1,sizeof(struct sockaddr));
	int lg_effective;
	int lg_addr_source;


	if (socket_local==-1)
	{
		printf("Erreur lors de la création du socket\n");
		exit(1);
	}

	memset((char *)& addr_local,0,sizeof(addr_local));
	addr_local.sin_family = AF_INET;
	addr_local.sin_port = port;
	addr_local.sin_addr.s_addr = INADDR_ANY;

	if (bind(socket_local,(struct sockaddr *) &addr_local, lg_addr_local) == -1)
	{
		printf("Erreur lors du bind\n");
		exit(1);
	}

	if (socket_bis = accept(socket_local,addr_source,&lg_addr_source)==-1)
	{
		printf("Erreur lors de l'accept\n");
		exit(1);
	}

	while(1)
	{
		lg_effective = read(socket_bis,message,lg_message);
		if (lg_effective == -1)
		{
			printf("Erreur lors du read\n");
			exit(1);
		}
		afficher_message(message,lg_effective);
	}

	close(socket_bis);
	close(socket_local);
}

void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int protocole = 0; /* 0=TCP, 1=UDP*/
	int lg_msg = 30; /*Taille des messages setup à 30 octets*/
	while ((c = getopt(argc, argv, "pusl:n:")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1) ;
			}
			source = 1;
			break;
		case 'n':
			nb_message = atoi(optarg);
			break;
		case 'l':
			lg_msg = atoi(optarg);
			break;
		case 'u':
			protocole =1;
			break;
		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	if (source == 1)
		printf("on est dans le source\n");
	else
		printf("on est dans le puits\n");

	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");

	}

	if (source==1 && protocole==1)
	{
		socket_S_UDP(atoi(argv[argc-1]),argv[argc-2],nb_message,lg_msg);
	}
	else if (source==0 && protocole==1)
	{
		socket_P_UDP(atoi(argv[argc-1]),lg_msg);
	}
}

