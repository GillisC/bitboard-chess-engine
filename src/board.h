#pragma once

#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define NO_EN_PASSANT 65

/* An integer that represents the index of a square (0-63), starting from the bottom left square. */
// TODO(tim): change this to lowercase
typedef int BitFieldIndex;
typedef uint64_t BoardBitField;
typedef uint16_t board_state_bit_field;

enum side {
    WHITE,
    BLACK
};

enum bitboard {
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

    /* 
     *  State is a packed uint32_t
     * bit 0: side-to-move (0 --> white, 1 --> black)
     * bits 1-4 castling rights: white first king then queen side.
     * bits 5-11 (7 bits) enpassant square BitFieldIndex
     */
    uint16_t state;
    unsigned int half_moves;
    unsigned int full_moves;
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
void ClearBoard(board *B);
BoardBitField GetWhiteOccupancy(const board *B);
BoardBitField GetBlackOccupancy(const board *B);
BoardBitField GetOccupancy(const board *B);
void PrintBoard(const board *B);
void ComputeMoves(const board *Board, side SideToMove, move_list *Moves);

constexpr side GetSideToMove(board *Board) { return (side)(Board->state & 0x1); }
constexpr bool CanWhiteKingsideCastle(board *Board)   { return (Board->state >> 1) & 0x1; }
constexpr bool CanWhiteQueensideCastle(board *Board)  { return (Board->state >> 2) & 0x1; }
constexpr bool CanBlackKingsideCastle(board *Board)   { return (Board->state >> 3) & 0x1; }
constexpr bool CanBlackQueensideCastle(board *Board)  { return (Board->state >> 4) & 0x1; }
constexpr bool IsEnPassantAvailable(board *Board) { return ((Board->state >> 5) & 0x7F) != NO_EN_PASSANT; }
constexpr BitFieldIndex EnPassantSquare(board *Board) { return (Board->state >> 5) & 0x7F; }

void SetSideToMove(board *Board, side SideToMove);
void SetCastlingRights(board *Board, bool WhiteKing, bool WhiteQueen, bool BlackKing, bool BlackQueen);
void SetEnpassantSquare(board *Board, unsigned int Index);

void LoadFEN(board *Board, char *Fen);
char *ExportFEN(board *Board);

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

void ExecuteMove(board *Board, move Move);
void UndoMove(board *Board, move Move);
