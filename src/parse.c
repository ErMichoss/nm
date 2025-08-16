#include "nm.h"

int parse_section(t_data *data){
    data->ehdr = (Elf64_Ehdr *)data->map;

    data->shdr = (Elf64_Shdr *)((char *)data->map + data->ehdr->e_shoff);

    Elf64_Shdr *shstr_shdr = &data->shdr[data->ehdr->e_shstrndx];
    data->shstrtab = (char *)data->map + shstr_shdr->sh_offset;

    for (int i = 0; i < data->ehdr->e_shnum; i++) {
        Elf64_Shdr *sh = &data->shdr[i];
        const char *name = data->shstrtab + sh->sh_name;

        if (sh->sh_type == SHT_SYMTAB) {
            data->symtab = (Elf64_Sym *)((char *)data->map + sh->sh_offset);
            data->sym_count = sh->sh_size / sizeof(Elf64_Sym);
            data->strtab = (char *)data->map + data->shdr[sh->sh_link].sh_offset;
        }
    }

    return 0;
}