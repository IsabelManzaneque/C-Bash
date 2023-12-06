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
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

char estado[3];

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

void indefenso(){
    printf("\nEl hijo %d ha sido emboscado mientras realizaba un ataque\n", getpid());
    fflush(stdout); 
    strcpy(estado, "KO");   	
}

void defensa(){
    printf("\nEl hijo %d ha repelido un ataque\n", getpid());
    fflush(stdout); 
    strcpy(estado, "OK");    
}

void ataque(pid_t *lista, int sem){
    
     // elegir aleatoriamente un PID que no sea 0 ni el propio
     srand((unsigned int)time(NULL));
     pid_t pidVictima = 0;
          
     while(pidVictima == 0 || pidVictima == getpid()){
         int ranIndx = rand() % 10;
	 waitSem(sem);
         pidVictima = lista[ranIndx];
         signalSem(sem);
     }   
     // envia SIGUSR1 a victima
     if (kill(pidVictima, SIGUSR1) == 0) {
	printf("\n%d Atacando al proceso %d\n", getpid(), pidVictima);      
        fflush(stdout);     
     }else{
        perror("Hijo: ataque");           
     }
 
}


struct mensaje{
    long tipo;
    pid_t pid;
    char state[3]; 
};

int main(int argc, char const *argv[]) {
    
    pid_t pid = getpid();
    //printf("Inicio hijo %d\n", pid);
    
    // ------------- INICIALIZACION --------------
    // Recuperar la clave
    key_t key = ftok(argv[1],'X');
    
    // Recuperar descriptores
    int descLectura = atoi(argv[2]);
    int descEscritura = atoi(argv[3]);
   
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
    pid_t *lista = (pid_t*)shmat(shrdMemId, NULL, 0);

    
    // ------------- RONDAS --------------
     
    
    char buffer[10];
    close(descEscritura);

    while(read(descLectura, buffer, sizeof(buffer)) > 0){       
	printf("hijo %d, mensaje recibido: %s\n", pid, buffer);
        // Hay un mensaje en la tuberia
        srand((unsigned int)time(NULL));
        int ranNum = rand() % 2;        

        if(ranNum == 0){
            // proceso ataca
	    // cuando el proceso reciba SIGUSR1 la acción que realiza el núcleo es ejecutar indefenso
            if(signal(SIGUSR1,indefenso)==SIG_ERR){
                perror("Hijo: indefenso");
	        exit(1);
            } 
            usleep(100000);   
            ataque(lista, sem); 
            usleep(100000);  
       
        }else{
            // proceso defiende
            // cuando el proceso reciba SIGUSR1 la acción que realiza el núcleo es ejecutar defensa
            if(signal(SIGUSR1,defensa)==SIG_ERR){
                perror("Hijo: defensa");
	        exit(1);
            } 
            usleep(200000);      
        }   
	
	// Configurar el tipo de mensaje, el PID y la cadena
        struct mensaje msg;
        msg.tipo = 1;  
        msg.pid = pid;
        strcpy(msg.state, estado);
        
	    printf("Hijo %d enviando\n", pid);
            fflush(stdout); 
            printf("Tipo: %ld - Estado: %s\n\n", msg.tipo, msg.state);            
            fflush(stdout); 
        // Enviar el mensaje a la cola
        if (msgsnd(mensajes, &msg, sizeof(struct mensaje) - sizeof(long), 0) == -1) {
            perror("Hijo: msgsnd");
            exit(-1);
        }

	 
        sleep(1);
	
    }   
    close(descLectura);
    

    printf("finalizando hijo %d\n", pid);
    exit(0);



}
