#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>


#define SIZE 2
typedef struct {
    uint8_t* ptrTab[SIZE];
    int fes;                // buffer[fes%SIZE] represent the first empty slot
    int ffs;                // buffer[ffs%SIZE] represent the first full slot
}sbuf_t;

int main(){
    sbuf_t test;
    char* ptr;
    ptr = "test.bin";
    FILE* file = fopen(ptr,"r");
    test.fes = 0;
    
    int i;
    for( i = 0; i<2;i++){
        test.ptrTab[i] = (uint8_t *)malloc(sizeof(uint8_t)*5); // hash de 5 uint8_t
    }
   int produit = 0;
    while(produit<4){ 
        while(fread(test.ptrTab[test.fes],sizeof(uint8_t),5,file)==5){
        test.fes = (test.fes+1)%SIZE;
        }
        produit=produit+1;
    }
    //premier hash de 5 uint8_t
    printf("content of tab[0] : %u\n",test.ptrTab[0][0]);
    printf("content of tab[1] : %u\n",test.ptrTab[0][1]);
    printf("content of tab[2] : %u\n",test.ptrTab[0][2]);
    printf("content of tab[3] : %u\n",test.ptrTab[0][3]);
    printf("content of tab[4] : %u\n",test.ptrTab[0][4]);
   //deuxieme hash
    printf("content of tab[0] : %u\n",test.ptrTab[1][0]);
    printf("content of tab[1] : %u\n",test.ptrTab[1][1]);
    printf("content of tab[2] : %u\n",test.ptrTab[1][2]);
    printf("content of tab[3] : %u\n",test.ptrTab[1][3]);
    printf("content of tab[4] : %u\n",test.ptrTab[1][4]);    
    
    fclose(file);
    free(test.ptrTab[0]);
    free(test.ptrTab[1]);
    return 0;
}