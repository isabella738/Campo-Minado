#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#define VERDE  "\033[32m"
#define AMARELO "\033[33m"
#define VERMELHO "\033[31m"
#define AZUL "\033[34m"
#define CINZA "\033[90m"
#define BRANCO "\033[97m"
#define RESET  "\033[0m"

char mapa[10][11]={
    "----------",
    "----------",
    "----------",
    "----------",
    "----------",
    "----------",
    "----------",
    "----------",
    "----------",
    "----------",
};

struct celula{
    int x, y;
    int tipo;//bomba(1), numero(2)
    int estado;//ocultada(0), revelada(1), marcada(2)
    int numero;//para celulas tipo 2
};

int num(int i, int c, struct celula campo[]){// checa quantas bombas existem ao redor de i célula
    int contagem=0;
    int ax=campo[i].x;
    int ay=campo[i].y;

    for(int j=0; j<c; j++){
        if(campo[j].tipo==1){
            int bx=campo[j].x;
            int by=campo[j].y;

            if(bx==ax+1 && by==ay){contagem++;}//+,0
            if(bx==ax-1 && by==ay){contagem++;}//-,0
            if(bx==ax && by==ay+1){contagem++;}//0,+
            if(bx==ax && by==ay-1){contagem++;}//0,-
            if(bx==ax+1 && by==ay+1){contagem++;}//+,+
            if(bx==ax-1 && by==ay-1){contagem++;}//-,-
            if(bx==ax+1 && by==ay-1){contagem++;}//+,-
            if(bx==ax-1 && by==ay+1){contagem++;}//-,+
        }
    }
    return contagem;
};

struct coord{
    int x, y;
};

char *cores[]={
    BRANCO,
    CINZA,
    VERMELHO,
    VERDE,
    AMARELO
};

int main(){
    srand(time(NULL));
    struct celula campo[100];
    struct coord player;
    player.x=0;
    player.y=0;

    int b=0;//contador de bombas
    int c=0;//numero de celulas
    int perder=0;//marcação booleana

    for(int y=0; y<10; y++){//inicializa todas as células
        for(int x=0; x<10; x++){
            campo[c].x=x;
            campo[c].y=y;
            campo[c].estado=0;
            campo[c].tipo=2;
            c++;
        }
    }

    while(b<10){//seleciona 10 celulas aleatorias e as declara como bombas(tipo 1)
        int a= rand()%c;
        if(campo[a].tipo != 1){campo[a].tipo=1;}
        b++;
    }

    for(int i=0; i<=c; i++){//atribui numeros às celulas tipo 2
        if(campo[i].tipo == 2){
            campo[i].numero=num(i, c, campo);
        }
    }

//-------------------
    while(1){
        printf("\033[H");

        //marcações booleanas
        int continuar=0;
        int cavar=0;
        int marcar=0;

        if (_kbhit()){ //comandos do player
            char comando = _getch();
            if(comando=='a' && player.x>=1){player.x--;}
            if(comando=='w' && player.y>=1){player.y--;}
            if(comando=='s' && player.y<=8){player.y++;}
            if(comando=='d' && player.x<=8){player.x++;}
            if(comando=='k'){cavar=1;}
            if(comando=='l'){marcar=1;}          
        }

        for(int i=0; i<=c; i++){//alterar estado da célula a partir da intervenção do player
            if(cavar && campo[i].x==player.x && campo[i].y==player.y){campo[i].estado=1; break;}
            if(marcar && campo[i].x==player.x && campo[i].y==player.y){campo[i].estado=2; break;}

            if(campo[i].estado==1 && campo[i].numero==0 && campo[i].tipo==2){//"reação em cadeia" de células nulas
                if(campo[i].x < 9) {campo[i+1].estado=1;}//direita
                if(campo[i].x > 0) {campo[i-1].estado=1;}//esquerda
                if(campo[i].y < 9) {campo[i+10].estado=1;}//baixo
                if(campo[i].y > 0) {campo[i-10].estado=1;}//cima

                if(campo[i].x > 0 && campo[i].y < 9) {campo[i+9].estado=1;}//esquerda, baixo
                if(campo[i].x < 9 && campo[i].y < 9) {campo[i+11].estado=1;}//direita, baixo
                if(campo[i].x > 0 && campo[i].y > 0) {campo[i-11].estado=1;}//esquerda, cima
                if(campo[i].x < 9 && campo[i].y > 0) {campo[i-9].estado=1;}//direita, cima
            }
        }

        for(int y=0; y<10; y++){//impressão
            for(int x=0; x<10; x++){
                int imp=0;
                int a=1;//cinza
                if(x==player.x && y==player.y){a=0;}//branco

                for(int i=0; i<=c; i++){//marcações e revelação de células
                    if(campo[i].x==x && campo[i].y==y){
                        switch(campo[i].estado){
                            case 0: //oculta
                                break;
                            case 1: //revelada
                                a=3;//azul
                                if(x==player.x && y==player.y){a=0;}
                                if(campo[i].tipo==1){printf("%s*%s", cores[a],RESET); imp=1;}
                                if(campo[i].tipo==2 && campo[i].numero !=0){printf("%s%d%s", cores[a], campo[i].numero, RESET); imp=1;}
                                if(campo[i].tipo==2 && campo[i].numero == 0){printf("%s/%s", cores[a], RESET); imp=1;}        
                                break;
                            case 2: //marcada
                                a=2;//amarelo
                                if(x==player.x && y==player.y){a=0;}
                                printf("%s!%s", cores[a], RESET); imp=1;
                                break;
                        }
                    }   
                }

                if(!imp){printf("%s%c%s", cores[a], mapa[y][x], RESET);}

            }
            printf("\n");
        }
    
        for(int i=0; i<=c; i++){//se a célula é uma bomba, e seu estado é "revelado": perder o jogo.
            if(campo[i].tipo==1 && campo[i].estado==1){perder=1;}
        }
        if(perder){break;}

        for(int i=0; i<=c; i++){//se ainda há células ocultas, continuar. se não, vitória
            if(campo[i].estado==0){continuar=1; break;}
        }
        if(continuar==0){break;}
    }
    

    if(perder){printf("\nVoce perdeu :(");}
    else{printf("\nParabens! Voce ganhou o jogo ;)");}

    return 0;
}