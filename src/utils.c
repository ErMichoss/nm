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