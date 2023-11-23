//este archivo es el fichero fuente que al compilarse produce el ejecutable PADRE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

int main(int argNum, char *args[]){
    
    if(argNum != 2){
        printf("Padre se invoca con un argumento!\n");
	return 0;
    } 
    // crea clave asociada a fichero ejecutable y letra 
    key_t key = ftok(args[0],'X');

    // crear cola de mensajes "mensajes"
    int mensajes = msgget(key, IPC_CREAT | 0600);
 
    // crear región de memoria compartida "lista" de tamano 10 pids
    // y enlazararla con un array con capacidad para N PIDs 
    int arraySize = (int)args[1]*sizeof(pid_t);
    int shrdMemId = shmget(key, arraySize, IPC_CREAT | 0600);      
    pid_t *lista = (pid_t*)shmat(shrdMemId, NULL, 0);

    // crear un semáforo para proteger acceso a "lista" 
    int semaphoreId = semget(key, 1, IPC_CREAT | 0600);

    // crear tuberia sin nombre "barrera"
    int pipeArray[2];
    int barrera = pipe(pipeArray);
    
    // crea N procesos hijos que ejecutaran HIJO
    for(int i = 0; i < 10; i++){

        pid_t pidFork = fork();
        
        if(pidFork == 0){	    
	    
            // Esto lo ejecuta solo el proceso hijo
            if(execlp("./Trabajo2/HIJO", "HIJO", NULL)==-1){
                printf("oh no\n");
                exit(EXIT_FAILURE);
            }else{
                printf("oh yes");
            }
        }
         
    }
    

    // Desvincular memoria compartida
    //shmdt();

    printf("Finalizando padre \n");       
    return 0;

}
