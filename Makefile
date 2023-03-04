.PHONY: all

build/glad.o: src/glad.c include/glad.h include/khrplatform.h
	gcc -c src/glad.c -o build/glad.o
#

build/poirot.o: src/poirot.c
	gcc -c src/poirot.c -o build/poirot.o
#

bin/poirot: build/poirot.o build/glad.o
	gcc build/poirot.o build/glad.o -ldl -lGL -lglfw -o bin/poirot
#

all: bin/poirot
	
