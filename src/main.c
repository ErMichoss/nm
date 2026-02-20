#include "ft_nm.h"

int     checkarg(char *file_name) {
    struct stat my_file_info;
    int         fd;
    int         success;
    
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

void    create_list(t_stack_file **sfile, char **str) {
    int         i;
    int         validity;
    t_nmflags   active_flags;

    i = 0;
    active_flags = NM_ARG_FILE;

    while (str[i] != NULL)
    {
        if (findflags(str[i]))
        {
            if (ft_strcmp(str[i], "-a") == 0)
                active_flags |= NM_FLAG_A;
            else if (ft_strcmp(str[i], "-g") == 0)
                active_flags |= NM_FLAG_G;
            else if (ft_strcmp(str[i], "-u") == 0)
                active_flags |= NM_FLAG_U;
            else if (ft_strcmp(str[i], "-r") == 0)
                active_flags |= NM_FLAG_R;
            else if (ft_strcmp(str[i], "-p") == 0)
                active_flags |= NM_FLAG_P;
        }
        i++;
    }

    i = 0;
    while (str[i] != NULL)
    {
        if (!findflags(str[i]))
        {
            validity = checkarg(str[i]);
            t_stack_file *node = create_node(str[i], i, validity);
            node->flag = active_flags;
            stack_node(sfile, node);
        }
        i++;
    }
}

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

    sfile = NULL;
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
        create_list(&sfile, arg);
    } else {
		argv++;
		create_list(&sfile, argv);
	}
    file_format_id(&sfile);
    process_file_list(&sfile); 
    ft_output(&sfile, argc);
    
    clear_closing(&sfile);
    
    return (0);
}