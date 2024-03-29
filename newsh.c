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
#include <sys/utsname.h>


void executa_bin(char *caminho, char *parametros) {
  pid_t filho;
  int i = 1, j, pos = -1;
  char *args[30];

  for (j = 0; caminho[j] != '\0' ; j++)
    if (caminho[j] == '/')
      pos = j;

  if (j != 0) {
    pos++;
    args[0] = &caminho[pos];

    while (parametros != NULL) {
      args[i] = strsep(&parametros, " ");
      i++;
    }

    args[i] = NULL;

    if ((filho = fork()) == 0) {
      if (execvp(caminho, args) != 0) {
        printf("Esse comando nao eh valido\n");
        exit(1);
      }
    }
    else {
      waitpid(-1, NULL, 0);
    }
  }
}

int executa_comando(char *comando, char *parametros) {
  int ligado = 1;

  if (strcmp(comando, "cd") == 0) {
    if (chdir(parametros) == 0) {
      printf("%s" , parametros);
    }
    else {
      printf("Erro\n");
    }
  }
  else if (strcmp(comando, "rm") == 0) {
    if (unlink(parametros) == 0) {
      printf("Arquivo removido: %s\n", parametros);
    }
    else {
      printf("Erro\n");
    }
  }
  else if (strcmp(comando, "uname") == 0) {
    struct utsname u;
    uname (&u);
    printf ("%s release %s (version %s) on %s\n", u.sysname, u.release, 
     u.version, u.machine);
  }
  else if (strcmp(comando, "exit") == 0) {
    ligado = 0;
  }
  else {
    executa_bin(comando, parametros);
  }

  return ligado;
}

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