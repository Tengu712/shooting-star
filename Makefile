main:	tmp/window.o src/main.c
	gcc -o main \
	  src/main.c \
	  tmp/window.o \
	  -lxcb
tmp/window.o:	src/window.c
	gcc -c -o tmp/window.o src/window.c
clean:
	rm -rf main tmp
