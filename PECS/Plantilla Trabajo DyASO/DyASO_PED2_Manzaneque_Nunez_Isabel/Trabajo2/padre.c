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
#include <sys/wait.h>


int main(int argc, char *argv[]){
    
    if(argc != 2){
        printf("Padre se invoca con un argumento!\n");
	return 0;
    } 
    int N = atoi(argv[1]);

    // crea clave asociada a fichero ejecutable y letra 
    key_t key = ftok(argv[0],'X');

    // crear cola de mensajes "mensajes"
    int mensajes = msgget(key, IPC_CREAT | 0600);
 
    // crear región de memoria compartida "lista" de tamano 10 pids
    // y enlazararla con un array con capacidad para N PIDs 
    int arraySize = N*sizeof(pid_t);
    int shrdMemId = shmget(key, arraySize, IPC_CREAT | 0600);      
    pid_t *lista = (pid_t*)shmat(shrdMemId, NULL, 0);

    // crear un semáforo para proteger acceso a "lista" 
    int sem = semget(key, 1, IPC_CREAT | 0600);

    // crear tuberia sin nombre "barrera"
    int barrera[2];
    if(pipe(barrera) == -1){
        perror("pipe");
        exit(-1);
    }
    
    // crea N procesos hijos que ejecutaran HIJO
    
    for(int i = 0; i < N; i++){

        pid_t resFork = fork();
        
	if(resFork == -1){
            perror("fork");
            exit(-1);
        }

        if(resFork == 0){	   	    
            //Esto lo ejecuta solo el hijo

            // Cierra la salida estándar y duplicar extremo de lectura de barrera
            // tambien: close(1); dup(barrera[0]);
            //close(1); 
            dup(barrera[0]);
            close(barrera[0]);
            close(barrera[1]);

	    printf("Ejecutando hijo %d con PID %d\n", i, getpid()); 
            execl("./Trabajo2/HIJO", "HIJO",argv[0], NULL); 

        }        
    }
    

    // Desvincular memoria compartida
    //shmdt();

    wait(0);
    printf("Finalizando padre\n");       
    return 0;

}
