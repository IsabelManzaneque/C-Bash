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
#include <errno.h>

struct mensaje{
    long tipo;
    pid_t pid;
    char state[3]; 
};

void initSem(int sem, int value){
    if (semctl(sem, 0, SETVAL, value) == -1) {
        perror("padre: semctl");
        exit(1);
    }
}

void waitSem(int sem){
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;  
    op.sem_flg = 0;
    if(semop(sem, &op, 1) == -1){
        perror("padre: semop");
        exit(1);
    }
    
}

void signalSem(int sem){
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;  
    op.sem_flg = 0;
    if(semop(sem, &op, 1) == -1){
        perror("padre: semop");
        exit(1);
    }
}

void nHijos(int N, char argv0[], pid_t *lista, int barrera[2], int sem){
    
    // crea N procesos hijos que ejecutaran HIJO
    int pidCounter = 0; 
    for(int i = 0; i < N; i++){

        pid_t resFork = fork();
        
	if(resFork == -1){
            perror("fork");
            exit(-1);
        }else if(resFork == 0){	   	    
            // Proceso Hijo
            close(barrera[1]);
            char lectura[10];
            char escritura[10];
            sprintf(lectura, "%d", barrera[0]);      
            sprintf(escritura, "%d", barrera[1]);      
            execl("./Trabajo2/HIJO", "HIJO", argv0, lectura, escritura, NULL); 
        }else{
            // Proceso Padre   
            waitSem(sem);
            lista[pidCounter] = resFork;
            pidCounter++;
	    signalSem(sem);
            
        }	   
    }
}

void matarProceso(int *K, pid_t pid, pid_t *lista) {
    if (kill(pid, SIGTERM) == 0) {
        printf("Señal SIGTERM enviada a %d\n", pid);
	// Esperar a que el proceso hijo termine
        int estado;
        wait(&estado);	         
    }else{
        perror("padre: sigterm");
    }
  
    // actualizar lista
    for(int i = 0; i < 10; i++){
	if(lista[i] == pid){ 
            lista[i] = 0;
            (*K)--;          
        }
    }   
}
/*
void actualizarLista(int *K, pid_t *lista, int sem){    
    
    size_t i = 0;
    while (i < *K) {
        if (kill(lista[i], 0) != 0) {
            // El proceso está muerto, eliminarlo de la lista
            printf("proceso %d esta muerto\n", lista[i]);
            fflush(stdout); 
            waitSem(sem);
            lista[i] = 0;
            signalSem(sem);
            (*K)--;
        } else {
            // El proceso está vivo, pasar al siguiente
            printf("proceso %d esta vivo\n", lista[i]);
            fflush(stdout); 
            i++;
        }
    }    
}

*/

int main(int argc, char *argv[]){
    
    // ------------- INICIALIZACION --------------
    int N, K;
    N = K = atoi(argv[1]);
    struct mensaje msgHijo;

    // crea clave asociada a fichero ejecutable y letra 
    key_t key = ftok(argv[0],'X');

    // crear cola de mensajes "mensajes"
    int mensajes = msgget(key, IPC_CREAT | 0600);
        if (mensajes == -1) {
        perror("Hijo: msgget");
        exit(-1);
    }
    // crear región de memoria compartida "lista" de tamano N pids
    // y enlazararla con un array con capacidad para N PIDs 
    int shrdMemId = shmget(key, N*sizeof(pid_t), IPC_CREAT | 0600);      
    pid_t *lista = (pid_t*)shmat(shrdMemId, NULL, 0);

    // crea semáforo para proteger acceso a "lista" 
    // lo iniciializa a 1 para exclusion mutua
    int sem = semget(key, 1, IPC_CREAT | 0600);  
    initSem(sem, 1);

    // crear tuberia sin nombre "barrera"
    int barrera[2];
    if(pipe(barrera) == -1){
        perror("padre: pipe");
        exit(-1);
    }
    
    // crea N procesos 
    nHijos(N, argv[0], lista, barrera, sem);
    
    
    // ------------- RONDAS --------------
    
    // mientras queden 2 o mas contendientes, se hara otra ronda 
        
    //while (K > 1){
	
	printf("\n ------ Hijos vivos: %d ------\n", K);
        fflush(stdout); 
        for(int i = 0; i < 10; i++){
            waitSem(sem);
	    printf("Hijo %d: %d\n", i, lista[i]);
            fflush(stdout);
            signalSem(sem);
        }
       
        
        printf("\n ------ Iniciando ronda de ataques ------\n");
        fflush(stdout); 

        // manda un mensaje de 1 byte K veces 

	for (int i = 0; i < K; i++) {
	    char msg[1];
	    msg[0] = 'P';  
	    write(barrera[1], msg, sizeof(msg));
	    usleep(10000);
	}
	// cieraa extremo escritura del padre
        close(barrera[1]);  

	// Esperar a que todos los hijos terminen
	for (int i = 0; i < K; i++) {
	    wait(NULL);
	}

	// Padre recibe los mensajes de los hijos           
	/*
        while(msgrcv(mensajes, &msgHijo, sizeof(struct mensaje) - sizeof(long), 1, 0) != -1) {
        	
            // Imprimir el mensaje recibido
            printf("Padre %d recibió un mensaje del Hijo %d:\n", getpid(), msgHijo.pid);
            fflush(stdout); 
            printf("Tipo: %ld - Estado: %s\n\n", msgHijo.tipo, msgHijo.state);            
            fflush(stdout); 
  	
            if(strcmp("KO", msgHijo.state) == 0){    
                printf("Matando %d\n", msgHijo.pid);
                fflush(stdout);  
                waitSem(sem);              
                matarProceso(&K, msgHijo.pid,lista);
	        signalSem(sem);
            }            

            sleep(1);
        }*/
        
    //}
    
   
    close(barrera[0]);
    
    

    // ------------- LIBERAR RECURSOS IPC --------------
   
    // Desvincular memoria compartida
    //shmdt();
    // borrar cola de mensajes
    // msgctl(mensajes, IPC_RMID,0);

    wait(0);
    printf("Finalizando padre\n");    
    fflush(stdout);    
    return 0;

}
