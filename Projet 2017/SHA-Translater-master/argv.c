#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>


int main(){
    char* s = "saluuuut";
    char* a = "yolooooo";
    s=a;
    printf("occurence = %s\n",s);
    return 0;
}