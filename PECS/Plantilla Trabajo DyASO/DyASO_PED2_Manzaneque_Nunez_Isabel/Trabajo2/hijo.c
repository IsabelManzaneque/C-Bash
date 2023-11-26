//este archivo es el fichero fuente que al compilarse produce el ejecutable HIJO

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

int main(int argc, char const *argv[]) {
   
    printf("\n------------ Inicio hijo ------------\n");
    
    // Recuperar la clave
    key_t key = ftok(argv[1],'X');

    // recuperar cola de mensajes
    int mensajes = msgget(key,0);
    if (mensajes == -1) {
        perror("mensajes");
        exit(-1);
    }

    // recuperar semaforo
    int sem = semget(key, 0, 0);
    if (sem == -1) {
        perror("semaforo");
        exit(-1);
    }

    // recuperar memoria compartida
    int shrdMemId = shmget(key, 0, 0);      
    if (mensajes == -1) {
        perror("memoriaCompartida");
        exit(-1);
    }

    char texto[20];
    strcpy(texto, argv[0]);
    printf("arg = %s\n\n",texto);

    for (int i=1;i<=5;i++){
        printf("%s --> %d\n",texto,i);
        sleep(1);
    }
    
    printf("-------------- Fin hijo -------------\n\n");
    exit(0);



}
