#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* An integer that represents the index of a square (0-63), starting from the bottom left square. */
// TODO(tim): change this to lowercase
typedef int BitFieldIndex;
typedef uint64_t BoardBitField;
typedef uint16_t BoardStateBitField;

enum side {
    WHITE,
    BLACK
};

enum bitboards {
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,

    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,

    NUM_BITBOARDS
};

struct board {
    BoardBitField bitboards[NUM_BITBOARDS];

    /* State is a packed integer */
    /* bit 0: side-to-move (0 --> white, 1 --> black)
       bits 1-4 castling rights: white first queen then king side.
       bits 5-10 enpassant square BitFieldIndex(0-63) */
    BoardStateBitField state;
};

/*
 * Packed integer
 * bits 0-5: Source square index
 * bits 6-11: Target square index
 * bits 12-15: Move type
 */
typedef uint16_t move;

enum move_type {
    QUIET                    = 0x0,
    DOUBLE_PAWN_PUSH         = 0x1,
    KING_CASTLE              = 0x2,
    QUEEN_CASTLE             = 0x3,
    CAPTURE                  = 0x4,
    EP_CAPTURE               = 0x5,
    KNIGHT_PROMOTION         = 0x6,
    BISHOP_PROMOTION         = 0x7,
    ROOK_PROMOTION           = 0x8,
    QUEEN_PROMOTION          = 0x9,
    KNIGHT_PROMOTION_CAPTURE = 0xA,
    BISHOP_PROMOTION_CAPTURE = 0xB,
    ROOK_PROMOTION_CAPTURE   = 0xC,
    QUEEN_PROMOTION_CAPTURE  = 0xD
};

struct move_list {
    move* Moves;
    size_t Size;
    size_t Capacity;
};

/* ============================================================================
   Board Management
 ============================================================================== */

void InitBoard(board *B);
BoardBitField GetWhiteOccupancy(const board *B);
BoardBitField GetBlackOccupancy(const board *B);
BoardBitField GetOccupancy(const board *B);
void PrintBoard(const board *B);
void ComputeMoves(const board *Board, move_list *Moves);

constexpr bool SideToMove(BoardStateBitField State) { return State & 0x1; }
// TODO(tim): Castling rights here.
constexpr BitFieldIndex EnPassantSquare(BoardStateBitField State) { return (State >> 5) & 0x3F; }

/* ============================================================================
    Move Generation
 ============================================================================== */

move_list ML_Create();
void ML_PushBack(move_list *MoveList, move Move);
void ML_Clear(move_list *MoveList);
void ML_Destroy(move_list *MoveList);

constexpr BitFieldIndex MoveSource(move Move) { return Move & 0x3F; }
constexpr BitFieldIndex MoveTarget(move Move) { return (Move >> 6) & 0x3F; }
constexpr move_type MoveType(move Move) { return (move_type)(Move >> 12); }
