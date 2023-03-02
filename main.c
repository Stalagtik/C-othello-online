#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <SDL/SDL.h>

#include "fonctions.h"

int main (int argc, char *argv[]) {
    tty_mode(0);
    set_cr_noecho_mode();
    char *arglist[2] = {"clear", NULL};
    Menu Menu_input = 0;
    execute(arglist);
    initEcranChoix();
    inputMenu(&Menu_input);
    if (Menu_input == Quit){
    	tty_mode(1);
    	execute(arglist);
    	exit(0); 
    }
    if (Menu_input == Doc){ 
    	execute(arglist); 
    	tty_mode(1);
    	printf("Bienvenue sur Othello !\n\n Nous sommes ravis que vous ayez décidé de jouer à notre jeu. Nous voulons que vous passiez un bon moment en jouant, c'est pourquoi nous avons créé un PDF qui contient des informations détaillées sur le jeu et les commandes. Ce document peut être utile si vous avez besoin d'aide pour jouer, pour connecter vos appareils ou si vous souhaitez simplement en savoir plus sur le projet.\n\nVous trouverez ce document à la racine du projet, nous vous recommandons de le lire pour comprendre le programme et de le conserver à portée de main pour toute référence future.\n\n"); 
    }
    if(Menu_input == Reseau){ 
    	int choixreseau;
	execute(arglist); 
	tty_mode(1);
        printf("\n1 - J1 (Hôte de la partie)\n2 - J2 (Se connecter à la partie de l'hôte)\n");
    	scanf("%d", &choixreseau);
    	switch(choixreseau){
        	case 1 : 
        		system("gcc -o j1 j1.c");
    			system("./j1");
			break;
		case 2 : 
			system("gcc -o j2 j2.c");
    			system("./j2");
    			break; 
    		default : 
    			printf("A la prochaine ;)");
    			exit(0);
		}
     }
     if( Menu_input == Local){
        	tty_mode(1);
        	system("gcc -o jlocal jlocal.c");
    		system("./jlocal");
        }
    return 0;
}
