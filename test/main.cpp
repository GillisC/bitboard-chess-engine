#include <catch2/catch_test_macros.hpp>

#include "../src/board.cpp"

TEST_CASE("LoadFEN") {
    board Board;
    ClearBoard(&Board);

    char FenTest1[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    LoadFEN(&Board, FenTest1);

    REQUIRE(GetSideToMove(&Board) == WHITE);
    REQUIRE(CanWhiteKingsideCastle(&Board) == true);
    REQUIRE(CanWhiteQueensideCastle(&Board) == true);
    REQUIRE(CanBlackKingsideCastle(&Board) == true);
    REQUIRE(CanBlackQueensideCastle(&Board) == true);
    REQUIRE(IsEnPassantAvailable(&Board) == false);
    REQUIRE(EnPassantSquare(&Board) == NO_EN_PASSANT);
    REQUIRE(Board.half_moves == 0);
    REQUIRE(Board.full_moves == 1);

    ClearBoard(&Board);
    REQUIRE(Board.state == 0);
    REQUIRE(GetOccupancy(&Board) == 0);

    char FenTest2[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b kq e6 10 100";
    LoadFEN(&Board, FenTest2);

    REQUIRE(GetSideToMove(&Board) == BLACK);
    REQUIRE(CanWhiteKingsideCastle(&Board) == false);
    REQUIRE(CanWhiteQueensideCastle(&Board) == false);
    REQUIRE(CanBlackKingsideCastle(&Board) == true);
    REQUIRE(CanBlackQueensideCastle(&Board) == true);
    REQUIRE(IsEnPassantAvailable(&Board) == true);
    REQUIRE(Board.half_moves == 10);
    REQUIRE(Board.full_moves == 100);
}

