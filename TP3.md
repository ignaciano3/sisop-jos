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
        readonly = !((uvpd[PDX(va)] & PTE_W) && (uvpt[PGNUM(va)] & PTE_W));
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

...


sys_ipc_try_send
----------------

...

