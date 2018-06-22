#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define PORT "8080"


int
main (int argc, char *argv[])
{
  // structure pour faire la demande de port
  struct addrinfo hints;
  // structure pour stocker et lire les résultats
  struct addrinfo *result, *rp;
  // socket d'attente (s) et de discution (t)
  int t, s=-1;
  // structures pour stocker les info concernant le client
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len = sizeof(peer_addr);
  // variables pour tester si les fonctions donnent un résultats ou une erreur
  int res;
  int bon;
  // Des variable pour contenir de adresse de machine et des numero de port afin de les afficher
  char hname[NI_MAXHOST], sname[NI_MAXSERV];

  

  // on rempli la structure hints de demande d'adresse
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* IPv4 ou IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* socket flux connectée */
  hints.ai_flags = AI_PASSIVE;    /* Les signifie que toutes les addresse de la machine seront utilisée */
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_addrlen = 0; 
  hints.ai_addr = NULL;           
  hints.ai_canonname = NULL;
  hints.ai_next = NULL;

  // on effectue la demande pour le port PORT défini par "8888"
  res = getaddrinfo(NULL, PORT, &hints, &result);
  if (res != 0) { // c'est une erreur
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
    exit(1);
  }
  
  // si res = 0 le véritable résultat de la fontion est l'argument result
  // qui contient une liste d'addresses correspondant à la demande on va les
  // rester jusqu'à en trouver une qui convient
  rp = result;
  bon = 0;
  while (rp != NULL) {
    // on parcourt la liste pour en trouver une qui convienne
    int yes = 1;

    s = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
    // si le résultat est -1 cela n'a pas fonctionné on recommence avec la prochaine
    if (s == -1) {
      perror("Création de la socket");
      rp = rp->ai_next;
      continue;
    }    
    
    // partie optionnelle pour éviter d'être rejeté par le système si le précédant test a planté
    res = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes,
		     sizeof(int));
    if (res == -1) {
            perror("setsockopt");
	    rp = rp->ai_next;
            continue;
    }
    // fin de la partie optionnelle

    // si la socket a été obtenue, on essaye de réserver le port
    res = bind(s, rp->ai_addr, rp->ai_addrlen);
    if (res == 0 ) {// cela a fonctionné on affiche l'information
      bon = 1;

      // on récupère des informations affichables
      res = getnameinfo(rp->ai_addr, rp->ai_addrlen,
			hname, NI_MAXHOST,
			sname, NI_MAXSERV,
			NI_NUMERICSERV|NI_NUMERICHOST);
      if (res != 0) {
	fprintf(stderr, "getnameinfo: %s\n", gai_strerror(res));
	exit (1);
      }
      printf ("La socket %d est maintenant en attente sur l'adresse %s le port %s\n",
	      s, hname, sname); 
      break; 
    }
    else { // sinon le bind a été impossible, il faut fermer la socket
      perror("Imposible de réserver l'adresse");
      close (s);
    }

    rp = rp->ai_next;
  }

  if (bon == 0) { // Cela n'a jamais fonctionné
    fprintf(stderr, "Impossible de faire un bind\n");
    exit(1);
  }

  
  // on libère la structure devenue inutile
  freeaddrinfo(result);

  // ######################################################
  // (2.2) Attente de connexion 
  // ######################################################
  res = listen (s, 5);
  if (res < 0) {
    perror("listen");
    close(s);
    exit(1);
  }

  // ######################################################
  // (4) Acceptation d'une connexion
  // Le serveur accepte l'une des demandes arrivées depuis le listen ou
  // attend s'il n'y en a pas
  // ######################################################

  t = accept (s, (struct sockaddr *)&peer_addr, &peer_addr_len);
  // s : la socket d'attente
  // peer_addr : la structure où on va stocker les infos
  //             sur le client
  // peer_addr_len : donnée = la taille de tadr (pour
  //                éviter le dépassement)
  // t : La socket qui servira pour la 
  //     discution

  if (t == -1) { // il y a eu une erreur
    perror("accept");
    close (s);
    exit(1);
  }
  res = getnameinfo((struct sockaddr*)&peer_addr, peer_addr_len,
		    hname, NI_MAXHOST,
		    sname, NI_MAXSERV,
		    NI_NUMERICSERV);
  if (res != 0) {
    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(res));
    exit (1);
  }
  printf ("La socket %d a eu un client depuis %s sur le port %s\n",
	  s, hname, sname);
  
  // ######################################################
  // (5) Dialogue
  // Une fois la connexion établie, le serveur et le client peuvent s'échanger des messages
  // envoyés par write() ou send() et lus par read()ou recv() selon un protocole établi.
  // Ici nous utiliseront le protocole suivant : seul le client écrit, le serveur lit octet
  // par octet
  // ######################################################  
  while(1) {
    // le caractère dans lequel on va stocke ce sui est envoyé par le client
    char buff;
    // le nombre de caractère lu
    size_t nbo;
    
    nbo = recv(t, &buff, 1, 0);
    if (nbo == -1) {
      perror("erreur à la réception");
    }
    if (nbo == 0) {
      // C'est fini
      break;
    }
    printf("recu : %c\n", buff);
    
  }
  
  // ######################################################
  // (6) Fermeture de la connexion
  // ######################################################
  if (close(t)< 0) {
    perror("Problème à la fermeture de la socket de discution");
  }
  if (close(s)< 0) {
    perror("Problème à la fermeture de la socket d'attente");
  }

  return 0;
}
