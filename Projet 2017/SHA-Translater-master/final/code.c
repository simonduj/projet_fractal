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

typedef struct link{
    char* mdp;
    struct link* next;
}link_t; 

typedef struct {
    char letter;
    int size;
    sbuf_t* nbr2; 
}argument_sort_t;
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
    int count = 0;
    while(filePointers[i] != NULL){
        int status=1;
        while(status){
            FILE* fileReceived = fopen(filePointers[i], "r");
            if(fileReceived == NULL){
                printf("Can't open input file!\n");
                status = 0;
            }
            uint8_t* temp;
            temp = (uint8_t *)malloc(sizeof(uint8_t)*32);
            count++;
            while(fread(temp,sizeof(uint8_t),32,fileReceived)==32){
                sem_wait(number1->empty);
                pthread_mutex_lock(&number1->mutex);
                // Ecriture dans section critique la sc
                number1->ptrTab[number1->fes] = temp;
                number1->fes = ((number1->fes)+1)%sizeOfBuffer;
                
                pthread_mutex_unlock(&number1->mutex);
                sem_post(number1->full);
                temp = (uint8_t *)malloc(sizeof(uint8_t)*32);
                count++; 
            }
            free(temp);
            count--;
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
    // Ecriture dans section critique la sc
    number1->ptrTab[number1->fes] = number1->stop;
    printf("stop buffer1 %d\n",number1->fes);
    number1->fes = ((number1->fes+1) % sizeOfBuffer);
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
 
void actionReverse(int nb_threads,sbuf_t* number1,sbuf_t* number2){
    uint8_t* temp;
    size_t size = 16;
    char* res = (char *)malloc(sizeof(char)*17);
    int status  = 1;
    while(status){
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
            
            reversehash(temp,res,size);
            //printf("reversed %s\n",res);
            free(temp);

            //producteur
            sem_wait(number2->empty);
            pthread_mutex_lock(&number2->mutex);
            number2->bufferChar[number2->fes] = res;
            number2->fes = ((number2->fes)+1)%nb_threads; //A check y avait une erreur de compilation
            pthread_mutex_unlock(&number2->mutex); 
            sem_post(number2->full);
            res = (char *)malloc(sizeof(char)*17);
        }else{
            printf("reverse stop trouve buffer1 %d\n",number1->ffs);
            pthread_mutex_unlock(&number1->mutex);
            sem_post(number1->empty); // il y a un slot de libre en plus
            status = 0;
            printf("PASSE AVANT EXIT REVERSE \n");
            sem_wait(number2->empty);
            pthread_mutex_lock(&number2->mutex);
            number2->bufferChar[number2->fes] = number2->stop;
            printf("stop envoye buffer2 %d\n",number2->fes); 
            number2->fes = ((number2->fes)+1)%nb_threads; //A check y avait une erreur de compilation
            pthread_mutex_unlock(&number2->mutex); 
            sem_post(number2->full);
            free(res);
        }
    }
    // on free le dernier qui n'a pas ete utilise
    
}
/*
void actionReverse(int nb_threads,sbuf_t* number1,sbuf_t* number2){
    uint8_t* temp;
    size_t size = 16;
    while(signal){
        sem_wait(number1->full); // attente du slot rempli
        pthread_mutex_lock(&number1->mutex);
        // save l'adresse du hash concerne 
        temp = number1->ptrTab[number1->ffs];
        if(temp == number1->stop){
        number1->ptrTab[number1->ffs] = 
        }else{

        }
    }
    */
/*
 * Accede a la sc tab2 pour selectionner le mdp candidat en comptant le 
 * nombre d'occ du mdp courant
 * /!\ malloc pour liste chainee et free si delete
 * si occ_cour < occ_prec --> rien faire
 * si occ_cour == occ_prec --> rajouter a la liste chainee ou la creer si liste empty
 * si occ_cour > occ_prec --> delete la liste et en creer une nouvelle avec le seul mdp courant
 * /!\ malloc pour ce cas.
 * pre : le buffer
 * post : 
 */
 
void candidatSorter(sbuf_t* number2, int sizeOfBuffer, char letter){
    int occurence = 0;
    link_t* head;
    char* translated="hello";
    head = (link_t*)malloc(sizeof(link_t)); //freed en l178/181
    head->mdp = translated;
    head->next = NULL;

    while(translated != number2->stop){
        int i;
        int yolo;
        sem_wait(number2->full); // attente du slot rempli
        pthread_mutex_lock(&number2->mutex);
        //pointeur du tableau char est free
        //number2->ptrTab[number2->ffs] =CONSOMME
        translated = number2->bufferChar[number2->ffs];
        //printf("sort buffer2 case %d\n",number2->ffs);
        number2->bufferChar[number2->ffs] = NULL;
        number2->ffs = ((number2->ffs)+1)%sizeOfBuffer;
        yolo = number2->ffs;
        pthread_mutex_unlock(&number2->mutex);
        sem_post(number2->empty); // il y a un slot de libre en plus
        if(translated==number2->stop){
            printf("candidat sorter stop trouve buffer2 %d \n",yolo);
            link_t* cursor = head;
            while(cursor != NULL){
                printf("%s\n",cursor->mdp);
                cursor = cursor->next;
                free(head); //Initialise par malloc en l159
                head = head->next;
            }
            free(head);
            return;
            //Initialise par malloc en l159
           // return 0; il n y avait pas de else, faut il en ajouter un?
        }else{
        /// BUG AVANCE DANS LE MOT
        char* ptr = translated;
        for (i=0; ptr[i]; ptr[i]==letter ? i++ : *ptr++){
            // occurence    
        }

        //printf("occurence calculee %d\n",i);
        if(i==occurence && i != 0){
            link_t* newelement = (link_t *)malloc(sizeof(link_t)); //freed?
            newelement->mdp = translated;
            newelement->next = head;
            head = newelement;
        }
        // *head == NULL
        if(occurence == 0){
            head->mdp = translated;
            occurence = i;
        }
        if(i>occurence){
            // free la list et puis recreer avec nouveau mot
            link_t* cursor; 
            link_t* tmp;
 
            if(head != NULL){
                cursor = head->next;
                head->next = NULL;
                while(cursor != NULL){
                    tmp = cursor->next;
                    free(cursor->mdp); 
                    free(cursor); 
                    cursor = tmp;
                }
            }
            free(head->mdp);   
            head->mdp = translated;
            occurence = i;

        }
        if(i<occurence){
            free(translated); 
        }
    }
    }
    return;
}

/*
void candidatSorter(sbuf_t* number2, int sizeOfBuffer, char letter){
    int current;
    char* temp;
    link_t* head;
    while(temp != number2->stop){
    
        sem_wait(number2->full); 
        pthread_mutex_lock(&number2->mutex);
    
        temp = number2->bufferChar[number2->ffs];
        number2->bufferChar[number2->ffs] = NULL;
        number2->ffs = ((number2->ffs)+1)%sizeOfBuffer;
    
        pthread_mutex_unlock(&number2->mutex);
        sem_post(number2->empty);

        char* ptr = temp;
        int i;
        for (i=0; ptr[i]; ptr[i]==letter ? i++ : *ptr++){
        }
        if(head==NULL){
            head = (link_t*)malloc(sizeof(link_t));
            head->mdp = temp;
            head->next = NULL;
            current = i;
        }
        if(i>current){
            link_t* cursor; 
            link_t* tmp;
            if(head != NULL){
                cursor = head->next;
                head->next = NULL;
                while(cursor != NULL){
                    tmp = cursor->next;
                    free(cursor->mdp); 
                    free(cursor); 
                    cursor = tmp;
                }
            }
            free(head->mdp);   
            head->mdp = temp;
        }else if(i == current){
            if(current == 0){

            }else{

            }

        }else{
            free(temp);
        }
    }
    // printer elem,free ses composants, le free
}
*/
////////////////////////////// METHODE POUR PASSER LES ARGUMENTS A READER ///////////////////////////////////////

//Methode du thread Reader
void* readerthread (void* param){ 
	argument_readr_t* actual_params =(argument_readr_t *)param;
    printf("thread reader cree\n");
	reader(actual_params->fp, actual_params->nbr1, actual_params->size, actual_params->pos);
    printf("thread reader close\n");
    return (void *) 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* reversethread (void* param) { //Parametres et param de retour a verifier
    argument_reverse_t* actual_params = (argument_reverse_t *) param;
    printf("thread reverse cree\n"); 	
	actionReverse(actual_params->nbrethreads, actual_params->nbr1, actual_params->nbr2);
    printf("thread reverse close\n"); 	
    return (void *) 0;
}

///////////////////////////////METHODE POUR PASSER LES ARGUMENTS A CANDIDATE SORTER///////////////////////////////
void* sorterthread (void* param){
    argument_sort_t* actual_params = (argument_sort_t *) param;
    printf("thread sorter cree\n"); 	
    //candidatSorter(sbuf_t* number2, int sizeOfBuffer, char letter)
    candidatSorter(actual_params->nbr2, actual_params->size, actual_params->letter);
    printf("thread sorter close\n");
    return (void *) 0;
}	

//Fonction pour gerer les erreurs
void error(int err, char *msg){
    fprintf(stderr, "%s\n a renvoyé %d, erreur: %s\n", msg, err, strerror(errno));
	exit(EXIT_FAILURE);
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
    sbuf_t* number2;

    argument_readr_t* argument_rdr;
    argument_reverse_t* argument_rvs;
    argument_sort_t* argument_srt;
    
    
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
    number2= (sbuf_t *)malloc(sizeof(sbuf_t));
    number1->stop = (void *)malloc(8);
    number2->stop = (void *)malloc(8);
    //////////////// MALLOC DU TAB DE POINTEUR /////////////////////////////////////////////
    number1->ptrTab = (uint8_t **) malloc(sizeof(uint8_t*)*nbrthreads);
    /////////////// MALLOC DU BUFFER CHAR //////////////////////////////////////////////////
    number2->	bufferChar = (char **)malloc(sizeof(char *)*nbrthreads);
    /*int j;
    for( j = 0; j<nbrthreads;j++){
        number2->bufferChar[j] = (char *)malloc(sizeof(char)*17); // hash de 5 uint8_t
    }
    */
 ///////////////////// SEM_INIT//////////////////////////////////////////////////
    number1->empty = (sem_t *)malloc(sizeof(sem_t));
    number2->empty = (sem_t *)malloc(sizeof(sem_t));
    number1->full = (sem_t *)malloc(sizeof(sem_t));
    number2->full = (sem_t *)malloc(sizeof(sem_t));


	sem_init(number1->full,0,0);  
	sem_init(number2->full,0,0);
	sem_init(number1->empty,0,nbrthreads);
	sem_init(number2->empty,0,nbrthreads); 
 //////////////////////INIT STRUCTURE ///////////////////////////////////////////
	number1->fes = 0;  
	number1->ffs = 0;
	pthread_mutex_init(&number1->mutex,NULL);

	number2->fes = 0;  
	number2->ffs = 0;
	pthread_mutex_init(&number2->mutex,NULL);

////////////////////////////// INIT STRUCTURE POUR PTHREAD READER CREATE /////////////////////    
	
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
	argument_rvs->nbr2 = number2;

    
   
////////////////////////////// PTHREAD CREATE  READER ///////////////////////////////////////
	pthread_t threadReader;
	if(pthread_create(&threadReader, NULL,readerthread, argument_rdr)){
        printf("error thread pthread_create threadReader");
    }
  
////////////////////////////// PTHREAD CREATE REVERSE //////////////////////////////////////    
 
    
    pthread_t* threads;
    threads = (pthread_t*)malloc(sizeof(pthread_t)*nbrthreads);
	for(g=0; g<nbrthreads ;g++){
		if(pthread_create(&(threads[g]), NULL, reversethread, argument_rvs)){
			printf("error pthread_create pour reversethread avec g = %d\n",g);
		}
	}

    ////////////////////////////// INIT STRUCTURE POUR PTHREAD CANDIDATE SORT  ///////////////////// 
    
    argument_srt = (argument_sort_t *) malloc (sizeof(argument_sort_t));
    argument_srt->letter = letter;
    argument_srt->nbr2 = number2;
    argument_srt->size = nbrthreads;

    ////////////////////////////// PTHREAD CREATE CANDIDATE SORT//////////////////////////////////////    
    pthread_t threadSorter;
    if(pthread_create(&threadSorter, NULL, sorterthread, argument_srt)){
            printf("error pthread_create pour reversethread avec g = %d\n",g);
    }    
   


    ////////////////////////////////// PTHREAD JOIN  READER //////////////////////////////////////
    if(pthread_join(threadReader, NULL)){
		printf("pthread_join pour readerthread\n");
	} 

     printf(" PASSSE LE THREADREADER\n");

    ////////////////////////////////// PTHREAD JOIN REVERSE ///////////////////////////////////
	for (h=nbrthreads-1; h>=0; h--){
    	if(pthread_join(threads[h], NULL)){
			printf("pthread_join pour reversethread avec h = %d\n",h);
		}
    }
    printf(" PASSSE LE THREADREVERSE\n");

////////////////////////////// PTHREAD JOIN CANDIDATE SORT//////////////////////////////////////    
    if(pthread_join(threadSorter, NULL)){
        printf("pthread_join pour readerthread\n");
    }
    printf(" PASSSE LE THREADSORTER\n");
    
    free(argument_rdr);
    free(argument_rvs);
    free(argument_srt);
    free(threads);

    free(number1->ptrTab);
    sem_destroy(number1->full);
    sem_destroy(number1->empty);

    free(number2->bufferChar);
    sem_destroy(number2->empty);
    sem_destroy(number2->full);
    free(number1->stop);
    free(number2->stop);
    free(number1->empty);
    free(number1->full);
    free(number2->empty);
    free(number2->full);
    free(number1);
    free(number2);
    pthread_mutex_destroy(&number1->mutex);
    pthread_mutex_destroy(&number2->mutex);
    

    /////////////////////////// CANDIDAT SORTER ///////////////////////////////
    //OK: Free le bufferUint8_t  -> "1 malloc = 1 free" OK


    pthread_exit(NULL);
    return 0;
}
//https://cboard.cprogramming.com/c-programming/126517-producer-consumer-w-mutual-exclusion.html
