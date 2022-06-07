TP3: Multitarea con desalojo
============================

sys_yield
---------

**Cambiar la función i386_init() para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de make qemu-nox.**

Al ejecutar make qemu-nox se obtiene:

```
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000.
Hello, I am environment 00001001.
Hello, I am environment 00001002.
Back in environment 00001000, iteration 0.
Back in environment 00001001, iteration 0.
Back in environment 00001002, iteration 0.
Back in environment 00001000, iteration 1.
Back in environment 00001001, iteration 1.
Back in environment 00001002, iteration 1.
Back in environment 00001000, iteration 2.
Back in environment 00001001, iteration 2.
Back in environment 00001002, iteration 2.
Back in environment 00001000, iteration 3.
Back in environment 00001001, iteration 3.
Back in environment 00001002, iteration 3.
Back in environment 00001000, iteration 4.
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Back in environment 00001001, iteration 4.
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
No runnable environments in the system!
```
Se muestra como se crean 3 procesos, con id 0x1000, 0x1001 y 0x1002, ejecutandose en dicho orden. El proceso 0x1000 entra y llama a la syscall yield. Como el siguiente proceso en lista es el 0x1001, se procede a ejecutar este, que hace lo mismo, por lo que se ejecuta el proceso 0x1002. Cada vez que uno de estos procesos llama la syscall yield, se busca a partir del proceso actual el siguiente proceso para correr. Se observa que en el programa se llama a esta syscall cinco veces.

dumbfork
--------

...


ipc_recv
--------

...


sys_ipc_try_send
----------------

...

