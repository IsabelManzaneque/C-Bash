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

hora=$(date +%H:%M:%S)
archivos=("procesos" "procesos_servicio" "procesos_periodicos")

# Bucle que se ejecuta mientras no se detecte el fichero apocalipsis
# -f comprueba si un file existe en el directorio del scrip

echo "Demonio: comenzando..."
while [ ! -f "Apocalipsis" ]
do
       
    # Itera por las listas realizando acciones
    for archivo in "${archivos[@]}"
    do
	# PROCESOS
        if [ "$archivo" == "procesos" ] 
	then
	    cat "$archivo" | while read line
            do
   	        pid=$(echo $line | awk -v N=1 '{print $N}')
		
		if [ -e "./Infierno/$pid" ]
                # el proceso esta en el infierno
                then    
                    # terminar arbol del proceso
		    arbolProcesos=$(pstree -p "$pid" | grep -o '[0-9]\+')
                    for proceso in $arbolProcesos
                    do
                        kill -15 "$proceso"
                    done
                    # eliminar entrada de la lista
	            sed -i "/$line/d" "$archivo"
                    # eliminar fichero del infierno
		    rm -f "./Infierno/$pid"
                    echo "$hora El proceso $line ha terminado" >> ./Biblia.txt
	        else
		# el proceso NO esta en el infierno  		    
	            if ! kill -0 "$pid" >/dev/null 
	            then
		        # si el proceso no se esta ejecutando lo elimina de la lista
			sed -i "/$line/d" "$archivo"	
                        echo "$hora El proceso $line ha terminado" >> ./Biblia.txt              
	            fi		    
	        fi
            done	    
	fi
        
	# PROCESOS SERVICIO
        if [ "$archivo" == "procesos_servicio" ] 
	then
	    cat "$archivo" | while read line
            do
   	        pid=$(echo $line | awk -v N=1 '{print $N}')
		
		if [ -e "./Infierno/$pid" ]
                # el proceso esta en el infierno
                then    
                    # terminar arbol del proceso
		    arbolProcesos=$(pstree -p "$pid" | grep -o '[0-9]\+')
                    for proceso in $arbolProcesos
                    do
                        kill -15 "$proceso"
                    done
                    # eliminar entrada de la lista
	            sed -i "/$line/d" "$archivo"
                    # eliminar fichero del infierno
		    rm -f "./Infierno/$pid"
                    echo "$hora El proceso $line ha terminado" >> ./Biblia.txt

	        else
		# el proceso NO esta en el infierno  		    
	            if ! kill -0 "$pid" >/dev/null 
	            then
		        # si el proceso no se esta ejecutando lo resucita
                        # ejecuta el comando
			comandoProceso=$(echo "$line" | grep -o "'.*'" | sed "s/'//g")
		        bash -c "$comandoProceso" &
                        # sustituye el pid
			pidNuevo="$!"
                        sed -i -e "s/$pid/$pidNuevo/g" ./procesos_servicio
			
                        echo "$hora El proceso $line resucita con el pid $pidNuevo" >> ./Biblia.txt              
	            fi	
	        fi		
            done
	fi

	# PROCESOS PERIODICOS
        if [ "$archivo" == "procesos_periodicos" ] 
	then
	    cat "$archivo" | while read line
            do
   	        pid=$(echo $line | awk -v N=3 '{print $N}')
                
	        if [ -e "./Infierno/$pid" ]
                # el proceso esta en el infierno
                then    
                    # terminar arbol del proceso
		    arbolProcesos=$(pstree -p "$pid" | grep -o '[0-9]\+')
                    for proceso in $arbolProcesos
                    do
                        kill -15 "$proceso"
                    done
                    # eliminar entrada de la lista
	            sed -i "/$line/d" "$archivo"
                    # eliminar fichero del infierno
		    rm -f "./Infierno/$pid"
                    echo "$hora El proceso $line ha terminado" >> ./Biblia.txt

	        else
		# el proceso NO esta en el infierno  	                    
		   
		    vectorLine=($line)                                      
                                        
                    # si el proceso no se esta ejecutando y el contador es mayor o igual al periodo         
	            if ! kill -0 "$pid" >/dev/null && [ "${vectorLine[0]}" -ge "${vectorLine[1]}" ]
	            then
		        
		        # volver a lanzar el proceso
			comandoProceso=$(echo "$line" | grep -o "'.*'" | sed "s/'//g")
		        bash -c "$comandoProceso" &
			
			#poner contador a 0                                         
                        vectorLine[0]=0
                        newLine="${vectorLine[*]}"
                        sed -i -e "s/$line/$newLine/g" ./procesos_periodicos  

                        # sustituye el pid
			pidNuevo="$!"
                        sed -i -e "s/$pid/$pidNuevo/g" ./procesos_periodicos	                      

                        echo "$hora El proceso "$pid" '"$comandoProceso"’ se ha reencarnado en el pid $pidNuevo" >> ./Biblia.txt 
	            
		    else
		
		        # incrementar contador     	    
                        ((vectorLine[0]++))                      
                        newLine="${vectorLine[*]}"		        
                        sed -i -e "s/$line/$newLine/g" ./procesos_periodicos
              
	            fi	
                    
		    
	        fi
            done
	fi

    done   

    sleep 1
      
done


# llega el Apocalipsis
echo "$hora ---------------Apocalipsis---------------" >> ./Biblia.txt

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
           echo "$hora El proceso $line ha terminado" >> ./Biblia.txt
           kill "$pid"       
       fi
    done	
done


# borrar listas, Apocalipsis e Infierno 
rm -f procesos procesos_servicio procesos_periodicos Apocalipsis SanPedro nohup.out
rm -fr Infierno

# Termina su ejecucion
echo "$hora Se acabo el mundo." >> ./Biblia.txt





