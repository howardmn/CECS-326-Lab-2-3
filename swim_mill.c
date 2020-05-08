#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

void delay(int number_of_seconds);
void createarray();
void fishprocess();

void siginthandler(int sig) 
{ 
    printf("Caught signal! %d, Process %d is terminating\n", sig, getpid());
	exit(0); 
};
  

void my_handler(int signum)
{
    /* my_handler is the function that is called when the fish 
       program sends the signal SIGUSR1. The reason why SIGUSR1 is used here even though this
       doesn't use it is because the default action when sending the signal is to terminate
       the program.
*/
};

int getrandomint(int lower, int upper)
{ 
     	srand(time(0)); 
        int num = (rand() % (upper - lower)) + lower; 
        return num;
};


const int pellet_total = 20;
int numofpellets = 0;

void createpellet(){ // process that creates a random location for the pellet to start at
	int rows=8;
	int columns=5;
	int min = 0;
	int max = (rows * columns) / 2;
	int randomint = getrandomint(min, max);
	char pellet[3]; 
	sprintf(pellet, "%d", randomint);

	char * file[] = {"./pellet", pellet, NULL}; // first element in array gives the name of the file, second gives the argument 	
						    // to pass argv[] in the main function of the pellet process

	//printf("This should show a random number: %s",file[1]);

		if (fork()==0){
		if(execv(file[0], file) == -1)
				{
					perror("Fish didn't execute"); // if exec didn't work properly, throw an error and exit process
					exit(EXIT_FAILURE);
				}
		}

};

void fishprocess(){
	puts("starting fish");
	int fid = fork();
	if(fid < 0)
		{
		perror("fork didnt work"); // if fork is less than 0, throw an error and exit process
		exit(EXIT_FAILURE);
		}

	if(fid == 0)
	{
		printf("The child process id is %d", getpid());

		static char * file[] = {"./fish", NULL}; // the first element is the name of the file to be executed 
		if(execv(file[0], file) == -1)
		{
			perror("Fish didn't execute"); // if exec didn't work properly, throw an error and exit process
			exit(EXIT_FAILURE);
		}
	
	}

}


void createarray()	// connects and creates the array for shared memory
{
	key_t key = 44344;
	int rows=8;
	int columns=5;
	int segid;
	segid=shmget(key, sizeof(char)*rows*columns, IPC_CREAT | 0666);
	if (segid < 0){
		perror("shmget");
		exit(1);
		}


	char * ptr = (char *) shmat(segid, 0, 0);
	int i, j;
	char water = '.';
	for(i=0; i < rows; i++)
	{
		for(j=0; j < columns; j++)
		{
			ptr[i*columns+j] = water;
		}

	}


}

int main(int argc, char* argv[])
{
signal(SIGUSR1, my_handler);
signal(SIGINT, siginthandler);

// Converting time into milli_seconds 
    int time_left = 1000 *2; 
  
    // Storing start time 
    clock_t start_time = clock();
	printf("start time: %ld", start_time);
createarray();
 int dontmake = 0;
 sleep(1);
fishprocess();	
pid_t pid;
int status;
while(1){
	
	if(clock() > (start_time + time_left))
	{
		printf("Times up!");
		kill(0,SIGINT);
	}
	

	if (numofpellets < 10 ) // if there are less than 10 pellets, the process is able to continue making them
	{
		createpellet();
		numofpellets++;
		
		srand(time(0));
		int time = rand() % 2;
		time = time + 3; // will randomly wait between 3 to 5 seconds
		sleep(time);
	}

	/*
		Utilized waitpid() to check on the status of the children.(In this case it would be the pellets)
		Once the pellet process has terminated, the status is returned
		the WNOHANG macro is used to allow the program to keep running past the waiting point 
	*/	
	while ((pid = waitpid(0, &status, WNOHANG) > 0)) // this function goes through any processes that returned
							 // once that is done, resume normally
	{
		numofpellets--;
	}
	printf("There are %d pellets on the field\n", numofpellets);

}


return 0;
}



