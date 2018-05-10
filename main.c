#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fractal.h>
#include <pthread.h>
#include <semaphore.h>

#define INPUT_LENGTH 5
#define MAX_LINE_LENGTH 100

size_t countLines(char *fileName);
int startWith(const char *start, const char *string);
struct fractal *getBestFractal(struct fractal **tab);
struct fractal **getFractal(char *fileName);
void* consumethread(void* param);
void* producethread(void* param);
void producer(struct buffr * bfr, char *fileName);
void consumer(struct buffr * bfr);


// GLOBAL VARIABLES 
char **fileTab;
const char *outPutFile;
struct fractal *best;
int isD = 0;	// default value = false 
int maxThreads = 1;	// default value = 1 thread
int numberFiles;
int startFiles = 1; 
//buffer 
struct fractal **tab;
//Threading 
int status = 1;
//structure to store buffers including the buffer with all the files and the buffer for the fractals 
typedef struct {
struct fractal** fractalbuf; //stores fractals 
void** filebuf; //stores files  	
sem_t* full; //full slots 
sem_t* empty; //empty slots
pthread_mutex_t mutexpc;
int** fractresults;
int idx; 
int idxf;
}buffr;

//function for producing thread 
void* producethread(void* param){
	buffr* actual_param = (buffr *) param;
	producer(actual_param);
	return (void*) 0;
}

void* consumethread(void* param){
	buffr* actual_param = (buffr*) param;
	consumer(actual_param);
	return (void*) 0;
}

int main(int argc, char *argv[])
{
	
	//declaration buffers 
	//buf1 for 
	buffr* buf1 = (buffr *) malloc(sizeof(buffr));
	buffr* buf2 = (buffr *) malloc(sizeof(buffr));
	//todo? initialise data of structs?

	//CHECK INPUT 
 
	// check number of arguments > 2 
	// need at least 3 : program_name, inputFile, outputFile 
	if(argc < 3)
	{
		printf("Error not enough arguments !");
		return 1;
	}	

	outPutFile = argv[argc-1];
	int i;
	numberFiles = argc-2; // argc-2 because of the output file and the name of the program
	//SET PARAMETER FOR MAXTHREAD AND ISD

	// start at 1 because arg[0] is the program name 
	// end at argc-1 because argv[argc] = output file 
	for(i = 1; i < argc-1 ; i++)
	{
		if(strcmp(argv[i], "-d") == 0)
		{
			isD = 1;
			numberFiles --;
			startFiles++;
		}

		else if(strcmp(argv[i], "--maxthreads") == 0)
		{
			i++;
			maxThreads = atoi(argv[i]);
			numberFiles = numberFiles - 2;
			startFiles = startFiles + 2;
		}
	}	

	//CREATE AN ARRAY OF INPUT FILES 
	
	fileTab = (char **)malloc(numberFiles*sizeof(char *));
	if(fileTab == NULL)
	{
		printf("Error using malloc !");
		return 1;
	}
	
	int j;
	int index = 0;
	for(j = startFiles; j < argc-1 ; j++)
	{
		fileTab[index] = (char *)malloc(sizeof(char)*strlen(argv[j]) + sizeof(char));
		if(fileTab[index] == NULL)
		{
			printf("Memory allocation error !");
		}
		fileTab[index] = argv[j];
		index++;
	}

	//for (int k = 0; i<maxThreads ;i++)
	//{
	
//	}
	// Initialising mutex of the ds & semaphores 
	pthread_mutex_init(buf1->mutexpc, NULL);
	sem_init(&buf1->empty, 0, numberFiles);  
	sem_init(&buf1->full, 0, 0);

	//threads creation & join
	pthread_t producerthread;
	pthread_t consumerthread;


	//pthread_create(thread,NULL, function, argument of function)
	if(pthread_create(&producerthread, NULL, producethread, buf1)){
	printf("Error creating pthread \n");	
	}

	if(pthread_join(producerthread, NULL)){
	printf("Error joining pthread \n");	
	}
	
	if(pthread_create(&consumerthread, NULL, consumethread, buf1)){
	printf("Error creating pthread \n");	
	}

	if(pthread_join(consumerthread, NULL)){
	printf("Error joining pthread \n");	
	}	
		
     




	//To do after the threads are OK 
	best = getBestFractal(getFractal(fileTab[0]));
	write_bitmap_sdl(best, outPutFile);	//ok
	//Destroys & frees 
	sem_destroy(&buf1->empty);
	sem_destroy(&buf1->full);
	pthread_exit(NULL);	
	pthread_mutex_destroy(buf1->mutexpc);

}



size_t countLines(char *fileName)
{
	FILE *file;

	file = fopen(fileName, "r");
	if(file == NULL)
	{
		printf("Error reading file");
	}
	
	size_t lines = 0;
	char ch;	

	while(!feof(file))
	{
  		ch = fgetc(file);
  		if(ch == '\n')
  		{
  		  lines++;
  		}
	}

	fclose(file);

	return lines;
}

//Each producer takes a file and writes all the fractals in the buffer 

void producer(struct buffr bfr, char *fileName){



	struct fractal ** tab = (struct fractal *) malloc(sizeof(fractal));
	size_t number =   countLines(fileName);	
	tab = malloc(sizeof(struct fract *)*number);	
	FILE *file;

	file = fopen(fileName, "r");
	if(file == NULL)
	{
		printf("Error reading file");
	}

	int i;
	char line [MAX_LINE_LENGTH];
	char *name;
	int width;
	int height;
	double a;
 	double b;

	for(int k = 0; k < number ; k++)
	{
		fgets(line, MAX_LINE_LENGTH, file);
		if(startWith("#", line) == 0)
		{

			name = strtok(line, " ");
		
			width = atoi(strtok(NULL, " "));
		
			height = atoi(strtok(NULL, " "));
		
			a = atof(strtok(NULL, " "));
		
			b = atof(strtok(NULL, " "));
			sem_wait(bfr->empty); //waiting for free slot
			pthread_mutex_lock(bfr->mutexpc);
			//Critical section
			//Add fractal to buffer
			bfr->fractalbuf[bfr->idx] = (struct fractal *) fractal_new(name, width, height, a, b);
			//Incrementing index with modulo to be sure to reset without overflowing
			bfr->idx = ((bfr->idx)+1)%maxThreads; 
			//Unlock buffer	
			pthread_mutex_unlock(bfr->mutexpc);
			sem_post(bfr->full); //One more slot is full 
		}
		else
		{
		;	// do nothing
		}
	}
}


void consumer(struct buffr * bfr) {

	//take as input the list of fractals
	//compute fractals
	//return fractals
	sem_wait(&full); //Waiting for full slot
	pthread_mutex_lock(bfr->mutexpc);
	// Critical Section 
	bfr->fractresults[bfr->idxf] = fractal_get_value(bfr->fractalbuf[bfr->idx]);
	bfr->idxf= bfr->idxf+1;
	bfr->idx = ((bfr->idx)+1)%maxThreads;//Incrementing buffer cursor
	pthread_mutex_unlock(bfr->mutexpc);
	sem_post(&empty);//One more free slot

;}


/**
Replaced by producer
struct fractal **getFractal(char *fileName)
{
	size_t number =   Lines(fileName);

	
	tab = malloc(sizeof(struct fract *)*number);	
	FILE *file;

	file = fopen(fileName, "r");
	if(file == NULL)
	{
		printf("Error reading file");
		return NULL;
	}

	int i;
	char line [MAX_LINE_LENGTH];
	char *name;
	int width;
	int height;
	double a;
 	double b;

	for(i = 0; i < number ; i++)
	{
		fgets(line, MAX_LINE_LENGTH, file);
		if(startWith("#", line) == 0)
		{

			name = strtok(line, " ");
		
			width = atoi(strtok(NULL, " "));
		
			height = atoi(strtok(NULL, " "));
		
			a = atof(strtok(NULL, " "));
		
			b = atof(strtok(NULL, " "));
		//buffer to be put in critical section 
		//useless? 
			tab[i] = (struct fractal *) fractal_new(name, width, height, a, b);
		}
		else
		{
		;	// do nothing
		}
	}

	return tab;
}
*/

struct fractal *getBestFractal(struct fractal **tab)
{
	unsigned long maximum = 0;
	unsigned long current = 0;

	struct fractal *best;

	int i = 0;
	while(i < 5)
	{
		unsigned int width = fractal_get_width(tab[i]);
		unsigned int height = fractal_get_height(tab[i]);

		
		int w;
		int h;
		for(w = 0 ; w < width ; w++)
		{	
			for(h = 0 ; h < height ; h++)
			{		
				current += fractal_compute_value(tab[i], w, h);
			}

		}
		if(current > maximum)
		{
			maximum = current;
			best = tab[i];
		}
		current = 0;
		i++;
	}
	return best;
}

int startWith(const char *start, const char *string)
{
    return strncmp(start, string, strlen(start)) == 0;
}




