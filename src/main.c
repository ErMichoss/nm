#include "ft_nm.h"

int     checkarg(char *file_name) {
    struct stat my_file_info;
    int         fd;
    int         success;
    
    if (findflags(file_name))
        return (1);
    fd = open(file_name, O_RDONLY);
    if (fd == -1)
        return (errno == EISDIR ? -2 : -1);
    success = fstat(fd, &my_file_info);
    if (success == -1)
    {
        close(fd);
        return (-3);
    }
    if (S_ISDIR(my_file_info.st_mode))
    {
        close(fd);
        return (-2);
    }
    
    close(fd);
    return (1);
}

void    create_list(t_stack_file **sfile, char **str, int *flag) {
    int i = 0;
    int validity = 1;

    while (str[i] != NULL)
    {   
        validity = checkarg(str[i]);
        if (!validity)
            *flag += 1;
        stack_node(sfile, create_node(str[i], i, validity));
        validity = 1;
        i++;
    }
    return;
}

/*int find_binaries(char*** binaries) {
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    int count = 0;
    int i = 0;

    dir = opendir(".");
    if (!dir)
        return (1);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (stat(entry->d_name, &st) != 0 || !S_ISREG(st.st_mode))
            continue;
        if (is_valid_binary(entry->d_name))
            count++;
    }

    *binaries = (char**)malloc((count + 1) * sizeof(char*));
    if (!*binaries) {
        closedir(dir);
        return (1);
    }

    rewinddir(dir);
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (stat(entry->d_name, &st) != 0 || !S_ISREG(st.st_mode))
            continue;
        if (is_valid_binary(entry->d_name)) {
            (*binaries)[i] = ft_strdup(entry->d_name);
            if (!(*binaries)[i]) {
                closedir(dir);
                return (1);
            }
            i++;
        }
    }
    (*binaries)[i] = NULL;
    closedir(dir);
    return (0);
}*/

int find_binaries() {
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    int flag = 1;

    dir = opendir(".");
    if (!dir)
        return (1);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (stat(entry->d_name, &st) != 0 || !S_ISREG(st.st_mode))
            continue;
        if (strcmp(entry->d_name, "a.out") == 0)
            flag = 0;
    }

    return flag;
}

int main(int argc, char **argv) {
    t_stack_file    *sfile;
    int             flag;

    sfile = NULL;
    flag = 0;
	init_host_endianness();
    if (argc == 1)
    {
		if (find_binaries() == 1){
			putstr_stderr("ft_nm: «a.out»: No such file\n");
        	return (1);
		}
        char* arg[2];
        arg[0] = "a.out";
        arg[1] = NULL;
        create_list(&sfile, arg, &flag);
    } else {
		argv++;
		create_list(&sfile, argv, &flag);
	}
    flag = 0;
    file_format_id(&sfile, flag);
    process_file_list(&sfile); 
    ft_output(&sfile, argc);
    
    clear_closing(&sfile);
    
    return (0);
}