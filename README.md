# DyASO

Proyectos en los que se practican algunos de los conceptos básicos de la administración y el diseño de sistemas operativos haciendo uso del lenguaje C y el lenguaje de script Bash, usando como plataforma el sistema operativo Linux.


## Problemas

* [A deal with the devil](#a-deal-with-the-devil)
* [Process contention](#process-contention)

## A deal with the devil

https://github.com/IsabelManzaneque/PECS_DYASO/tree/main/A%20deal%20with%20the%20devil

Los procesos demonio en los sistemas operativos UNIX son críticos para realizar tareas de administración del sistema. En este proyecto, se explora el funcionamiento de estos procesos a través de la implementación de un gestor de procesos consistente en una interfaz de entrada de comandos y un proceso demonio asociado a ella. 

La interfaz de entrada, Fausto.sh, recibirá órdenes por la línea de comandos y lanzará una serie de procesos que Demonio.sh se encargará de gestionar, monitorizando su estado y acciones cuando sea necesario. La comunicación y sincronización entre ambos se producirá a través de listas y ficheros en los que se registrarán las acciones que se van realizando: las listas “procesos”, “procesos_servicio” y “procesos_periodicos” registrarán los correspondientes tipos de procesos en ejecución; el directorio “Infierno” contendrá archivos que identificaran aquellos procesos que se desea finalizar; el fichero “SanPedro” permitirá la sincronización entre Fausto y el Demonio; la aparición del fichero “Apocalipsis” indicará que se debe finalizar la ejecución del sistema; todas las acciones se irán recogiendo en la “Biblia.txt”



## Process contention

https://github.com/IsabelManzaneque/PECS_DYASO/tree/main/Process%20contention

Utilizamos dos programas escritos en C y un script en Shell para simular un combate entre procesos: los programas padre.c e hijo.c junto con el script Ejercicio2.sh,
orquestarán un enfrentamiento entre procesos hijos arbitrado por el proceso padre. La estructura del combate consiste en un entramado de mecanismos IPC que involucra colas de mensajes, regiones de memoria compartida, semáforos y tuberías sin nombre.

La dinámica del enfrentamiento se basa en rondas, donde cada proceso hijo decide entre atacar o defenderse, siguiendo un protocolo de preparación y ataque. El proceso padre sincroniza las acciones de los contendientes. Tras cada ronda, los hijos comunicarán al padre los resultados de las mismas y el padre tomará decisiones en función de estos: eliminar los procesos derrotados, actualizar la lista de contendientes y, en última instancia, anunciar un ganador. Una vez se obtiene el ganador, se liberan
los mecanismos IPC.



