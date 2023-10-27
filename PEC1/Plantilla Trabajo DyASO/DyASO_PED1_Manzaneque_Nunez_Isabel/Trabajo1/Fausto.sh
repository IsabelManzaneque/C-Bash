#!/bin/bash

#Recibe órdenes creando los procesos y listas adecuadas
#Si el Demonio no está vivo lo crea
#Al leer/escribir en las listas hay que usar bloqueo para no coincidir con el Demonio

# --------------------------------------------------------------------------------------

# Primero Fausto y luego el demonio. Fausto, si le meten esa entrada, ejecuta el comando y escribe en el fichero de procesos servicio el comando correspondiente y ya demonio hace lo suyo

#----------------------------------------------------------------------------------------

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
    exec nohup "./Demonio.sh" & # >/dev/null &   redirijir la salida y error a /dev/null?
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


# Case basado en el primer arg
case "$1" in
    "run")
        
	if [ "$#" -eq 2 ]
        then
	    comando="$2"
	    bash -c "$comando" &	
	    pidBash="$!"
	    # crea una entrada en la lista de procesos y la biblia
	    echo "$pidBash '"$comando"'" >> ./procesos
            echo "$hora El proceso $pidBash ‘"$comando"’ ha nacido." >> ./Biblia.txt
 	else
	    echo "Error! $1 admite un solo parametro"
	fi
        ;;
    "run-service")
        echo "Error: comando $1 sin implementar"
        # Add your commands for Option 2 here
        ;;
    "run-periodic")
        echo "Error: comando $1 sin implementar"
        # Add your commands for Option 3 here
        ;;
    "list")
        echo "Error: comando $1 sin implementar"
        # Add your commands for Option 3 here
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
        echo "Error: comando $arg sin implementar"
        # Add your commands for Option 3 here
        ;;
    "end")
	# Crea un fichero "Apocalipsis" en el directorio del script
        touch Apocalipsis
	echo "Se ha creado el fichero Apocalipsis"
        ;;

    *)
	
        echo "Error! no existe la orden '$1', consulte las órdenes disponibles con ./Fausto.sh help"
        exit 1
        ;;
esac


