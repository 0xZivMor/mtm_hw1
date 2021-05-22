#ifndef _MATCH_H
#define _MATCH_H

#include <stdbool.h>
#include "chessSystem.h"
#include "utils.h"

typedef struct match_t *Match;

/**
 * Creates new Match instance
 * 
 * @param first_player first participant's ID
 * @param second_player second participant's ID
 * @param winner ID of participant who won the match. Either first_player,
 *               second_player or 0 if match ended in a draw.
 * @param tournament ID of tournament of which the match is a part of
 * @param duration duration of match, in seconds.
 * @return A new Match instance if all parameters are valid and all memory 
 *         allocation operation succeded. NULL otherwise.
 */
Match matchCreate(ChessId first_player, 
                  ChessId second_player, 
                  ChessId winner,
                  ChessId tournament, 
                  int duration);


/**
 * retrieves the ID of first player of the match
 * @param match Match in question
 * @return first player's ID or 0 if recieves NULL arguments
 */
ChessId matchGetFirst(Match match);

/**
 * retrieves the ID of second player of the match
 * @param match Match in question
 * @return second player's ID or 0 if recieves NULL arguments
 */
ChessId matchGetSecond(Match match);

/**
 * Sets the match winner.
 * 
 * @param match Match in question
 * @param winner ID winner of the match. May be 0 for draw, but must be
 *               one of the participants otherwise.
 * @return CHESS_NULL_ARGUMENT - provided match was NULL; 
 *         CHESS_PLAYER_NOT_EXIST - provided winner is not one of the 
 *                                  participants; 
 *         CHESS_SUCCESS - winner was set successfully.
 */
ChessResult matchSetWinner(Match match, ChessId winner);

/**
 * Sets the match winner, thus the player that is not the loser.
 * 
 * @param match Match in question
 * @param loser ID loser of the match. May be NULL for draw, but must be
 *               one of the participants otherwise.
 * @return
 *     CHESS_NULL_ARGUMENT - provided match was NULL
 *     CHESS_PLAYER_NOT_EXIST - provided loser is not one of the participants.
 *     CHESS_SUCCESS - winner was set successfully.
 */
ChessResult matchSetLoser(Match match, ChessId loser);
ChessResult matchSetLoser(Match match, chessId loser);

/**
 * Checks if the provided player is a participant in the match
 * 
 * @param match Match in question
 * @param player ID to check
 * @return true - player is a participant in the match
 * @return false - player is not a participant in the match, or NULL parameters
 *         were provided
 */
bool matchIsParticipant(Match match, ChessId player);

/**
 * @brief Get the ID of the winner of the match.
 * winner will be 0 if result != CHESS_SUCCESS or match ended in a draw
 * 
 * @param match Match in question
 * @param winner OUT for the found parameter
 * @return  
 *        CHESS_NULL_ARGUMENT - NULL parameter was provided
 *        CHESS_SUCCESS - winner was returned successfully.
 */
ChessResult matchGetWinner(Match match, ChessId *winner);

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
 * Gets the ID of the tournament associated with the match
 * 
 * @param match Match is question
 * @return Tournament ID associated with the match, 0 if NULL parameter was
 *         passed
 */
ChessId matchGetTournament(Match match);

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
Match matchCopy(Match original);

#endif // _MATCH_H