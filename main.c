#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "fractal.h"

#define INPUT_LENGTH 5
#define MAX_LINE_LENGTH 100

size_t countLines(char *fileName);
int startWith(const char *start, const char *string);
struct fractal *getBestFractal(struct fractal **tab);
struct fractal **getFractal(char *fileName);

// GLOBAL VARIABLES 
char **fileTab;
const char *outPutFile;
struct fractal *best;
int isD = 0;	// default value = false 
int maxThreads = 1;	// default value = 1 thread
int numberFiles;
int startFiles = 1; 

int main(int argc, char *argv[])
{
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
	
	best = getBestFractal(getFractal(fileTab[0]));
	write_bitmap_sdl(best, outPutFile);	
	
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

struct fractal **getFractal(char *fileName)
{
	size_t number = countLines(fileName);

	struct fractal **tab;
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
		
			tab[i] = (struct fractal *) fractal_new(name, width, height, a, b);
		}
		else
		{
			// do nothing
		}
	}

	return tab;
}

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


