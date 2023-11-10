#!/bin/bash



archivos=("procesos" "procesos_servicio" "procesos_periodicos")

# BUCLE PRINCIPAL
while [ ! -f "Apocalipsis" ]
do

       
    # Itera por cada lista
    for archivo in "${archivos[@]}"
    do
        n=1
        if [ "$archivo" == "procesos_periodicos" ] 
        then
	    n=3
        fi
    
        # Itera por cada entrada de la lista
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
	        rm -f "./Infierno/$pid"
                flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $line ha terminado\" >> ./Biblia.txt"
                
            # el proceso no esta en el infierno, acciones distintas
	    else
               
                # Procesos: si no se esta ejecutando lo elimina de la lista
                if ! kill -0 "$pid" >/dev/null && [ "$archivo" == "procesos" ] 
                then         
                    flock SanPedro -c "sed -i \"\~$line~d\" \"$archivo\""	
                    flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $line ha terminado\" >> ./Biblia.txt"  
                fi
	
                # Procesos Servicio: si no se esta ejecutando lo resucita
                if ! kill -0 "$pid" >/dev/null && [ "$archivo" == "procesos_servicio" ] 
	        then

                    # ejecuta el comando
		    comandoProceso=$(echo "$line" | grep -o "'.*'" | sed "s/'//g")
	            bash -c "$comandoProceso" &
                    pidNuevo="$!"

                    # sustituye el pid		
                    flock SanPedro -c "sed -i -e \"s/$pid/$pidNuevo/g\" \"$archivo\""		
                    flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $line resucita con el pid $pidNuevo\" >> ./Biblia.txt"             
	        fi	       

	        # Procesos periodicos 
                if [ "$archivo" == "procesos_periodicos" ] 
                then   
		    # incrementar contador 
                    vectorLine=($line)                                      
                    ((vectorLine[0]++))                    
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

                        flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $pid '$comandoProceso' se ha reencarnado en el pid $pidNuevo\" >> ./Biblia.txt"
	            
		    else		                    
                        newLine="${vectorLine[*]}"		        
                        flock SanPedro -c "sed -i \"s~$line~$newLine~g\" \"$archivo\""              
	            fi			    
	        fi
            fi
        done
    done   

    sleep 1
      
done



# APOCALIPSIS
flock SanPedro -c "echo \"$(date +%H:%M:%S) ---------------Apocalipsis---------------\" >> ./Biblia.txt"

# terminar todos los procesos de todas las listas
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
       
       # si el proceso esta en ejecucion, lo termina
       if kill -0 "$pid" >/dev/null 
       then
           arbolProcesos=$(pstree -p "$pid" | grep -o '[0-9]\+')
           for proceso in $arbolProcesos
           do
               kill -15 "$proceso"
           done     
       fi      

       flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $line ha terminado\" >> ./Biblia.txt"
    done	
done


# borra todo menos Fausto, Demonio y Biblia 
rm -f procesos procesos_servicio procesos_periodicos Apocalipsis SanPedro 
rm -fr Infierno
echo "$(date +%H:%M:%S) Se acabo el mundo." >> ./Biblia.txt





