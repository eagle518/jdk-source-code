#include <windows.h>
#include <stdio.h>
#define BUFFER_SIZE 1024
#include "UpdateUtils.hpp"

int main(int argc, char *argv[])
{
    TCHAR szHash[1+2*SHA1LEN];

    if ( argc==2) {
        if (getSHA1(argv[1], szHash))
            printf("%s  %s", argv[1], szHash);
        return 0;
    } 
    if ( argc  < 2 ) return 1;
    printf("static char* filelist[]={\n");
    for (int i=1; i < argc; i++) {
        printf("\"%s\",", argv[i]);
        if (getSHA1(argv[i], szHash))
            printf("\"%s\"", szHash);
        else
            printf("\"0\"");
        if (i != (argc-1))
            printf(",\n");
    }
    printf("};");
}
