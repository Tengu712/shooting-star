.PHONY: build_linux
.PHONY: clean

build_linux:	tmp/window_linux.o tmp/vulkan.o
	ar r libfireball.a \
	  tmp/window_linux.o \
	  tmp/vulkan.o
tmp/window_linux.o:	src/window_linux.c
	gcc -c -o tmp/window_linux.o src/window_linux.c
tmp/vulkan.o:	src/vulkan.c
	gcc -c -o tmp/vulkan.o src/vulkan.c
clean:
	rm -rf libfireball.a tmp
	mkdir tmp
