#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "fonctions.h"



/*Ce fichier fournit toutes les fonctions nécéssaire au bon fonctionnement du projet : OTHELLO*/

/*------------------------------------------------------------------------------------------------------------*/

void gotoxy(int x, int y){
    printf("\033[%d;%df",y,x);
    fflush(stdout);
}


void initEcranReseau(){
    printf("\n ********************************\n");
    printf("         Mode réseau | Othello\n");
    printf(" ********************************\n");
    printf("\n     ☞  Retour\n");
    printf("\n     ☞  Jouer en tant qu'hôte\n");
    printf("\n     ☞  Aide\n");
    printf("\n     ☞  Quitter\n");
    printf("\n\n Codé par BERQUER Théo | BOUZOUBAA Yassine | FERREIRA Jiullian");
}

void initEcranChoix(){
    printf("\n ********************************\n");
    printf("         Othello\n");
    printf(" ********************************\n");
    printf("\n     ☞  Jouer en Local\n");
    printf("\n     ☞  Jouer en réseau\n");
    printf("\n     ☞  Documentation\n");
    printf("\n     ☞  Quitter\n");
    printf("\n\n Codé par BERQUER Théo | BOUZOUBAA Yassine | FERREIRA Jiullian");
}

void inputMenu(Menu *M){ // Menu et curseur
    int x,y;
    int ch1, ch2, ch3;
    *M = 0;
    x = 6;
    y = 6;
    
    gotoxy(x,y);
    
    while(true){
        ch1 = getchar();
        if( ch1 == ' ')
            break;
        if ( ch1 == 27 )
        {
            ch2 = getchar();
            if ( ch2 == 91 )
            {
                ch3 = getchar();
                switch( ch3 ){
                    case 65 :
                        if ( *M >= 1 ){
                            y -= 2;
                            (*M)--;
                            gotoxy(x, y);
                            break;
                        }
                        break;
                    case 66 :
                        if ( *M <= 2 ){
                            y += 2;
                            (*M)++;
                            gotoxy(x, y);
                            break;
                        }
                        break;
                    default:
                        break;
                }
                
            }
        }
    }
    
}



void set_cr_noecho_mode(){
    struct termios ttystate;
    
    tcgetattr(0, &ttystate);
    ttystate.c_lflag &= ~ICANON;
    ttystate.c_lflag &= ~ECHO;
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &ttystate);
}

void tty_mode(int how){
    static struct termios original_mode;
    if ( how == 0 )
        tcgetattr(0, &original_mode);
    else
        tcsetattr(0, TCSANOW, &original_mode);
}

void execute( char *arglist[] )
{
    const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J ";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}


void execute_m( char *arglist[] )
{
    int pid,exitstatus;
    
    pid = fork();
    switch( pid ){
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            execvp(arglist[0], arglist);
            perror("execvp failed");
            exit(1);
        default:
            while( wait(&exitstatus) != pid )
                ;
    }
}

