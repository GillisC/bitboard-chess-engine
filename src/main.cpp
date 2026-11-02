#include <stdio.h>

#include "board.h"
#include "board.cpp"

int main(void) {
    board Board;
    InitBoard(&Board);

    move_list Moves = ML_Create(32);
    ComputeMoves(&Board, WHITE, &Moves);
    PrintBoard(&Board);
    printf("Moves found: %zu\n", Moves.Size);

    return 0;
}
