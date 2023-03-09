.PHONY: linux
.PHONY: windows
.PHONY: clean_linux
.PHONY: clean_windows

fireball_dependencies = \
	tmp/vulkan/general.o \
	tmp/vulkan/rendering.o \
	tmp/vulkan/image.o \
	tmp/vulkan/descriptor_sets.o \
	tmp/shader.vert.o \
	tmp/shader.frag.o \
	tmp/main.o
window_dependencies = src/common_window_vulkan.h src/window.h
vulkan_dependencies = src/common_window_vulkan.h src/vulkan.h src/vulkan/private.h
bin2c = bin2c/bin2c
ifeq ($(OS),Windows_NT)
	bin2c = bin2c/bin2c.exe
endif

linux:		$(fireball_dependencies) tmp/window/linux.o
	gcc -o fireball \
	  tmp/main.o \
	  tmp/window/linux.o \
	  tmp/vulkan/general.o \
	  tmp/vulkan/rendering.o \
	  tmp/vulkan/image.o \
	  tmp/vulkan/descriptor_sets.o \
	  tmp/shader.vert.o \
	  tmp/shader.frag.o \
	  -lm \
	  -lxcb \
	  -lvulkan
windows:	$(fireball_dependencies) tmp/window/windows.o
	gcc -o fireball.exe \
	  $(fireball_dependencies) \
	  tmp/window/windows.o \
	  -lvulkan-1

tmp/main.o:		src/common_window_vulkan.h src/window.h src/vulkan.h src/main.c
	gcc -c -o tmp/main.o src/main.c

tmp/window/linux.o:		$(window_dependencies) src/window/linux.c
	gcc -c -o tmp/window/linux.o src/window/linux.c
tmp/window/windows.o:	$(window_dependencies) src/window/windows.c
	gcc -c -o tmp/window/windows.o src/window/windows.c

tmp/vulkan/general.o:			$(vulkan_dependencies) src/vulkan/general.c
	gcc -c -o tmp/vulkan/general.o src/vulkan/general.c
tmp/vulkan/rendering.o:			$(vulkan_dependencies) src/vulkan/rendering.c
	gcc -c -o tmp/vulkan/rendering.o src/vulkan/rendering.c
tmp/vulkan/image.o:				$(vulkan_dependencies) src/vulkan/image.c
	gcc -c -o tmp/vulkan/image.o src/vulkan/image.c
tmp/vulkan/descriptor_sets.o:	$(vulkan_dependencies) src/vulkan/descriptor_sets.c
	gcc -c -o tmp/vulkan/descriptor_sets.o src/vulkan/descriptor_sets.c

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

clean_linux:
	rm -rf fireball bin2c/bin2c tmp
	mkdir tmp
	mkdir tmp/window
	mkdir tmp/vulkan
clean_windows:
	del /F fireball.exe bin2c\bin2c.exe
	rd /s /q tmp
	mkdir tmp
	mkdir tmp\window
	mkdir tmp\vulkan
