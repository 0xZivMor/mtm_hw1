#ifndef PLAYER_H
#define PLAYER_H

#include "chessSystem.h"
#include "map.h"

typedef struct player_t *Player;

/**
 * Creates a new instance of Player
 * 
 * @param id player's id
 * @return 
 *    A new Player instance if id is valid and all memory 
 *    allocation operation succeded.
 *    NULL otherwise.
 */
Player playerCreate(int id);

/**
 * Retrieves a player's ID
 * 
 * @param player Player in question
 * @return 
 *    > 0 player's ID
 *    0   provided player was NULL
 */
int playerGetId(Player player);

/**
 * Adds a match to player's record.
 * 
 * @param player Player in question
 * @param Match match to be added
 * @return
 *    CHESS_NULL_ARGUMENT - NULL argument was provided
 *    CHESS_OUT_OF_MEMORY - memory related failure
 *    CHESS_SUCCESS - match was added successfully.
 */
ChessResult playerAddMatch(Player player, Match match);

/**
 * Removes a match from player's record.
 * 
 * @param player Player in question
 * @param Match match to be removed
 * @return
 *    CHESS_NULL_ARGUMENT - NULL argument was provided
 *    CHESS_SUCCESS - match was removed successfully.
 */
ChessResult playerRemoveMatch(Player player, Match match);

/**
 * Calculates Player's score throughout the games in the system
 * 
 * @param player Player to be scored
 * @return
 *    != 0.0  - Player's score
 *    0.0 NULL argument was provided
 */
double playerGetScore(Player player);

/**
 * Compares two players based on their performance through all matches
 * participated.
 * Player's score is calculated in the following manner:
 * score = (6*wins - 10*losses + 2*draws) / number of matches participated.
 * 
 * If both players has the same score, the 
 * 
 * @param player1 first Player to compare
 * @param player2 second Player to compare
 * @return
 *    >0 if player1 > player2
 *    <0 if player1 < player2
 *    0 if both arguments are the same
 */
int playerCompare(Player player1, Player player2);

/**
 * Destroys a Player instance and frees all private memory.
 * 
 * @param player Player to be destroyed.
 * @param remove_from_tournaments set as loser in running turnaments
 */
void playerDestroy(Player player, bool remove_from_tournaments);

/**
 * Creates a copy of the provided Player for the Map object.
 * 
 * @param original Player to be copied
 * @return 
 *    New exact copy of the provided Player
 *    NULL if memory allocation failed
 */
MapDataElement playerCopy(MapDataElement original);

#endif // PLAYER_H