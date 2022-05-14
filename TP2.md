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

**Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:**
        **el tope de la pila justo antes popal**
Es la dirección base del Trapframe, es decir a la dirección base del parámetro, que tiene como primer campo PushRegs.
        **el tope de la pila justo antes iret**
Estaría apuntando al campo eip (instruction pointer).
        **el tercer elemento de la pila justo antes de iret**
El tercer elemento de la pila justo antes del iret es el campo eflags.


**    En la documentación de iret en [IA32-2A] se dice:**

**        If the return is to another privilege level, the IRET instruction also pops the stack pointer and SS from the stack, before resuming program execution.**

**    ¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?**

En la arquitectura x86, el nivel de privilegio se guarda en los últimos 2 bits del registro %cs. Se cuentan con los siguientes niveles de privilegio, el de mayor privilegio siendo el ring 0 (modo kernel, los dos bits son 00) y el otro el ring 3 (modo usuario, los dos bits son 11). 

La CPU (en x86) compara el %cs con el %cs del Trapframe: si hay diferencia, entonces hay un cambio en el nivel de privilegio.


----------

...


gdb_hello
---------

...


kern_idt
--------

...


user_evilhello
--------------

...

