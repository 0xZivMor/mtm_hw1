#ifndef _TOURNAMENT_H
#define _TOURNAMENT_H

#include "matchnode.h"
#include "player.h"
#include "match.h"
#include "utils.h"

typedef struct tournament_t *Tournament;

/**
 * Create a new instance of Tournament
 * 
 * @param id tournament's id
 * @param location tournament's location
 * @param max_games_per_player maximum games allowed per player
 * @return 
 *    A new Tournament instance if all parameters are valid and all memory 
 *    allocation operation succeded.
 *    NULL otherwise.
 */
Tournament tournamentCreate(int id, const char *location, int max_games_per_player);

/**
 * Adds a new match to the tournament.
 * Matches can't be added after the tournament is ended.
 * 
 * If any of the players isn't found in the tournament's players map, it 
 * will be added
 * 
 * @param tournament tournament to add the match to
 * @param match Match to be added
 * @return  
 *     CHESS_NULL_ARGUMENT - provided argument was NULL
 *     CHESS_TOURNAMENT_ENDED - the tournament is over
 *     CHESS_GAME_ALREADY_EXIST - match with the same participants was already
 *                                added to the tournament
 *     CHESS_INVALID_MAX_GAMES - one of the particiapnts has already reached the
 *                               maximum games allowed
 *     CHESS_SUCCESS - match was added successfully.
 */
ChessResult tournamentAddMatch(Tournament tournament, Match match);

/**
 * Remove a player from a tournament. 
 * All matches the player participated in will be forfeited.
 * If player wasn't in the tournament - nothing happens and the method will
 * be considered successful.
 * 
 * @param tournament Tournament to remove the player from
 * @param int id of the player to be removed
 * @return 
 *     CHESS_NULL_ARGUMENT - provided argument was NULL
 *     CHESS_SUCCESS - player was removed successfully.
 */
ChessResult tournamentRemovePlayer(Tournament tournament, int player_id);

/**
 * Removes a match from a tournament's matches list.
 * If match wasn't in the tournament - nothing happens and the method will
 * be considered successful.
 * 
 * @param tournament Tournament to remove the player from
 * @param match Match to be removed
 * @return 
 *     CHESS_NULL_ARGUMENT - provided argument was NULL
 *     CHESS_SUCCESS - match was removed successfully.
 */
ChessResult tournamentRemoveMatch(Tournament tournament, Match match);

/**
 * Ends the current tournament and sets the winner.
 * The winner is the highest scoring player according to the following scoring:
 *  +2 points per win
 *  +1 point per draw
 *  +0 points per loss
 * Tie breaker: lowest player ID wins.
 * 
 * @param tournament Tournament to end
 * @return  
 *     CHESS_NULL_ARGUMENT - provided argument was NULL
 *     CHESS_SUCCESS - Tournament was ended successfully.
 */
ChessResult tournamentEnd(Tournament tournament);

/**
 * Compiles a list of Matches of which the provided player was a participant.
 * 
 * @param tournament Tournament in question
 * @param player_id Id of player in question
 * @param list OUT pointer to a linked list of matches.
 *             NULL if return value != CHESS_SUCCESS
 *             caller's responsibility to free the list.
 * @return
 *    CHESS_NULL_ARGUMENT - provided argument was NULL
 *    CHESS_PLAYER_NOT_EXIST - player didn't play in the tournament
 *    CHESS_SUCCESS - list was compiled successfully.
 */
ChessResult tournamentGetMatchesByPlayer(Tournament tournament, 
                                         int player_id, 
                                         matchNode *list);

/**
 * Destroys a Tournament instance
 * Frees all private memory.
 * 
 * @param tournament Tournament to destory
 */
void tournamentDestroy(Tournament tournament);

/**
 * Compares two tournaments based on their ids, in increasing order.
 * 
 * @param tournament1 first tournament to compare
 * @param tournament2 second tournament to compare
 * @return
 *    >0 if tournament1 > tournament2
 *    <0 if tournament1 < tournament2
 *    0 if both arguments has the same id
 */
int tournamentCompare(Tournament tournament1, Tournament tournament2);

/**
 * Allows for the user to check on the status of the tournament
 * 
 * @param tournament tournament in question
 * @return true Tournament has ended
 * @return false Tournament hasn't ended
 */
bool tournamentIsEnded(Tournament tournament);

/**
 * Creates a copy of the provided Tournament for the Map object.
 * 
 * @param original Tournament to be copied
 * @return 
 *    New exact copy of the provided Tournament
 *    NULL if memory allocation failed
 */
MapDataElement tournamentCopy(MapDataElement original);

#endif // _TOURNAMENT_H