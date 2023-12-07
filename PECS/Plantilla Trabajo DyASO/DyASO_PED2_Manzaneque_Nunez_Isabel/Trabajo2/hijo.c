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
 * Manejador de la senal SIGUSR1 cuando un proceso esta indefenso
 */
void indefenso();

/**
 * Manejador de la senal SIGUSR1 cuando un proceso se defiende
 */
void defensa();

/**
 * Ataca a un proceso aleatorio de la lista de procesos
 */
void ataque(pid_t *lista, int sem);

/**
 * Cuando el proceso recibe la SIGUSR1, realizara la accion del 
 * manejador de defensa o indefenso aleatoriamente
 */
int preparacion();

/**
 * Prepara el mensaje con el resultado del ataque y
 * se lo manda al padre
 */
void enviarMensajeAPadre(int mensajes);


int main(int argc, char const *argv[]) {
    
    // ------------- INICIALIZACION --------------   
    printf("Inicio hijo %d\n", getpid());
    
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
    // bucle principal del hijo
    while(1){
        char buffer[1];
        if(read(descLectura, buffer, sizeof(buffer)) > 0){       
	    printf("hijo %d, mensaje recibido: %s\n", getpid(), buffer);                        
            // si la la ronda de preparacion devuelve 0, ataca
	    if(preparacion() == 0){	         
                ataque(lista, sem);                 
            }
	    
	    // envia resultado de la ronda a padre
  	    enviarMensajeAPadre(mensajes);	    
        } 
        sleep(1);	
    } 
    close(descLectura);       
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
          
     srand((unsigned int)time(NULL));
     pid_t pidVictima = 0;
     
     // Escoge un proceso cuyo pid no sea 0 ni el propio
     while(pidVictima == 0 || pidVictima == getpid()){
         int ranIndx = rand() % 10;
	 waitSem(sem);
         pidVictima = lista[ranIndx];
         signalSem(sem);
     }   
     // envia SIGUSR1 al proceso victima
     if (kill(pidVictima, SIGUSR1) == 0) {
	printf("\n%d Atacando al proceso %d\n", getpid(), pidVictima);      
        fflush(stdout);     
     }else{
        perror("Hijo: ataque");           
     }
     // Espera a que el proceso victima termine
     usleep(100000);
}


int preparacion(){
    
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

