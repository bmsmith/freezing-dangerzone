#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int errno;

typedef void (*sighandler_t)(int);
static char *my_argv[100], *my_envp[100];
static char *search_path[10];

void handle_signal(int signo)
{
    printf("\n[MY_SHELL ] ");
    fflush(stdout);
}

void fill_argv(char *tmp_argv)
{
    char *foo = tmp_argv;
    int index = 0;
    char ret[100];
    bzero(ret, 100);
    while(*foo != '\0') {
        if(index == 10)
            break;

        if(*foo == ' ') {
            if(my_argv[index] == NULL)
                my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
            else {
                bzero(my_argv[index], strlen(my_argv[index]));
            }
            strncpy(my_argv[index], ret, strlen(ret));
            strncat(my_argv[index], "\0", 1);
            bzero(ret, 100);
            index++;
        } else {
            strncat(ret, foo, 1);
        }
        foo++;
        /*printf("foo is %c\n", *foo);*/
    }
    my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
    strncpy(my_argv[index], ret, strlen(ret));
    strncat(my_argv[index], "\0", 1);
}

void copy_envp(char **envp)
{
    int index = 0;
    for(;envp[index] != NULL; index++) {
        my_envp[index] = (char *)
		malloc(sizeof(char) * (strlen(envp[index]) + 1));
        memcpy(my_envp[index], envp[index], strlen(envp[index]));
    }
}

void get_path_string(char **tmp_envp, char *bin_path)
{
    int count = 0;
    char *tmp;
    while(1) {
        tmp = strstr(tmp_envp[count], "PATH");
        if(tmp == NULL) {
            count++;
        } else {
            break;
        }
    }
        strncpy(bin_path, tmp, strlen(tmp));
}

void insert_path_str_to_search(char *path_str) 
{
    int index=0;
    char *tmp = path_str;
    char ret[100];

    while(*tmp != '=')
        tmp++;
    tmp++;

    while(*tmp != '\0') {
        if(*tmp == ':') {
            strncat(ret, "/", 1);
            search_path[index] = 
		(char *) malloc(sizeof(char) * (strlen(ret) + 1));
            strncat(search_path[index], ret, strlen(ret));
            strncat(search_path[index], "\0", 1);
            index++;
            bzero(ret, 100);
        } else {
            strncat(ret, tmp, 1);
        }
        tmp++;
    }
}

int attach_path(char *cmd)
{
    char ret[100];
    int index;
    int fd;
    bzero(ret, 100);
    for(index=0;search_path[index]!=NULL;index++) {
        strcpy(ret, search_path[index]);
        strncat(ret, cmd, strlen(cmd));
        if((fd = open(ret, O_RDONLY)) > 0) {
            strncpy(cmd, ret, strlen(ret));
            close(fd);
            return 0;
        }
    }
    return 0;
}

void call_execve(char *cmd)
{
    int i;
    printf("cmd is %s\n", cmd);
    if(fork() == 0) {
        i = execve(cmd, my_argv, my_envp);
        printf("errno is %d\n", errno);
        if(i < 0) {
            printf("%s: %s\n", cmd, "command not found");
            exit(1);        
        }
    } else {
        wait(NULL);
    }
}

void free_argv()
{
    int index;
    for(index=0;my_argv[index]!=NULL;index++) {
        bzero(my_argv[index], strlen(my_argv[index])+1);
        my_argv[index] = NULL;
        free(my_argv[index]);
    }
}

int get_split_index() 
{
	int i;
	int return_value = -1;
	for(i=0; my_argv[i] != NULL; i++)
	{
		//printf("%s\n", my_argv[i] );
		if( strcmp(my_argv[i], ">") == 0)
			return_value = i;
		else if(strcmp(my_argv[i], "<") == 0)
			return_value = i;
		else if(strcmp(my_argv[i], "|") == 0)
			return_value = i;
	}
	return return_value;
}

void our_pipe(split_index)
{
  int fds[2];
  int child[2];
  char *args_out[100];
  char *args_in[100];
  char *argv[3];
  int i;
  
  for(i=0; i<100; i++)
  {
	args_out[i] = NULL;
	args_in[i] = NULL;
  }
  
  for(i=0; i < split_index; i++)
  {
	args_out[i] = my_argv[i];
	//printf("%s ", args_out[i]);
  }
  
  //printf("\n");
  for(i=1; my_argv[i+split_index] != NULL; i++)
  {
	args_in[i-1] = my_argv[i+split_index];
	//printf("%s ", args_in[i-1]);
  }
  //printf("\n");
  
  
  
  pipe(fds);
  if (fork()== 0) {
    close(fds[1]);
    close(STDIN_FILENO); dup(fds[0]); /* redirect standard input to fds[1] */
    argv[0] = "superbash.bash";
    argv[1] = NULL;           /* check how the argv array is set */
    execv(args_in[0], argv);// here how execv reads from stdin ??
    exit(0);


  }
  if (fork() == 0) {
    close(fds[0]);
    close(STDOUT_FILENO); dup(fds[1]);  /* redirect standard output to fds[0] */
    argv[0] = "echo";
    argv[1] = NULL;
    execv(args_out[0], args_out);
    exit(0);

  }

  close(fds[1]);
  wait(&child[0]);
  wait(&child[0]);  

} 

void write_file(split_index)
{
  int fds[2];
  int child[2];
  char *args_exe[100];
  char *args_file[100];
  char *argv[3];
  char c;
  //char c[1000];
  int i;

  
  FILE *out_file;
  
  for(i=0; i<100; i++)
  {
	args_file[i] = NULL;
	args_exe[i] = NULL;
  }
  
  for(i=0; i < split_index; i++)
  {
	args_exe[i] = my_argv[i];
	//printf("%s ", args_out[i]);
  }
  
  //printf("\n");
  for(i=1; my_argv[i+split_index] != NULL; i++)
  {
	args_file[i-1] = my_argv[i+split_index];
	//printf("%s ", args_in[i-1]);
  }
  //printf("\n");
  
  
  
  pipe(fds);

  if (fork() == 0) {
    close(fds[0]);
    close(STDOUT_FILENO); dup(fds[1]); 
    execv(args_exe[0], args_exe);
    exit(0);
	
	//printf("wat is happen?");

  }
  if (fork()== 0) {
    close(fds[1]);
    close(STDIN_FILENO); dup(fds[0]); 
   
	//printf("i = 5\n");
	
	out_file  = fopen(args_file[0], "w");
	//in_file  = fopen("input", "r");
	
	if( out_file != NULL)
	{
		while(c != EOF) {
			c = getchar();
			fprintf(out_file, "%c", c);
		}
		
		fclose(out_file);
	}
	
	
	

	
	
    exit(0);


  }

  close(fds[1]);
  wait(&child[0]);
  wait(&child[0]);  
  
} 

void read_file(split_index)
{
  int fds[2];
  int child[2];
  char *args_exe[100];
  char *args_file[100];
  char *argv[3];
  //char c[1000];
  int i;
  
  char * buffer = 0;
  int length;
  
  FILE *in_file;
  
  for(i=0; i<100; i++)
  {
	args_file[i] = NULL;
	args_exe[i] = NULL;
  }
  
  for(i=0; i < split_index; i++)
  {
	args_exe[i] = my_argv[i];
	//printf("%s ", args_out[i]);
  }
  
  //printf("\n");
  for(i=1; my_argv[i+split_index] != NULL; i++)
  {
	args_file[i-1] = my_argv[i+split_index];
	//printf("%s ", args_in[i-1]);
  }
  //printf("\n");
  
  
  
  pipe(fds);

  if (fork() == 0) {
    close(fds[1]);
    close(STDIN_FILENO); dup(fds[0]);  /* redirect standard output to fds[0] */
    execv(args_exe[0], args_exe);
    exit(0);
	
	//printf("wat is happen?");

  }
  if (fork()== 0) {
    close(fds[0]);
    close(STDOUT_FILENO); dup(fds[1]); /* redirect standard input to fds[1] */ 
   
	//printf("i = 5\n");
	
	in_file  = fopen(args_file[0], "r");
	//in_file  = fopen("input", "r");
	
	if( in_file != NULL)
	{
		/*
		fscanf(in_file,"%s",c);
		printf("%s", c);
		fclose(in_file);*/
		
		fseek (in_file, 0, SEEK_END);
		length = ftell (in_file);
		fseek (in_file, 0, SEEK_SET);
		buffer = malloc (length);
		  if (buffer)
		  {
			fread (buffer, 1, length, in_file);
		  }
	}
	
	
	
	if (buffer)
	{
		printf("%s", buffer);
	}
	
	
    exit(0);


  }

  close(fds[1]);
  wait(&child[0]);
  wait(&child[0]);  
  
} 

int get_command_type(int split_index)
{	
	if( split_index == -1)
	{
		//i may regret my specific implementation here. It will work for now
		return 0;
	}
	else if(my_argv[split_index][0] == '<') //redirection
		return 2;
	else if(my_argv[split_index][0] == '>')
		return 3;
	else if(my_argv[split_index][0] == '|') //pipe
		return 1;
}



int main(int argc, char *argv[], char *envp[])
{
	// here be some code from Jonathan
	int command_type = 0; //this helps differentiate what we're doing. 0 is the normal execution, 1 is for pipes, 2 and 3 are for read from and write to file, 4 can be used for background process
	int split_index; //this is the index of <, >, or | should there be one.
	
	
    char c;
    int i, fd;
    char *tmp = (char *)malloc(sizeof(char) * 100);
    char *path_str = (char *)malloc(sizeof(char) * 256);
    char *cmd = (char *)malloc(sizeof(char) * 100);
    
    signal(SIGINT, SIG_IGN);
    signal(SIGINT, handle_signal);

    copy_envp(envp);

    get_path_string(my_envp, path_str);   
    insert_path_str_to_search(path_str);

    if(fork() == 0) {
        execve("/usr/bin/clear", argv, my_envp);
        exit(1);
    } else {
        wait(NULL);
    }
    printf("[MY_SHELL ] ");
    fflush(stdout);

    while(c != EOF) {
        c = getchar();
        switch(c) {
            case '\n': if(tmp[0] == '\0') {
                       printf("[MY_SHELL ] ");
                   } else {
				   
						//we need to determine if there was a <, a >, or a |
						//search the args until we find one, then go with it.
						
						// if one of those symbols exist, then we find a second command to execute.
						
				   
				   
                       fill_argv(tmp);
                       strncpy(cmd, my_argv[0], strlen(my_argv[0]));
                       strncat(cmd, "\0", 1);
					   
					   //mor of jonathan's silly code. This is here to select what kind of operation we're doing. Trust me, you want to use the default.
					   split_index = get_split_index();
					   command_type = get_command_type(split_index);
					   
					   if(command_type == 0)
					   {
						   //sweany's original code. will execute if command type 0 is chosen
						   if(index(cmd, '/') == NULL) {
								//printf("forgive me for failing to understand sweany's silliness.");
							   if(attach_path(cmd) == 0) {
								   call_execve(cmd);
							   } else {
								   printf("%s: command not found\n", cmd);
							   }
						   } else {
							   if((fd = open(cmd, O_RDONLY)) > 0) {
								   close(fd);
								   call_execve(cmd);
							   } else {
								   printf("%s: command not found\n", cmd);
							   }
						   }
						}
						else if (command_type == 1)
						{							
							our_pipe(split_index);							
						}
						else if (command_type == 2)
						{							
							//our_pipe(split_index);
							//printf("you chose <");
							read_file(split_index);
						}
						else if (command_type == 3)
						{							
							//our_pipe(split_index);	
							write_file(split_index);
						}
					   
					   
                       free_argv();
                       printf("[MY_SHELL ] ");
                       bzero(cmd, 100);
					   
					   
					   
					   
                   }
                   bzero(tmp, 100);
                   break;
            default: strncat(tmp, &c, 1);
                 break;
        }
    }
    free(tmp);
    free(path_str);
    for(i=0;my_envp[i]!=NULL;i++)
        free(my_envp[i]);
    for(i=0;i<10;i++)
        free(search_path[i]);
    printf("\n");
    return 0;
}
