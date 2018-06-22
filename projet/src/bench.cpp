// Ce code n'est pas à comprendre. Il sert à envoyer des données
// à votre serveur pour tester son comportement.
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <socklib.hpp>
#include <bufferedreaderwriter.hpp>

using namespace socklib;

const int MODE_LIGNE = 1;
const int MODE_BINARY = 2;

void usage(const std::string &nomprog) {
  std::cerr << "Usage "
	    << nomprog
	    << " [-h host] [-p port] [-m (BINAIRE|LIGNE)]  (<nom fichier>)"
	    << std::endl;
}


int main(int argc, char* argv[]){
  std::string host = "localhost";
  std::string port = "8080";
  int seed = time(NULL);
  
  int mode = MODE_LIGNE;

  int option;
  while ((option = getopt(argc, argv, "p:h:m:v:s:")) != -1) {
    switch (option) {
    case 'p' :
      port = optarg;
      break;
    case 'h' :
      host = optarg;
      break;
    case 's' :
      seed = atoi(optarg);
      break;
    case 'm' : {
      std::string argmode = optarg;
      if (argmode == "BINAIRE") {
	mode = MODE_BINARY; 
      } 
      else if (argmode == "LIGNE") {
	mode = MODE_LIGNE;
      }
      else {
	std::cerr << "Impossible de recconaitre le mode " << argmode
		  << std::endl;;
      }
      break;
    }
    default :
        std::ostringstream c;
        c << "!!!l'option " << optopt
          << " est inconnue ou mal utilisée !!!";
        std::cerr << c.str() << std::endl;
        usage(argv[0]);
	exit_error(c.str().c_str(), true, 0);
    }
  }

  srand(seed);
  
  // on teste si il reste un argument
  if (argc != optind+1) {
    std::cerr << "Il n'y a pas de fichier a lire " << std::endl;
    usage(argv[0]);
    exit(1);
  }

  int fdin = open(argv[optind], O_RDONLY);
  exit_error(std::string("Impossible de ouvrir le fichier ")+argv[optind],
	     fdin == -1, errno);
  
  std::cerr << "Envoie du fichier " << argv[optind]
	    << " vers " << host << ":" << port << std::endl;
  if (mode == MODE_LIGNE) {
    std::cerr << "En mode ligne par ligne" << std::endl;
  } else {
    std::cerr << "En mode binaire" << std::endl;
  }

  BufferedReaderWriter in(fdin);
  int fdout;
  try {
    fdout = CreeSocketClient(host, port);
  } catch (std::runtime_error e) {
    std::cerr << "Je n'arrive pas à me connecter, avez-vous bien lancé le serveur ?"
	 << std::endl;
    exit(1);
  }
  BufferedReaderWriter out(fdout);
  
  if (mode == MODE_LIGNE) {
    while(true) {
      std::string l = in.read_line();
      if (l.size() == 0) {
	// c'est la fin du fichier
	break;
      }
      int nbw = rand()%4;
      usleep(nbw*10000);
      out.write(l);
    }
  }
  else {
    while (true) {
      std::vector<char> data = in.read();
      if (data.size() == 0) {
	// c'est la fin du fichier
	break;
      }
      std::cerr << data.size() << " octets lus ...";
      std::cerr.flush();
      int nbw = rand()%4;
      usleep(nbw*10000);
      out.write(data);
      std::cerr << data.size() << " octets envoyé " << std::endl;
    }
  }
  
  return 0;
}
  
