// g++ -g -Wall -std=c++11 shell_skel.cpp -o monshell.exx

#if (!defined(__cplusplus) || (__cplusplus != 201103L))
#error Il faut utiliser c++11 apprenez à configurer votre IDE favori ou a lire les informations en début de fichier
#endif

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <regex>
#include <signal.h>
#include <sys/wait.h>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>

using namespace std;


/**
   @brief fonction pour afficher les erreurs
*/
void
__test_error_errno(const char *file, const int ligne, const char *msg, int condition, int sortie) {
  if (condition) {
    fprintf(stderr, "ERREUR fichier %s ligne %d : %s car %s\n", file, ligne, msg, strerror(errno));
    if (sortie) {
      exit(errno);
    }
  }
}
/**
 @brief macro pour les erreur
*/
#define test_error_exit(msg, condition) __test_error_errno(__FILE__, __LINE__, (msg), condition, 1)
#define test_error_warning(msg, condition) __test_error_errno(__FILE__, __LINE__, (msg), condition, 0)

/**
   @brief fonction utilitaire pour découper une chaine de carractère en morceau
   @param s : la chaine à découper
   @param e : la regex pour détecter les séparateurs
   @return un tablea de chaines de carractères terminé par null.
   @see delTabChar;

   Le tableau retourné à été réservé par malloc et doit être libéré par fonction
   delTabChar. Attention la fonction ignore les tockens vides
   
 */
char **splitToChar(string s,const std::regex &e) {
  unsigned int i;
  std::vector<string> spl;
  std::smatch m;
  // découpage
  while (std::regex_search(s, m, e)) {
    string pre = m.prefix().str();
    if (pre.size() != 0) {
      spl.push_back(pre);
    }
    s = m.suffix();
  }
  if (s.size() != 0) {
    spl.push_back(s);
  }

  // conversion 
  char **res = new char*[spl.size()+1];
  for (i=0; i<spl.size(); i++) {
    
    res[i]=new char[spl[i].size()+1];
    strcpy(res[i], spl[i].c_str());
  }
  res[i] = NULL;

  return res;
}

void delTabChar(char **t) {
  int i;
  for (i=0; t[i]!= NULL; i++) {
    delete[] t[i];
    t[i] = NULL;
  }
  delete[] t;
}

/**
   @brief fonction pour simuler l'execution d'un programme
   Elle prend les même paramêtre que execvp la fonction qu'il faut appeler.
   @param file : le nom du programme à éxécuter
   @param argv : un tableau de chaines de caractère contenant les arguments
   et terminé par NULL
*/
int executevp(const char *file, char *const argv[]) {
  char *const *pt;
  
  cout << "Le shell doit éxécuter la commande '" << file << "' avec les arguments :" << endl;
    
  pt = argv;
  while (*pt != NULL) {
    cout << "\t- '" << *pt << "'" << endl;
    pt++;
  }
  return 0;
}


int main(int argc, char *argv[]) {

  while (1) {
    int attend;

    string text_com;

    //    text_com = readline("entrez une commande>");
    cout << "entrez une commande>" << std::flush;
    
    // lecture de la ligne
    getline(cin, text_com);


    // on normalise la ligne en supprimant les espace double en début ou fin de ligne
    std::regex sps("[ \t\n]+");
    text_com = std::regex_replace(text_com, sps, " ");
    text_com = std::regex_replace(text_com, std::regex("^ +"), "");
    text_com = std::regex_replace(text_com, std::regex(" +$"), "");

    cerr << "lecture de la commande \"" << text_com << "\"" << endl;

    if (text_com == "fin") {
      cout << "Bye" << endl;
      break;
    }

    if (text_com == "") {
      cerr << "La comande est vide" << endl;
      continue;
    }
      
    if (text_com.back() == '&') {
      // il y a un & à la fin
      // on doit faire une commande en tâche de fond.
      if (attend) {
	fprintf(stdout, "Au premier plan\n");
      } else {
	fprintf(stdout, "En tache de fond\n");	  
      }	
      attend = false;
      text_com.pop_back();
      text_com = std::regex_replace(text_com, std::regex(" +$"), "");
    } else {
      // Le shell devra attendre la fin de la commande
      attend = true;
    }

    // y a t'il un pipe | dans la commande
    if (!std::regex_match(text_com, std::regex(".*[|].*"))) {
      // non il n'y en a pas
      // conversion de la commande en tableau utilisable par les fonctions systèmes
      char **arg_com = splitToChar(text_com, std::regex(" "));
      
      // ######################################
      // Code à modifier ici
      // ######################################
      /**
	 @todo partie 1
	 vous devez appeler la fonction execvp qui prend les mêmes arguments que executevp.
	 attention, vous devez faire en sorte que
	 - shell execute la commande en premier plan si attend = true
	 - en tache de fond sinon
	 - continue après l'appel à cette fonction
      */
     
		 
		/*	executevp(arg_com[0], arg_com); */

//			execvp(arg_com[0], arg_com);

		  pid_t  pid=fork();
			test_error_exit("fork",pid==-1);
			
      if (pid==0)
			{
				execvp(arg_com[0],arg_com);
				cerr << "la commande ne fonctionne pas "<<arg_com[0]<< " car " << strerror(errno) << endl;

			//strerror permet de convertire l'erreur en texte
			//arg_com[0]permet de voir ou est l'erreur
			//cerr sortie pour les erreurs	
				exit(1);
			}
			
      if (attend) {
	fprintf(stdout, "Au premier plan\n");
//	execvp(arg_com[0],arg_com);
	/// @todo attendre
 				sleep(1);
				waitpid(pid,0,0);
 			} else {
	fprintf(stdout, "En tache de fond\n");
//	execvp(arg_com[0],arg_com);	
	/// @todo ne pas attendre
      }

      
      // libération du tableau arg_com
      delTabChar(arg_com);
    } else {
      // il y a au moins un pipe on test s'il n'y en a qu'un

      std::smatch mpipe;
      if (!std::regex_search(text_com, mpipe,
			    std::regex("^([^|]+)\b*[|]\b*([^|]+)$"))) {
	// il y a plus de 1 pipe, on ne traite pas
	cerr << "la commande " << text_com << " contient plusieurs pipes, on ne la traite pas" << endl;
	continue;
      }
      // il n'y a qu'un seul pipe donc 2 commande à faire
      char **arg_com1 = splitToChar(mpipe[1].str(), std::regex("[ \t]"));
      char **arg_com2 = splitToChar(mpipe[2].str(), std::regex("[ \t]"));
      
      // ######################################
      // Code à modifier ici
      // ######################################
			//
      // vous devez appeler la fonction execvp avec les arguments identiques à executevp
      // de manière à ce que le shell execute les deux  commandes avec un pipe entre elle
      executevp(arg_com1[0], arg_com1);
      fprintf(stdout, "Dont la sortie est redirigée vers\n");
      executevp(arg_com2[0], arg_com2);
      
      if (attend) {
	fprintf(stdout, "Au premier plan\n");
	/// @todo attendre
      } else {
	fprintf(stdout, "En tache de fond\n");	  
	/// @todo ne pas attendre
      }
      
    }
  }   

  return 0;
}
