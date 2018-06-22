#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

// #define PORT "8080"
// webcache est le port 8080 (cf /etc/services)
#define PORT "webcache"  //on peut remplacer par 8080
#define HOST "localhost"
#define TAILLE_BUFF 100


/* test d'abord l'adresse en IPV6 si elle échoue elle test les adresses IPV4 */

int
main (int argc, char *argv[])
{
  // structure pour faire la demande
  struct addrinfo hints;
  // structure pour stocker et lire les résultats
  struct addrinfo *result, *rp;
  // socket  (s)
  int s=-1;
  // variables pour tester si les fonctions donnent un résultats ou une erreur
  int res;
  int bon;
  // Des variable pour contenir de adresse de machine et des numero de port afin de les afficher
  char hname[NI_MAXHOST], sname[NI_MAXSERV];
  // Des variable pour stocker des info sur le client lui même.
  struct sockaddr_storage my_addr;
  socklen_t my_addr_len = sizeof(my_addr);


  // on rempli la structure hints de demande d'adresse
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* IPv4 ou IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* socket flux connectée */
  hints.ai_flags = 0;  
  hints.ai_protocol = 0;          /* Any protocol */
  hints.ai_addrlen = 0; 
  hints.ai_addr = NULL;           
  hints.ai_canonname = NULL;
  hints.ai_next = NULL;
 
  res = getaddrinfo(HOST, PORT, &hints, &result);
  if (res != 0) { // c'est une erreur
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
    exit(1);
  }
  
  // si res = 0 le véritable résultat de la fontion est l'argument result
  // qui contient une liste d'addresse correspondant à la demande on va les
  // rester jusqu'a trouver une qui convient
  rp = result;
  bon = 0;
  while (rp != NULL) {
    // on parcourt la liste pour en trouver une qui convienne

    // on récupère des informations affichables
    res = getnameinfo(rp->ai_addr, rp->ai_addrlen,
		      hname, NI_MAXHOST,
		      sname, NI_MAXSERV,
		      NI_NUMERICSERV|NI_NUMERICHOST);
    if (res != 0) {
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(res));
      exit (1);
    }
    fprintf (stderr, "On tente l'adresse %s sur le port %s .....",
	    hname, sname);
    
    // on essaye
    s = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
    // si le résultat est -1 cela n'a pas fonctionné on recommence avec la prochaine
    if (s == -1) {
      perror("Création de la socket");
      rp = rp->ai_next;
      continue;
    }    
   
    // si la socket a été obtenue, on essaye de se connecter
    res = connect(s, rp->ai_addr, rp->ai_addrlen);
    if (res == 0 ) {// cela a fonctionné on est connecté
      bon = 1;
      fprintf (stderr, "OK\n");
      break; 
    }
    else { // sinon le bind a été impossible, il faut fermer la socket
      perror("Imposible de se connecter");
      close (s);
    }

    rp = rp->ai_next;
  }

  if (bon == 0) { // Cela n'a jamais fonctionné
    fprintf(stderr, "Aucune connexion possible\n");
    exit(1);
  }

  // affichage des infos sur la connexion
  // on affiche des info sur le serveur
  res = getnameinfo(rp->ai_addr, rp->ai_addrlen,
		    hname, NI_MAXHOST,
		    sname, NI_MAXSERV,
		    NI_NUMERICSERV|NI_NUMERICHOST);
  if (res != 0) {
    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(res));
    exit (1);
  }
  printf ("Le serveur contacté est %s sur le port %s\n", hname, sname); 
  // on affiche des info sur le client
  res = getsockname(s, (struct sockaddr *) &my_addr, &my_addr_len);
  if (res != 0) {
    fprintf(stderr, "getnameinfo: %s\n", gai_strerror(res));
    exit (1);
  }
  res = getnameinfo((struct sockaddr *) &my_addr, my_addr_len,
		    hname, NI_MAXHOST,
		    sname, NI_MAXSERV,
		    NI_NUMERICSERV|NI_NUMERICHOST);
  if (res != 0) {
    fprintf(stderr, "getnameinfo: %s\n", strerror(errno));
    exit (1);
  }
  printf ("Je suis %s et j'utilise le port le port %s\n", hname, sname); 
  

  freeaddrinfo(result);           /* No longer needed */

  
  
  // ######################################################
  // (5) Dialogue
  // Une fois la connexion établie, le serveur et le client peuvent s'échanger des messages
  // envoyés par write() ou send() et lus par read()ou recv() selon un protocole établi.
  // Ici nous utiliseront le protocole suivant : seul le client écrit, le serveur lit octet
  // par octet
  // ######################################################
  while (1) {
    char buff[TAILLE_BUFF];
    fgets(buff, TAILLE_BUFF, stdin);
    // Le dernier carractère est un retour chariot
    buff[strlen(buff)-1] = '\0';

    if (strcmp(buff, "fin")==0) {
	// Quand l'utilisateur tape fin on sort
	break;
      }
    res = send(s, (char*)buff, (strlen(buff)+1)*sizeof(char), MSG_NOSIGNAL);
    // s le socket sur laquel on écrit
    // buff le message écrit
    // (strlen(buff)+1)*sizeof(char) la longueur du message
    // MSG_NOSIGNAL permet de choisir la façon dont le système réagira si la conexion est brisée
    // lors du send. Dans ce cas, par defaut, le send génère un signal, il faudrait donc prévoir le cas
    // et le traiter avec un sigaction. Or je n'ai pas envie de le faire.
    // Avec l'option MSG_NOSIGNAL, la fonction send s'arrèterra sur une erreur ce qui est plus
    // simple à traiter.
    if (res < 0) {
      perror("erreur dans le send");
      break;
    }
  }
  
  
  // ######################################################
  // (6) Fermeture de la connexion
  // ######################################################
  if (close(s)< 0) {
    perror("Problème à la fermeture de la socket");
  }

  fprintf(stdout, "bye\n");
  return 0;
}

