#include <stdio.h>
#include <stdlib.h>

void main(int argc, char *argv[]){
	// Declaration du string contenant le 1er fichier
	const char *my_str; 	 // On prend le fichier passe en input qui doit necessairement exister, ce qui peut ou non etre le cas pour argv[n]
	// Declaration variable nbrthreads et k
	int nbrthreads =0;
	char letter;
	int k = 4;
	int pos;
    char *fichier;


	if(strcmp(argv[1],"-t")==0){
        nbrthreads = atoi(argv[2]);
        printf("nbrthreads %d\n",nbrthreads);
        if(argv[3] != NULL && strcmp(argv[3],"-l")==0){
        	letter = argv[4][0];
        	printf("letter %c\n",letter);
        	pos = 5;
        	printf("position %d\n", pos);
        } else{
        	letter='a';
        	printf("letter %c\n",letter);
        	pos = 3;
        	printf("position %d\n", pos);

        }
    }
    else{
        nbrthreads = 1;
        printf("nbrthreads %d\n",nbrthreads);
        if(strcmp(argv[1],"-l")==0){
        	letter = argv[2][0];
        	printf("letter %c\n",letter);
        	pos = 3;
        	printf("position %d\n", pos);
        } else {
        	letter = 'a';
        	printf("letter %c\n",letter);
        	pos = 1;
        	printf("position %d\n", pos);
        }
    }
    fichier = argv[pos];
    printf("nom de fichier : %s\n",fichier);
    return 0;
}