TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------

**Un cálculo manual de la primera dirección de memoria que devolverá `boot_alloc()` tras el arranque. 
Se puede calcular a partir del binario compilado (`obj/kern/kernel`), 
usando los comandos `readelf` y/o `nm` y operaciones matemáticas.**

Si ejecutamos tanto `readelf -s obj/kern/kernel | grep end` como `nm obj/kern/kernel | grep end` vemos la siguiente salida 

```bash
$ readelf -s obj/kern/kernel | grep end`
101: f0114950     0 NOTYPE  GLOBAL DEFAULT    6 end
$ nm obj/kern/kernel | grep end
f0114950 B end
```
Por lo tanto,  `end` posee la dirección virtual `0xf0114950` en la primera ejecución de `boot_alloc`. Como en el primer llamado a esta
función `nextfree` es `NULL`, entonces se redondea la dirección virtual de `end` a 4096 (`PGSIZE`) llamando a `ROUNDUP(0xf0114950, PGSIZE)`,
y el resultado de este redondeo será el valor devuelto por `boot_alloc()`.
Entonces, considerando que `ROUNDUP(a, n)` redondea `a` hacia arriba al múltiplo más cercano de `n` y que 0xf0114950 = 4027664720 (decimal),
el múltiplo más cercano de 4096 será 4027666432. Si lo vemos en Python, y teniendo en cuenta la implementación de `ROUNDUP` y `ROUNDDOWN`:

```python
Python 3.8.10 (default, Mar 15 2022, 12:22:08)
[GCC 9.4.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> a = 0xf0114950
>>> n = 4096
>>> def rounddown(a,n):
    ...     return a - a % n
...
>>> def roundup(a, n):
    ...     return rounddown(a + n - 1, n)
...
>>> result_boot_alloc = roundup(int(a), n)
>>> result_boot_alloc
4027666432
>>> hex(result_boot_alloc)
'0xf0115000'
```

Por lo tanto, el resultado de `boot_alloc()` será `0xf0115000`.

**Una sesión de GDB en la que, poniendo un breakpoint en la función `boot_alloc()`, se muestre el valor devuelto en esa primera llamada,
usando el comando GDB `finish`.**

```bash
(gdb) b boot_alloc
Breakpoint 1 at 0xf0100aad: file kern/pmap.c, line 89.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0100aad <boot_alloc>:	push   %ebp

Breakpoint 1, boot_alloc (n=65684) at kern/pmap.c:89
89	{
(gdb) finish
Run till exit from #0  boot_alloc (n=65684) at kern/pmap.c:89
=> 0xf0100ebb <mem_init+26>:	mov    %eax,0xf0114948
mem_init () at kern/pmap.c:139
139		kern_pgdir = (pde_t *) boot_alloc(PGSIZE);
Value returned is $1 = (void *) 0xf0115000
```


map_region_large
**Modificar la función boot_map_region() para que use page directory entries de 4 MiB cuando sea apropiado. (En particular, sólo se pueden usar en direcciones alineadas a 22 bits.)**
**¿cuánta memoria se ahorró de este modo? (en KiB)**
Para responder esto, primero hay que ver qué es lo que pasaba cuando solo se usaban páginas de 4KiB. La función boot_map_region() hace por cada bloque de 4KiB lo siguiente: llama a pgdir_walk (creando una pageTable si es que no había sido creada ya), y en la posición de la pageTable obtenida se escribe la dirección física correspondiente junto con los permisos.

Esto significa que por cada bloque de 4KiB virtuales que se quieren vincular con el respectivo bloque de memoria física, se necesita una PageTable que ocupa exactamente 4KiB. Por lo tanto, memoria total usada: (size / 4KiB) * 4KiB = size. Esto es, si queremos asociar 64KiB de memoria virtual con 64KiB de memoria física, se necesitan 64KiB.

Ahora, se procede a explicar lo que pasa con las páginas largas. Por cada bloque de 4MB, se carga en una fila del PageDirectory la dirección física del bloque de 4MB correspondiente. Esto significa que no hay un intermediario entre la memoria física y el PageDirectory, como si lo había con las páginas de tamaño 4KiB.

Por lo tanto, por cada bloque de memoria virtual de 4MB (equivalente a 1024 bloques de 4KiB) que se asocia al bloque físico mediante páginas largas, se ahorran 4MB de memoria.

**¿es una cantidad fija, o depende de la memoria física de la computadora?**

----------------

...

