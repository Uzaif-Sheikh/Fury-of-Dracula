////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
///////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"

#include "Queue.h"

#define START	0
#define VISITED	1

#define HIDEAVAILABLE(c) (c == 0)
#define DBAVAILABLE(c) (c == 0)

// Function Prototypes
PlaceId* DvGetValidMovesbyType(DraculaView dv,int *numReturnedMoves, bool road, bool rail, bool boat);
PlaceId *DvWhereCanDracGoByType(DraculaView dv,int *numLocs, bool road,bool rail,bool boat);
void DoubleBackCalc(int *num_reachable, char *visited, PlaceId *Reachable_places,
					int *Trail_moves, PlaceId *Draculas_trail,
					PlaceId *Valid_moves, int *ValidMovesCounter,
					int *DB_moves);

void MoveOrLocation(int *Trail_moves, PlaceId *Draculas_trail, 
				int *visited_places, int *hide_moves,
				int *DB_moves, char *visited);

void AdjVisitedOrNot(int *num_reachable, char *visited, PlaceId *Reachable_places,
					PlaceId *Valid_moves, int *ValidMovesCountCopy,
					int *teleport_check, PlaceId Last_loc);

void WhereCanHideOrDnGO(int *Hi_Db_moves, int *HI_DB_move, PlaceId *WhereCanIgo,
					int *num_hide_Db, PlaceId *Location_History,
					int *numLoc, int *WhereCanIgoCount);


struct draculaView {
	GameView gv;
};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

// Creates a new view to summarise the current state of the game.
DraculaView DvNew(char *pastPlays, Message messages[])
{
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}

	new->gv = GvNew(pastPlays, messages);
	
	return new;
}

// Frees all memory allocated for `dv`.
// After this has been called, `dv` should not be accessed.
void DvFree(DraculaView dv)
{
	GvFree(dv->gv);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

// Get the current round number
Round DvGetRound(DraculaView dv)
{   
	return GvGetRound(dv->gv);
}

// Gets the current game score; a positive integer between 0 and 366.
int DvGetScore(DraculaView dv)
{   
	return GvGetScore(dv->gv);
}

// Gets the current health points for the given player; an value between
// 0 and 9 for a hunter, or >= 0 for Dracula.
int DvGetHealth(DraculaView dv, Player player) 
{	
	int health = GvGetHealth(dv->gv, player);
	return health;
}

// Gets the current location of a given player.
PlaceId DvGetPlayerLocation(DraculaView dv, Player player) 
{
	return GvGetPlayerLocation(dv->gv, player);
}

// Gets the location of the sleeping immature vampire.
PlaceId DvGetVampireLocation(DraculaView dv)
{
	return GvGetVampireLocation(dv->gv);
}

// Gets the locations of all active traps.
PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{	
	int trap_count = *numTraps;
	PlaceId *traps = GvGetTrapLocations(dv->gv, &trap_count);
	*numTraps = trap_count;
	return traps;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

// Gets all the moves that Dracula can validly make this turn.
PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{	
	bool road = true;
	bool boat = true;
	bool rail = false;
	int numLocs = *numReturnedMoves;
	PlaceId* Dracula_Valid_moves = DvGetValidMovesbyType(dv, &numLocs, road, rail, boat);
	
	*numReturnedMoves = numLocs;
	return Dracula_Valid_moves;
}

// Gets all the locations that Dracula can move to this turn.
PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs) 
{	
	bool road = true;
	bool boat = true;
	bool rail = false;
	int numLocs = *numReturnedLocs;
	PlaceId* Possible_places = DvWhereCanDracGoByType(dv, &numLocs, road, rail, boat);
	
	*numReturnedLocs = numLocs;
	return Possible_places;
}

// Similar  to  DvWhereCanIGo, but the caller can restrict the transport
// types to be considered. For example, if road is  true,  but  boat  is
// false, boat connections will be ignored.
PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{	
	int numLocs = START;
	PlaceId *Dv_by_type = DvWhereCanDracGoByType(dv, &numLocs, road, false, boat);

	*numReturnedLocs = numLocs;
	return Dv_by_type;
}

// Gets  all  the  locations  that the given player can move to in their
// next move (for Dracula that is this turn).
PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{	
	assert(player != PLAYER_DRACULA);
	
	int numLocs = START;
	
	PlaceId from = GvGetPlayerLocation(dv->gv, player);

	if (from == NOWHERE) {
		*numReturnedLocs = 0;
		return NULL;
	}
	
	int round = GvGetRound(dv->gv);
	
	PlaceId *locs = GvGetReachable(dv->gv, player,round, from, &numLocs);
	
	*numReturnedLocs = numLocs;
	return locs;
}

// Similar to DvWhereCanTheyGo but the caller can restrict the transport
// types  to  be considered. For example, if road and rail are true, but
// boat is false, boat connections will be ignored.
PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	int numLocs = START;
	PlaceId from = GvGetPlayerLocation(dv->gv, player);

	if (from == NOWHERE) {
		*numReturnedLocs = 0;
		return NULL;
	}

	int round = GvGetRound(dv->gv);
	PlaceId *locs = GvGetReachableByType(dv->gv, player,round, from, road, rail, boat, &numLocs);
	*numReturnedLocs = numLocs;
	return locs;
}

// Similar to DvGetValidMoves, but modified to be a helper function for all 'ByType' functions
// as well. This is done by telling us which of road, rail and boat are true.
PlaceId* DvGetValidMovesbyType(DraculaView dv,int *numReturnedMoves, bool road, bool rail, bool boat) 
{
	// Find the current location of the player
	PlaceId Last_loc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
	// If this is the first turn, return NULL and set
	// numReturnedMoves to 0
	if (Last_loc == NOWHERE){
		*numReturnedMoves = START;
		return NULL;
	}
	
	int DB_moves = START;
	int hide_moves = START;
	int Trail_moves = *numReturnedMoves;
	bool canfree = false;
	int visited_places = START;
	
	char *visited = calloc(NUM_REAL_PLACES, sizeof(*visited));
	PlaceId *Draculas_trail = GvGetLastMoves(dv->gv, PLAYER_DRACULA, 
					TRAIL_SIZE-1, &Trail_moves, &canfree);
	
	int numMovesHist = -1;
	printf ("\n\n");
	printf ("Move_History\n");
	
	PlaceId *Move_History = GvGetMoveHistory(dv->gv, PLAYER_DRACULA, &numMovesHist, &canfree);
	printf ("%d\n", numMovesHist);
	for (int i = 0; i < numMovesHist; i++) {
		printf ("%d\n", i);
		printf ("%s\n", placeIdToName(Move_History[i]));
	}
	// printf ("Dracula's Trail\n\n");
	// for (int i = 0; i < Trail_moves; i++) {
	// 	printf ("%s\n", placeIdToName(Draculas_trail[i]));
	// }
	// printf ("\n\n");
	PlaceId *Valid_moves = calloc (NUM_REAL_PLACES+6, sizeof (*Valid_moves));

	
	MoveOrLocation(&Trail_moves, Draculas_trail, &visited_places, 
					&hide_moves, &DB_moves, visited);
	
	// for (int i = 0; i < NUM_REAL_PLACES; i++) {
	// 	printf ("%s\n", visited)
	// }
	int num_reachable = START;
	int teleport_check = START;
	int round = DvGetRound(dv);
	PlaceId *Reachable_places = GvGetReachableByType(dv->gv, PLAYER_DRACULA,
							round, Last_loc, road, rail, 
							boat, &num_reachable);	

	// for (int i = 0; i < num_reachable; i++) {
	// 	printf ("%s\n", placeIdToName(Reachable_places[i]));
	// }
	// If the only option is to TELEPORT, then return NULL and set
	// the numReturnedMoves to 0
	// if (num_reachable == 1 && Reachable_places[0] == Last_loc) {
	// 	*numReturnedMoves = START;
	// 	return NULL;
	// }		

	int ValidMovesCounter = START;
	
	DoubleBackCalc(&num_reachable, visited, Reachable_places,
					&Trail_moves, Draculas_trail,
					Valid_moves, &ValidMovesCounter, &DB_moves);
	
	// The num_reachable currently contains the current location,
	// we must adjust for this
	int num_real_reachable = num_reachable - 1;
	
	AdjVisitedOrNot(&num_reachable, visited, Reachable_places,
					Valid_moves, &ValidMovesCounter,
					&teleport_check, Last_loc);

	
	// The number of adjacent places to current location 
	// which are in trail is equal to the teleport check minus
	// as the teleport check will count current location as well
	int visited_adjacent = teleport_check - 1;
	
	// If a hide move is available, add current location
	// to valid moves array
	if (HIDEAVAILABLE(hide_moves) && (placeIdToType(Last_loc) != SEA)) {
		Valid_moves[ValidMovesCounter] = HIDE;
		ValidMovesCounter++;
	}
	
	// If HIDE move not available and DOUBLEBACK move not available
	// and all reachable places are in the trail, then TELEPORT and return
	// numReturnedMoves = 0 as well as return NULL array.
	if (hide_moves + DB_moves == 2 && visited_adjacent == num_real_reachable) {
	 	Valid_moves = NULL;
	 	ValidMovesCounter = START;
	}
	// printf ("Valid Moves inside:\n");
	// for (int i = 0; i < ValidMovesCounter; i++) {
	// 	printf ("%s\n", placeIdToName(Valid_moves[i]));
	// }
	free (Draculas_trail);
	*numReturnedMoves = ValidMovesCounter;
	return Valid_moves;

}

// Helper function for GetValidMovesByType to calculate
// The number of moves which are HIDE or DOUBLE_BACK
// as well as store and calculate which places have already been visited
// and are still in the trail
void MoveOrLocation(int *Trail_moves, PlaceId *Draculas_trail, 
				int *visited_places, int *hide_moves,
				int *DB_moves, char *visited) 
{
	for (int i = 0; i < *Trail_moves; i++) {
		if (NOT_HI_DB_MOVE(Draculas_trail[i]) && Draculas_trail[i] != TELEPORT ) {
			visited[Draculas_trail[i]] = VISITED;
			*visited_places = *visited_places + 1;
		}
		if (HIDE_MOVE(Draculas_trail[i])) {
			*hide_moves = *hide_moves + 1;
		}
		if (DB_MOVE(Draculas_trail[i])) {
			*DB_moves = *DB_moves + 1;
		}
	}
}

//  Helper function for GetValidMovesByType to calculate whether the DOUBLE_BACK
// is D1, D2, D3, D4 or D5. This will be stored in the ValidMoves array created
void DoubleBackCalc(int *num_reachable, char *visited, PlaceId *Reachable_places,
						int *Trail_moves, PlaceId *Draculas_trail,
						PlaceId *Valid_moves, int *ValidMovesCounter,
						int *DB_moves)
{
	// If DOUBLEBACK move available, loop through the reachable places
	// which are in the trail and find out which DOUBLEBACK moves, e.g. D3, 
	// are available - add the PlaceId of this to the valid moves array
	if (DBAVAILABLE(*DB_moves)) {
		for (int i = 0; i < *num_reachable; i++) {
			if (visited[Reachable_places[i]]) {
				for (int k = 0; k < *Trail_moves; k++) {
					if (Reachable_places[i] == Draculas_trail[k]) {
						// If the trail is less than 6
						if (k == 0 && *Trail_moves < 6) {
							int DB_value = *Trail_moves - k;

							// DOUBLEBACKS (PlaceIds 103, 104, 105, 106 and 107) come after HIDE
							// (102) in the PlaceId positions, to the number of backward moves
							// for a DOUBLEACK is equal to that DOUBLEBACK + HIDE
							// e.ge D3 = 3, and to move 3 places back we move
							// D3 + HI =  3 + 102 moves back
							int Double_back_what = DB_value + HIDE;
							Valid_moves[*ValidMovesCounter] = Double_back_what;
							*ValidMovesCounter = *ValidMovesCounter + 1;

						} 

						// If trail is 6 places long
						else if (k != 0) {
							int DB_value = *Trail_moves - k;
							int Double_back_what = DB_value + HIDE;
							Valid_moves[*ValidMovesCounter] = Double_back_what;
							*ValidMovesCounter = *ValidMovesCounter + 1;
						}
					}
				}
				
			}
				
		}

		// If the last move played was HIDE into a city in his trail,
		// Then DOUBLE_BACK_1 must be added to the valid moves array
		if (Draculas_trail[*Trail_moves-1] == HIDE) {
	 		Valid_moves[*ValidMovesCounter] = DOUBLE_BACK_1;
			*ValidMovesCounter = *ValidMovesCounter + 1;
		}
	}
}

// Check to see how many of the adjacent reachable places are already visited and in
// the trail, and teleport_check to count how many of the reachable places
// are in the trail. This will ultimately help us figure out whether or not we need to
// TELEPORT as our next move
void AdjVisitedOrNot(int *num_reachable, char *visited, PlaceId *Reachable_places,
					PlaceId *Valid_moves, int *ValidMovesCountCopy,
					int *teleport_check, PlaceId Last_loc)
{	
	for (int i = 0; i < *num_reachable; i++) {
		if (!visited[Reachable_places[i]] && Reachable_places[i] != Last_loc) {
			Valid_moves[*ValidMovesCountCopy] = Reachable_places[i];
			*ValidMovesCountCopy = *ValidMovesCountCopy + 1;
		}

		if (visited[Reachable_places[i]]) {
			*teleport_check = *teleport_check + 1;
		}

	}
}

// Helper function for WHereCanIgoByType to find where Dracula can go given the transport
// restrictions he faces
PlaceId *DvWhereCanDracGoByType(DraculaView dv,int *numLocs, bool road,bool rail,bool boat) {
	
	int numLocsMoveable = START;
	PlaceId *Possible_moves = DvGetValidMovesbyType(dv, &numLocsMoveable, road, rail, boat);

	// If Dracula hasn't played yet return NULL
	if (Possible_moves == NULL) {
		*numLocs = START;
		return NULL;
	}

	int numLoc = START;
	bool canfree = false;
	PlaceId *Location_History = GvGetLocationHistory(dv->gv, PLAYER_DRACULA, &numLoc, &canfree);

	int *Hi_Db_moves = calloc (6, sizeof(*Hi_Db_moves));
	
	// Count How many HIDE or DOUBLE_BACK moves are possible
	int HI_DB_move = START;
	for (int i = 0; i < numLocsMoveable; i++) {
		if (HIDE_MOVE(Possible_moves[i]) || DB_MOVE(Possible_moves[i])) {
			Hi_Db_moves[HI_DB_move] = Possible_moves[i];
			HI_DB_move++;
		}
	}
	
	int WhereCanIgoCount = START;
	PlaceId *WhereCanIgo = calloc(numLocsMoveable, sizeof(*WhereCanIgo));
	
	int num_hide_Db = START;
	
	// If possible move is a HIDE or DOUBLE_BACK move, find the location
	// that this moves should take Dracula too
	WhereCanHideOrDnGO(Hi_Db_moves, &HI_DB_move, WhereCanIgo,
					&num_hide_Db, Location_History,
					&numLoc, &WhereCanIgoCount);
	
	// If not a HIDE or DOUBLE_BACK move, then add the location 
	// to wherecaniigo array
	for (int i = 0; i < numLocsMoveable; i++) {
		if(NOT_HI_DB_MOVE(Possible_moves[i])) {
			WhereCanIgo[WhereCanIgoCount] = Possible_moves[i];
			WhereCanIgoCount++;
		} 
		
	}

	*numLocs = WhereCanIgoCount;
	return WhereCanIgo;

}

// If Dracula is to HIDE or DOUBLE_BACK, then find the location which he must go to
void WhereCanHideOrDnGO(int *Hi_Db_moves, int *HI_DB_move, PlaceId *WhereCanIgo,
						int *num_hide_Db, PlaceId *Location_History,
						int *numLoc, int *WhereCanIgoCount)
{
	while (*num_hide_Db < *HI_DB_move) {
		
		if (HIDE_MOVE(Hi_Db_moves[*num_hide_Db])) {
			WhereCanIgo[*WhereCanIgoCount] = Location_History[*numLoc-1];
			*WhereCanIgoCount = *WhereCanIgoCount + 1;
		}
		
		else if (DB_MOVE(Hi_Db_moves[*num_hide_Db])) {
			
			// The DOUBLE_BACK value is equal to the DOUBLEBACK move
			// (103,104,105,106 or 107) PlaceId minus the PlaceId of HIDE (102)
			int Db_value = Hi_Db_moves[*num_hide_Db] - HIDE;
			int compare_index = *numLoc - Db_value;
			
			// Compare index will be the the nth index from the end 
			// of the location history for a DOUBLEBACK move Dn. 
			if (compare_index != 0) {
				if (Location_History[compare_index] != Location_History[compare_index-1]) {
					WhereCanIgo[*WhereCanIgoCount] = Location_History[*numLoc-Db_value];
					*WhereCanIgoCount = *WhereCanIgoCount + 1;
				}
			}
			
		}

		*num_hide_Db = *num_hide_Db + 1;
	}
}

PlaceId* DvReachablePlaces(DraculaView dv, Player hunter,int round,PlaceId p,int *numReturnedLocs) {
	
	if(DvGetPlayerLocation(dv,hunter) == NOWHERE) return NULL;

	int turn = DvGetRound(dv) + round;
	
	int num_max = START;
	
	PlaceId* reachable = GvGetReachable(dv->gv, hunter, turn, p, &num_max);

	*numReturnedLocs = num_max;
	return reachable;
}