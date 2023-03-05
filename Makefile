.PHONY: build_linux
.PHONY: build_windows
.PHONY: clean

ifeq ($(OS),Windows_NT)
	bin2c := bin2c/bin2c.exe
else
	bin2c := bin2c/bin2c
endif

linux:		src/*.h tmp/window_linux.o tmp/vulkan.o tmp/shader.vert.o tmp/shader.frag.o
	gcc -o fireball \
	  tmp/window_linux.o \
	  tmp/vulkan.o \
	  tmp/shader.vert.o \
	  tmp/shader.frag.o \
	  test.c \
	  -L. \
	  -lm \
	  -lxcb \
	  -lvulkan
windows:	tmp/window_windows.o tmp/vulkan.o tmp/shader.vert.o tmp/shader.frag.o
	gcc -o fireball.exe \
	  tmp/window_windows.o \
	  tmp/vulkan.o \
	  tmp/shader.vert.o \
	  tmp/shader.frag.o \
	  test.c \
	  -L. \
	  -lvulkan-1
tmp/window_linux.o:	src/common_window_vulkan.h src/window.h src/window_linux.c
	gcc -c -o tmp/window_linux.o src/window_linux.c
tmp/window_windows.o:	src/common_window_vulkan.h src/window.h src/window_windows.c
	gcc -c -o tmp/window_windows.o src/window_windows.c
tmp/vulkan.o:		src/vulkan.h src/vulkan.c
	gcc -c -o tmp/vulkan.o src/vulkan.c
tmp/shader.vert.o:	$(bin2c) src/shader.vert
	glslc src/shader.vert -o tmp/shader.vert.spv
	./bin2c/bin2c tmp/shader.vert.spv tmp/shader.vert.c shader_vert
	gcc -c -o tmp/shader.vert.o tmp/shader.vert.c
tmp/shader.frag.o:	$(bin2c) src/shader.frag
	glslc src/shader.frag -o tmp/shader.frag.spv
	./bin2c/bin2c tmp/shader.frag.spv tmp/shader.frag.c shader_frag
	gcc -c -o tmp/shader.frag.o tmp/shader.frag.c
$(bin2c):	bin2c/bin2c.rs
	rustc -o bin2c/bin2c bin2c/bin2c.rs
clean:
	rm -rf fireball bin2c/bin2c tmp
	mkdir tmp
