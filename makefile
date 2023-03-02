GCC = gcc
EXE = main

$(EXE): fonctions.o main.o
	$(GCC) -o $(EXE) fonctions.o main.o -lSDL -lSDL_ttf -lSDL_gfx -lSDL_mixer

fonctions.o: fonctions.c fonctions.h
	$(GCC) -c fonctions.c -o fonctions.o

	
main.o: main.c fonctions.h  
	$(GCC) -c main.c -o main.o -lSDL -lSDL_ttf -lSDL_gfx -lSDL_mixer
	
clean:
	rm *.o
	rm $(EXE).exe

execute: $(EXE)
	./$(EXE)
