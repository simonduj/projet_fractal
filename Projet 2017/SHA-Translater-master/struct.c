#include <stdio.h> // Utilise pour lecture du fichier en input


typedef struct{
    int buffery;
}Test_t;

int modify(Test_t* structured){
    structured->buffery = 80;
    return 0;
}

int main(int argc, char* argv[]){
    Test_t number1 = {10};
    printf("before : %d \n", number1.buffery );
    modify(&number1);
    printf("after : %d \n", number1.buffery );
    return 0;
}