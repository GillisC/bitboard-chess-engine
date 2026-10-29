CXX=g++

CFLAGS = -Wall -Wextra -g -O1
LIBS = -lraylib

BIN = chess_engine

all:
	$(CXX) $(CFLAGS) src/main.cpp -o $(BIN) $(LIBS)
	./$(BIN)

clean:
	rm -rf $(BIN)
