TP4: Sistema de archivos e intérprete de comandos
=================================================

caché de bloques
----------------

**Se recomienda leer la función `diskaddr() `en el archivo _fs/bc.c_. Responder:**

**¿Qué es `super->s_nblocks`?**

`super->s_nblocks` es el número total de bloques del disco. Esta información se encuentra en el `struct Super`
que representa un superbloque.

**¿Dónde y cómo se configura este bloque especial?**

El superbloque se configura en el archivo _fs/fsformat.c_ en la función `opendisk(const char *name)`, y se puede notar
que se lo configura de la siguiente manera:

```C
alloc(BLKSIZE);
super = alloc(BLKSIZE);
super->s_magic = FS_MAGIC;
super->s_nblocks = nblocks;
super->s_root.f_type = FTYPE_DIR;
strcpy(super->s_root.f_name, "/");
```

