#!/bin/bash
#este archivo es un scrip que:
#1 compila los fuentes padre.c e hijo.c con gcc
gcc ./Trabajo2/padre.c -o ./Trabajo2/PADRE
gcc ./Trabajo2/hijo.c -o ./Trabajo2/HIJO
#2 crea el fihero fifo "resultado"
mkfifo resultado
#lanza un cat en segundo plano para leer "resultado"  
cat resultado &
#lanza el proceso padre
./Trabajo2/PADRE 10
#al acabar limpia todos los ficheros que ha creado
rm -f resultado ./Trabajo2/PADRE ./Trabajo2/HIJO
rm -f ./Trabajo2/PADRE ./Trabajo2/HIJO
