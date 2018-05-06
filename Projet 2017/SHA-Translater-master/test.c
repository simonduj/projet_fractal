#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


typedef struct{
   void* stop;
   uint8_t* ptr; 
}buf_t;


void fun(buf_t* arg){
    arg->ptr = arg->stop;
    if(arg->ptr == arg->stop){
        printf("success\n");
    }else{
        printf("pas succes\n");
    }
}

int main(){
    buf_t* test = (buf_t*)malloc(sizeof(buf_t));
    test->stop  = (void*)malloc(8);
    test->ptr = NULL;
    fun(test);
    free(test->stop);
    free(test);
    return 0;
}

 