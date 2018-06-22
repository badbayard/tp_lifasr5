#include <iostream>
#include <stdlib.h>
#include "socklib.hpp"
#include <string>

using namespace std;



int main (int argc, char *argv[])
{
	bool estServeur;
	bool estClient;
	string port;
  int s;

	if (argc==2)
	{
		int at;
		estServeur=true;
		port=argv[1];
		at = socklib::CreeSocketServeur(port);
	
		s = socklib::AcceptConnexion(at);

	}

	if (argc==3)
	{
		estClient =true;
		port=argv[2];
		s = socklib::CreeSocketClient(argv[1],port);
	}

	if ((argc != 3) && (argc != 2))
	{
		cout<<"probleme" <<endl;
	}
		

	return 0;
}
