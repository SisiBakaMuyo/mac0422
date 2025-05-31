// Isis Ardisson Logullo nUSP: 7577410

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *arquivoTrace = NULL;
FILE *arquivoSaida = NULL; 
int falhas = 0;
int nextFitIni = 0; 
long cabecalho = 0;


// Manipulador de arquivo
void geraSaida(FILE *arquivoEntrada) {

    char buffer[100];
    int bit;

    fgets(buffer, sizeof(buffer), arquivoEntrada);
    fgets(buffer, sizeof(buffer), arquivoEntrada);
    fgets(buffer, sizeof(buffer), arquivoEntrada);

    fprintf(arquivoSaida, "P2\n");
    fprintf(arquivoSaida, "256 256\n");
    fprintf(arquivoSaida, "255\n");

    for (int i = 0; i < 65536; i++) {

        fscanf(arquivoEntrada, "%d", &bit);

        if (i % 16 == 0) {
            fprintf(arquivoSaida, "%*d", 3, bit);
        } else if ((i + 1) % 16 == 0) {
            fprintf(arquivoSaida, " %*d\n", 3, bit);
        } else {
            fprintf(arquivoSaida, " %*d", 3, bit);
        }
    }

    fclose(arquivoEntrada);
}

// Gerenciadores
long getCabecalho() {

    long posicaoIni = ftell(arquivoSaida);
    char buffer[100];
    long cabecalhoTam = 0;

    fseek(arquivoSaida, 0, SEEK_SET);
    fgets(buffer, sizeof(buffer), arquivoSaida);
    fgets(buffer, sizeof(buffer), arquivoSaida);
    fgets(buffer, sizeof(buffer), arquivoSaida);
    cabecalhoTam = ftell(arquivoSaida);
    fseek(arquivoSaida, posicaoIni, SEEK_SET);

    return cabecalhoTam;
}

long contaEspaco(int posicao) {

    int m = posicao / 16;      
    int n = posicao % 16;    
    long espacoAnt = (long)m * 64;
    long espaco = (long)n * 3;

    if (n > 0) {
        espaco += n; 
    }

    return cabecalho + espacoAnt + espaco;
}

int getBit(int posicao) {

    int bit = -1;
    long espaco = contaEspaco(posicao);
    long posicaoAtual = ftell(arquivoSaida);
    
    fseek(arquivoSaida, espaco, SEEK_SET); 
    fscanf(arquivoSaida, "%d", &bit);
    fseek(arquivoSaida, posicaoAtual, SEEK_SET);

    return bit;
}

void setBit(int posicao, int bit) {
    
    long espaco = contaEspaco(posicao);
    long posicaoAtual = ftell(arquivoSaida);
    
    if (fseek(arquivoSaida, espaco, SEEK_SET) != 0) {
        fseek(arquivoSaida, posicaoAtual, SEEK_SET);
        return;
    }

    if (fprintf(arquivoSaida, "%*d", 3, bit) < 0) {
        fseek(arquivoSaida, posicaoAtual, SEEK_SET);
        return;
    }

    fflush(arquivoSaida);
    fseek(arquivoSaida, posicaoAtual, SEEK_SET);
}

void compactar() {

    int pR = 0; 
    int pW = 0; 
    int bit;          

    while (pR < 65536) {
        bit = getBit(pR); 

        if (bit == 0) {
            setBit(pW, 0); 
            pW++; 
        }

        pR++; 
    }
    
    while (pW < 65536) {
        setBit(pW, 255); 
        pW++; 
    }
}

// Algoritmos
int firstFit(int tam) {
    
    int bit;
    int blocoLivre = -1; 
    int blocoLivreTam = 0;    
                        
    for (int i = 0; i < 65536; i++) {
        bit = getBit(i); 

        if (bit == 255) { 
            if (blocoLivre == -1) {
                blocoLivre = i;
            }
            blocoLivreTam++; 

        } else { 

            if (blocoLivre != -1 && blocoLivreTam >= tam) {
                
                for (int j = 0; j < tam; j++) {
                    setBit(blocoLivre + j, 0); 
                }

                return blocoLivre; 
            }
            
            blocoLivre = -1;
            blocoLivreTam = 0;
        }
    }

    if (blocoLivre != -1 && blocoLivreTam >= tam) {
        
        for (int j = 0; j < tam; j++) {
            setBit(blocoLivre + j, 0); 
        }
        
        return blocoLivre;
    }

    falhas++;
    return -1;
}

int nextFit(int tam) {
    
    int bit;
    int bitAtual;
    int blocoLivre = -1; 
    int blocoLivreTam = 0;    
   
    for (int i = 0; i < 65536; i++) {
        
        bitAtual = (nextFitIni + i) % 65536;
        bit = getBit(bitAtual); 

        if (bit == 255) { 
            if (blocoLivre == -1) {
                blocoLivre = bitAtual;
            }

            blocoLivreTam++; 

        } else { 

            if (blocoLivre != -1 && blocoLivreTam >= tam) {
                
                for (int j = 0; j < tam; j++) {
                    setBit(blocoLivre + j, 0); 
                }

                nextFitIni = (blocoLivre + tam) % 65536;
                return blocoLivre; 
            }
            
            blocoLivre = -1;
            blocoLivreTam = 0;
        }
    }

    if (blocoLivre != -1 && blocoLivreTam >= tam) {
        
        for (int j = 0; j < tam; j++) {
            setBit(blocoLivre + j, 0); 
        }
        
        nextFitIni = (blocoLivre + tam) % 65536;
        return blocoLivre;
    }

    falhas++;
    return -1; 
}

int bestFit(int tam) {
    
    int bit;
    int blocoLivre = -1;
    int blocoLivreTam = 0;
    int bestFitIni = -1;
    int bestFitTam = 65536 + 1; 
    
    for (int i = 0; i < 65536; i++) {
        bit = getBit(i); 

        if (bit == 255) { 
            if (blocoLivre == -1) {
                blocoLivre = i;
            }

            blocoLivreTam++; 

        } else { 

            if (blocoLivre != -1 && blocoLivreTam >= tam) {
                if (blocoLivreTam < bestFitTam) {
                    bestFitTam = blocoLivreTam;
                    bestFitIni = blocoLivre;
                }
            }
            
            blocoLivre = -1;
            blocoLivreTam = 0;
        }
    }

    if (blocoLivre != -1 && blocoLivreTam >= tam) {
        if (blocoLivreTam < bestFitTam) {
            bestFitTam = blocoLivreTam;
            bestFitIni = blocoLivre;
        }
    }

    if (bestFitIni == -1) {
        falhas++;
        return -1; 

    } else {
        
        for (int j = 0; j < tam; j++) {
            setBit(bestFitIni + j, 0); 
        }

        return bestFitIni; 
    }
}

int worstFit(int tam) {
    
    int bit;
    int blocoLivre = -1;
    int blocoLivreTam = 0;
    int worstFitIni = -1;
    int worstFitTam = 0; 
    
    for (int i = 0; i < 65536; i++) {
        bit = getBit(i); 

        if (bit == 255) { 
            if (blocoLivre == -1) {
                blocoLivre = i;
            }

            blocoLivreTam++; 

        } else { 

            if (blocoLivre != -1 && blocoLivreTam >= tam) {
                if (blocoLivreTam > worstFitTam) {
                    worstFitTam = blocoLivreTam;
                    worstFitIni = blocoLivre;
                }
            }
            
            blocoLivre = -1;
            blocoLivreTam = 0;
        }
    }

    if (blocoLivre != -1 && blocoLivreTam >= tam) {
        if (blocoLivreTam > worstFitTam) {
            worstFitTam = blocoLivreTam;
            worstFitIni = blocoLivre;
        }
    }

    if (worstFitIni == -1) {
        falhas++;
        return -1; 

    } else {
        
        for (int j = 0; j < tam; j++) {
            setBit(worstFitIni + j, 0); 
        }

        return worstFitIni; 
    }
}

int setAlgoritmo(int algoritmo, int tam) {
    
    switch (algoritmo) {
        case 1:
            return firstFit(tam);
        case 2:
            return nextFit(tam);
        case 3:
            return bestFit(tam);
        case 4:
            return worstFit(tam);
        default:
            fprintf(stderr, "ERRO ");
            return -1; 
    }
}

int main(int argc, char *argv[]) {

    int linhas;
    int memoria;
    char buffer[50];
    char comp[20];
    int algoritmo = atoi(argv[1]);
    char *entrada = argv[2];
    char *trace = argv[3];
    char *saida = argv[4];
    
    FILE *entrada_file = fopen(entrada, "r");
    arquivoSaida = fopen(saida, "w"); 

    geraSaida(entrada_file);
    fflush(arquivoSaida);
    fclose(arquivoSaida);

    arquivoSaida = NULL;
    arquivoSaida = fopen(saida, "r+");
    cabecalho = getCabecalho(); 
    arquivoTrace = fopen(trace, "r");

    while (fgets(buffer, sizeof(buffer), arquivoTrace) != NULL) {

        if (sscanf(buffer, "%d %d", &linhas, &memoria) == 2) {
            setAlgoritmo(algoritmo, memoria);
        }
        else if (sscanf(buffer, "%d %s", &linhas, comp) == 2 && strcmp(comp, "COMPACTAR") == 0) {
            compactar();
        }
    }

    printf("%d\n", falhas);
    fclose(arquivoSaida);
    fclose(arquivoTrace);

    return 0;
}