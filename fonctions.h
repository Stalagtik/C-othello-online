typedef enum Menu {Local, Reseau, Doc, Quit} Menu;

extern void set_cr_noecho_mode(void);
extern void tty_mode(int);
extern void execute(char **);
extern void execute_m(char **);
extern void initEcranChoix();
extern void inputMenu(Menu *);
extern void gotoxy(int, int);

