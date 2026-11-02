#include "board.h"
#include <assert.h>
#include <cstdint>
#include <functional>

static bool IsQueensideCastlingAvailable(const board *Board, side Side) {
    uint16_t QueensideCastling;
    if (Side == WHITE) {
        QueensideCastling = (Board->state >> 1) & 0x1;
    }
    else {
        QueensideCastling = (Board->state >> 3) & 0x1;
    }
    return QueensideCastling > 0 ? true : false;
}

static bool IsKingsideCastlingAvailable(const board *Board, side Side) {
    uint16_t KingsideCastling;
    if (Side == WHITE) {
        KingsideCastling = (Board->state >> 2) & 0x1;
    }
    else {
        KingsideCastling = (Board->state >> 4) & 0x1;
    }
    return KingsideCastling > 0 ? true : false;
}

move_list ML_Create(size_t InitialCapacity) {
    struct move_list Result = {};
    Result.Moves = (move *)calloc(InitialCapacity, sizeof(move));
    if (!Result.Moves) {
        fprintf(stderr, "Error: calloc failed\n");
        exit(EXIT_FAILURE);
    }
    Result.Size = 0;
    Result.Capacity = InitialCapacity;

    return(Result);
}

void ML_PushBack(move_list *MoveList, move Move) {
    if (MoveList->Size + 1 > MoveList->Capacity) {
        size_t NewCapacity = MoveList->Capacity * 2;
        move *Temp = (move *)realloc(MoveList->Moves, NewCapacity * sizeof(move));
        if (!Temp) {
            fprintf(stderr, "Error: realloc failed\n");
            exit(EXIT_FAILURE);
        }
        MoveList->Capacity = NewCapacity;
        MoveList->Moves = Temp;
    }
    MoveList->Moves[MoveList->Size++] = Move;
}

void ML_Clear(move_list *MoveList) {
    MoveList->Size = 0;
}

void ML_Destroy(move_list *MoveList) {
    free(MoveList->Moves);
    free(MoveList);
}

void InitBoard(board *B) {
    assert(B);

    B->bitboards[WHITE_PAWN]   = 0x000000000000FF00;
    B->bitboards[WHITE_KNIGHT] = 0x0000000000000042;
    B->bitboards[WHITE_BISHOP] = 0x0000000000000024;
    B->bitboards[WHITE_ROOK]   = 0x0000000000000081;
    B->bitboards[WHITE_QUEEN]  = 0x0000000000000008;
    B->bitboards[WHITE_KING]   = 0x0000000000000010;

    B->bitboards[BLACK_PAWN]   = 0x00FF000000000000;
    B->bitboards[BLACK_KNIGHT] = 0x4200000000000000;
    B->bitboards[BLACK_BISHOP] = 0x2400000000000000;
    B->bitboards[BLACK_ROOK]   = 0x8100000000000000;
    B->bitboards[BLACK_QUEEN]  = 0x0800000000000000;
    B->bitboards[BLACK_KING]   = 0x1000000000000000;

    B->state = 30; // white side to move (0), all castling rights available, no enpassant
}

BoardBitField GetWhiteOccupancy(const board *B) {
    BoardBitField Result =
        B->bitboards[WHITE_PAWN]   |
        B->bitboards[WHITE_KNIGHT] |
        B->bitboards[WHITE_BISHOP] |
        B->bitboards[WHITE_ROOK]   |
        B->bitboards[WHITE_QUEEN]  |
        B->bitboards[WHITE_KING];

    return(Result);
}

BoardBitField GetBlackOccupancy(const board *B) {
    BoardBitField Result =
        B->bitboards[BLACK_PAWN]   |
        B->bitboards[BLACK_KNIGHT] |
        B->bitboards[BLACK_BISHOP] |
        B->bitboards[BLACK_ROOK]   |
        B->bitboards[BLACK_QUEEN]  |
        B->bitboards[BLACK_KING];

    return(Result);
}

BoardBitField GetOccupancy(const board *B) {
    BoardBitField Result = GetWhiteOccupancy(B) | GetBlackOccupancy(B);

    return(Result);
}

void PrintBoard(const board *B) {
    char PieceSymbols[] = {'p', 'n', 'b', 'r', 'q', 'k', 'P', 'N', 'B', 'R', 'Q', 'K'};
    char RankChar[] = {'1', '2', '3', '4', '5', '6', '7', '8'};
    char FileChar[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

    /* Starting from the bottom left corner of the board. */
    for (int Rank = 7; Rank >= 0; Rank--) {
        for (int File = 0; File < 8; File++) {
            if (File == 0) {
                printf(" %c  ", RankChar[Rank]);
            }
            /* Look through all bitboards, print the corresponding character if found */
            bool FoundPiece = false;
            for (int BoardSquareIndex = 0; BoardSquareIndex < NUM_BITBOARDS; BoardSquareIndex++) {
                uint64_t BitIndex = 1ULL << (Rank * 8 + File);
                if ((BitIndex & B->bitboards[BoardSquareIndex]) > 0) {
                    printf("%c ", PieceSymbols[BoardSquareIndex]);
                    FoundPiece = true;
                    break;
                }
            }
            if (!FoundPiece) {
                printf(". ");
            }
        }
        printf("\n");
    }
}

#define RANK_1 0x00000000000000FFULL
#define RANK_2 0x000000000000FF00ULL
#define RANK_3 0x0000000000FF0000ULL
#define RANK_4 0x00000000FF000000ULL
#define RANK_5 0x000000FF00000000ULL
#define RANK_6 0x0000FF0000000000ULL
#define RANK_7 0x00FF000000000000ULL
#define RANK_8 0xFF00000000000000ULL

#define FILE_A 0x0101010101010101ULL
#define FILE_B 0x0202020202020202ULL
#define FILE_C 0x0404040404040404ULL
#define FILE_D 0x0808080808080808ULL
#define FILE_E 0x1010101010101010ULL
#define FILE_F 0x2020202020202020ULL
#define FILE_G 0x4040404040404040ULL
#define FILE_H 0x8080808080808080ULL

BoardBitField GetKnightAttacks(BitFieldIndex KnightPosition) {
    BoardBitField Result = 0;
    Result |= (KnightPosition & ~FILE_H) << 17;
    Result |= (KnightPosition & ~FILE_A) << 15;

    Result |= (KnightPosition & ~(FILE_G | FILE_H)) << 10;
    Result |= (KnightPosition & ~(FILE_A | FILE_B)) << 6;

    Result |= (KnightPosition & ~FILE_H) >> 15;
    Result |= (KnightPosition & ~FILE_A) >> 17;

    Result |= (KnightPosition & ~(FILE_G | FILE_H)) >> 6;
    Result |= (KnightPosition & ~(FILE_A | FILE_B)) >> 10;

    return(Result);
}


move EncodeMove(BitFieldIndex Source, BitFieldIndex Target, move_type MoveType) {
    move Result = 0;

    Result |= Source;
    Result |= (Target << 6);
    Result |= (MoveType << 12);

    return(Result);
}

void CalculateSlidingMoves(
        move_list *Moves,
        const board *Board,
        const BoardBitField SourcePiece,
        const side SourcePieceColor,
        const std::function<BoardBitField(BoardBitField)> ShiftFunction,
        const BoardBitField Mask) 
{
    BitFieldIndex SourceIndex = __builtin_ctzll(SourcePiece);
    BoardBitField TargetSquare = SourcePiece & Mask;
    BoardBitField OpposingOccupied = SourcePieceColor == WHITE ? GetBlackOccupancy(Board) : GetWhiteOccupancy(Board);
    BoardBitField Occupied = GetOccupancy(Board);

    while (TargetSquare) {
        TargetSquare = ShiftFunction( (TargetSquare & Mask) );
        BitFieldIndex TargetIndex = __builtin_ctzll(TargetSquare);

        if (TargetSquare & OpposingOccupied) {
            ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, CAPTURE));
            break;
        }
        else if (TargetSquare & ~Occupied) {
            ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, QUIET));
        }
        else {
            break;
        }
    }

}
/*
 * Given a board state, and a side to calculate from generate all available moves.
 */
void ComputeMoves(const board *Board, side Side, move_list *Moves) {
    ML_Clear(Moves);

    BoardBitField Occupied = GetOccupancy(Board);
    BoardBitField OpposingOccupied = Side == WHITE ? GetBlackOccupancy(Board) : GetWhiteOccupancy(Board);

    int BitBoardPawnIndex = Side == WHITE ? WHITE_PAWN : BLACK_PAWN;
    int BitBoardKnightIndex = BitBoardPawnIndex + 1;
    int BitBoardBishopIndex = BitBoardPawnIndex + 2;
    int BitBoardRookIndex = BitBoardPawnIndex + 3;
    int BitBoardQueenIndex = BitBoardPawnIndex + 4;
    int BitBoardKingIndex = BitBoardPawnIndex + 5;

    BoardBitField SingleStepForward = 0;
    BoardBitField DoubleStepForward = 0;
    BoardBitField DiagonalLeftAttack = 0;
    BoardBitField DiagonalRightAttack = 0;
    BoardBitField PromotionForward = 0;
    BoardBitField PromotionAttackLeft = 0;
    BoardBitField PromotionAttackRight = 0;

    /* Offsets used to get the correct source square */
    int ForwardStep        = Side == WHITE ? 8 : -8;
    int ForwardUpLeftStep  = Side == WHITE ? 7 : -9;
    int ForwardUpRightStep = Side == WHITE ? 9 : -7;

    /* Pawn moves generation changes based on color. (Shift direction) */
    BoardBitField PawnSource = Board->bitboards[BitBoardPawnIndex];
    if (Side == WHITE) {
        /* Moves 'upward' */
        SingleStepForward = (PawnSource << 8) & ~Occupied;
        DoubleStepForward = ((PawnSource & RANK_2) << 16) & ~Occupied;
        DiagonalLeftAttack = ((PawnSource & ~FILE_A) << 7) & OpposingOccupied;
        DiagonalRightAttack = ((PawnSource & ~FILE_H) << 9) & OpposingOccupied;
        PromotionForward = ((PawnSource & RANK_7) << 8) & ~Occupied;
        PromotionAttackLeft = ((PawnSource & RANK_7 & ~FILE_A) << 7) & OpposingOccupied;
        PromotionAttackRight = ((PawnSource & RANK_7 & ~FILE_H) << 9) & OpposingOccupied;
    }
    else if (Side == BLACK) {
        /* Moves 'downward' */
        SingleStepForward = (PawnSource >> 8) & ~Occupied;
        DoubleStepForward = ((PawnSource & RANK_7) >> 16) & ~Occupied;
        DiagonalLeftAttack = ((PawnSource & ~FILE_A) >> 9) & OpposingOccupied;
        DiagonalRightAttack = ((PawnSource & ~FILE_H) >> 7) & OpposingOccupied;
        PromotionForward = ((PawnSource & RANK_2) >> 8) & ~Occupied;
        PromotionAttackLeft = ((PawnSource & RANK_2 & ~FILE_A) >> 9) & OpposingOccupied;
        PromotionAttackRight = ((PawnSource & RANK_2 & ~FILE_H) >> 7) & OpposingOccupied;
    }
    else {
        assert(false && "We should not reach here");
    }

    while (SingleStepForward) {
        BitFieldIndex TargetIndex = __builtin_ctzll(SingleStepForward);
        BitFieldIndex SourceIndex = TargetIndex - ForwardStep;
        ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, QUIET));

        SingleStepForward &= SingleStepForward - 1;
    }

    while (DoubleStepForward) {
        BitFieldIndex TargetIndex = __builtin_ctzll(SingleStepForward);
        BitFieldIndex SourceIndex = TargetIndex - ForwardStep * 2;
        ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, QUIET));

        DoubleStepForward &= DoubleStepForward - 1;
    }

    while (DiagonalLeftAttack) {
        BitFieldIndex TargetIndex = __builtin_ctzll(DiagonalLeftAttack);
        BitFieldIndex SourceIndex = TargetIndex - 7;
        ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, QUIET));

        DiagonalLeftAttack &= DiagonalLeftAttack - ForwardUpLeftStep;
    }

    while (DiagonalRightAttack) {
        BitFieldIndex TargetIndex = __builtin_ctzll(DiagonalRightAttack);
        BitFieldIndex SourceIndex = TargetIndex - ForwardUpRightStep;
        ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, QUIET));

        DiagonalRightAttack &= DiagonalRightAttack - 1;
    }

    while (PromotionForward) {
        BitFieldIndex TargetIndex = __builtin_ctzll(PromotionForward);
        BitFieldIndex Source = TargetIndex - ForwardStep;

        move KnightPromotion = EncodeMove(Source, TargetIndex, KNIGHT_PROMOTION);
        move BishopPromotion = EncodeMove(Source, TargetIndex, BISHOP_PROMOTION);
        move RookPromotion   = EncodeMove(Source, TargetIndex, ROOK_PROMOTION);
        move QueenPromotion  = EncodeMove(Source, TargetIndex, QUEEN_PROMOTION);

        ML_PushBack(Moves, KnightPromotion);
        ML_PushBack(Moves, BishopPromotion);
        ML_PushBack(Moves, RookPromotion);
        ML_PushBack(Moves, QueenPromotion);

        PromotionForward &= PromotionForward - 1;
    }

    while (PromotionAttackLeft) {
        BitFieldIndex TargetIndex = __builtin_ctzll(PromotionAttackLeft);
        BitFieldIndex Source = TargetIndex - ForwardUpLeftStep;

        move KnightPromotion = EncodeMove(Source, TargetIndex, KNIGHT_PROMOTION_CAPTURE);
        move BishopPromotion = EncodeMove(Source, TargetIndex, BISHOP_PROMOTION_CAPTURE);
        move RookPromotion   = EncodeMove(Source, TargetIndex, ROOK_PROMOTION_CAPTURE);
        move QueenPromotion  = EncodeMove(Source, TargetIndex, QUEEN_PROMOTION_CAPTURE);

        ML_PushBack(Moves, KnightPromotion);
        ML_PushBack(Moves, BishopPromotion);
        ML_PushBack(Moves, RookPromotion);
        ML_PushBack(Moves, QueenPromotion);

        PromotionAttackLeft &= PromotionAttackLeft - 1;
    }

    while (PromotionAttackRight) {
        BitFieldIndex TargetIndex = __builtin_ctzll(PromotionAttackRight);
        BitFieldIndex Source = TargetIndex - ForwardUpRightStep;

        move KnightPromotion = EncodeMove(Source, TargetIndex, KNIGHT_PROMOTION_CAPTURE);
        move BishopPromotion = EncodeMove(Source, TargetIndex, BISHOP_PROMOTION_CAPTURE);
        move RookPromotion   = EncodeMove(Source, TargetIndex, ROOK_PROMOTION_CAPTURE);
        move QueenPromotion  = EncodeMove(Source, TargetIndex, QUEEN_PROMOTION_CAPTURE);

        ML_PushBack(Moves, KnightPromotion);
        ML_PushBack(Moves, BishopPromotion);
        ML_PushBack(Moves, RookPromotion);
        ML_PushBack(Moves, QueenPromotion);

        PromotionAttackRight &= PromotionAttackRight - 1;
    }

    /* White Rook move generation */
    BoardBitField KnightSource = Board->bitboards[BitBoardRookIndex];

    while (KnightSource) {
        BitFieldIndex SourceSquareIndex = __builtin_ctzll(PromotionAttackRight);
        BoardBitField Targets = GetKnightAttacks(SourceSquareIndex);

        BoardBitField QuietTargets  = Targets & ~Occupied;
        BoardBitField CaptureTargets = Targets & OpposingOccupied;

        while (QuietTargets) {
            BitFieldIndex TargetIndex = __builtin_ctzll(QuietTargets);
            ML_PushBack(Moves, EncodeMove(SourceSquareIndex, TargetIndex, QUIET));
            QuietTargets &= QuietTargets - 1;
        }

        while (CaptureTargets) {
            BitFieldIndex TargetIndex = __builtin_ctzll(CaptureTargets);
            ML_PushBack(Moves, EncodeMove(SourceSquareIndex, TargetIndex, CAPTURE));
            CaptureTargets &= CaptureTargets - 1;
        }

        KnightSource &= KnightSource - 1;
    }

    /* White Bishop move generation */
    BoardBitField BishopsSource = Board->bitboards[BitBoardBishopIndex];

    while (BishopsSource) {
        BitFieldIndex SourceSquareIndex = __builtin_ctzll(BishopsSource);
        BoardBitField BishopSource = 1ULL << SourceSquareIndex;

        /* Diagonal left-up */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            BishopSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 7; },
            ~FILE_A);

        /* Diagonal left-down */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            BishopSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 9; },
            ~FILE_A);

        /* Diagonal right-up */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            BishopSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 9; },
            ~FILE_H);

        /* Diagonal right-down */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            BishopSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 7; },
            ~FILE_H);

        BishopsSource &= BishopsSource - 1;
    }

    /* Rook move generation */
    BoardBitField RooksSource = Board->bitboards[WHITE_ROOK];

    while (RooksSource) {
        BitFieldIndex SourceSquareIndex = __builtin_ctzll(RooksSource);
        BoardBitField RookSource = 1ULL << SourceSquareIndex;

        /* Up */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            RookSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 8; },
            ~RANK_8);

        /* Down */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            RookSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 8; },
            ~RANK_1);

        /* Left */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            RookSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 1; },
            ~FILE_A);

        /* Right */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            RookSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 1; },
            ~FILE_H);

        RooksSource &= RooksSource - 1;
    }

    /* Queen move generation */
    BoardBitField QueensSource = Board->bitboards[BitBoardQueenIndex];

    while (QueensSource) {
        BitFieldIndex SourceSquareIndex = __builtin_ctzll(QueensSource);
        BoardBitField QueenSource = 1ULL << SourceSquareIndex;

        /* Up */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 8; },
            ~RANK_8);

        /* Down */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 8; },
            ~RANK_1);

        /* Left */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 1; },
            ~FILE_A);

        /* Right */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 1; },
            ~FILE_H);

        /* Diagonal left-up */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 7; },
            ~FILE_A);

        /* Diagonal left-down */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 9; },
            ~FILE_A);

        /* Diagonal right-up */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField << 9; },
            ~FILE_H);

        /* Diagonal right-down */
        CalculateSlidingMoves(
            Moves, 
            Board, 
            QueenSource, 
            WHITE, 
            [](BoardBitField BitField) { return BitField >> 7; },
            ~FILE_H);

        QueensSource &= QueensSource - 1;
    }
    /* King move generation */
    // TODO(tim): Make this two bitmasks instead of shifting around
    BoardBitField KingSource = Board->bitboards[BitBoardKingIndex];
    BitFieldIndex SourceIndex = __builtin_ctzll(KingSource); 

    BoardBitField KingQuietTargets = 0;
    BoardBitField KingCaptureTargets = 0;

    /* Move up */
    KingCaptureTargets |= ((KingSource & RANK_8) << 8) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & RANK_8) << 8) & ~Occupied;

    /* Move down */
    KingCaptureTargets |= ((KingSource & RANK_1) >> 8) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & RANK_1) >> 8) & ~Occupied;

    /* Move left */
    KingCaptureTargets |= ((KingSource & RANK_1) >> 1) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & RANK_1) >> 1) & ~Occupied;

    /* Move right */
    KingCaptureTargets |= ((KingSource & RANK_1) << 1) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & RANK_1) << 1) & ~Occupied;

    /* Diagonals */
    KingCaptureTargets |= ((KingSource & FILE_A) << 7) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & FILE_A) << 7) & ~Occupied;
    KingCaptureTargets |= ((KingSource & FILE_H) << 9) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & FILE_H) << 9) & ~Occupied;

    KingCaptureTargets |= ((KingSource & FILE_A) >> 9) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & FILE_A) >> 9) & ~Occupied;
    KingCaptureTargets |= ((KingSource & FILE_H) >> 7) & OpposingOccupied;
    KingQuietTargets   |= ((KingSource & FILE_H) >> 7) & ~Occupied;

    while (KingCaptureTargets) {
        BitFieldIndex TargetIndex = __builtin_ctzll(KingCaptureTargets);
        ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, CAPTURE));
        KingCaptureTargets &= KingCaptureTargets - 1;
    }

    while (KingQuietTargets) {
        BitFieldIndex TargetIndex = __builtin_ctzll(KingQuietTargets);
        ML_PushBack(Moves, EncodeMove(SourceIndex, TargetIndex, QUIET));
        KingQuietTargets &= KingQuietTargets - 1;
    }

    /* Generate castling moves */
    BitFieldIndex KingSourceIndex;
    BitFieldIndex QueensideTarget;
    BitFieldIndex KingsideTarget;
    
    /* Mask used to isolate part of a rank */
    BoardBitField QueensideMask;
    BoardBitField KingsideMask;

    /* Expected bitfield value after applying the mask. Makes sure the rank is empty apart from the rook and king */
    BoardBitField QueensideExpected;
    BoardBitField KingsideExpected;


    if (Side == WHITE) {
        KingSourceIndex   = 4;
        QueensideTarget   = 0;
        KingsideTarget    = 7;

        QueensideMask     = 0xF8;
        KingsideMask      = 0xF;
        QueensideExpected = 0x88;
        KingsideExpected  = 0x9;
    }
    else {
        KingSourceIndex   = 60;
        QueensideTarget   = 56;
        KingsideTarget    = 63;

        QueensideMask     = 0xF800000000000000;
        KingsideMask      = 0xF000000000000000;
        QueensideExpected = 0x8800000000000000;
        KingsideExpected  = 0x9000000000000000;
    }

    /* Queenside */
    if ((Occupied & QueensideMask) == QueensideExpected && IsQueensideCastlingAvailable(Board, Side)) {
        ML_PushBack(Moves, EncodeMove(KingSourceIndex, QueensideTarget, QUEEN_CASTLE));
    }
    /* Kingside */
    if ((Occupied & KingsideMask) == KingsideExpected && IsKingsideCastlingAvailable(Board, Side)) {
        ML_PushBack(Moves, EncodeMove(KingSourceIndex, KingsideTarget, KING_CASTLE));
    }
    
}
