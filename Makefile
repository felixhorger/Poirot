.PHONY: all clean

build/poirot.o: src/*.c
	gcc -c src/poirot.c -o build/poirot.o
#

bin/poirot: build/poirot.o ../glaze/lib/libglaze.a
	#gcc -L../glaze/lib -I../glaze/lib -o bin/poirot build/poirot.o -lglaze -ldl -lm -lGL -lglfw
	gcc -o bin/poirot build/poirot.o ../glaze/lib/libglaze.a -ldl -lm -lGL -lglfw
#

all: bin/poirot

clean:
	rm -f bin/*
	rm -f build/*
