#ifndef NM_H
 #define NM_h

 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/mman.h>
 #include <errno.h>
 #include <elf.h>

 typedef struct s_data
 {
    char*         filename;
    size_t        filesize;
    struct stat   st;
    void*         map;

    Elf64_Ehdr*   ehdr;
    Elf64_Shdr*   shdr;
    char*         shstrtab;

    Elf64_Sym*    symtab;
    char*         strtab;
    size_t        sym_count;

    char*         error;
 } t_data;
 

 //<-- UTILS -->
 int    ft_strlen(char *str);
 void   init_data(t_data* data);
 int    check_if_elf(int fd);

#endif