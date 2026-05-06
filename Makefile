CC = gcc
CFLAGS = -Iinclude
LIBS = -lws2_32

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

build/app.exe: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o build/app.exe

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del build\*.o build\*.exe