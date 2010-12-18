TARGET = frogger

all:
	g++ -Wall -Wextra main.cpp -o $(TARGET) `pkg-config QtGui sdl --cflags --libs` -lSDL_mixer

check: all
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o
