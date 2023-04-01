.PHONY: linux
.PHONY: windows
.PHONY: clean_linux
.PHONY: clean_windows

fireball_dependencies = \
    tmp/fireball/fireball.o \
    tmp/fireball/logger.o \
	tmp/vulkan/general.o \
	tmp/vulkan/rendering.o \
	tmp/vulkan/image.o \
	tmp/vulkan/descriptor_sets.o \
	tmp/shader.vert.o \
	tmp/shader.frag.o
window_dependencies = src/fireball.h src/common_window_vulkan.h src/window.h
vulkan_dependencies = src/fireball.h src/common_window_vulkan.h src/vulkan.h src/vulkan/private.h

linux:		$(fireball_dependencies) tmp/window/linux.o
	gcc -Wall -shared -fvisibility=hidden -o build/fireball.so \
	  $(fireball_dependencies) \
	  tmp/window/linux.o \
	  -lm \
	  -lxcb \
	  -lvulkan
	cp src/fireball.h build/fireball.h
	cp build/fireball.h sample/fireball.h
	cp build/fireball.so sample/fireball.so
windows:	$(fireball_dependencies) tmp/window/windows.o
	gcc -Wall -shared -o build/fireball.dll \
	  $(fireball_dependencies) \
	  tmp/window/windows.o \
	  -lvulkan-1
	copy src\fireball.h build\fireball.h > nul
	copy build\fireball.h sample\fireball.h > nul
	copy build\fireball.dll sample\fireball.dll > nul

tmp/fireball/fireball.o:	src/fireball.h src/fireball/fireball.c
	gcc -Wall -c -o tmp/fireball/fireball.o src/fireball/fireball.c
tmp/fireball/logger.o:		src/fireball.h src/fireball/logger.c
	gcc -Wall -c -o tmp/fireball/logger.o src/fireball/logger.c

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

clean_linux:
	rm -rf fireball bin2c/bin2c build tmp sample/fireball.h sample/fireball.so
	mkdir build
	mkdir tmp
	mkdir tmp/fireball
	mkdir tmp/window
	mkdir tmp/vulkan
clean_windows:
	del /F bin2c\bin2c bin2c\bin2c.pdb sample\fireball.h sample\fireball.dll > nul 2>&1
	if exist build ( rd /s /q build )
	if exist tmp ( rd /s /q tmp )
	mkdir build
	mkdir tmp
	mkdir tmp\fireball
	mkdir tmp\window
	mkdir tmp\vulkan
