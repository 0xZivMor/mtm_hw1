#ifndef _TOURNAMENT_H
#define _TOURNAMENT_H

#include "chessSystem.h"
#include "matchnode.h"
#include "match.h"
#include "utils.h"

typedef struct tournament_t *Tournament;

/**
 * Create a new instance of Tournament
 * 
 * @param id tournament's id
 * @param location tournament's location
 * @param max_games_per_player maximum games allowed per player
 * @return A new Tournament instance, if all parameters are valid and all 
 *         memory allocation operation succeded. NULL otherwise.
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
 * @return CHESS_NULL_ARGUMENT - provided argument was NULL;
 *         CHESS_TOURNAMENT_ENDED - the tournament is over;
 *         CHESS_GAME_ALREADY_EXIST - match with the same participants was 
 *                                    already added to the tournament;
 *         CHESS_PLAYER_NOT_EXIST - one of the players in the match wasn't
 *                                  added to the tournament ahead of the
 *                                  match; 
 *         CHESS_INVALID_MAX_GAMES - one of the particiapnts has already 
 *                                   reached the maximum games allowed;
 *         CHESS_SUCCESS - match was added successfully.
 */
ChessResult tournamentAddMatch(Tournament tournament, Match match);

/**
 * returns the winner of the given tournament
 * 
 * @param tournament tournament in question
 * @return the id of the winner, 0 if no winner yet
 */ 
int tournamentGetWinner(Tournament tournament);

/**
 * returns the location of the given tournament
 * 
 * @param tournament tournament in question
 * @return the location of the tournament or
 *   NULL if given null argument
 */ 
char* tournamentGetLocation(Tournament tournament);

/**
 * Remove a player from a tournament. 
 * All matches the player participated in will be forfeited.
 * If player wasn't in the tournament, the function will be considered successful.
 * If the tournament has ended or any of the parameters wasn't valid,
 * nothing happens and the player won't be removed
 * 
 * @param tournament Tournament to remove the player from
 * @param int id of the player to be removed
 * @return true player was remove 
 */
bool tournamentRemovePlayer(Tournament tournament, int player_id);

/**
 * Removes a match from a tournament's matches list.
 * If match wasn't in the tournament - nothing happens and the method will
 * be considered successful.
 * 
 * @param tournament Tournament to remove the match from
 * @param match Match to be removed
 * @return CHESS_NULL_ARGUMENT - provided argument was NULL; 
 *         CHESS_SUCCESS - match was removed successfully.
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
 * @return CHESS_NULL_ARGUMENT - provided argument was NULL;
 *         CHESS_SUCCESS - Tournament was ended successfully.
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
 * @return CHESS_NULL_ARGUMENT - provided argument was NULL; 
 *         CHESS_OUT_OF_MEMORY - memory error occured; 
 *         CHESS_PLAYER_NOT_EXIST - player didn't play in the tournament; 
 *         CHESS_SUCCESS - list was compiled successfully.
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
 * @param tournament1 ID of first tournament to compare
 * @param tournament2 ID of second tournament to compare
 * @return >0 if tournament1 > tournament2; 
 *         <0 if tournament1 < tournament2;   
 *          0 if both arguments has the same id
 */
int tournamentCompare(MapKeyElement tournament1, MapKeyElement tournament2);

/**
 * Allows for the user to check on the status of the tournament
 * 
 * @param tournament tournament in question
 * @return true Tournament has ended
 * @return false Tournament hasn't ended
 */
bool tournamentIsEnded(Tournament tournament);

/**
 * Checks if a player is a participant in the tournament
 * 
 * @param tournament Tournament to query
 * @param player_id ID to check
 * @return true player is a participant
 * @return false player is not a participant
 */
bool tournamentIsParticipant(Tournament tournament, int player_id);

/**
 * Creates a copy of the provided Tournament for the Map object.
 * 
 * @param original_tournament Tournament to be copied
 * @return New exact copy of the provided Tournament;
 *         NULL if memory allocation failed
 */
MapDataElement tournamentCopy(MapDataElement original_tournament);

/**
 * finds the longest match in the tournament.
 * 
 * @param tournament tournament in question
 * @return the duration of the longest match
 */ 
int tournamentLongestPlayTime(Tournament tournament);

/**
 * counts how many matches were in the tournament.
 * 
 * @param tournament tournament in question
 * @return number of matches in the tournament
 */
int tournamentNumberOfMatches(Tournament tournament);

/**
 * counts how many different players took part in the tournament.
 * 
 * @param tournament tournament in question
 * @return
 *      number of players in the tournament
 */
int tournamentNumberOfPlayers(Tournament tournament);

/**
 * calculates the average game time in the tournament.
 * 
 * @param tournament tournament in question
 * @return (sum of all durations) / (num of games),
 *          0 if no games played
 */
double tournamentAveragePlayTime(Tournament tournament);

/**
 * Wrapper for tournamentDestroy for GDT maps
 * 
 * @param tournament Tournament to be destroyed.
 */
void tournamentDestroyMap(MapDataElement tournament);

#endif // _TOURNAMENT_H