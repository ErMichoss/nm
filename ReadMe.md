# ft_nm

Reimplementación del comando `nm` de Unix/Linux en C, capaz de listar los símbolos de archivos objeto ELF (32 y 64 bits) y archivos estáticos `.a`.

---

## Compilación

```bash
make        # Compila el binario ft_nm
make clean  # Elimina los archivos objeto (.o)
make fclean # Elimina objetos y el binario
make re     # Recompila desde cero
```

---

## Uso

```bash
./ft_nm [opciones] [archivo1] [archivo2] ...
```

Si no se especifica ningún archivo, `ft_nm` busca `a.out` en el directorio actual.

### Argumentos

| Argumento | Descripción |
|-----------|-------------|
| `archivo` | Archivo ELF (binario, `.o`, `.so`) o archivo estático (`.a`) |

### Opciones / Flags

| Flag | Descripción |
|------|-------------|
| `-a` | Muestra **todos** los símbolos, incluyendo los de tipo `FILE` y `SECTION` que normalmente se ocultan |
| `-g` | Muestra solo símbolos **globales** (excluye los locales / `STB_LOCAL`) |
| `-u` | Muestra solo símbolos **indefinidos** (`SHN_UNDEF`) |
| `-r` | Muestra los símbolos en orden **inverso** (de Z a A) |
| `-p` | **No ordena**: muestra los símbolos en el orden en que aparecen en la tabla de símbolos |

Los flags son acumulables entre sí y pueden combinarse con múltiples archivos.

---

## Ejemplos de uso

```bash
# Símbos del binario actual
./ft_nm a.out

# Archivo objeto
./ft_nm main.o

# Solo símbolos globales de una librería estática
./ft_nm -g libfoo.a

# Varios archivos a la vez
./ft_nm file1.o file2.o

# Símbolos no definidos (dependencias externas)
./ft_nm -u programa

# Orden inverso sin ordenar
./ft_nm -p -r archivo.o

# Incluir símbolos de sección y archivo
./ft_nm -a archivo.o
```

---

## Formato de salida

Cada línea de salida tiene el siguiente formato:

```
<valor>  <tipo>  <nombre>
```

- **Valor**: dirección en hexadecimal (16 dígitos para 64 bits, 8 para 32 bits). Si el símbolo es indefinido (`U`), se muestran espacios en blanco.
- **Tipo**: un carácter que indica la naturaleza del símbolo (ver tabla).
- **Nombre**: el nombre del símbolo.

### Tabla de tipos de símbolo

| Carácter | Significado |
|----------|-------------|
| `T` / `t` | Sección de código (`.text`) — global / local |
| `D` / `d` | Datos inicializados (`.data`) — global / local |
| `B` / `b` | Datos no inicializados (`.bss`) — global / local |
| `R` / `r` | Solo lectura (`.rodata`) — global / local |
| `U` | Símbolo **indefinido** (externo, no resuelto) |
| `W` / `w` | Símbolo **débil** (`STB_WEAK`) definido / indefinido |
| `V` / `v` | Objeto débil (`STB_WEAK + STT_OBJECT`) definido / indefinido |
| `A` / `a` | Símbolo **absoluto** (`SHN_ABS`) — global / local |
| `C` / `c` | Símbolo **común** (`SHN_COMMON`) — global / local |
| `N` | Símbolo de depuración / metadatos (`.debug`, `.comment`, `.note`) |
| `I` / `i` | Función GNU indirecta (`STT_GNU_IFUNC`) — global / local |
| `u` | Símbolo GNU único (`STB_GNU_UNIQUE`) |
| `?` | Tipo desconocido |

Las letras en **mayúscula** indican símbolo global; en **minúscula**, local.

---

## Tipos de archivo soportados

- **ELF 32 bits**: binarios y objetos compilados para arquitecturas x86 o ARM 32 bits.
- **ELF 64 bits**: binarios y objetos compilados para x86_64, ARM64, etc.
- **Archivos estáticos `.a`**: contienen múltiples objetos ELF. Se muestran con cabecera por miembro.

---

## Mensajes de error

| Mensaje | Causa |
|---------|-------|
| `ft_nm: «archivo»: No such file or directory` | El archivo no existe |
| `ft_nm: Warning: «dir»: Is a directory` | Se pasó un directorio como argumento |
| `ft_nm: archivo: file format not recognized` | El archivo no es ELF ni `.a` |
| `ft_nm: archivo: no symbols` | El archivo ELF no tiene tabla de símbolos |
| `ft_nm: «a.out»: No such file` | Se ejecutó sin argumentos y no existe `a.out` |