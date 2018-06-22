#include <iostream>
#include <cstdlib>
#include <sys/wait.h>
#include <cstdio>
#include <unistd.h>

using namespace std;

int main()
{
  
	for(int i=0;i<2;i++)
	{
		fork();
	}
	sleep(10);
	return 0;

 
}


