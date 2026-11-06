CXX=g++

CFLAGS = -Wall -Wextra -g -O1
LIBS = -lraylib

TEST_LIBS = -lCatch2Main -lCatch2

BIN = chess_engine
TEST_BIN = test_chess_engine

.PHONY: all test clean

all:
	$(CXX) $(CFLAGS) src/main.cpp -o $(BIN) $(LIBS)
	./$(BIN)

test:
	$(CXX) $(CFLAGS) test/main.cpp -o $(TEST_BIN) $(TEST_LIBS)
	./$(TEST_BIN)

clean:
	rm -rf $(BIN) $(TEST_BIN)
