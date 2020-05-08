
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


FILE *f ;
int fishlocation;

void my_handler(int signum)
{
    if (signum == SIGUSR1)
    {
        //printf("Sent signal SIGUSR1!\n");
    }
};

void siginthandler(int sig) 
{ 
    printf("Caught signal! %d, Process %d is terminating\n", sig, getpid());
	exit(0); 
};

int getsegid(){ // process that returns the id of the shared memory that we are using
key_t key = 44344;
int rows=8;
int columns=5;
int segid;
segid=shmget(key, sizeof(char)*rows*columns, 0666);
if (segid < 0){
	perror("shmget");
	exit(1);
	}

return segid;
}

void placefish() // process that places fish into the shared memory array
{
//puts("placefish has started");
	char *ptr;

	int rows=8;
	int columns=5;

	int segid = getsegid();
	ptr = (char *) shmat(segid, 0, 0);

	int mid = columns / 2 + 1;
	
	ptr[columns * rows - mid] = 'F';
	fishlocation = columns * rows - mid;
	

}

void movefish(int shmid, int location) // moves fish by utilizing modulus to figure out which column the nearest pellet is on
{
	//puts("movefish has started");
	int rows=8;
	int columns=5;
	
	char * ptr = (char *) shmat(shmid, 0, 0);

	int A = fishlocation % columns;
	int B = location % columns;
	
	
	//printf("The fish is at %d while the pellet is at column %d\n, the location of the pellet is %d\n", A, B, location);
	if (location >= 0){
		if (A != B)
		{
			if (A < B)
			{
			ptr[fishlocation] = '.';
			fishlocation += 1;
			ptr[fishlocation] = 'F';
			}
			if (A > B)
			{
			ptr[fishlocation] = '.';
			fishlocation -= 1;
			ptr[fishlocation] = 'F';
			} 
		}

		int i,j;
		f = fopen("swim_mill[log].txt", "a");
			for(i=0; i < rows; i++){
				for(j=0; j < columns; j++){
					printf("%5c", ptr[i*columns+j]);
					fprintf(f, "%5c",ptr[i*columns+j]); 		
				}
			printf("\n");
			fprintf(f, "\n");
			}
	}
	printf("\n\n\n");
	fprintf(f,"\n\n\n");
	fclose(f);
	sleep(1);
}

int findpellet(int shmid) // process that finds the pellet
{
	//puts("findpellet has started");
	int rows=8;
	int columns=5;
	int start = rows * columns - 1;
	char pellet = '.';
	pellet |= 0x80;
	//printf("The pellet is %c", pellet);
	char * ptr = (char *) shmat(shmid, 0, 0);
	
	int notfound = 1;

	for (int i = start; start >= 0 && notfound == 1; start--)
	{
		//printf("The value at index (%d) = %c\n", start, ptr[start]);
		if (ptr[start] == pellet)
		{
		//printf("\n\nThe pellet was found at %d\n\n", start);
		notfound = 0;
		}
	}
	//printf("start value is: %d\n", start);
	return start + 1; // starts at end of array and returns the first instance where a pellet is found
	
}

int main(int argc, char *argv[]){

signal(SIGINT, siginthandler);
f = fopen("swim_mill[log].txt", "w");
if (f == NULL)
{
    printf("Error opening file!\n");
    exit(1);
}

fclose(f);



signal(SIGUSR1, my_handler);
//printf("Fish process has started: %d", getppid());
int parent = getpid();
placefish();
int id = getsegid();
int location;

while(1){ 
/*while loop that 1st: looks for pellet 
		  2nd: aligns fish to move in front of pellet
		  3rd: once the fish is done looking and moving
		       it sends a signal to all processes it has permission to
*/
location = findpellet(id);
//printf("The location of pellet is at %d", location);
movefish(id, location);
if(kill(0, SIGUSR1)>0){ // this signal is a user-defined signal utilizing the SIGUSR1 macro
	perror("Kill failed");
}
//printf("Kill command sent");
sleep(2);
}

return 0;

}

