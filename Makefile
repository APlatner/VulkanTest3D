CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -L/home/andrew/vulkan/1.2.176.1/x86_64/etc/vulkan/explicit_layer.d -ldl -lpthread -lX11 -lXxf86vm -lXrandr -L/usr/include/X11/extensions/XI.h

SOURCES = *.cpp
HEADERS = *.hpp

a.out: $(SOURCES) $(HEADERS)
	g++ $(CFLAGS) -o a.out $(SOURCES) $(LDFLAGS)

.PHONY: test clean

test: a.out
	./a.out

clean:
	rm -f a.out