#include "nm.h"

/**
 * @brief Checks if the binary can be access correctly for the program
 * @param filename The binary's name
 * @param data THe data struct
 * @return on success it returns 0, on failure 1
 */
int ft_check_binary(char *filename, t_data* data) {//TODO checker si es un archivo o un dir.
    int         fd;
    struct stat buf;
    void        *map;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        data->error = "The file can not be opened\n";
        return 1;
    }

    if (check_if_elf(fd) == -1){
        data->error == "Not an ELF file\n";
        close(fd);
        return 1;
    }

    if (fstat(fd, &buf) == -1) {
        data->error = strerror(errno);
        close(fd);
        return 1;
    }

    map = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (map == MAP_FAILED){
        data->error = strerror(errno);
        close(fd);
        return 1;
    }

    data->map = map;
    data->filesize = buf.st_size;

    return 0;
}

/**
 * @brief Checks the ELF header
 * @param data The data struct
 * @return on success 0, on failure 1
 */
int ft_read_efl_header(t_data* data) { //TODO, añadir para 32 bits una vez completado para 64.
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)data->map;
    
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        data->error = "The file is not 64bits\n";
        return 1;
    }

    data->ehdr = ehdr;
    return 0;
}

/** 
 * Interpretar ELF header → validar e inicializar punteros.
 *
 * Acceder a la tabla de secciones (Elf64_Shdr).
 *
 * Usar .shstrtab para encontrar .symtab y .strtab.
 *
 * Leer símbolos desde .symtab (Elf64_Sym).
 *
 * Imprimirlos (imitando nm).
*/

/**
 * @brief It will reproduce nm return
 * @param filename The binary's name
 * @param data The data struct
 * @return on success 0, on failure 1
 */
int ft_get_binary_data(char *filename, t_data* data) { //revisar endianness porsiaca
    if (ft_check_binary(filename, data) == 1) {
        return 1;
    }
    if (ft_read_elf_header(data) == 1) {
        return 1;
    }
}

int main(int argc, char *argv[]) {
    t_data* data;

    if (argc < 2) {
        write(2, "ft_nm: «a.out»: No such file\n", 32);
        return 1;
    }

    data = malloc(sizeof(t_data) * (argc - 1));
    if (!data) {
        write(2, "Memory allocation failed\n", 26);
        return 1;
    }

    for (int i = 1; argv[i] != NULL; i++) {
        init_data(&data[i - 1]);
        ft_get_binary_data(argv[i], &data[i - 1]);
    }

    free(data);

    return 0;
}