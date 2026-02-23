# ft_nm — Apuntes técnicos: lógica y flujo del código

---

## Visión general

`ft_nm` replica el comportamiento del comando `nm` de GNU Binutils. Su función es leer archivos binarios ELF (32/64 bits) y archivos estáticos `.a`, extraer sus tablas de símbolos y mostrarlos con su tipo, valor y nombre.

El programa sigue un **pipeline lineal de fases**: parseo de argumentos → identificación de formato → lectura de cabeceras ELF → extracción de símbolos → clasificación → filtrado → ordenación → salida.

---

## Estructuras de datos principales

### `t_stack_file`

Representa un archivo procesado. Es un nodo de una lista enlazada.

```
t_stack_file
├── file               → Nombre/ruta del archivo
├── validity           → 1=válido, 0=error, -2=directorio
├── elf                → 0=no ELF, 1=ELF, 2=archivo .a
├── bits               → BITS_32 o BITS_64
├── endianness         → ENDIAN_LSB o ENDIAN_MSB
├── file_content_ptr   → Puntero al mmap() del archivo
├── file_size          → Tamaño en bytes del archivo
├── flag               → Máscara de flags activos (-a/-g/-u/-r/-p)
├── elf32/64_header    → Puntero a la cabecera ELF casteada
├── elf32/64_sh_table  → Puntero a la Section Header Table
├── shstrtab_ptr/size  → Tabla de nombres de sección
├── strtab_ptr/size    → Tabla de strings de símbolos
├── symtab_ptr/size    → Tabla de símbolos
├── symbol_list        → Lista enlazada de t_symbol_info
├── dinamic_bin        → Sub-lista de miembros (solo para .a)
└── next               → Siguiente archivo en la lista
```

### `t_symbol_info`

Representa un símbolo extraído de la tabla de símbolos.

```
t_symbol_info
├── name        → Nombre del símbolo (strdup desde strtab)
├── value       → Dirección / valor del símbolo
├── char_type   → Carácter de tipo nm ('T', 'U', 'd', ...)
├── st_info     → Byte raw de ELF (codifica type + bind)
├── shndx       → Índice de sección (SHN_UNDEF, SHN_ABS, ...)
├── visible     → Si debe mostrarse en la salida (post-filtro)
└── next        → Siguiente símbolo
```

### `t_nmflags` (bitmask)

```c
NM_ARG_FILE = 0        // sin flags
NM_FLAG_A   = 1 << 0   // -a
NM_FLAG_G   = 1 << 1   // -g
NM_FLAG_U   = 1 << 2   // -u
NM_FLAG_R   = 1 << 3   // -r
NM_FLAG_P   = 1 << 4   // -p
```

---

## Flujo de ejecución (main.c)

```
main()
 │
 ├─ init_host_endianness()       → detecta si el host es LSB o MSB
 │
 ├─ create_list(&sfile, argv)    → parsea args, crea lista de t_stack_file
 │
 ├─ file_format_id(&sfile)       → mmap + detecta ELF/AR/desconocido
 │
 ├─ process_file_list(&sfile)    → para cada archivo:
 │     ├─ process_elf_file()         → pipeline ELF completo
 │     └─ process_static_archive()   → itera miembros del .a
 │
 ├─ ft_output(&sfile, argc)      → imprime resultados
 │
 └─ clear_closing(&sfile)        → libera toda la memoria
```

---

## Fase 1: Parseo de argumentos (`main.c`)

### `create_list()`

Recorre `argv` en dos pasadas:

1. **Primera pasada**: detecta todos los flags (`-a`, `-g`, `-u`, `-r`, `-p`) y construye una máscara `active_flags` con OR de bits.
2. **Segunda pasada**: para cada argumento que no sea flag, llama a `checkarg()` y crea un nodo `t_stack_file` con el flag acumulado.

### `checkarg()`

Abre el archivo con `open()` + `fstat()`:
- Retorna `-1` si no existe (`errno == ENOENT`)
- Retorna `-2` si es directorio (`S_ISDIR`)
- Retorna `-3` si falla `fstat()`
- Retorna `1` si es un archivo regular válido

### Sin argumentos

Si `argc == 1`, llama a `find_binaries()` que busca `a.out` en el directorio actual con `opendir()`. Si no lo encuentra, imprime error y sale.

---

## Fase 2: Identificación de formato (`format.c`)

### `file_format_id()`

Para cada nodo válido con `ARG_TYPE_FILE`:

1. Abre el archivo con `open()` + `fstat()` para obtener el tamaño.
2. Mapea el contenido completo en memoria con `mmap(PROT_READ, MAP_PRIVATE)`.
3. Identifica el formato:
   - **ELF**: magic bytes `0x7F 'E' 'L' 'F'` → `aux->elf = 1`
   - **Archivo `.a`**: magic `"!<arch>\n"` (8 bytes, `ARMAG`) → `aux->elf = 2`
   - **Desconocido**: `aux->elf = 0`
4. Si es ELF, llama a `find_bits()` y `find_endianness()` para rellenar `bits` y `endianness`.

### `find_bits()` y `find_endianness()`

Leen los bytes `e_ident[EI_CLASS]` y `e_ident[EI_DATA]` del array de identificación ELF (primeros 16 bytes del archivo). No necesitan conocer el endianness aún porque `e_ident` es siempre byte a byte.

---

## Fase 3: Pipeline ELF (`process_elf.c`, `parsheader.c`, `readheader.c`)

Función coordinadora: `process_elf_file(sfile)`. Llama en orden:

### 3.1 `parsing_header()`

Castea el puntero `file_content_ptr` a `Elf32_Ehdr*` o `Elf64_Ehdr*` según los bits detectados. Valida:
- Que el archivo tiene el tamaño mínimo de una cabecera ELF.
- Que los magic bytes son correctos.

### 3.2 `location_headings()`

Lee `e_shoff` (offset de la Section Header Table), `e_shnum` (número de secciones) y `e_shentsize` desde la cabecera. Valida rangos y castea el puntero a `Elf32_Shdr*` / `Elf64_Shdr*`.

> **Nota**: todos los valores leídos de la cabecera pasan por `get_elf_u16/32/64()` para manejar diferencias de endianness.

### 3.3 `location_names()`

Localiza la sección `.shstrtab` usando el índice `e_shstrndx` de la cabecera. Guarda el puntero y el tamaño en `shstrtab_ptr` / `shstrtab_size`. Esta tabla se usa para obtener los nombres de las demás secciones.

### 3.4 `iterytable()`

Itera todas las secciones del SHT buscando:
- `SHT_SYMTAB` → guarda `symtab_ptr`, `symtab_size` y `symtab_link` (índice del strtab asociado).
- `SHT_STRTAB` con nombre `.strtab` → guarda `strtab_ptr` y `strtab_size`.

Solo guarda la primera coincidencia de cada tipo.

### 3.5 `parsing_symbol_ent()`

Itera la tabla de símbolos (`symtab_ptr`) entrada a entrada:
- Para cada entrada `Elf32_Sym` / `Elf64_Sym`, extrae: nombre (offset en strtab), valor, `st_info`, `shndx`.
- Si el símbolo es de tipo `STT_SECTION`, usa el nombre de la sección correspondiente como nombre del símbolo.
- Crea un nodo `t_symbol_info` con `create_symbnode()` y lo encola en `symbol_list`.

---

## Fase 4: Clasificación de tipo de símbolo (`extrac_symbol.c`)

### `extr_detc_symbol_type()`

Itera todos los símbolos y llama a `determine_symbol_type()` para cada uno.

### `determine_symbol_type()`

Asigna el carácter de tipo siguiendo este orden de prioridad:

```
1. SHN_UNDEF  → 'U' (o 'w'/'v' si es STB_WEAK)
2. SHN_ABS    → 'A' / 'a'
3. SHN_COMMON → 'C' / 'c'
4. STB_WEAK   → 'W'/'w' o 'V'/'v' (objeto débil)
5. STB_GNU_UNIQUE → 'u'
6. STT_GNU_IFUNC  → 'I' / 'i'
7. STT_SECTION / resto → analyze_section()
```

### `analyze_section()`

Cuando el símbolo pertenece a una sección normal, busca el `Elf32/64_Shdr` correspondiente y llama a `assign_type_from_section()`.

### `assign_type_from_section()`

Determina el tipo según el nombre y los flags de la sección:

| Condición | Tipo |
|-----------|------|
| `.debug*`, `.comment`, `.note*` | `N` |
| `.text` o flag `SHF_EXECINSTR` | `T` / `t` |
| `.bss`, `.tbss` | `B` / `b` |
| flag `SHF_TLS` | `G` / `g` |
| `.sdata`, `.sbss` | `S` / `s` |
| `.data`, `.got`, `.data.rel.ro`, o `SHF_WRITE` | `D` / `d` |
| `.rodata`, `SHF_MERGE`, o `SHF_ALLOC && !SHF_WRITE` | `R` / `r` |
| resto | `N` / `n` |

La mayúscula/minúscula depende de si el bind es `STB_LOCAL` (local → minúscula).

---

## Fase 5: Filtrado (`tilter.c`)

### `tilter_collecting()`

Marca `sym->visible = false` en los símbolos que deben ocultarse según los flags activos:

- `STT_FILE` y `STT_SECTION` se ocultan **salvo** que esté activo `-a`.
- Símbolos con nombre vacío se ocultan (excepto tipo `A`/`a`).
- Con flag `-g`: se ocultan los locales (`STB_LOCAL`).
- Con flag `-u`: se ocultan todos los que no sean `SHN_UNDEF`.

---

## Fase 6: Ordenación (`symsort.c`)

### `ordering_symbols()`

- Si el flag `-p` **no** está activo: aplica `merge_sort()`.
- Si además está activo `-r`: aplica `reverse_list()` tras el sort.

### `merge_sort()`

Implementación estándar de merge sort sobre lista enlazada:
1. `ft_split_list()` divide la lista en dos mitades usando el truco de puntero lento/rápido (slow/fast).
2. Ordena recursivamente cada mitad.
3. `ft_merge()` combina usando `compare_symbols()`.

### `compare_symbols()`

Compara dos símbolos con este orden de criterios:

1. **`ignore_underscores()`**: comparación case-insensitive ignorando guiones bajos y caracteres no alfanuméricos al inicio.
2. Comparación exacta del nombre completo (`ft_strcmp`).
3. Comparación por valor numérico.
4. Comparación por prioridad de tipo (`get_type_sort_priority()`).

### `ignore_underscores()`

Avanza sobre los guiones bajos iniciales de ambos strings, luego avanza saltando caracteres que no sean alfanuméricos ni `.`, comparando en lowercase carácter a carácter. Replica el comportamiento de GNU `nm`.

### `get_type_sort_priority()`

Busca el carácter en la cadena de orden `"UuWwVvTtDdRrBbCcSsIiAa?"`. La posición es la prioridad.

---

## Fase 7: Archivos estáticos `.a` (`process_elf.c`)

### `process_static_archive()`

Un archivo `.a` es un archivo `ar` con la siguiente estructura:

```
[ARMAG: "!<arch>\n" 8 bytes]
[ar_hdr: 60 bytes]  [datos del miembro: ar_size bytes]  [padding si ar_size impar]
[ar_hdr: 60 bytes]  [datos del miembro]
...
```

El código:
1. Empieza en `offset = SARMAG` (8 bytes).
2. Lee cada `ar_hdr` (nombre, tamaño en decimal ASCII).
3. Ignora las entradas especiales (`//` y `/\0`, que son tablas de símbolos del archivo ar).
4. Para cada miembro, crea un `t_stack_file` temporal con `file_content_ptr` apuntando al interior del mmap del archivo padre (sin hacer un nuevo mmap).
5. Si el miembro es un ELF, llama a `process_elf_file()` recursivamente.
6. Encola el nodo en `sfile->dinamic_bin`.
7. Avanza el offset: `sizeof(ar_hdr) + member_size`, más 1 si el tamaño es impar (alineación a 2 bytes).

### `clean_ar_name()`

El campo `ar_name` del header tiene 16 bytes con padding de espacios y termina en `/`. Esta función extrae el nombre real hasta el primer `/` o espacio.

---

## Fase 8: Salida (`output_nm.c`)

### `ft_output()`

Itera la lista de archivos y según `aux->elf`:

- **`elf == 0`** o `validity == 0`: imprime mensaje de error por stderr.
- **`elf == 1`** (ELF directo): si `argc > 2` imprime cabecera con el nombre del archivo. Itera `symbol_list` imprimiendo solo los `visible`.
- **`elf == 2`** (archivo `.a`): si `argc > 2` imprime cabecera del archivo `.a`. Itera `dinamic_bin` e imprime cabecera y símbolos por cada miembro.

### Formato de cada línea

```c
// Si shndx == SHN_UNDEF: imprime 16 o 8 espacios
// Si no: convierte value a hex de ancho fijo manualmente
write(1, hex_buffer, width);   // "0000000000401234"
write(1, " ", 1);
write(1, &symb->char_type, 1); // "T"
write(1, " ", 1);
write(1, symb->name, len);     // "main"
write(1, "\n", 1);
```

La conversión a hex se hace manualmente (sin `printf`) llenando un buffer de derecha a izquierda con `n & 0xF` y `n >>= 4`.

---

## Manejo de endianness (`endian_utils.c`)

El ELF puede ser little-endian (x86) o big-endian (SPARC, MIPS...). El host también tiene su propio endianness.

### `init_host_endianness()`

Detecta el endianness del host escribiendo `0x0001` en un `uint16_t` y leyendo el primer byte: si es `0x01`, es little-endian.

### `get_elf_u16/32/64(val, file_endianness)`

Compara `file_endianness` con `g_host_endianness`. Si difieren, aplica la función `swap16/32/64()` correspondiente (inversión de bytes). Si coinciden, devuelve el valor tal cual.

Este wrapper se usa en **todas** las lecturas de campos de la cabecera ELF y de las entradas de sección/símbolo.

---

## Gestión de memoria

- El contenido de cada archivo se mapea **una sola vez** con `mmap()` y se libera con `munmap()` en `clear_closing()`.
- Los nodos de archivos `.a` comparten el mmap del padre (`file_content_ptr` apunta al interior del buffer del padre), por lo que **no se hace `munmap` de ellos** — solo se libera la estructura.
- Los nombres de los símbolos se copian con `ft_strdup()` (sí se liberan en `clear_symbol_list()`).
- Los nombres de miembros de `.a` se alocan con `malloc` en `clean_ar_name()` (se liberan con el nodo).

---

## Diagrama resumido del pipeline

```
argv
  │
  ▼
create_list()          → lista de t_stack_file
  │
  ▼
file_format_id()       → mmap + detección ELF/AR + bits/endianness
  │
  ▼
process_file_list()
  ├─ process_elf_file()
  │     ├─ parsing_header()        → castear Ehdr
  │     ├─ location_headings()     → castear SHT
  │     ├─ location_names()        → localizar .shstrtab
  │     ├─ iterytable()            → localizar .symtab y .strtab
  │     ├─ parsing_symbol_ent()    → crear lista de t_symbol_info
  │     ├─ extr_detc_symbol_type() → asignar char_type
  │     ├─ tilter_collecting()     → marcar visible/invisible
  │     └─ ordering_symbols()      → merge sort (y reverse si -r)
  │
  └─ process_static_archive()
        └─ por cada miembro → process_elf_file() recursivo
  │
  ▼
ft_output()            → write() formateado por stdout/stderr
  │
  ▼
clear_closing()        → munmap + free de toda la memoria
```