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
# demonio es un bucle hile infinito hasta que se notifique el fin. Una de las cosas que hace en el bucle es abrir el fichero de servicios y por cada linea obtiene el pid, comprueba si existe un proceso corriendo con ese pid y en caso negativo lo lanza y actualiza los datos (si cambia el pid volver a lanzarlo, hay que actualizar el pid antiguo por el nuevo). Si esta corriendo, lo ignora y pasa al siguiente

# Bucle que se ejecuta mientras no se detecte el fichero apocalipsis
# -f comprueba si un file existe en el directorio del scrip

echo "Demonio: comenzando..."
while [ ! -f "Apocalipsis" ]
do
       
    # Itera por las listas realizando acciones
    for archivo in "${archivos[@]}"
    do

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
        

        if [ "$archivo" == "procesos_servicio" ] 
	then
	    cat "$archivo" | while read line
            do
   	        pid=$(echo $line | awk -v N=1 '{print $N}')
		
            done
	fi

        if [ "$archivo" == "procesos_periodicos" ] 
	then
	    cat "$archivo" | while read line
            do
   	        pid=$(echo $line | awk -v N=3 '{print $N}')
                

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





