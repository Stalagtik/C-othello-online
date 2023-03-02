#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

//socket
#include<signal.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define MAXLINE 512

//FONCTIONS RESEAU
void z_handler();// fonction pour le traitement du signal
int server();

//VARIABLES NECESSAIRES AU BON FONCTIONNEMENT DES SOCKETS
int server_sock;
int client_sock;
int clntlen;
int num;
char sendline[MAXLINE];
char recvline[MAXLINE];
int size;
pid_t fork_ret;
struct sockaddr_in client_addr;
struct sockaddr_in server_addr;
int state;
struct sigaction act;


#define HIGHT 8
#define WIDTH 8
#define dx 2
#define dy 1

typedef enum Cell {Blank, Black, White} Cell;
typedef enum MenuR {Retour, Jouer, Aide} MenuR;

///FONCTIONS GESTION ECRAN/MENU
void initEcranReseau();
void inputMenuR(MenuR *);
void gotoxy(int, int);
void set_cr_noecho_mode(void);
void tty_mode(int);
void execute(char **);

///FONCTIONS JEU GLOBAL
void getCells(bool, Cell[HIGHT][WIDTH], bool[HIGHT][WIDTH]);
bool existCells(bool[WIDTH][HIGHT]);
bool canPutLine(bool, int[2], int, Cell[HIGHT][WIDTH]);
void add(int[2], int[2]);
bool canPutLineIter(bool, int[2], int, Cell[HIGHT][WIDTH], bool);
bool GrillePleine(Cell[HIGHT][WIDTH]);
void EndGame(Cell[HIGHT][WIDTH], char player[2][30]);
bool CouleurUnique(Cell[HIGHT][WIDTH]);
void inputCell(int selectedCell[2], int *flag);
bool canPut(int[2], bool[WIDTH][HIGHT]);
void reverse(bool, int[2], Cell[HIGHT][WIDTH]);
void reverseIter(bool, int[2], int, Cell[WIDTH][HIGHT]);
void afficherGrille(Cell[HIGHT][WIDTH], bool enableCells[WIDTH][HIGHT]);
Cell getCellColor(bool);
void Score(Cell board[HIGHT][WIDTH], char player[2][30]);

//FONCTIONS JEU EN RESEAU
void NomJoueur(char player[2][30]);
char getData(void);
int getDataCount=0;

int main(int argc, char *argv[]) {
    tty_mode(0);
    set_cr_noecho_mode();
    
    while(true)
    {
        Cell board[HIGHT][WIDTH];
        bool enableCells[WIDTH][HIGHT] = {0};
        char *arglist[2] = {"clear", NULL};
        bool isFirst = true;
        bool isSecond = true;
        int flag = 0; // si touche "z" alors 1
        char player[2][30];
        int enable_count = 0;
        MenuR Menu_input = 0;
        
        for ( int j=0; j < HIGHT; j++) {
            for( int i=0; i < WIDTH ; i++) {
                board[i][j] = Blank;
            }
        }
        board[3][3] = White;
        board[4][4] = White;
        board[3][4] = Black;
        board[4][3] = Black;
        
        if( isFirst ){
            isSecond = false;
        }
        
        execute(arglist);
        initEcranReseau();
        inputMenuR(&Menu_input);
        
        execute(arglist);
        
        if (Menu_input == Aide){ 
            execute(arglist); 
            printf("Bienvenue sur Othello !\n\n Nous sommes ravis que vous ayez décidé de jouer à notre jeu. Nous voulons que vous passiez un bon moment en jouant, c'est pourquoi nous avons créé un PDF qui contient des informations détaillées sur le jeu et les commandes. Ce document peut être utile si vous avez besoin d'aide pour jouer, pour connecter vos appareils ou si vous souhaitez simplement en savoir plus sur le projet.\n\nVous trouverez ce document à la racine du projet, nous vous recommandons de le lire pour comprendre le programme et de le conserver à portée de main pour toute référence future.\n\n");
            break;
        }
        
        if(Menu_input == Jouer){ // Lancer la partie, notez que les deux environnements doivent etre connecte au meme réseau internet
            //connexion au serveur
            server();
            execute(arglist);
            NomJoueur(player);
            execute(arglist);
        }
        
        if( Menu_input == Retour){
            break;
        }
        
        while(true) {
            enable_count = 0;
            
            for (int x=0; x<WIDTH; x++) {
                for (int y=0; y<HIGHT; y++) {
                    enableCells[x][y] = false;
                }
            }
            getCells(isFirst, board, enableCells);// trouve les cases disponibles
            afficherGrille(board, enableCells); 
            Score(board, player);
            
            int selectedCell[2] = {-1, -1};
            int checkPossible=0;
            if(!existCells(enableCells)) {
                
                // Condition de fin de jeu
                if(GrillePleine(board)){ // Condition de fin n'1 : plateau plein
                    EndGame(board,player);
                    printf("\n\n\nAppuyez sur n'importe quelle touche afin de quitter.\n");
                    getchar();//
                    break;
                }
                if (CouleurUnique(board)) { // Condition de fin n'2 : plateau que d'une seule couleur
                    EndGame(board,player);
                    printf("\n\n\nAppuyez sur n'importe quelle touche afin de quitter.\n");
                    getchar();//
                    break;
                }
                
            }  else{
                if ( Menu_input == Jouer){ 
                    if( isFirst ){
                        printf("\n\nC'est au tour des Noirs.\nLe symbole ☆ correspond à vos coups valides.\n");
                    }
                    else{
                        printf("\n\nC'est au tour des Blancs.\nLe symbole ☆ correspond à vos coups valides.\n");
                    }
                    printf("\n\n\n\nAfin de quitter cette partie, cliquez sur 'z'.\n");
                    inputCell(selectedCell,&flag); // premier coup
                    if ( flag )
                        break;
                    
                    if (!(checkPossible=canPut(selectedCell, enableCells))) { 
                        printf("Coup non valide. Réessayez.\n");
                        selectedCell[0] = -1;
                        selectedCell[1] = -1;
                        isFirst = !isFirst;
                        isSecond = !isSecond;
                        checkPossible=1;
                    }
                }
            }
            
            // Gestion de tour
            if(checkPossible!=0&&Menu_input==Jouer)
            {
                getDataCount++;
            }
            execute(arglist); 
            reverse(isFirst, selectedCell, board); // retourner la case (si blanche devient noir et inversement)
            isFirst = !isFirst; // changement de joueur
            isSecond = !isSecond; // changement de joueur
            checkPossible=0;
        }
        
    }
    
    tty_mode(1);
    return 0;
}

void initEcranReseau(){
    printf("\n ********************************\n");
    printf("         Mode réseau | Othello\n");
    printf(" ********************************\n");
    printf("\n     ☞  Retour\n");
    printf("\n     ☞  Jouer en tant qu'hôte\n");
    printf("\n     ☞  Aide\n");
    printf("\n\n Codé par BERQUER Théo | BOUZOUBAA Yassine | FERREIRA Jiullian");
}

void inputMenuR(MenuR *M){ // Menu et curseur
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

void NomJoueur(char player[2][30]){
    tty_mode(1);
    
    gotoxy(10,3);
    
    printf("Entrez votre nom :\n");
    
    gotoxy(10,5);
    printf("J1 : ");
    gotoxy(20,5);
    fgets(player[1],30,stdin);
    //envoi du nom de j1
    write(client_sock,player[1],30);
    player[1][strlen(player[1])-1] = '\0';
    
    gotoxy(10,7);
    printf("J2 : ");
    gotoxy(20,7);
    //reception du nom de j2
    read(client_sock,player[0],30);
    printf("%s",player[0]);
    printf("%s",player[1]);
    player[0][strlen(player[0])-1] = '\0';
    
    set_cr_noecho_mode();
}

void getCells(bool isFirst, Cell board[HIGHT][WIDTH], bool enableCells[WIDTH][HIGHT]) {
    for (int x=0; x<WIDTH; x++) {
        for (int y=0; y<HIGHT; y++) {
            int selectedCell[2] = {x, y};
            for(int i=0; i<8; i++) {
                if (board[x][y] == Blank
                    && canPutLine(isFirst, selectedCell, i, board)) {
                    enableCells[x][y] = true;
                }
            }
        }
    }
}

bool canPutLine(bool isFirst, int selectedCell[2], int directionIndex, Cell board[HIGHT][WIDTH]) {
    return canPutLineIter(isFirst, selectedCell, directionIndex, board, false);
}


bool canPutLineIter(bool isFirst, int selectedCell[2], int directionIndex, Cell board[HIGHT][WIDTH],
                    bool flag) {
    int directions[8][2] = {
        {-1,-1}, {0,-1}, {1,-1},
        {-1, 0}, {1, 0},
        {-1,1}, {0,1}, {1,1}
    };
    
    int tmp[2] = {selectedCell[0], selectedCell[1]};
    add(tmp, directions[directionIndex]);
    
    Cell myColor = getCellColor(isFirst);
    if (tmp[0] <= -1 || tmp[1] <= -1 || tmp[0] > WIDTH || tmp[1] > HIGHT) {
        return false;
    } else if (board[tmp[0]][tmp[1]] == Blank) {
        return false;
    }
    
    if (board[tmp[0]][tmp[1]] != myColor && !flag) {
        return canPutLineIter(isFirst, tmp, directionIndex, board, true);
    } else if (board[tmp[0]][tmp[1]] == myColor && !flag) {
        return false;
    }  else if (board[tmp[0]][tmp[1]] == myColor && flag) {
        return true;
    }
    return canPutLineIter(isFirst, tmp, directionIndex, board, true);
}

void add(int target[2], int a[2]) {
    target[0] = target[0] + a[0];
    target[1] = target[1] + a[1];
}

bool existCells(bool enableCells[WIDTH][HIGHT]) {
    for (int x=0; x<WIDTH; x++) {
        for (int y=0; y<HIGHT; y++) {
            if (enableCells[x][y]) {
                return true;
            }
        }
    }
    return false;
}

bool GrillePleine(Cell board[HIGHT][WIDTH]) {
    for (int y=0; y < HIGHT; y++) {
        for(int x=0; x < WIDTH ; x++) {
            if (board[x][y] == Blank)
                return false;
        }
    }
    return true;
}

void Score(Cell board[HIGHT][WIDTH], char player[2][30]) {
    int countblack = 0, countwhite = 0;
    for (int y=0; y < HIGHT; y++) {
        for(int x=0; x < WIDTH ; x++) {
            switch (board[x][y]) {
                case Black:
                    countblack++;
                    break;
                case White:
                    countwhite++;
                    break;
                default:
                    break;
            }
        }
    }
    
    gotoxy(25 , 5 );
    printf("Noir ○- %s %d",player[1],countblack);
    gotoxy(25 , 6 );
    printf("Blanc ● - %s %d",player[0], countwhite);
    gotoxy(0, 12);
    
}

void EndGame(Cell board[HIGHT][WIDTH], char player[2][30]) {
    int countblack = 0, countwhite = 0;
    for (int y=0; y < HIGHT; y++) {
        for(int x=0; x < WIDTH ; x++) {
            switch (board[x][y]) {
                case Black:
                    countblack++;
                    break;
                case White:
                    countwhite++;
                    break;
                default:
                    break;
            }
        }
    }
    
    printf("SCORE BLANC:%d\n",countwhite);
    printf("SCORE NOIR:%d\n",countblack);
    
    printf("\n\n\n\n\nGAME OVER ! \n");
    if(countwhite > countblack) {
        printf("Vainqueur : Blanc - %s\n",player[0]);
    } else if (countwhite < countblack) {
        printf("Vainqueur : Noir - %s\n",player[1]);
    } else {
        printf("Egalité\n");
    }
}

bool CouleurUnique(Cell board[HIGHT][WIDTH]) {
    enum Cell color = Blank;
    for (int y=0; y < HIGHT; y++) {
        for(int x=0; x < WIDTH ; x++) {
            if(board[x][y]!=Blank) {
                if(color == Blank) {
                    color = board[x][y];
                } else if(board[x][y]!=color) {
                    return false;
                }
            }
            if(y == HIGHT-1 && x == WIDTH-1)
                return true;
        }
    }
    return false;
}

void inputCell(int selectedCell[2], int *flag){
    int x,y;
    int ch1, ch2, ch3;
    int xx = 0, yy = 0;
    x = 10;
    y = 6;
    
    gotoxy(x,y);
    while(true)
    {
        //printf("-1");
        ch1 = getData();
        if( ch1 == ' ')
            break;
        if ( ch1 == 'z' || ch1 == 'Z' )
            break;
        if ( ch1 == 27 )
        {
            ch2 = getData();
            if ( ch2 == 91 )
            {
                ch3 = getData();
                switch( ch3 ){
                    case 68 :
                        x -= dx;
                        xx--;
                        gotoxy(x, y);
                        break;
                    case 67 :
                        xx++;
                        x += dx;
                        gotoxy(x, y);
                        break;
                    case 65 :
                        y -= dy;
                        yy--;
                        gotoxy(x, y);
                        break;
                    case 66 :
                        y += dy;
                        yy++;
                        gotoxy(x, y);
                        break;
                    default:
                        break;
                }
                
            }
        }
    }
    
    if ( ch1 == 'z' || ch1 == 'Z'){
        *flag = 1;
    }else{
        selectedCell[0] = 4 + xx -1;
        selectedCell[1] = 4 + yy -1;
    }

    return;
}
bool canPut(int selectedCell[2], bool enableCells[WIDTH][HIGHT]) {
    return enableCells[selectedCell[0]][selectedCell[1]];
}

void reverse(bool isFirst, int selectedCell[2], Cell board[HIGHT][WIDTH]) {
    int x = selectedCell[0], y = selectedCell[1];
    Cell myColor = getCellColor(isFirst);
    
    board[x][y] = myColor;
    for (int i=0; i < 8; i++) {
        if (canPutLine(isFirst, selectedCell, i, board)) {
            reverseIter(isFirst, selectedCell, i, board);
        }
    }
}

void reverseIter(bool isFirst, int scanningCell[2], int directionIndex, Cell board[WIDTH][HIGHT]) {
    int directions[8][2] = {
        {-1,-1}, {0,-1}, {1,-1},
        {-1, 0}, {1, 0},
        {-1,1}, {0,1}, {1,1}
    };
    
    int tmp[2] = {scanningCell[0], scanningCell[1]};
    add(tmp, directions[directionIndex]);
    
    int x = tmp[0];
    int y = tmp[1];
    
    Cell myColor = getCellColor(isFirst);
    
    if (board[x][y] == myColor || board[x][y] == Blank) {
        return;
    }
    
    board[x][y] = myColor;
    reverseIter(isFirst, tmp, directionIndex, board);
}

void afficherGrille(Cell board[HIGHT][WIDTH], bool enableCells[WIDTH][HIGHT]) {
    printf("\n");
    printf("   ");
    for(int i = 1; i <= WIDTH; i++){
        printf("%d ", i);
    }
    printf("\n");
    
    for (int y=0; y < HIGHT; y++) {
        printf(" %d ", y+1);
        for(int x=0; x < WIDTH ; x++) {
            switch (board[x][y]) {
                case Black:
                    printf("○ ");
                    break;
                case White:
                    printf("● ");
                    break;
                default:
                    if ( enableCells[x][y] ){
                        printf("☆ ");
                    }
                    else{
                        printf("- ");
                    }
                    
                    break;
            }
        }
        printf("\n");
    }
}

Cell getCellColor(bool isFirst) {
    if(isFirst) {
        return Black;
    }
    return White;
}

void gotoxy(int x, int y){
    printf("\033[%d;%df",y,x);
    fflush(stdout);
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
    const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}


//socket

int server(){
    act.sa_handler = z_handler;
    
    if((server_sock = socket(PF_INET, SOCK_STREAM, 0)) <0)
    {
        printf("Hôte : impossible d'ouvrir les sockets.\n");
        exit(0);
    }
    // Adresse IP dans la variable d'adresse de socket
    bzero((char *)&server_addr, sizeof(server_addr)); // Initialisation de la variable d'adresse de socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.168.32.158"); // !!!! A noter, mettre l'adresse IP de l'hôte ici !!!!
    server_addr.sin_port = htons(30000);
    
    sigaction(SIGCHLD, &act, 0);
    
    // Association de l'adresse IP au socket et erreurs
    if(bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Erreur dans l'adressage IP. Consultez l'aide.\n");
        exit(0);
    }
    printf("Connexion établie. \nEn attente de J2... \n");
    listen(server_sock, 1);
    
    // Accepter la connexion de J2
    clntlen = sizeof(client_addr);
    if((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &clntlen)) <0)
    {
        printf("Erreur : J2 n'a pas été accepté. \n");
    }
    return 0;
}

void z_handler()
{
    int state;
    waitpid(-1, &state, WNOHANG);
    exit(0);
    
    return ;
}

char getData(void){
    // printf("getDataCount %d",getDataCount);
    if(getDataCount%2==0){
        char data[2];
        data[0]=getchar();
        //pour envoyer les infos à j2
        write(client_sock,data,2);
        return data[0];
    }
    else{
        //pour recevoir les infos de j2
        char data[2];
        read(client_sock,data,2);
        return data[0];
    }
}
