#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
#elif defined(__linux__)
    #define LINUX
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#if defined(LINUX)
    #include <fcntl.h>
    #include <unistd.h>
    #include <termios.h>

    void mudar_terminal(){
        struct termios velho_terminal, novo_terminal;

        tcgetattr(STDIN_FILENO, &velho_terminal);
        novo_terminal = velho_terminal;

        novo_terminal.c_lflag &= ~(ICANON | ECHO);

        tcsetattr(STDIN_FILENO, TCSANOW, &novo_terminal);
    }

#endif

#define VERDE  "\033[32m"
#define AMARELO "\033[33m"
#define VERMELHO "\033[31m"
#define AZUL "\033[34m"
#define CINZA "\033[90m"
#define BRANCO "\033[97m"
#define RESET  "\033[0m"
#define ALT 16
#define LARG 30
#define N_BOMBAS 99

char cores[][5]={
    BRANCO,
    CINZA,
    VERMELHO,
    VERDE,
    AMARELO
};

typedef struct{
    int y, x;
}Coordenada;

typedef struct{
    int tipo;//bomba(1), numero(0)
    int estado;//ocultada(0), marcada(1), revelada
    int numero;//para celulas tipo 0
}Mapa;
Mapa mapa[ALT][LARG] = {0};
Coordenada player = {0};

void checar_arredores(Coordenada p, int *m, int *b){
    for(int i=-1; i<2; i++){
        for(int j=-1; j<2; j++){
            if(i == j && i == 0) continue;
            int dy = p.y + i, dx = p.x + j;
            if(
                dy < 0 || dy >= ALT ||
                dx < 0 || dx >= LARG
            ) continue;

            if(mapa[dy][dx].tipo) (*b)++;
            if(mapa[dy][dx].estado == 1) (*m)++;
        }
    }
}

void revelar_arredores(Coordenada p){
    for(int i=-1; i<2; i++){
        for(int j=-1; j<2; j++){
            if(i == j && i == 0) continue;
            int dy = p.y + i, dx = p.x + j;
            if(
                dy < 0 || dy >= ALT ||
                dx < 0 || dx >= LARG
            ) continue;

            if(mapa[dy][dx].estado != 0) continue;

            mapa[dy][dx].estado = 2;
            
            if(mapa[dy][dx].numero == 0 && mapa[dy][dx].estado == 2 && mapa[dy][dx].tipo == 0)
                revelar_arredores((Coordenada){dy, dx});
            /*
            for(int y=0; y<ALT; y++){
                for(int x=0; x<LARG; x++){
                    printf("%d ", mapa[y][x].estado);
                }
                printf("\n");
            }
            
            getchar();
            */
        }
    }
}

void inicializacao(){//define posição das bombas e os números
    int contador=0;
    do{
        int y = rand()%ALT;
        int x = rand()%LARG;

        if(mapa[y][x].tipo)continue;
        if(player.y == y && player.x == x) continue;
        //garante que o jogador não abra uma bomba na primeira jogada

        mapa[y][x].tipo=1;
        contador++;

    }while(contador != N_BOMBAS);

    for(int y=0; y<ALT; y++){
        for(int x=0; x<LARG; x++){
            if(mapa[y][x].tipo == 0){
                int temp;
                checar_arredores((Coordenada){y, x}, &temp, &mapa[y][x].numero);
            }
            //mapa[y][x].estado=2;
        }
    }
}

void cavar(){//retornou 1=perdeu
    Mapa *mp = &mapa[player.y][player.x];
    
    mp->estado = 2;

    //revelou uma bomba
    if(mp->tipo) return;

    if(mp->numero == 0){ revelar_arredores(player); return;}

    int m=0, n=0; checar_arredores(player, &m, &n);
    if(m == mp->numero && mp->numero){
        revelar_arredores(player);
    }
}

void comando(){
    
    #ifdef WINDOWS
        char c = _getch();
    #elif defined (LINUX)
        char c = getchar();
    #endif
    
    switch(c){
        case 'w': 
            if(player.y>0)player.y--; break;
        case 'a': 
            if(player.x>0)player.x--; break;
        case 's': 
            if(player.y<ALT-1)player.y++; break;
        case 'd': 
            if(player.x<LARG-1)player.x++; break;
        case 'j':
            if(mapa[player.y][player.x].estado < 2)
                mapa[player.y][player.x].estado = !mapa[player.y][player.x].estado; 
            break;
        case 'k':
            cavar();
    }
}

int jogo(){
    int vitoria=1;
    for(int y=0; y<ALT; y++){
        for(int x=0; x<LARG; x++){
            if(mapa[y][x].tipo && mapa[y][x].estado == 2) return 2;
            if(mapa[y][x].estado == 0) vitoria = 0;
        }
    }
    return vitoria;
}

void imprimir(){
    for(int y=0; y<ALT; y++){
        for(int x=0; x<LARG; x++){
            if(y == player.y && x == player.x){
                printf(BRANCO);
            }
            else switch(mapa[y][x].estado){
                case 0: printf(CINZA); break;
                case 1: printf(VERMELHO); break;
                case 2: 
                    if(mapa[y][x].tipo) printf(AMARELO);
                    else if(mapa[y][x].numero) printf(VERDE);
                    else printf(CINZA);
                    break;
            }

            switch(mapa[y][x].estado){
                case 0: printf("-"); break;
                case 1: printf("!"); break;
                case 2: 
                    if(mapa[y][x].tipo) printf("*");
                    else if(mapa[y][x].numero) printf("%d", mapa[y][x].numero);
                    else printf("/");
                    break;
            }

            printf(" "RESET);
        }
        printf("\n");
    }
    printf("\n");
}

int main(){
    srand(time(NULL));
    #if defined(LINUX)
        mudar_terminal();
        system("clear");
    #else
        system("cls");
    #endif

    inicializacao();

    int acabou=0;
    do{
        imprimir();
        comando();
        acabou = jogo();
        printf("\033[H");
    }while(!acabou);

    imprimir();
    if(acabou == 2){
        printf("Voce perdeu :(\n");
    } else printf("Voce ganhou! :)\n");
    getchar();
}