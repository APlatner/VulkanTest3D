CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -L/home/andrew/vulkan/1.2.176.1/x86_64/etc/vulkan/explicit_layer.d -ldl -lpthread -lX11 -lXxf86vm -lXrandr -L/usr/include/X11/extensions/XI.h
GLSLC = /usr/local/bin/glslc

vertSources = $(shell find ./shaders -type f -name "*.vert")
vertObjFiles = $(patsubst %.vert, %.vert.spv, $(vertSources))
fragSources = $(shell find ./shaders -type f -name "*.frag")
fragObjFiles = $(patsubst %.frag, %.frag.spv, $(fragSources))

TARGET = a.out
$(TARGET): $(vertObjFiles) $(fragObjFiles)
$(TARGET): *.cpp *.hpp
	g++ $(CFLAGS) -o $(TARGET) *.cpp $(LDFLAGS)

%.spv: %
	${GLSLC} $< -o $@

.PHONY: test clean

test: a.out
	./a.out

clean:
	rm -f a.out
	rm -f shaders/*.spv