#include "nm.h"

/**
 * @brief Calculates the length of the string
 * @param str The string
 * @return Returns the number of bytes in the string
 */
int ft_strlen(char *str){
    int i = 0;
    while (str[i] != '\0'){
        i++;
    }
    return i;
}

/**
 * @brief Initialices de struc so its ready to be used
 * @param data A pointer to the struct
 */
void init_data(t_data* data) {
    data->error = NULL;
}

/**
 * @brief Checks if the file is a ELF file.
 * @param fd The file descriptor of the file
 * @return If it is a ELF file it returns 0, if not it will return -1
 */
int check_if_elf(int fd){
    unsigned char buf[4];
    ssize_t bytes_read;

    bytes_read = read(fd, buf, 4);
    if (bytes_read < 4) {
        return -1;
    }

    if (buf[0] == 0x7f && buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F') {
        return 0;
    } else {
        return -1;
    }
}