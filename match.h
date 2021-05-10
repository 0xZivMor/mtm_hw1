#ifndef _MATCH_H
#define _MATCH_H

#include <stdbool.h>
#include "chessSystem.h"
#include "tournament.h"
#include "player.h"

typedef struct match_t *Match;

/**
 * Creates new Match instance
 * 
 * @param first_player first participant
 * @param second_player second participant
 * @param winner The participant who won the match. Either first_player,
 *               second_player or NULL if match ended in a draw.
 * @param tournamet The tournament of which the match is a part of
 * @param duration duration of match, in seconds.
 * @return
 *    A new Match instance if all parameters are valid and all memory 
 *    allocation operation succeded.
 *    NULL otherwise.
 */
Match matchCreate(Player first_player, 
                  Player second_player, 
                  Player winner,
                  Tournament tournament, 
                  int duration);

/**
 * Sets the match winner.
 * 
 * @param match Match in question
 * @param winner winner of the match. May be NULL for draw, but must be
 *               one of the participants otherwise.
 * @return
 *     CHESS_NULL_ARGUMENT - provided match was NULL
 *     CHESS_PLAYER_NOT_EXIST - provided winner is not one of the participants.
 *     CHESS_SUCCESS - winner was set successfully.
 */
ChessResult matchSetWinner(Match match, Player winner);

/**
 * Checks if the provided player is a participant in the match
 * 
 * @param match Match in question
 * @param player Player to check
 * @return true - player is a participant in the match
 * @return false - player is not a participant in the match, or NULL parameters
 *         were provided
 */
bool matchIsParticipant(Match match, Player player);

/**
 * @brief Get the winner of the match.
 * winner will be NULL if result != CHESS_SUCCESS or match ended in a draw
 * 
 * @param match Match in question
 * @param winner OUT for the found parameter
 * @return  
 *        CHESS_NULL_ARGUMENT - NULL parameter was provided
 *        CHESS_SUCCESS - winner was returned successfully.
 */
ChessResult matchGetWinner(Match match, Player *winner);

/**
 * Gets the duration of the match, in seconds.
 * 
 * @param match Match in question
 * @return
 *         >= 0 match duration
 *         -1 or error
 */
int matchGetDuration(Match match);

/**
 * Destorys match instance and frees all private memory
 * 
 * @param Match Match to be destroyed
 */
void matchDestroy(Match match);

/**
 * Gets the tournament associated with the match
 * 
 * @param match Match is question
 * @return Tournament associated with the match
 */
Tournament matchGetTournament(Match match);

/**
 * Compares Matches based on their tournament and participants.
 * Matches are regarded as "same" if they have the same tournament and
 * participants. If they defer is one of those properties, "greater match"
 * is the first argument is considered "greater"
 * 
 * @param match1 first Match to compare
 * @param match2 second Match to compare
 * @return 
 *    >0 if match1 >= match2
 *    <0 if match1 < match2
 *    0 both arguments are the same match (same instance)
 */
int matchCompare(Match match1, Match match2);

/**
 * Creates a copy of the provided Match for the Map object.
 * 
 * @param original Match to be copied
 * @return 
 *    New exact copy of the provided Match
 *    NULL if memory allocation failed
 */
MapDataElement matchCopy(MapDataElement original);

#endif // _MATCH_H