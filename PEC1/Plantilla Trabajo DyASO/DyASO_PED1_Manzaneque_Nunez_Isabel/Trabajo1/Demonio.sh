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
#Apocalipsis: termino todos los procesos y limpio todo dejando sólo Fausto, el Demonio y la Biblia
# demonio es un bucle hile infinito hasta que se notifique el fin. Una de las cosas que hace en el bucle es abrir el fichero de servicios y por cada linea obtiene el pid, comprueba si existe un proceso corriendo con ese pid y en caso negativo lo lanza y actualiza los datos (si cambia el pid volver a lanzarlo, hay que actualizar el pid antiguo por el nuevo). Si esta corriendo, lo ignora y pasa al siguiente

# Bucle que se ejecuta mientras no se detecte el fichero apocalipsis
# -f comprueba si un file existe en el directorio del script
while [ ! -f "Apocalipsis" ]
do
    echo "Demonio dice: No ha llegado el apocalipsis..."
    # Esperar 1 segundo
    sleep 1  
    # Realizar las acciones 
done

echo "Demonio dice: Ha llegado el apocalipsis!"


