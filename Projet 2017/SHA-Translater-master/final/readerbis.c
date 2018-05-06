#include <stdio.h> 
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h> 
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "reverse.h"
#include <pthread.h>
// the % sign is used to make a 'loop' when an overflow occurs
//bool reversehash(const uint8_t *hash, char *res, size_t len);
typedef struct {
	uint8_t** ptrTab; // taille indefinie
    char** bufferChar;  // taille indefinie car nbr_thread indefini 
    int fes;                
    int ffs;
    void* stop;                
    sem_t* full;             // Nombre de slots remplis
    sem_t* empty;            // Nombre de slots vides 
    pthread_mutex_t mutex;            // Outil pour partager le buffer
}sbuf_t;

typedef struct{
	char** fp; 
	sbuf_t* nbr1;
    int pos;
    int size;
}argument_readr_t;

typedef struct {
	int nbrethreads;
	sbuf_t* nbr1;
	sbuf_t* nbr2;
}argument_reverse_t; 

/*
 * La methode reader extrait tous les hashs du fichier pour les 
 * placer dans un tableau sc jusqu'a la fin du fichier.
 * pre : le file n'est pas null'
 * post : stocke les hashs du fichier dans le tableau buffer sc
 * le uint8_t buffer de number1 est deja malloc
 * signaler : -1 --> il reste plusieurs fichiers
 * signaler : 0 --> dernier fichier
 * signaler : 1 --> lecture des fichiers termines
 */

void reader(char** filePointers, sbuf_t* number1,int sizeOfBuffer,int pos){
    int i = pos;
    while(filePointers[i] != NULL){
        printf("file name : %s\n", filePointers[i]);
        int status=1;
        while(status){
            uint8_t* temp;
            FILE* fileReceived = fopen(filePointers[i], "r");
            if(fileReceived == NULL){
                printf("Can't open input file!\n");
                status = 0;
            }
            
            temp = (uint8_t *)malloc(sizeof(uint8_t)*32);
            while(fread(temp,sizeof(uint8_t),32,fileReceived)==32){
                sem_wait(number1->empty);
                pthread_mutex_lock(&number1->mutex);
                // Ecriture dans section critique la sc
                printf("section crit\n");
                number1->ptrTab[number1->fes] = temp;
                number1->fes = (number1->fes+1)%sizeOfBuffer;
                
                pthread_mutex_unlock(&number1->mutex);
                sem_post(number1->full);
            }
            if(feof(fileReceived)){
                printf("fichier terminé \n");
                fclose(fileReceived);
                //free(temp);
                status = 0;
            }else{
                printf("erreur de lecture\n");
                fclose(fileReceived);
                //free(temp);
                status = 0;
            }
        }
        i++;
    }
    sem_wait(number1->empty);
    pthread_mutex_lock(&number1->mutex);
    printf("section crit finaaale");
    // Ecriture dans section critique la sc
    number1->ptrTab[number1->fes] = number1->stop;
    number1->fes = (number1->fes+1)%sizeOfBuffer; 
    pthread_mutex_unlock(&number1->mutex);
    sem_post(number1->full);
}


/* 
 * Accede a la sc du tab1 pour y extraire un hash et le stocker le resultat 
 * du reverseHash dans le tab2 (sc)
 * n represente le nombre d'elements dans le buffer
 * len represente la longueur du mdp max (donnee lors de l'execution du programme)
 * pre : number-> est malloc et rempli
 *       number2.bufferChar est malloc et rempli
 * post : reverse le hash du tableau tab1 pour le stocker dans le tab2.
 */
void actionReverse(int nb_threads,sbuf_t* number1){
    uint8_t* temp;
    size_t size = 16;
    char* res = (char *)malloc(sizeof(char)*17);
    // consommateur
    sem_wait(number1->full); // attente du slot rempli
    pthread_mutex_lock(&number1->mutex);
    // save l'adresse du hash concerne 
    temp = number1->ptrTab[number1->ffs];
    if(temp != number1->stop){
        number1->ptrTab[number1->ffs] = NULL;
        number1->ffs = ((number1->ffs)+1)%nb_threads;
        pthread_mutex_unlock(&number1->mutex);
        sem_post(number1->empty); // il y a un slot de libre en plus
        printf("entre dans reverse\n");
        reversehash(temp,res,size);
        printf("reversed %s\n",res);
        free(temp);
    }else{
        printf("stop trouve \n");
        pthread_mutex_unlock(&number1->mutex);
        sem_post(number1->empty); // il y a un slot de libre en plus
        free(res);
    }
}


////////////////////////////// METHODE POUR PASSER LES ARGUMENTS A READER ///////////////////////////////////////

//Methode du thread Reader
void* readerthread (void* param){ 
	argument_readr_t* actual_params =(argument_readr_t *)param;
    printf("thread reader cree\n");
	reader(actual_params->fp, actual_params->nbr1, actual_params->size, actual_params->pos);
    printf("thread reader close\n");

    pthread_exit(NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* reversethread (void* param) { //Parametres et param de retour a verifier
    argument_reverse_t* actual_params = (argument_reverse_t *) param;
    printf("thread reverse cree\n"); 	
	actionReverse(actual_params->nbrethreads, actual_params->nbr1);
    printf("thread reverse close\n"); 	

    pthread_exit(NULL);
}




int main(int argc, char *argv[])
{
	
    // Declaration du string contenant le 1er fichier
	// Declaration variable nbrthreads et k
	int nbrthreads =1;
	char letter = 'a';
	int pos;
    char *fichier;
    int g;
    int h;
 

    sbuf_t* number1;

    argument_readr_t* argument_rdr;
    argument_reverse_t* argument_rvs;
    
    
	if(strcmp(argv[1],"-t")==0){ //Jerem dit que ce truc tend a foirer
        nbrthreads = atoi(argv[2]);
        printf("nbrthreads %d\n",nbrthreads);
        if(argv[3] != NULL && strcmp(argv[3],"-l")==0){
        	letter = argv[4][0];
        	printf("letter %c\n",letter);
        	pos = 5;
        	printf("position %d\n", pos);
        } else{
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
        	printf("letter %c\n",letter);
        	pos = 1;
        	printf("position %d\n", pos);
        }
    }
    fichier = argv[pos];
    printf("nom de fichier : %s\n",fichier);
    
    //////////////// MALLOC DES STRUCTURES /////////////////////////////////////////////////
    
    number1= (sbuf_t *)malloc(sizeof(sbuf_t));
    number1->stop = (void *)malloc(8);
    //////////////// MALLOC DU TAB DE POINTEUR /////////////////////////////////////////////
    number1->ptrTab = (uint8_t **) malloc(sizeof(uint8_t*)*nbrthreads);
    /////////////// MALLOC DU BUFFER CHAR //////////////////////////////////////////////////
    /*int j;
    for( j = 0; j<nbrthreads;j++){
        number2->bufferChar[j] = (char *)malloc(sizeof(char)*17); // hash de 5 uint8_t
    }
    */
 ///////////////////// SEM_INIT//////////////////////////////////////////////////
    number1->empty = (sem_t *)malloc(sizeof(sem_t));
    number1->full = (sem_t *)malloc(sizeof(sem_t));


	sem_init(number1->full,0,0);  
	sem_init(number1->empty,0,nbrthreads);
 //////////////////////INIT STRUCTURE ///////////////////////////////////////////
	number1->fes = 0;  
	number1->ffs = 0;
	pthread_mutex_init(&number1->mutex,NULL);


////////////////////////////// INIT STRUCTURE POUR PTHREAD CREATE READER /////////////////////    
	
    argument_rdr = (argument_readr_t *)malloc(sizeof(argument_readr_t));
	//Passage des adresses des paramètres
	argument_rdr->fp = argv ;
	argument_rdr->nbr1 = number1;
	argument_rdr->pos = pos;
    argument_rdr->size = nbrthreads;

 ////////////////////////////// INIT STRUCTURE POUR PTHREAD CREATE REVERSE //////////////////////    
	
	argument_rvs = (argument_reverse_t *) malloc (sizeof(argument_reverse_t));
    argument_rvs->nbrethreads = nbrthreads;
	argument_rvs->nbr1 = number1;

    
   
////////////////////////////// PTHREAD CREATE  READER ///////////////////////////////////////
	pthread_t threadReader;
	if(pthread_create(&threadReader, NULL,readerthread, argument_rdr)){
        printf("error thread pthread_create threadReader");
    }
  
////////////////////////////// PTHREAD CREATE REVERSE //////////////////////////////////////    
 
    
    pthread_t* threads;
    threads = (pthread_t*)malloc(sizeof(pthread_t)*nbrthreads);
	for(g=0; g<=nbrthreads ;g++){
		if(pthread_create(&(threads[g]), NULL, reversethread, argument_rvs)){
			printf("error pthread_create pour reversethread avec g = %d\n",g);
		}
	}
    
    ////////////////////////////////// PTHREAD JOIN  READER //////////////////////////////////////
    if(pthread_join(threadReader, NULL)){
		printf("pthread_join pour readerthread\n");
	} 

 
    ////////////////////////////////// PTHREAD JOIN REVERSE ///////////////////////////////////
	
    for (h=nbrthreads-1; h>=0; h--){
    	if(pthread_join(threads[h], NULL)){
			printf("pthread_join pour reversethread avec h = %d\n",h);
		}
    }
    
    free(argument_rdr);
    free(argument_rvs);

    free(number1->ptrTab);
    sem_destroy(number1->full);
    sem_destroy(number1->empty);

 
    free(number1->stop);
    free(number1->empty);
    free(number1->full);
    free(number1);
    pthread_mutex_destroy(&number1->mutex);
    

    /////////////////////////// CANDIDAT SORTER ///////////////////////////////
    //OK: Free le bufferUint8_t  -> "1 malloc = 1 free" OK


    pthread_exit(NULL);
    return 0;
}
//https://cboard.cprogramming.com/c-programming/126517-producer-consumer-w-mutual-exclusion.html
