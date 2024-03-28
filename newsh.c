/* Isis Ardisson Logullo 7577410
* 
* gcc newsh.c -o shell -lreadline -ltermcap
*/

#include "newsh.h"
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <readline/readline.h> 
#include <readline/history.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

void config_terminal(char * terminal) {
  char *usuario;
  usuario = getenv("USER");
  char hora[40];
  struct timeval time;
  gettimeofday(&time, NULL);
  strftime(hora, 40, "%T", localtime(&time.tv_sec));

  strcat(terminal, usuario);
  strcat(terminal, " [");
  strcat(terminal, hora);
  strcat(terminal, "]: ");
}

void le_comando(char **comando, char **parametros) {
  char terminal[1024] = "";
  char *linha;

  config_terminal(terminal);
  linha = readline(terminal);

  add_history(linha);

  *comando = strsep(&linha, " ");
  *parametros = linha;
}

void executa_bin(char *path, char *parametros) {
  pid_t child;
  int i = 1, j, pos = -1;
  char *args[30];

  for (j = 0; path[j] != '\0' ; j++)
    if (path[j] == '/')
      pos = j;

  if (j != 0) {
    pos++;
    args[0] = &path[pos];

    while (parametros != NULL) {
      args[i] = strsep(&parametros, " ");
      i++;
    }

    args[i] = NULL;

    if ((child = fork()) == 0) {
      if (execvp(path, args) != 0) {
        printf("COMANDO INVÁLIDO!!!\n");
        exit(1);
      }
    }
    else {
      waitpid(-1, NULL, 0);
    }
  }
}

int executa_comando(char *comando, char *parametros) {
  char *flag, *oldname, *newname;
  int executing = 1;

  if (strcmp(comando, "mkdir") == 0) {
    if (mkdir(parametros, 0755) == 0) {
      printf("Criado diretório %s\n", parametros);
    }
    else {
      printf("Deu ruim!\n");
    }
  }
  else if (strcmp(comando, "kill") == 0) {
    int signal;
    pid_t pid;

    flag = strsep(&parametros, " ");
    signal = flag[1] - '0';
    pid = atoi(parametros);

    printf("Flag: %s\n", flag);
    printf("Signal: %d\n", signal);
    printf("PID: %d\n", pid);
    
    if (kill(pid, signal) == 0) {
      printf("Processo %d finalizado!\n", pid);
    }
    else {
      printf("Deu ruim!\n");
    }
  }
  else if (strcmp(comando, "ln") == 0) {
    flag = strsep(&parametros, " ");
    oldname = strsep(&parametros, " ");
    newname = parametros;

    printf("Flag: %s\n", flag);
    printf("Oldname: %s\n", oldname);
    printf("Newname: %s\n", newname);
    
    if (symlink(oldname, newname) == 0) {
      printf("Link para %s criado com sucesso!\n", oldname);
    }
    else {
      printf("Deu ruim!\n");
    }
  }
  else if (strcmp(comando, "cd") == 0) {
    if (chdir(parametros) == 0) {
      printf("Deu certo!\n");
    }
    else {
      printf("Diretório inválido!\n");
    }
  }
  else if (strcmp(comando, "exit") == 0) {
    executing = 0;
  }
  else {
    executa_bin(comando, parametros);
  }

  return executing;
}

int main(int argc, char **argv) {
  char *comando, *parametros;
  int ativo = 1;

  while (ativo) {
    comando = NULL;
    parametros = NULL;

    le_comando(&comando, &parametros);
    ativo = executa_comando(comando, parametros);

    if (comando != NULL)
      free(comando);
  }

  exit(0);
}