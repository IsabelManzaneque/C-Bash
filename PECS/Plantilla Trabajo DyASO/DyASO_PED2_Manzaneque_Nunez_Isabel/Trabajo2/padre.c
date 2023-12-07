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
#include <fcntl.h>
#include <unistd.h>


struct mensaje{
    long tipo;
    pid_t pid;
    char state[3]; 
};

/**
 * Inicializa un semaforo a un valor
 */
void initSem(int sem, int value);

/**
 * Decrementa en 1 el valor de un semaforo
 */
void waitSem(int sem);

/**
 * Incrementa en 1 el valor de un semaforo
 */
void signalSem(int sem);

/**
 * Crea N procesos hijo utilizando fork
 */
void crearNHijos(int N, char argv0[], pid_t *lista, int barrera[2], int sem);

/**
 * Mata un proceso mandandole la senal SIGTERM 
 */
void matarProceso(int *K, pid_t pid, pid_t *lista, int sem);


int main(int argc, char *argv[]){
    
    // ------------- INICIALIZACION --------------
    int N, K;
    N = K = atoi(argv[1]);
    struct mensaje msgHijo;

    // crear clave 
    key_t key = ftok(argv[0],'X');

    // crear cola de mensajes 
    int mensajes = msgget(key, IPC_CREAT | 0600);    
        if (mensajes == -1) {
        perror("Hijo: msgget");
        exit(-1);
    }
        
    // crear región de memoria compartida de tamano N pids
    // y enlazararla con un array con capacidad para N PIDs 
    int shrdMemId = shmget(key, N*sizeof(pid_t), IPC_CREAT | 0600);      
    pid_t *lista = (pid_t*)shmat(shrdMemId, NULL, 0);

    // crea semáforo para proteger acceso a memoria compartida
    // inicializar a 1 para exclusion mutua
    int sem = semget(key, 1, IPC_CREAT | 0600);  
    initSem(sem, 1);

    // crear tuberia sin nombre 
    int barrera[2];
    if(pipe(barrera) == -1){
        perror("padre: pipe");
        exit(-1);
    }    

    // crea N procesos y espera a que se inicien todos
    crearNHijos(N, argv[0], lista, barrera, sem);
    usleep(20000);

    
    
    // ------------- RONDAS --------------
    
    // mientras queden 2 o mas contendientes, se hara otra ronda     
    while (K > 1){               
        
        printf("\n ------ Iniciando ronda de ataques ------\n");
        printf("Quedan %d hijos vivos\n", K);
        for(int i = 0; i < 10; i++){
            waitSem(sem);
	    printf("Hijo %d: %d\n", i, lista[i]);
            signalSem(sem);                      
        }
        fflush(stdout); 

        // manda un mensaje de 1 byte por cada hijo vivo
	char msg = 'P';
	for (int i = 0; i < K; i++) {	    	   
	    if(write(barrera[1], &msg, sizeof(msg)) < 0){
                perror("padre: write");
            }
	    printf("padre enviado mensaje\n");	
	}
	// espera a que los hijos reciban mensaje y terminen sus rondas
        usleep(500000);	


	// Padre recibe los resultados de los hijos       
	int counter = K;
        while(msgrcv(mensajes, &msgHijo, sizeof(struct mensaje) - sizeof(long), 2, 0) != -1) {
                      
            // Imprimir el mensaje recibido
           printf("Counter: %d -- Padre %d recibe de %d: Tipo: %ld - Estado: %s\n", counter, getpid(), msgHijo.pid, msgHijo.tipo, msgHijo.state);
           
  	
            if(strcmp("KO", msgHijo.state) == 0){                       
                matarProceso(&K, msgHijo.pid, lista, sem);    
            }            
	    
	    // Si ya ha recibido mensajes de todos los hijos sale del bucle
            counter --;
            if(counter == 0 ){
                break;
            }           
        }
    }
    
    // Escribir en resultado el ganador de la ronda
    char ganador[100]; 
    if(K == 1){ 
       // 1 ganador
       for(int i = 0; i < 10; i++){   
       
            waitSem(sem);
	    if(lista[i] != 0){     
                int resultado = open("resultado", O_WRONLY);
                snprintf(ganador, sizeof(ganador), "\n\nEl hijo %d ha ganado\n\n", lista[i]);
                write(resultado, ganador, strlen(ganador));
            }            
            signalSem(sem);
        }
    }else if(K == 0){
        // empate	
        strcpy(ganador, "\n\nEmpate\n\n");
        int resultado = open("resultado", O_WRONLY);        
        write(resultado, ganador, strlen(ganador));
    }
    
    

    // ------------- LIBERAR RECURSOS IPC --------------

    printf("\nsemid : %d - msqid: %d\n", sem, mensajes);

    // cerrar cola de mensajes   
    msgctl(mensajes, IPC_RMID,0);

    // cerrar tuberia
    close(barrera[0]);
    close(barrera[1]); 

    // Desvincular memoria compartida
    shmdt(lista);
    shmctl(shrdMemId, IPC_RMID,0);

    // cerrar semaforo
    semctl(sem, IPC_RMID,0);

    //Mostramos semáforos y colas de mensajes activos
    printf ("\nRecursos IPC activos:\n");
    system("ipcs -sq");
    
    printf("Finalizando padre\n");    
    fflush(stdout);    
    return 0;

}


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


void crearNHijos(int N, char argv0[], pid_t *lista, int barrera[2], int sem){    
    
    int pidCounter = 0; 

    for(int i = 0; i < N; i++){

        pid_t resFork = fork();
        
	if(resFork == -1){
            perror("padre: fork");
            exit(-1);
        }else if(resFork == 0){	   	    
            // Proceso Hijo ejecuta HIJO          
            char lectura[10];
            char escritura[10];
            sprintf(lectura, "%d", barrera[0]);      
            sprintf(escritura, "%d", barrera[1]);      
            execl("./Trabajo2/HIJO", "HIJO", argv0, lectura, escritura, NULL); 
        }else{
            // Proceso Padre va guardando los pids de los hijos en lista
            waitSem(sem);	
            lista[pidCounter] = resFork;
            signalSem(sem);
            pidCounter++;            
        }	   
    }
}


void matarProceso(int *K, pid_t pid, pid_t *lista, int sem) {
    if (kill(pid, SIGTERM) == 0) {        
	// Esperar a que el proceso hijo termine
        waitpid(pid, 0, 0);                
    }else{
        perror("padre: sigterm");
    }
  
    // actualizar lista    
    for(int i = 0; i < 10; i++){
        waitSem(sem);
	if(lista[i] == pid){ 
            lista[i] = 0;
            (*K)--;          
        }
        signalSem(sem);
    }       
}

