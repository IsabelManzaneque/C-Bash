#!/bin/bash


# Comprueba si proceso Demonio existe 
if ! pgrep -x "Demonio.sh" > /dev/null 
then
    echo "----------Comenzando-----------"
    # Borra ficheros y carpetas residuales y los vuelve a crear vacios
    rm -f procesos procesos_servicio procesos_periodicos Biblia.txt Apocalipsis SanPedro
    rm -fr Infierno
    touch procesos procesos_servicio procesos_periodicos Biblia.txt SanPedro
    mkdir Infierno 
    # Lanza el Demonio en segundo plano 
    nohup ./Demonio.sh >/dev/null 2>&1 &   
    # Entrada genesis en la biblia 
    flock SanPedro -c "{
                        echo \"$(date +%H:%M:%S) ---------------Génesis---------------\" 
                        echo \"$(date +%H:%M:%S) El demonio ha sido creado\" 
                       } >> ./Biblia.txt"
   
fi



case "$1" in
    "run")        
	if [ "$#" -eq 2 ]
        then
	    # Crea una nueva entrada en la lista de procesos y en la biblia
	    comando="$2"
	    bash -c "$comando" &	
	    pidBash="$!"
	    
	    flock SanPedro -c "echo \"$pidBash '$comando'\" >> ./procesos"
            flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $pidBash '$comando' ha nacido.\" >> ./Biblia.txt"
 	else
	    echo "Error! $1 admite un parametro"
	fi
        ;;
    "run-service")
        # Crea una nueva entrada en la lista de procesos servicio y en la biblia
        if [ "$#" -eq 2 ]
        then
	    comando="$2"
	    bash -c "$comando" &	
	    pidBash="$!"
	    
	    flock SanPedro -c "echo \"$pidBash '$comando'\" >> ./procesos_servicio"
            flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $pidBash '$comando' ha nacido.\" >> ./Biblia.txt"
 	else
	    echo "Error! $1 admite un parametro"
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
	  
	    flock SanPedro -c "echo \"0 $T $pidBash '$comando'\" >> ./procesos_periodicos"
            flock SanPedro -c "echo \"$(date +%H:%M:%S) El proceso $pidBash '$comando' ha nacido.\" >> ./Biblia.txt"
 	else
	    echo "Error! $1 admite un parametro"
	fi
	;;
    "list")
        # Muestra una lista de los procesos creados
        if [ "$#" -eq 1 ]
        then
	    echo "***** Procesos normales *****"
	    cat procesos 
	    echo "***** Procesos servicio *****"
	    cat procesos_servicio 
	    echo "***** Procesos periodicos *****"
	    cat procesos_periodicos
 	else
	    echo "Error! $1 admite un solo parametro"
	fi
        ;;
    "help")
	# Muestra los comandos disponibles
        echo "Sintaxis:"
	echo "./Fausto.sh run comando"
	echo "./Fausto.sh run-service comando"
	echo "./Fausto.sh run-periodic T comando"
	echo "./Fausto.sh list"
	echo "./Fausto.sh help"
	echo "./Fausto.sh stop PID"
	echo "./Fausto.sh end"        
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
	    echo "Error! $1 debe recibir un parametro"
	fi
        ;;
    "end")
        # Crea el fichero Apocalipsis
        touch Apocalipsis	
        ;;
    *)	
        echo "Error! No existe la orden '$1'. Consulte las órdenes disponibles con ./Fausto.sh help"
        exit 1
        ;;
esac


