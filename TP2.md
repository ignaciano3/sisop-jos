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
`generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);`

    1) 0x1000 + 0x276 = 0x1276 (4096 + 630 = 4726) [creación del proceso inicial]

Comienzo de secuencia de destrucción/creación:

    2) 0x1276 + 0x276 + 0x1000 = 0x24ec (4726 + 630 + 4096 = 9452)
    3) 0x24ec + 0x276 + 0x1000 = 0x3762 (9452 + 630 + 4096 = 14178)
    4) 0x3762 + 0x276 + 0x1000 = 0x49d8 (14178 + 630 + 4096 = 18904)
    5) 0x49d8 + 0x276 + 0x1000 = 0x5c4e (18904 + 630 + 4096 = 23630)
    6) 0x5c4e + 0x276 + 0x1000 = 0x6ec4 (23630 + 630 + 4096 = 28356)




env_pop_tf
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

