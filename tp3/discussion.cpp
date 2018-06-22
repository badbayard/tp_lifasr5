#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "socklib.hpp"

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;

const size_t TAILLE_BUFF = 100;

int main(int argc, char *argv[]) {
  bool serveur;
  
  string port;
  string adresse;

  
  // on teste le nombre d'arguments.
  // Attention, il y a toujours le nom du programme donc argc==
  // signifie un argument.
  if (argc == 2) {
    serveur = true;
    port = argv[1];

    cerr << "Je suis serveur sur le port " << port << endl; 
  }
  else if (argc == 3) {
    serveur = false;
    adresse = argv[1];
    port = argv[2];
    
    cerr << "Je suis client je contacte " << adresse << ":" << port << endl; 
  }
  else {
    cerr << "usage " << argv[0] << " <port> : pour le serveur" << endl
	 << "      " << argv[0] << " <serveur> <port> : pour le clinet" << endl;
    exit(1);
  }

  if (serveur) {
    // #########################
    // code du serveur
    // #########################
    
    // création de la socket d'écoute
    int s_serv = socklib::CreeSocketServeur(port);

    // attente d'un nouveau client
    int s = socklib::AcceptConnexion(s_serv);

    // la lecture que je fait là n'est pas très propre,
    // il faudra l'améliorer.

    while (true) {
      vector<char> mess(TAILLE_BUFF, 0);
      
      int res = recv(s, mess.data(), TAILLE_BUFF, 0);
      exit_error("serveur : lecture sur la socket", res == -1, errno);

      if (res == 0) {
	cout << "fermeture de la connexion"  << endl;
	break;
      }
      
      cout << mess.data() << endl;
    }
    
    close (s_serv);
    close (s);
  } else {
    // #########################
    // Code du client
    // #########################
    int s = socklib::CreeSocketClient(adresse, port);

    while (true) {
      string mess;
      // j'utilise getline pour lire plus d'un mot (>> s'arrete
      // au premier espace)
      getline(cin, mess);

      if (mess == "fin") {
	cout << "fermeture de la connexion"  << endl;
	break;
      }

      int res = send(s, mess.data(), mess.size(), 0);
      exit_error("client : envoie sur la socket", res == -1, errno);
    }

    close(s);
  }
  
  return 0;
}
