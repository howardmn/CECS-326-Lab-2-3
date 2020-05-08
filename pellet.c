
#include <time.h> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int action = 1;

/* my_handler is the function that is called when the fish 
   program sends the signal SIGUSR1. The reason why SIGUSR1 is used is because 
   it is a signal that is designated for the user to specify what the signal does.
   by default if no handler is used when SIGUSR1 is called, the program terminates by default.
	In this case, the variable action is set to one in the beginning and set to zero to stop the pellet from moving
	Once the signal is caught, the action variable is set back to one, allowing the pellet to move again
*/
void my_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        //printf("allow pellet to take action\n");
	action = 1;
    }
};

void siginthandler(int sig) 
{ 
    printf("Caught signal! %d, Process %d is terminating\n", sig, getpid());
	exit(0); 
};

int main(int argc, char *argv[]){ // main handles all the main logic in the program for the pellet
signal(SIGUSR1, my_handler); // creates the listener that determines what happens once the signal SIGUSR1 is used
signal(SIGINT, siginthandler); // listens for when ^C signal is sent


key_t key = 44344;
int rows=8;
int columns=5;
int segid;
segid=shmget(key, sizeof(char)*rows*columns, 0666);
if (segid < 0){ // if segid < 0, creation of Id has failed
	perror("shmget");
	exit(1);
	}
char * ptr = (char *) shmat(segid, 0, 0);

int pelletlocation = atoi(argv[1]);

ptr[pelletlocation] |= 0x80;

while(pelletlocation < (rows*columns)-columns){ // while the pellet is still not at the last row
	//printf("action = %d", action);
	if (action == 1)
	{
	ptr[pelletlocation] &= ~0x80;
	pelletlocation = pelletlocation + columns;
	ptr[pelletlocation] |= 0x80;
	action = 0;
	}
}
; 
char fish = 'F';

ptr[pelletlocation] &= ~0x80;


if (ptr[pelletlocation] == fish)
	{
		printf("Pellet (%d) has been eaten\n", getpid());
		
		exit(0);
	}

else
	{
	printf("Pellet (%d) has not been eaten\n", getpid());
	exit(0);
	}
}
