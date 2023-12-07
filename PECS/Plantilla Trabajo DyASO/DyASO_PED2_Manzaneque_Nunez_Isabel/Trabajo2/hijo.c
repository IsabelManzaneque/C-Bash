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
     /*
     El proceso elige aleatoriamente un pid distinto de 0 y del propio
     y lo mata enviandole la senal SIGUSR1
     */
     
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
     usleep(100000);
}

int preparacion(){
    /*
     Cuando el proceso recibe la SIGUSR1, realizara la accion del manejador
    */
    srand((unsigned int)time(NULL));      
    strcpy(estado, "OK");

    if(rand() % 2 == 0){
        // proceso ataca        
        if(signal(SIGUSR1,indefenso)==SIG_ERR){
            perror("Hijo: indefenso");
            exit(1);
        } 
        usleep(100000);          
        return 0;
    }else{
        // proceso defiende       
        if(signal(SIGUSR1,defensa)==SIG_ERR){
            perror("Hijo: defensa");
            exit(1);
        } 
        usleep(200000);      
    } 
    return 1; 
}


struct mensaje{
    long tipo;
    pid_t pid;
    char state[3]; 
};

void enviarMensajeAPadre(int mensajes){

    // Configurar el tipo de mensaje, el PID y la cadena
    struct mensaje msg;
    msg.tipo = 2;  
    msg.pid = getpid();
    strcpy(msg.state, estado);

    printf("Hijo %d enviando: Tipo: %ld - Estado: %s\n", msg.pid, msg.tipo, msg.state);
           
    // Enviar el mensaje a la cola
    if (msgsnd(mensajes, &msg, sizeof(struct mensaje) - sizeof(long), 0) == -1) {
        perror("Hijo: msgsnd");
        exit(-1);
    }
}


int main(int argc, char const *argv[]) {
    
    pid_t pid = getpid();
    printf("Inicio hijo %d\n", pid);
    
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
     
    
    
    close(descEscritura);
    while(1){
        char buffer[1];
        if(read(descLectura, buffer, sizeof(buffer)) > 0){       
	    printf("hijo %d, mensaje recibido: %s\n", pid, buffer);            
             
	    if(preparacion() == 0){	         
                ataque(lista, sem);                 
            }
	 
  	    enviarMensajeAPadre(mensajes);	    
        } 
        sleep(1);	
    } 

    close(descLectura);
    

    printf("finalizando hijo %d\n", pid);
    exit(0);



}
