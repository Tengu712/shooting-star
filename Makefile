.PHONY: linux
.PHONY: windows
.PHONY: clean_linux
.PHONY: clean_windows

sstar_dependencies = \
    tmp/sstar/sstar.o \
    tmp/sstar/logger.o \
	tmp/vulkan/general.o \
	tmp/vulkan/rendering.o \
	tmp/vulkan/image.o \
	tmp/vulkan/descriptor_sets.o \
	tmp/shader.vert.o \
	tmp/shader.frag.o
window_dependencies = src/sstar.h src/common_window_vulkan.h src/window.h
vulkan_dependencies = src/sstar.h src/common_window_vulkan.h src/vulkan.h src/vulkan/private.h

linux:		$(sstar_dependencies) tmp/window/linux.o exh2imh/exh2imh
	gcc -Wall -shared -fvisibility=hidden -o build/sstar.so \
	  $(sstar_dependencies) \
	  tmp/window/linux.o \
	  -lm \
	  -lxcb \
	  -lvulkan
	./exh2imh/exh2imh src/sstar.h build/sstar.h
	cp build/sstar.h sample/sstar.h
	cp build/sstar.so sample/sstar.so
windows:	$(sstar_dependencies) tmp/window/windows.o exh2imh/exh2imh
	gcc -Wall -shared -o build/sstar.dll \
	  $(sstar_dependencies) \
	  tmp/window/windows.o \
	  -lvulkan-1
	./exh2imh/exh2imh src/sstar.h build/sstar.h
	copy build\sstar.h sample\sstar.h > nul
	copy build\sstar.dll sample\sstar.dll > nul

tmp/sstar/sstar.o:	src/sstar.h src/sstar/sstar.c
	gcc -Wall -c -o tmp/sstar/sstar.o src/sstar/sstar.c
tmp/sstar/logger.o:		src/sstar.h src/sstar/logger.c
	gcc -Wall -c -o tmp/sstar/logger.o src/sstar/logger.c

tmp/window/linux.o:		$(window_dependencies) src/window/linux.c
	gcc -Wall -c -o tmp/window/linux.o src/window/linux.c
tmp/window/windows.o:	$(window_dependencies) src/window/windows.c
	gcc -Wall -c -o tmp/window/windows.o src/window/windows.c

tmp/vulkan/general.o:			$(vulkan_dependencies) src/vulkan/general.c
	gcc -Wall -c -o tmp/vulkan/general.o src/vulkan/general.c
tmp/vulkan/rendering.o:			$(vulkan_dependencies) src/vulkan/rendering.c
	gcc -Wall -c -o tmp/vulkan/rendering.o src/vulkan/rendering.c
tmp/vulkan/image.o:				$(vulkan_dependencies) src/vulkan/image.c
	gcc -Wall -c -o tmp/vulkan/image.o src/vulkan/image.c
tmp/vulkan/descriptor_sets.o:	$(vulkan_dependencies) src/vulkan/descriptor_sets.c
	gcc -Wall -c -o tmp/vulkan/descriptor_sets.o src/vulkan/descriptor_sets.c

tmp/shader.vert.o:	bin2c/bin2c src/shader.vert
	glslc src/shader.vert -o tmp/shader.vert.spv
	./bin2c/bin2c tmp/shader.vert.spv tmp/shader.vert.c shader_vert
	gcc -Wall -c -o tmp/shader.vert.o tmp/shader.vert.c
tmp/shader.frag.o:	bin2c/bin2c src/shader.frag
	glslc src/shader.frag -o tmp/shader.frag.spv
	./bin2c/bin2c tmp/shader.frag.spv tmp/shader.frag.c shader_frag
	gcc -Wall -c -o tmp/shader.frag.o tmp/shader.frag.c

bin2c/bin2c:	bin2c/bin2c.rs
	rustc -o bin2c/bin2c bin2c/bin2c.rs
exh2imh/exh2imh:	exh2imh/exh2imh.rs
	rustc -o exh2imh/exh2imh exh2imh/exh2imh.rs

clean_linux:
	rm -rf sstar bin2c/bin2c build tmp sample/sstar.h sample/sstar.so
	mkdir build
	mkdir tmp
	mkdir tmp/sstar
	mkdir tmp/window
	mkdir tmp/vulkan
clean_windows:
	del /F bin2c\bin2c bin2c\bin2c.pdb sample\sstar.h sample\sstar.dll > nul 2>&1
	if exist build ( rd /s /q build )
	if exist tmp ( rd /s /q tmp )
	mkdir build
	mkdir tmp
	mkdir tmp\sstar
	mkdir tmp\window
	mkdir tmp\vulkan
