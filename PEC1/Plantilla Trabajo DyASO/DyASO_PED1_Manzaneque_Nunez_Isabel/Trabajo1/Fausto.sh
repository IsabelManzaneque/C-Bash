#!/bin/bash

#Recibe órdenes creando los procesos y listas adecuadas
#Si el Demonio no está vivo lo crea
#Al leer/escribir en las listas hay que usar bloqueo para no coincidir con el Demonio


hora=$(date +%H:%M:%S)

# Comprueba si proceso Demonio existe y si no, realiza las acciones
#-x solo match procesos cuyo nombre es el mismo
if ! pgrep -x "Demonio.sh" > /dev/null 
then
    echo "Fausto: El Demonio no esta activo."
    # Borra ficheros y carpetas. Usando -f (--force) no nos da Error si los files no existen
    # Conveniente si en lugar de comprobar si estan, solo queremos asegurarnos de que NO estan
    echo "Fausto: Borrando ficheros residuales."
    rm -f procesos procesos_servicio procesos_periodicos Biblia.txt Apocalipsis SanPedro
    rm -fr Infierno
    # Los vuelve a crear vacios
    echo "Fausto: Creando ficheros nuevos."
    touch procesos procesos_servicio procesos_periodicos Biblia.txt SanPedro
    mkdir Infierno 
    # Lanza el Demonio en segundo plano
    echo "Fausto: Lanzando Demonio en segundo plano."
    exec nohup "./Demonio.sh" & #>/dev/null &   
    # Entrada genesis en la biblia 
    echo "$hora ---------------Génesis---------------" >> ./Biblia.txt
    echo "$hora El demonio ha sido creado" >> ./Biblia.txt
   
fi


# Guarda los argumentos en un array
args=("$@")

#echo "Numero de argumentos: $#"
#echo "Todos los argumentos como cadena: $*"
#echo "Todos los argumentos como palabras separadas: $@"
#echo "Todos los argumentos del array: ${args[@]}"
#echo "Primer argumento: $1"
#echo "Primer argumento del array: ${args[0]}"

#for arg in "${args[@]}"; do
#    echo "Iterando por argumento: $arg"
#done


case "$1" in
    "run")
        
	if [ "$#" -eq 2 ]
        then
	    # Crea una nueva entrada en la lista de procesos y en la biblia
	    comando="$2"
	    bash -c "$comando" &	
	    pidBash="$!"
	    
	    echo "$pidBash '"$comando"'" >> ./procesos
            echo "$hora El proceso $pidBash ‘"$comando"’ ha nacido." >> ./Biblia.txt
 	else
	    echo "Error! $1 admite un solo parametro"
	fi
        ;;
    "run-service")
        # Crea una nueva entrada en la lista de procesos servicio y en la biblia
        if [ "$#" -eq 2 ]
        then
	    comando="$2"
	    bash -c "$comando" &	
	    pidBash="$!"
	    
	    echo "$pidBash '"$comando"'" >> ./procesos_servicio
            echo "$hora El proceso $pidBash ‘"$comando"’ ha nacido." >> ./Biblia.txt
 	else
	    echo "Error! $1 admite un solo parametro"
	fi
	;;
    "run-periodic")
	# Crea una nueva entrada en la lista de procesos periodicos y en la biblia
        if [ "$#" -eq 3 ]
        then
	    T="$2"
	    comando="$3"
	    bash -c "$comando" &	
	    pidBash="$!"
	    tArranque=$(ps -p $pidBash -o etimes=)	
	  
	    echo "$tArranque $T $pidBash '"$comando"'" >> ./procesos_periodicos
            echo "$hora El proceso $pidBash ‘"$comando"’ ha nacido." >> ./Biblia.txt
 	else
	    echo "Error! $1 admite un solo parametro"
	fi
	;;
    "list")
        # Muestra una lista de los procesos creados
        if [ "$#" -eq 1 ]
        then
	    cat procesos procesos_servicio procesos_periodicos
 	else
	    echo "Error! $1 admite un solo parametro"
	fi
        ;;
    "help")
	# Muestra los comandos disponibles
        echo "Comandos disponibles:"
        echo "* run comando"
	echo "* run-service comando"
	echo "* run-periodic comando"
	echo "* list"
	echo "* help"
	echo "* stop"
	echo "* end"
        ;;
    "stop")
	# Si existe el proceso, crea un archivo con su pid en Infierno
        if [ "$#" -eq 2 ]
        then
	    pid="$2"	    
	    if grep -q "$pid" "procesos" || grep -q "$pid" "procesos_servicio" || grep -q "$pid" "procesos_periodicos"
            then
                touch ./Infierno/"$pid"	        
	    else
	        echo "Error! No existe el proceso $pid. Consulte la lista de procesos con './Fausto.sh list'" 
	    fi	    
 	else
	    echo "Error! $1 admite un solo parametro"
	fi
        ;;
    "end")
        # Crea el fichero Apocalipsis
        touch Apocalipsis
	echo "Se ha creado el fichero Apocalipsis"
        ;;

    *)	
        echo "Error! No existe la orden '$1'. Consulte las órdenes disponibles con ./Fausto.sh help"
        exit 1
        ;;
esac


