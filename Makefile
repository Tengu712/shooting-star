.PHONY: build_linux
.PHONY: clean

build_linux:	window_linux.o
	ar r libfireball.a \
	 tmp/window_linux.o
window_linux.o:	src/window_linux.c
	gcc -c -o tmp/window_linux.o src/window_linux.c
clean:
	rm -rf libfireball.a tmp
	mkdir tmp
