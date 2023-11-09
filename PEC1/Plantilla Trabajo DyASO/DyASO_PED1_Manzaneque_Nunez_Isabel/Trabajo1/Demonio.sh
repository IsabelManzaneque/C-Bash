#!/bin/bash

#Demonio Dummie, tenéis que completarlo para que haga algo

#Bucle mientras que no llegue el apocalipsis
#   -Espera un segundo
#   -Lee las listas y revive los procesos cuando sea necaario dejando entradas en la biblia
#   	-Puede usar todos los ficheros temporales que quiera pero luego en el Apocalipsis hay que borrarlos
#   	-Hay que usar un lock para no acceder a las listas a la vez que Fausto
#   	-Ojo al cerrar los proceos, hay que terminar el arbol completo no sólo uno de ellos
#Fin bucle

#---------------------------------------------------
# SanPedro es como una llave, si Demonio hace flock SanPedro porque va a escribir algo y luego Fausto hace flock SanPedro antes de escribir algo, Fausto no podra bloquear SanPedro antes que demonio lo suelte. Anadir en lecturas y escrituras de fausto la orden flock san pedro. En demonio no hacerlo al empezar el while o se bloquea para siempre, solo atomico en lecturas y escrituras

#En SanPedro no se escribe nada. Cuando vaya a ejecutar algo que escribe en un fichero, que puede sobreescribir otro, usar flock SanPedro xxxx. El flock se libera solo cuando acaba la accion que lo ha bloqueado
# flock [options] file|directory command [arguments] -> si le pasas algo que lleva argumentos flock SanPedro script param1 param2
# flock [options] file|directory -c command


hora=$(date +%H:%M:%S)
archivos=("procesos" "procesos_servicio" "procesos_periodicos")

# Bucle que se ejecuta mientras no se detecte el fichero apocalipsis
# -f comprueba si un file existe en el directorio del scrip

while [ ! -f "Apocalipsis" ]
do
       
    # Itera por las listas realizando acciones
    for archivo in "${archivos[@]}"
    do
        n=1
        if [ "$archivo" == "procesos_periodicos" ] 
        then
	    n=3
        fi
    
    
        cat "$archivo" | while read line
        do
            pid=$(echo $line | awk -v N=$n '{print $N}')

            # el proceso esta en el infierno, todos realizan la misma accion
            if [ -e "./Infierno/$pid" ]           
            then    

                # terminar arbol del proceso
                arbolProcesos=$(pstree -p "$pid" | grep -o '[0-9]\+')
                for proceso in $arbolProcesos
                do	            
                    kill -15 "$proceso"
                done
                # eliminar entrada de la lista y fichero del infierno       
	        flock SanPedro -c "sed -i \"\~$line~d\" \"$archivo\""
	        flock SanPedro -c "rm -f \"./Infierno/$pid\""
                flock SanPedro -c "echo \"$hora El proceso $line ha terminado\" >> ./Biblia.txt"
                
            # el proceso no esta en el infierno, acciones distintas
	    else
               
                # PROCESOS -----------------------------------------------------
                # si no se esta ejecutando lo elimina de la lista
                if ! kill -0 "$pid" >/dev/null && [ "$archivo" == "procesos" ] 
                then         
                    flock SanPedro -c "sed -i \"\~$line~d\" \"$archivo\""	
                    flock SanPedro -c "echo \"$hora El proceso $line ha terminado\" >> ./Biblia.txt"  
                fi
	
                # PROCESOS SERVICIO -------------------------------------------- 
                # si no se esta ejecutando lo resucita
                if ! kill -0 "$pid" >/dev/null && [ "$archivo" == "procesos_servicio" ] 
	        then

                    # ejecuta el comando
		    comandoProceso=$(echo "$line" | grep -o "'.*'" | sed "s/'//g")
	            bash -c "$comandoProceso" &
                    pidNuevo="$!"

                    # sustituye el pid
		
                    flock SanPedro -c "sed -i -e \"s/$pid/$pidNuevo/g\" \"$archivo\""		
                    flock SanPedro -c "echo \"$hora El proceso $line resucita con el pid $pidNuevo\" >> ./Biblia.txt"             
	        fi	       

	        # PROCESOS PERIODICOS -------------------------------------------
                if [ "$archivo" == "procesos_periodicos" ] 
                then   
                    vectorLine=($line)                                      
                                        
                    # si el proceso no se esta ejecutando y el contador es mayor o igual al periodo         
	            if ! kill -0 "$pid" >/dev/null && [ "${vectorLine[0]}" -ge "${vectorLine[1]}" ]
	            then
		        
		        # volver a lanzar el proceso
		        comandoProceso=$(echo "$line" | grep -o "'.*'" | sed "s/'//g")
		        bash -c "$comandoProceso" &
			echo "comandoproceso: $comandoProceso"
		        #poner contador a 0 y sustituye el pid                                        
                        vectorLine[0]=0
                        newLine="${vectorLine[*]}"
                        pidNuevo="$!"
			
			flock SanPedro -c "{
                            sed -i \"s~$line~$newLine~g\" \"$archivo\"
                            sed -i \"s~$pid~$pidNuevo~g\" \"$archivo\"                           
                        }"                     

                        flock SanPedro -c "echo \"$hora El proceso $pid '$comandoProceso' se ha reencarnado en el pid $pidNuevo\" >> ./Biblia.txt"
	            
		    else		
		        # incrementar contador     	    
                        ((vectorLine[0]++))                      
                        newLine="${vectorLine[*]}"		        
                        flock SanPedro -c "sed -i \"s~$line~$newLine~g\" \"$archivo\""              
	            fi			    
	        fi
            fi
        done
    done   

    sleep 1
      
done



# llega el Apocalipsis
flock SanPedro -c "echo \"$hora ---------------Apocalipsis---------------\" >> ./Biblia.txt"

# terminar todos los procesos de todas las listas

for archivo in "${archivos[@]}"
do  
    n=1
    if [ "$archivo" == "procesos_periodicos" ] 
    then
	n=3
    fi
    # Itera por cada línea del archivo y obtiene el PID (primera o 
    # tercera palabra de cada linea en funcion del archivo)
    cat "$archivo" | while read line
    do
       pid=$(echo $line | awk -v N=$n '{print $N}')
       
       # si el proceso esta en ejecucion, lo termina
       if kill -0 "$pid" >/dev/null 
       then
           flock SanPedro -c "echo \"$hora El proceso $line ha terminado\" >> ./Biblia.txt"
           kill "$pid"       
       fi
    done	
done


# borrar listas, Apocalipsis e Infierno 
rm -f procesos procesos_servicio procesos_periodicos Apocalipsis SanPedro 
rm -fr Infierno

# Termina su ejecucion
echo \"$hora Se acabo el mundo.\" >> ./Biblia.txt





