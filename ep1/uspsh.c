/* Isis Ardisson Logullo 7577410 */

#include "uspsh.h"
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <readline/readline.h> 
#include <readline/history.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>


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
  else if (strcmp(comando, "whoami") == 0) {
    uid_t uid;
    struct passwd *user;
    uid = geteuid();
    if((user = getpwuid(uid)) == NULL)
      perror("erro");
    else  
      printf("%s\n" , user->pw_name);
  }
  else if (strcmp(comando, "chmod") == 0) {
    char *modo = strsep(&parametros, " ");; 
    char *file = parametros; 
    chmod(file, atoi(modo));
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
  char computador[30];
  gethostname(computador, 30);
  char cwd[100];
  getcwd(cwd, 100);

  strcat(terminal, "[");
  strcat(terminal, computador);
  strcat(terminal, ":");
  strcat(terminal, cwd);
  strcat(terminal, "]$ ");
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