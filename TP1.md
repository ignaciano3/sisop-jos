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

Una sesión de GDB en la que, poniendo un breakpoint en la función `boot_alloc()`, se muestre el valor devuelto en esa primera llamada,
usando el comando GDB `finish`.

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


page_alloc
----------

...


map_region_large
----------------

...

