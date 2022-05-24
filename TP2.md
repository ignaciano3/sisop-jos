TP2: Procesos de usuario
========================

env_alloc
---------

**¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal)**

Sabemos que `generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);` siempre devuelve `0x1000` (4096 en decimal) pues
`env_id = 0`. Luego,`e->env_id = generation | (e - envs);` toma `0x1000` y le va sumando `(e - envs)` que 
devuelve 0, 1, 2, 3, 4... Entonces, para los primeros 5 procesos, el valor de identificador será:

1) 0x1000
2) 0x1001
3) 0x1002
4) 0x1003
5) 0x1004

**Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación, se destruye el proceso
asociado a `envs[630]` y se lanza un proceso que cada segundo, muere y se vuelve a lanzar (se destruye, y se vuelve a crear).
¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?**

Sabemos que en la primera vez que se crean los procesos, cada uno de ellos tendrá un identificador a partir de `0x1000` sumandole
la cantidad de procesos que se crearon hasta ese momento. Cuando se destruye un proceso, ese identificador permanece para ese 
environment de acuerdo a `env_free()`, entonces `e->env_id` será distinto para cada creación/destrucción en el cálculo de:
`generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);` Además, `(e - envs)` valdrá siempre `0x276 (630)`
de `e->env_id = generation | (e - envs);`

```python3
>>> def generate_id(previous_id):
...     return hex((previous_id + 0x1000 & ~(0x03FF)) + 0x276)
...
>>> generate_id(0x1276)
'0x2276'
>>> generate_id(0x2276)
'0x3276'
```


1) 0x1000 + 0x276 = 0x1276 (4096 + 630 = 4726) [creación del proceso inicial]

Comienzo de secuencia de destrucción/creación:

2) 0x1276 + 0x1000 = 0x2276 (4726 + 4096 = 8822)
3) 0x2276 + 0x1000 = 0x3276 (9452 + 4096 = 12918)
4) 0x3276 + 0x1000 = 0x5276 (14178 + 4096 = 21110)
5) 0x5276 + 0x1000 = 0x6276 (18904 + 4096 = 25206)
6) 0x6276 + 0x1000 = 0x7276 (23630 + 4096 = 29302)




env_pop_tf
---------

**Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:**
- **el tope de la pila justo antes popal**: Es la dirección base del Trapframe, es decir a la dirección base del parámetro, que tiene como primer campo PushRegs.
- **el tope de la pila justo antes iret**: Estaría apuntando al campo eip (instruction pointer).
- **el tercer elemento de la pila justo antes de iret**: El tercer elemento de la pila justo antes del iret es el campo eflags.


**¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual.
¿Cuántos bits almacenan ese privilegio?**

En la arquitectura x86, el nivel de privilegio se guarda en los últimos 2 bits del registro %cs. Se cuentan con los siguientes niveles de privilegio, el de mayor privilegio siendo el ring 0 (modo kernel, los dos bits son 00) y el otro el ring 3 (modo usuario, los dos bits son 11). 

La CPU (en x86) compara el %cs con el %cs del Trapframe: si hay diferencia, entonces hay un cambio en el nivel de privilegio.

gdb_hello
---------
**Poner un breakpoint en `env_pop_tf()` y continuar la ejecución hasta allí.**
```
The target architecture is assumed to be i386
=> 0xf0102ed8 <env_pop_tf>:     endbr32

Breakpoint 1, env_pop_tf (tf=0xf01c7000) at kern/env.c:478  
```

**En QEMU, entrar en modo monitor (`Ctrl-a c`), y mostrar las cinco primeras líneas del comando `info registers`.**
```
(qemu) info registers
EAX=003bc000 EBX=00010094 ECX=f03bc000 EDX=00000209
ESI=00010094 EDI=00000000 EBP=f0119fd8 ESP=f0119fbc
EIP=f0102f5c EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```
**De vuelta a GDB, imprimir el valor del argumento _tf_:**

```  
(gdb) p tf
$1 = (struct Trapframe *) 0xf01c8000
```
**Imprimir, con `x/Nx tf` tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).**

```
(gdb) print sizeof(struct Trapframe) / sizeof(int) = 17

(gdb) x/17x tf
0xf01c8000:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c8010:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c8020:     0x00000023      0x00000023      0x00000000      0x00000000
0xf01c8030:     0x00800020      0x0000001b      0x00000000      0xeebfe000
0xf01c8040:     0x00000023
```

**Avanzar hasta justo después del `movl ...,%esp`, usando `si M` para ejecutar tantas instrucciones como sea necesario en un solo paso:**

```
(gdb) disas
(gdb) si 5
(gdb) disas
Dump of assembler code for function env_pop_tf:
   0xf0102ed8 <+0>:     endbr32
   0xf0102edc <+4>:     push   %ebp
   0xf0102edd <+5>:     mov    %esp,%ebp
   0xf0102edf <+7>:     sub    $0xc,%esp
   0xf0102ee2 <+10>:    mov    0x8(%ebp),%esp
=> 0xf0102ee5 <+13>:    popa
```

**Comprobar, con `x/Nx $sp` que los contenidos son los mismos que tf (donde N es el tamaño de tf).**
```
(gdb) x/17x $sp
0xf01c8000:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c8010:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01c8020:     0x00000023      0x00000023      0x00000000      0x00000000
0xf01c8030:     0x00800020      0x0000001b      0x00000000      0xeebfe000
0xf01c8040:     0x00000023
```

**Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.**

| Pos | Valor | Atributo | Descripcion | 
  |:---:|:---:|:---:|:---:|
  |  1 | 0x0  | reg_edi  | registro de proposito general |
  |  2 | 0x0  | reg_esi  | registro de proposito general |
  |  3 | 0x0  | reg_ebp  | registro de proposito general |
  |  4 | 0x0  | reg_oesp | registro de proposito general |
  |  5 | 0x0  | reg_ebx  | registro de proposito general |
  |  6 | 0x0  | reg_edx  | registro de proposito general |
  |  7 | 0x0  | reg_ecx  | registro de proposito general |
  |  8 | 0x0  | reg_eax  | registro de proposito general |
  |  9 | 0x23 | tf_es    | segmento de registros |
  | 10 | 0x23 | tf_ds    | segmento de registros |
  | 11 | 0x0  | tf_trapno | número de trap |
  | 12 | 0x0  | tf_err   | número de error  |
  | 13 | 0x00800020 | tf_eip   | puntero de instrucción |
  | 14 | 0x1b | tf_cs | entero al segmento de código|
  | 15 | 0x0  | tf_eflags  | flags |
  | 16 | 0xeebfe000 | tf_esp | puntero al stack |
  | 17 | 0x23 | tf_ss  | puntero de segmento |

De los registros más relevantes e inicializados (6 en total), 5 se inicializaron de la siguiente forma, en la función `env_alloc`:

```
e->env_tf.tf_ds = GD_UD | 3;
e->env_tf.tf_es = GD_UD | 3;
e->env_tf.tf_ss = GD_UD | 3;
e->env_tf.tf_esp = USTACKTOP;
e->env_tf.tf_cs = GD_UT | 3;
```
El registro restante es el `tf_eip`, que se inicializó en la función `load_icode`:

```
e->env_tf.tf_eip = elf->e_entry;
```

**Continuar hasta la instrucción `iret`, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de `info registers` 
en el monitor de QEMU. Explicar los cambios producidos.**

Se recuerda el info registers justo al entrar a la función `env_pop_tf`:

```
(qemu) info registers
EAX=003bc000 EBX=00010094 ECX=f03bc000 EDX=00000209
ESI=00010094 EDI=00000000 EBP=f0119fd8 ESP=f0119fbc
EIP=f0102f5c EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```


Ahora se tiene, justo antes de ejecutar `iret`:

```
=> 0xf0102eeb <+19>:    iret

(qemu) info registers 
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c8030
EIP=f0102f6f EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```
Primero se popearon los registros de propósito general (popal), obteniendo así los valores del trapframe del proceso (que eran todos ceros), después el registro `es` (pop es) y `ds` (pop ds), 
este último cambiando el nivel de privilegio (nótese como pasa de nivel 0 (kernel) a nivel 3 (usuario)). 

El puntero al stack también cambió debido a la ejecución de instrucciones del programa, al igual que el puntero a instrucción.

**Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.**

**imprimir el valor del contador de programa con `p $pc` o `p $eip`**

```
(gdb) p $pc
$1 = (void (*)()) 0x800020
(gdb) p $eip
$2 = (void (*)()) 0x800020
```

**cargar los símbolos de hello con el comando add-symbol-file, volver a imprimir el valor del contador de programa**

```
(gdb) p $pc
$3 = (void (*)()) 0x800020 <_start>
(gdb) p $eip
$4 = (void (*)()) 0x800020 <_start>
```

**Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.**


La última vez que se imprimió, se obtuvo:
```
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c8030
EIP=f0102f6f EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

Ahora se tiene:

```
(qemu) info registers 
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c8030
EIP=f0102f6f EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```
No hubo cambios.

**Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la función syscall() y explicar qué ocurre justo tras ejecutar la instrucción int $0x30. Usar, de ser necesario, el monitor de QEMU.**

Int N es una excepción producida por el mismo software. En este caso, es un int 48, que es el número reservado para producir una excepción y poder ejecutar una syscall.


kern_idt
--------

**Leer user/softint.c y ejecutarlo con `make run-softint-nox`. 
¿Qué interrupción trata de generar? ¿Qué interrupción se genera? Si son diferentes a la que invoca el programa… 
¿cuál es el mecanismo por el que ocurrió esto, y por qué motivos? ¿Qué modificarían en JOS para cambiar este comportamiento?**

La interrupción que trata de generar es la de _page fault_ (es la 14 en `idt`), pero se genera una _general protection_.
Esto se debe a que en el llamado a `softinc` se tienen privilegios de usuarios (ring 3) pero la interrupción _page fault_
fue declarada en `trap.c` en `SETGATE(idt[T_PGFLT], 0, GD_KT, trap_14, 0);` con privilegios de kernel (ring 0) lo que significa
que solo el kernel puede invocarla y transferir la ejecución a la misma. Para cambiar este comportamiento, deberíamos configurar
la interrupción _page fault_ como `SETGATE(idt[T_PGFLT], 0, GD_KT, trap_14, 3);`


user_evilhello
--------------
Se tiene en _evilhello.c_:  

```C
void
umain(int argc, char **argv)
{
	// try to print the kernel entry point as a string!  mua ha ha!
	sys_cputs((char*)0xf010000c, 100);
}
```

Al ejecutar _evilhello_ se muestra:
```bash
[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
f�rIncoming TRAP frame at 0xefffffbc
[00001000] exiting gracefully
[00001000] free env 00001000
```
Se observa cómo se crea el environment, se hace un cambio de contexto al programa, 
se imprime en pantalla 'f�r' y se vuelve hacer otro cambio de contexto, finalizando el proceso y consecuentemente liberando los recursos del ambiente.

Se tiene, por otro lado, user_evilhello:

```C
void
umain(int argc, char **argv)
{
    char *entry = (char *) 0xf010000c;
    char first = *entry;
    sys_cputs(&first, 1);
}
```

Que al ejecutarse genera un page fault:

```bash
[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
[00001000] user fault va f010000c ip 0080003d
TRAP frame at 0xf01c8000
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdfd0
  oesp 0xefffffdc
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000e Page Fault
  cr2  0xf010000c
  err  0x00000005 [user, read, protection]
  eip  0x0080003d
  cs   0x----001b
  flag 0x00000082
  esp  0xeebfdfb0
  ss   0x----0023
[00001000] free env 00001000
```

**¿En qué se diferencia el código de la versión en evilhello.c mostrada arriba?**

En la version de _evilhello.c_ se llama a `sys_cputs()` directamente con la dirección del entry point del kernel, mientras
que en _user_evilhello_ se intenta 'engañar' desreferenciado el contenido del puntero al entry point del kernel desde la 
aplicación de usuario ocurriendo un _page fault_ antes de llamar `sys_cputs()`.

**¿En qué cambia el comportamiento durante la ejecución? ¿Por qué? ¿Cuál es el mecanismo?**

Al mirar por encima ambos programas, uno pensaría que deberían hacer lo mismo. Al final, el flujo del programa termina llevando a imprimir el primer carácter de la dirección `0xf010000c`. 
Sin embargo, se mostró que uno sí logra cumplir con su objetivo, y el otro genera un _Page Fault_ en el intento. 

La principal diferencia en el código es cómo se obtiene el carácter a imprimir: 
el primer programa le manda el puntero a la syscall, mientras que el segundo desreferencia en el mismo programa el puntero a la dirección que se quiere imprimir.

En _user_evilhello_ la dirección que se desea acceder pertenece al kernel y no es accesible por el usuario, la MMU detecta
el acceso sin permisos, ocurre el _page fault_ y destruye el proceso. En _evilhello.c_, la misma dirección se accede directamente
en modo kernel mediante el handler de la syscall, por lo cual el proceso finaliza con éxito.

**Listar las direcciones de memoria que se acceden en ambos casos, y en qué ring se realizan.**

En ambos casos, la dirección de memoria a ser accedida es la misma `0xf010000c`, solo que en _evilhello.c_ se accede desde 
el kernel (ring 0) y en _user_evilhello_ se accede desde modo usuario (ring 3)

**¿Es esto un problema? ¿Por qué?**

Sí, esto es lo que genera una inconsistencia entre ambos programas, porque aunque ambos estén intentando acceder a la misma dirección, 
el primero la accede desde el kernel (por lo que no hay ningún problema en acceder a esta) y el segundo desde el modo usuario (generando así un Page Fault, ya que el usuario no tiene permisos a acceder a esta dirección de memoria).

Se concluye entonces que el error está en que el kernel accede a esta dirección de memoria mediante un syscall,
sin antes verificar que el usuario puede acceder a la misma.

