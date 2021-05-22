#include <stdlib.h>
#include "../chessSystem.h"
#include "../test_utilities.h"
#define NUMBER_OF_TESTS 2

bool testChessAddGame() {
    ChessSystem chess = chessCreate();
    ASSERT_TEST(chessAddTournament(chess, 1, 2, "London") == CHESS_SUCCESS);
    ASSERT_TEST(chessAddTournament(chess, 2, 5, "London") == CHESS_SUCCESS);
    ASSERT_TEST(chessAddGame(chess,1,1,2,SECOND_PLAYER,30) == CHESS_SUCCESS);
    ASSERT_TEST(chessAddGame(chess,1,2,3,SECOND_PLAYER,30) == CHESS_SUCCESS);
    ASSERT_TEST(chessAddGame(chess,1,3,2,FIRST_PLAYER,20) == CHESS_GAME_ALREADY_EXISTS);
    ASSERT_TEST(chessAddGame(chess,1,2,4,DRAW,20) == CHESS_EXCEEDED_GAMES);
    ASSERT_TEST(chessAddGame(chess,3,1,3,DRAW,50) == CHESS_TOURNAMENT_NOT_EXIST);
    ASSERT_TEST(chessAddGame(chess,2,2,-4,SECOND_PLAYER,10) == CHESS_INVALID_ID);
    ASSERT_TEST(chessAddGame(chess, 2, 1,1,FIRST_PLAYER,20) == CHESS_INVALID_ID);

    chessDestroy(chess);
    return true;
}

bool testChessRemoveTournament() {
    ChessSystem chess = chessCreate();
    ASSERT_TEST(chessAddTournament(chess, 1, 4, "London") == CHESS_SUCCESS);
    ASSERT_TEST(chessAddTournament(chess, 2, 3, "Tel Aviv") == CHESS_SUCCESS); 
    ASSERT_TEST(chessRemoveTournament(chess,1) == CHESS_SUCCESS);
    ASSERT_TEST(chessRemoveTournament(chess,3) == CHESS_TOURNAMENT_NOT_EXIST);

    chessDestroy(chess);
    return true;
}

bool testChessRemovePlayer() {
    ChessSystem chess = chessCreate();
    ASSERT_TEST(chessAddTournament(chess, 1, 4, "London") == CHESS_SUCCESS);
    ASSERT_TEST(chessAddTournament(chess, 2, 3, "Tel Aviv") == CHESS_SUCCESS); 
    //adding 2 games in London and making player 1 the winner 
    ASSERT_TEST(chessAddGame(chess,1,1,2,FIRST_PLAYER,20) == CHESS_SUCCESS);
    ASSERT_TEST(chessAddGame(chess,1,1,3,FIRST_PLAYER,25) == CHESS_SUCCESS);
    //adding some more games in Tel Aviv
    ASSERT_TEST(chessAddGame(chess,2,1,2,FIRST_PLAYER,30) == CHESS_SUCCESS); //a similar match took part in London. supposed to be added
    //successfully? 
    ASSERT_TEST(chessAddGame(chess, 1, 3, 1, SECOND_PLAYER,45) == CHESS_SUCCESS);
    ASSERT_TEST(chessAddGame(chess, 2, 2, 3, DRAW, 20) == CHESS_SUCCESS);
    ASSERT_TEST(chessAddGame(chess, 2, 3, 4, FIRST_PLAYER, 20) == CHESS_SUCCESS);
    ASSERT_TEST(chessEndTournament(chess,2) == CHESS_SUCCESS); //ending Tel Aviv tournament
    //testing Remove
    ASSERT_TEST(chessRemovePlayer(chess,5) == CHESS_PLAYER_NOT_EXIST);
    ASSERT_TEST(chessRemovePlayer(chess,1) == CHESS_SUCCESS); //removing player 1 from not ended tournament
    ASSERT_TEST(chessSavePlayersLevels(chess,"../outputForTests2.txt") == CHESS_SUCCESS);

    chessDestroy(chess);
    return true;
    
}

bool (*tests[]) (void) = {
        testChessRemoveTournament,
        testChessAddGame
};

/*The names of the test functions should be added here*/
const char* testNames[] = {
        "testChessRemoveTournament",
        "testChessAddGame"
};

int main()
{
    for(int i=0; i< NUMBER_OF_TESTS; i++)
    {
        printf("%s\n", testNames[i]);
        tests[i];
    }
}
