.PHONY: build_linux
.PHONY: clean

build_linux:	tmp/window_linux.o tmp/vulkan.o tmp/shader.vert.o tmp/shader.frag.o
	gcc -o fireball \
	  tmp/window_linux.o \
	  tmp/vulkan.o \
	  tmp/shader.vert.o \
	  tmp/shader.frag.o \
	  test.c \
	  -L. \
	  -lxcb \
	  -lvulkan
tmp/window_linux.o:	src/window_linux.c
	gcc -c -o tmp/window_linux.o src/window_linux.c
tmp/vulkan.o:	src/vulkan.c
	gcc -c -o tmp/vulkan.o src/vulkan.c
tmp/shader.vert.o:	bin2c/bin2c src/shader.vert
	glslc src/shader.vert -o tmp/shader.vert.spv
	./bin2c/bin2c tmp/shader.vert.spv tmp/shader.vert.c shader_vert
	gcc -c -o tmp/shader.vert.o tmp/shader.vert.c
tmp/shader.frag.o:	bin2c/bin2c src/shader.frag
	glslc src/shader.frag -o tmp/shader.frag.spv
	./bin2c/bin2c tmp/shader.frag.spv tmp/shader.frag.c shader_frag
	gcc -c -o tmp/shader.frag.o tmp/shader.frag.c
bin2c/bin2c:	bin2c/bin2c.rs
	rustc -o bin2c/bin2c bin2c/bin2c.rs
clean:
	rm -rf fireball bin2c/bin2c tmp
	mkdir tmp
