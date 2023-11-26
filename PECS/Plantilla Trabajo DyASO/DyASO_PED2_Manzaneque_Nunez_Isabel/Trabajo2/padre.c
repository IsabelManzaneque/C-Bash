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



void init_sem(int sem, int value){
    if (semctl(sem, 0, SETVAL, value) == -1) {
        perror("padre: semctl");
        exit(1);
    }
}

void wait_sem(int sem){
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;  
    op.sem_flg = 0;
    if(semop(sem, &op, 1) == -1){
        perror("padre: semop");
        exit(1);
    }
    
}

void signal_sem(int sem){
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;  
    op.sem_flg = 0;
    if(semop(sem, &op, 1) == -1){
        perror("padre: semop");
        exit(1);
    }
}

void nHijos(int N, char argv0[], pid_t *lista, int barrera[2]){
    
    // crea N procesos hijos que ejecutaran HIJO
    int pidCounter = 0; 
    for(int i = 0; i < N; i++){

        pid_t resFork = fork();
        
	if(resFork == -1){
            perror("fork");
            exit(-1);
        }else if(resFork == 0){	   	    
            // Proceso Hijo
            // Cierra la salida estándar y duplica extremo de lectura de barrera
            //close(1); 
            //dup(barrera[0]);
            //close(barrera[0]);
            //close(barrera[1]);
            char pipeString[10];
            sprintf(pipeString, "%d", barrera[0]);            
            execl("./Trabajo2/HIJO", "HIJO", argv0, pipeString, NULL); 
        }else{
            // Proceso Padre     
            lista[pidCounter] = resFork;
            pidCounter++;
            
        }	   
    }
}

void actualizarLista(int *childCounter, pid_t *lista, int sem){    
    
    size_t i = 0;
    while (i < *childCounter) {
        if (kill(lista[i], 0) != 0) {
            // El proceso está muerto, eliminarlo de la lista
            printf("proceso %d esta muerto\n", lista[i]);
            wait_sem(sem);
            for (size_t j = i; j < *childCounter - 1; j++) {
                lista[j] = lista[j + 1];
            }
            signal_sem(sem);
            (*childCounter)--;
        } else {
            // El proceso está vivo, pasar al siguiente
            printf("proceso %d esta vivo\n", lista[i]);
            i++;
        }
    }    
}

void matarProceso(int index, pid_t *lista) {
    if (kill(lista[index], SIGTERM) == 0) {
        printf("Señal SIGTERM enviada a %d\n", lista[index]);
	// Esperar a que el proceso hijo termine
        int estado;
        pid_t pidTerminado = waitpid(lista[index], &estado, 0);
	if (pidTerminado == -1) {
            perror("padre: waitpid");
        }
    }else{
        perror("Error al enviar señal SIGTERM");           
    }
}

int main(int argc, char *argv[]){
    
    // ------------- INICIALIZACION --------------
    int N, childCounter;
    N = childCounter = atoi(argv[1]);

    // crea clave asociada a fichero ejecutable y letra 
    key_t key = ftok(argv[0],'X');

    // crear cola de mensajes "mensajes"
    int mensajes = msgget(key, IPC_CREAT | 0600);
 
    // crear región de memoria compartida "lista" de tamano N pids
    // y enlazararla con un array con capacidad para N PIDs 
    int shrdMemId = shmget(key, N*sizeof(pid_t), IPC_CREAT | 0600);      
    pid_t *lista = (pid_t*)shmat(shrdMemId, NULL, 0);

    // crea semáforo para proteger acceso a "lista" 
    // lo iniciializa a 1 para exclusion mutua
    int sem = semget(key, 1, IPC_CREAT | 0600);  
    init_sem(sem, 1);

    // crear tuberia sin nombre "barrera"
    int barrera[2];
    if(pipe(barrera) == -1){
        perror("padre: pipe");
        exit(-1);
    }
    
    // crea N procesos 
    nHijos(N, argv[0], lista, barrera);
    
    
    // ------------- RONDAS --------------
    
    // mientras queden 2 o mas contendientes, se hara otra ronda 
    //while (childCounter > 1){

        // actualiza la lista de procesos
        actualizarLista(&childCounter, lista, sem);
        
        printf("Iniciando ronda de ataques\n");
        // manda mensaje de tantos bytes como hijos queden vivos
        char msg[childCounter];
        write(barrera[1], msg, sizeof(msg));
   
    //}
    
 
    // ------------- LIBERAR RECURSOS IPC --------------
   
    // Desvincular memoria compartida
    //shmdt();

    wait(0);
    printf("Finalizando padre\n");       
    return 0;

}
