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

**1. Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: 
¿se preserva, en el hijo, el flag de solo-lectura en las páginas copiadas?**

No, todas las páginas allocadas en el proceso hijo están hechas desde la función `duppage()`, y viendo el código:

```C
r = sys_page_alloc(dstenv, addr, PTE_P|PTE_U|PTE_W)
```

Todas las páginas allocadas tienen el flag de escritura, sin importar los permisos en el padre.

**2. Mostrar, con código en espacio de usuario, cómo podría `dumbfork()` verificar si una dirección en el padre es de solo lectura,
de tal manera que pudiera pasar como tercer parámetro a `duppage()` un booleano llamado readonly que indicase si la página es modificable o no:**

_Ayuda_: usar las variables globales `uvpd` y/o `uvpt`.

```C
envid_t dumbfork(void) {
    // ...
    for (addr = UTEXT; addr < end; addr += PGSIZE) {
        bool readonly;
        // uvtp = VA of "virtual page table"
        // uvpd = VA of current page directory
        pde_t pde = uvpd[PDX(addr)];
		if (pde & PTE_P) {
			pte_t pte = uvpt[PGNUM(addr)];
			if (pte & PTE_P) {
				readonly = !(pte & PTE_W);
			}
        duppage(envid, addr, readonly);
    }
```

**3. Supongamos que se desea actualizar el código de `duppage()` para tener en cuenta el argumento _readonly_: si este es verdadero,
la página copiada no debe ser modificable en el hijo. Se pide mostrar una versión en el que se implemente la misma funcionalidad readonly,
pero sin usar en ningún caso más de tres llamadas al sistema.**

_Ayuda_: Leer con atención la documentación de `sys_page_map()` en _kern/syscall.c_, en particular donde avisa que se devuelve error:

```C
void duppage(envid_t dstenv, void *addr, bool readonly) {
    int flag_write = readonly ? 0 : PTE_W;
    int r;

	if ((r = sys_page_alloc(dstenv, addr, PTE_P|PTE_U|flag_write)) < 0)
		panic("sys_page_alloc: %e", r);
	if ((r = sys_page_map(dstenv, addr, 0, UTEMP, PTE_P|PTE_U|flag_write)) < 0)
		panic("sys_page_map: %e", r);
	memmove(UTEMP, addr, PGSIZE);
	if ((r = sys_page_unmap(0, UTEMP)) < 0)
		panic("sys_page_unmap: %e", r);
}
```


ipc_recv
--------

**Un proceso podría intentar enviar el valor númerico `-E_INVAL` vía `ipc_send()`. 
¿Cómo es posible distinguir si es un error, o no?**

```C
envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

if (r < 0)
  if (/* ??? */)
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")
```

En la documentación de `ipc_recv()` se observa que _"If the system call fails, then store 0 in *fromenv and *perm (if
they're nonnull) and return the error."_. Por lo tanto con revisar el valor de src, se puede determinar si se produjo un error.

```C
envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

if (r < 0)
  if (src == 0)
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")
```



sys_ipc_try_send
----------------

Para que la función `sys_ipc_send()` sea bloqueante, se propone el siguiente modelo:

```C
struct Env {
//...
  bool env_ipc_recving;    // Env is blocked receiving
  void *env_ipc_dstva;     // VA at which to map received page
  uint32_t env_ipc_value;  // Data value sent to us
  envid_t env_ipc_from;    // envid of the sender
  int env_ipc_perm;        // Perm of page mapping received
  envid_t senders_queue[10];   // NEW FIELD: vector that has the ids of the processes that are attempting to send.
  int next_sender_pos;   // NEW FIELD: Number that stores the position of the next process to be received.
  bool sender_added_to_queue; //NEW FIELD: Boolean that states if a sender id was added to queue
}
```

Se propone que con el vector de ids y la posición actual, se puede hacer que `sys_ipc_send()` puede ser bloqueante.
Primero se analiza el caso 'feliz' en el que el proceso A llama a `sys_ipc_recv()` y se detiene a esperar a que algún otro proceso le envíe algo.
Cuando el proceso B le intente enviar el mensaje, va a actuar de la misma forma que ya está implementada: ve que el proceso A está esperando el mensaje y le setea los campos correspondientemente (sin tener que hacer uso del vector de ids ni de la posición de dicho vector).

Ahora, el otro caso es que el proceso B envía un mensaje al proceso A, cuando el proceso A todavía no está recibiéndolo. Se aclara que cuando se crea un proceso, el vector de ids `senders_queue` tiene todas las posiciones inicializadas en -1, `next_sender_pos` en cero, y el booleano `sender_added_to_queue` en false. 
Ahora, en la situación dada se sigue la siguiente secuencia: como el proceso A no está esperando, cuando el proceso B llama `sys_ipc_send()`, lo que hace al darse cuenta de que el proceso A no está bloqueado recibiendo nada, es guardarse su mismo id en el vector (en la posición `next_sender_pos`), aumenta en 1 el `next_sender_pos`, y le setea el booleano `sender_added_to_queue` a true. 
Por último procede a ponerse en `ENV_NOT_RUNNABLE` y llama a `sched_yield()`. Ahora, cuando el proceso A llame a `sys_ipc_recv()`, va a verificar si el booleano es true. Si este es el caso, significa que otro proceso (el B) le intentó mandar mensaje, por lo que busca el id de la primera posición después del último -1 y antes del `next_sender_pos` en el vector `senders_queue`.
'Despierta' al proceso B (poniéndolo en `ENV_RUNNABLE`), se pone a sí mismo en `ENV_NOT_RUNNABLE` y llama `sched_yield()` para que el proceso B haga la secuencia que ya está implementada. 
Además, antes de llamar a `sched_yield()` el proceso A modifica la posición `senders_queue` de dónde sacó el `envid`, seteándola a -1.

Digamos ahora el caso en el que el proceso B y C le envían algo al proceso A, sin que A esté recibiéndolo. 
Esto es, el proceso B hace un send, después el proceso C hace un send, y por último el proceso A hace un receive. El send del B se maneja de la misma forma que se explicó antes, al igual que el proceso C. 
Esto significa que cuando el proceso A llame a receive, va a encontrar lo siguiente: 

```C
senders_queue[10] == {envid_B, envid_C, -1, -1, ...}
next_sender_pos == 2
sender_added_to_queue == true;
```

Siguiendo los pasos de antes, como `sender_added_to_queue == true` se busca la primera posición distinta de -1 que esté antes de la posición `next_sender_pos`. 
Imaginándose un vector cíclico en el que la última posición viene antes que la primera, entonces el id escogido es el envid_B. Por lo tanto, se hace la secuencia de antes para que el proceso B pueda concretar el envío del mensaje y a su vez se modifica el vector `senders_queue` acorde,
quedando {-1, envid_C, -1, -1, ...}. Cuando el proceso A vuelva a hacer un receive, se siguen los mismos pasos. 

Si el `next_sender_pos` llega a ser igual a 10, se setea para que sea igual a cero.

**¿existe posibilidad de deadlock?**

No existe posibilidad de un deadlock, ya que en todos los pasos descritos, cada vez que se 'lockea' el mutex, se hace un unlock ya sea porque el send/recv terminó o porque se llamó a `sched_yield()`.

**¿funciona que varios procesos (A₁, A₂, …) puedan enviar a B, y quedar cada uno bloqueado mientras B no consuma su mensaje? ¿en qué orden despertarían?**

Se mostró que esta posible implementación permite que varios procesos le puedan enviar a B, recibiendo así los mensajes según el orden en el que se intentaron enviar.

Se observa que es propenso a errores, ya que por ejemplo si más de 10 procesos intentan enviarle un mensaje al mismo proceso sin que este los consuma, se comienza a perder rastro de procesos que intentaron hacer el send.
Para remediar esto se puede modificar el número mágico 10 a uno más grande, o en caso de poder usar memoria dinámica usarla.

