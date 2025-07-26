#include "nm.h"

/**
 * @brief Checks if the binary can be access correctly for the program
 * @param filename The binary's name
 * @return on success it returns 0, on failure 1
 */
int ft_check_binary(char *filename, t_data* data) { //TODO Analizar tabla de symbolos, Limpieza, esos son los paso.
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
    if (map == MAP_FAILED){
        data->error = strerror(errno);
        close(fd);
        return 1;
    }
}

/**
 * @brief It will reproduce nm return
 * @param filename The binary's name
 * @return on success 0, on failure 1
 */
int ft_get_binary_data(char *filename, t_data* data) {
    if (ft_check_binary(filename, data) == 1) {
        return 1;
    }
}

int main(int argc, char *argv[]) {
    t_data* data;

    if (argc < 2) {
        write(2, "You must intorduce at leat on file name\n", 41);
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