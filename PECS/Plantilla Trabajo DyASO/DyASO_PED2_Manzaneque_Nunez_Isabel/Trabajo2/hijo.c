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

char estado[100];
// semilla para numeros aleatorios


void indefenso(){
    printf("\nEl hijo %d ha sido emboscado mientras realizaba un ataque\n", getpid());
    strcpy(estado, "KO");   	
}

void defensa(){
    printf("\nEl hijo %d ha repelido un ataque\n", getpid());
    strcpy(estado, "OK");    
}

void ataque(pid_t *lista){
    
     // elegirán aleatoriamente un PID diferente de 0 de la lista (menos el suyo)
     srand((unsigned int)time(NULL));
     pid_t pidVictima = 0;
     int ranIndx = rand() % 10;
     
     while(pidVictima == 0 || pidVictima == getpid()){
         pidVictima = lista[ranIndx];
     }
     
     printf("\nAtacando al proceso %d\n", pidVictima);

     if (kill(pidVictima, SIGUSR1) == 0) {
	// Esperar a que el proceso hijo termine
        int estado;
        pid_t pidTerminado = waitpid(pidVictima, &estado, 0);
	if (pidTerminado == -1) {
            perror("Hijo: waitpid");
        }
    }else{
        perror("Hijo: ataque");           
    }
 
}



int main(int argc, char const *argv[]) {
   
    printf("\n------------ Inicio hijo ------------\n");
    
    // ------------- INICIALIZACION --------------
    // Recuperar la clave
    key_t key = ftok(argv[1],'X');
    
    // Recuperar descriptor de lectura
    int descLectura = atoi(argv[2]);
   
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
     
    
    char buffer[100];

    while(read(descLectura, buffer, sizeof(buffer)) > 0){       
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
            ataque(lista); 
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
        
        sleep(1);
    }
   
 
    
    
 //   char texto[20];
 //   strcpy(texto, argv[0]);
 //   printf("arg = %s\n\n",texto);

 //   for (int i=1;i<=5;i++){
 //       printf("%s --> %d\n",texto,i);
 //       sleep(1);
 //   }
    
    printf("-------------- Fin hijo -------------\n\n");
    exit(0);



}
