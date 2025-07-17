#include "nm.h"

/**
 * @brief Checks if the binary can be access correctly for the program
 * @param filename The binary's name
 * @return on success it returns 0, on failure 1
 */
int ft_check_binary(char *filename) { //TODO Debemos almacenar el error, porque se imprimen al final de la ejecucion
    int         fd;
    struct stat buf;
    void        *map;
    const char  *err;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        write(2, "The file can not be opened\n", 28);
        return 1;
    }

    if (fstat(fd, &buf) == -1) {
        err = strerror(errno);
        write(2, err, ft_strlen(err));
        write(2, "\n", 1);
        return 1;
    }

    map = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED){
        err = strerror(errno);
        write(2, err, ft_strlen(err));
        write(2, "\n", 1);
        return 1;
    }
}

/**
 * @brief It will reproduce nm return
 * @param filename The binary's name
 * @return on success 0, on failure 1
 */
int ft_get_binary_data(char *filename) {
    //TODO agregar la struct y pasarsela a ft_check_binary para que se inicialice, desde la struct empezare con el programa de verdad

    if (ft_check_binary(filename) == 1) {
        return 1;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        write(2, "You must intorduce at leat on file name\n", 41);
        return 1;
    }

    for (int i = 1; argv[i] != NULL; i++) {
        ft_get_binary_data(argv[i]);
    }
    return 0;
}