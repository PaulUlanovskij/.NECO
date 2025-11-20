main: main.c ./core/ ./web/ ./ffs/
	gcc main.c -D_GNU_SOURCE ./core/*.c ./web/[!templater]*.c ./ffs/*.c -ggdb -std=c23 -o main
templater: ./web/templater.c ./web/ ./core/
	gcc -D_GNU_SOURCE ./core/*.c ./web/*.c -ggdb -std=c23 -o templater 
