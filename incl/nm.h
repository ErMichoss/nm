#ifndef NM_H
 #define NM_h

 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/mman.h>
 #include <errno.h>
 
 #define EI_NIDENT 16

 //<-- ELF_64 typedefs
 /**
  * @brief Unsigned 16-bit integer (2 bytes). Used for small fields like counts or indexes.
  */
 typedef unsigned short Elf64_Half;
 /**
  * @brief Unsigned 32-bit integer (4 bytes). Used for standard-sized integer fields.
  */
 typedef unsigned int   Elf64_Word;
 /**
  * @brief Unsigned 64-bit integer (8 bytes). Used for large size or offset fields.
  */
 typedef unsigned long  Elf64_Xword;
 typedef unsigned long  Elf64_Addr;
 typedef unsigned long  Elf64_Off;
 
 //<-- STRUCTS -->

 /**
  * @brief This is the main header at the start of an ELF file. It contains essential info about the file format and where to find other parts
  * @param e_indent ELF identification
  * @param e_type File type
  * @param e_machine Target architecture
  * @param e_version ELF version
  * @param e_entry Entry point address
  * @param e_phoff Offset to program header table
  * @param e_shoff Offset to section header table
  * @param e_flags Processor specific flags
  * @param e_ehsize ELF header size
  * @param e_phentsize Size of each program header entry
  * @param e_phnum Number of program header entries
  * @param e_shentsize Size of each section header entry
  * @param e_shnum Number of sections header entrues
  * @param e_shstrndx Index of section header string table (section names)
 */  
 typedef struct s_Elf64_Ehdr{
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half    e_type;
    Elf64_Half    e_machine;
    Elf64_Word    e_version;
    Elf64_Addr    e_entry;
    Elf64_Off     e_phoff;
    Elf64_Off     e_shoff;
    Elf64_Word    e_flags;
    Elf64_Half    e_ehsize;
    Elf64_Half    e_phentsize;
    Elf64_Half    e_phnum;
    Elf64_Half    e_shentsize;
    Elf64_Half    e_shnum;
    Elf64_Half    e_shstrndx;
 } Elf64_Ehdr;

 typedef struct s_Elf64_Shdr{
    Elf64_Word  sh_name;
    Elf64_Word  sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr  sh_addr;
    Elf64_Off   sh_offset;
    Elf64_Xword sh_size;
    Elf64_Word  sh_link;
    Elf64_Word  sh_info;
    Elf64_Xword sh_addralign;
    Elf64_Xword sh_entsize;
 } Elf64_Shdr;

 typedef struct s_Elf64_Sym{
    Elf64_Word      st_name;
    unsigned char   st_info;
    unsigned char   st_other;
    Elf64_Half      st_shndx;
    Elf64_Addr      st_value;
    Elf64_Xword     st_size;
 } Elf64_Sym;

 typedef struct s_data
 {
    char *        filename;
    int           fd;
    struct stat   st;
    void *        map;

    Elf64_Ehdr *  ehdr;
    Elf64_Shdr *  shdr;
    char *        shstrtab;

    Elf64_Sym *   symtab;
    char *        strtab;
    size_t        sym_count;

    char *        error;
 } t_data;
 

 //<-- UTILS -->
 int    ft_strlen(char *str);
 void   init_data(t_data* data);
 int    check_if_elf(int fd);

#endif