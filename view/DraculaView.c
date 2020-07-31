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
// add your own #includes here
#include "Queue.h"

#define START	0

// Function Prototypes
//PlaceId* GetDraculasTrail (DraculaView dv, Player player, int *num_moves);
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

struct draculaView {
	GameView gv;
	
};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

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

void DvFree(DraculaView dv)
{
	
	GvFree(dv->gv);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{   
	return GvGetRound(dv->gv);
}

int DvGetScore(DraculaView dv)
{   
	return GvGetScore(dv->gv);
}

int DvGetHealth(DraculaView dv, Player player) {	
	
	
	int health = GvGetHealth(dv->gv, player);
	return health;

}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player) {
	
	return GvGetPlayerLocation(dv->gv, player);

}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	return GvGetVampireLocation(dv->gv);
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{	
	int trap_count = *numTraps;
	PlaceId *traps = GvGetTrapLocations(dv->gv, &trap_count);
	*numTraps = trap_count;
	return traps;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

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

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs) {
	
	bool road = true;
	bool boat = true;
	bool rail = false;
	int numLocs = *numReturnedLocs;
	PlaceId* Possible_places = DvWhereCanDracGoByType(dv, &numLocs, road, rail, boat);
	
	*numReturnedLocs = numLocs;
	return Possible_places;
	
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{	
	int numLocs = -1;
	PlaceId *Dv_by_type = DvWhereCanDracGoByType(dv, &numLocs, road, false, boat);
	*numReturnedLocs = numLocs;
	return Dv_by_type;
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{	
	assert(player != PLAYER_DRACULA);
	
	int numLocs = -1;
	
	PlaceId from = GvGetPlayerLocation(dv->gv, player);
	int round = GvGetRound(dv->gv);
	
	PlaceId *locs = GvGetReachable(dv->gv, player,round, from, &numLocs);
	
	*numReturnedLocs = numLocs;
	return locs;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	int numLocs = -1;
	PlaceId from = GvGetPlayerLocation(dv->gv, player);
	int round = GvGetRound(dv->gv);
	PlaceId *locs = GvGetReachableByType(dv->gv, player,round, from, road, rail, boat, &numLocs);
	*numReturnedLocs = numLocs;
	return locs;
}

PlaceId* DvGetValidMovesbyType(DraculaView dv,int *numReturnedMoves, bool road, bool rail, bool boat) {
	
	PlaceId Last_loc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
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
									TRAIL_SIZE, &Trail_moves, &canfree);

	PlaceId *Valid_moves = calloc (NUM_REAL_PLACES+6, sizeof (*Valid_moves));

	
	MoveOrLocation(&Trail_moves, Draculas_trail, &visited_places, 
								&hide_moves, &DB_moves, visited);
	
	
	int num_reachable = -1;
	int teleport_check = 0;
	int round = DvGetRound(dv);
	PlaceId *Reachable_places = GvGetReachableByType(dv->gv, PLAYER_DRACULA,
											round, Last_loc, road, rail, 
											boat, &num_reachable);	
	

	if (num_reachable == 1 && Reachable_places[0] == Last_loc) {
		*numReturnedMoves = 0;
		return NULL;
	}		

	int ValidMovesCounter = 0;
	
	DoubleBackCalc(&num_reachable, visited, Reachable_places,
							&Trail_moves, Draculas_trail,
							Valid_moves, &ValidMovesCounter, &DB_moves);
	
	int num_real_reachable = num_reachable - 1;
	
	int ValidMovesCountCopy = ValidMovesCounter;
	
	AdjVisitedOrNot(&num_reachable, visited, Reachable_places,
								Valid_moves, &ValidMovesCountCopy,
								&teleport_check, Last_loc);

	
	
	int visited_adjacent = teleport_check - 1;
	
	if (hide_moves == 0) {
		Valid_moves[ValidMovesCountCopy] = HIDE;
		ValidMovesCountCopy++;
	}
	
	if (hide_moves + DB_moves == 2 && visited_adjacent == num_real_reachable) {
	 	Valid_moves = NULL;
	 	ValidMovesCountCopy = 0;
	}
	
	free (Draculas_trail);
	*numReturnedMoves = ValidMovesCountCopy;
	return Valid_moves;

}


void MoveOrLocation(int *Trail_moves, PlaceId *Draculas_trail, 
								int *visited_places, int *hide_moves,
								int *DB_moves, char *visited) 
{
	for (int i = 0; i < *Trail_moves; i++) {
		if (NOT_HI_DB_MOVE(Draculas_trail[i]) && Draculas_trail[i] != TELEPORT ) {
			visited[Draculas_trail[i]] = 1;
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

void DoubleBackCalc(int *num_reachable, char *visited, PlaceId *Reachable_places,
										int *Trail_moves, PlaceId *Draculas_trail,
										PlaceId *Valid_moves, int *ValidMovesCounter,
										int *DB_moves)
{
	if (*DB_moves == 0) {
		for (int i = 0; i < *num_reachable; i++) {
			if (visited[Reachable_places[i]]) {
				for (int k = 0; k < *Trail_moves; k++) {
						if (Reachable_places[i] == Draculas_trail[k]) {
							if (k == 0 && *Trail_moves < 6) {
								int DB_value = *Trail_moves - k;
								int Double_back_what = DB_value + HIDE;
								Valid_moves[*ValidMovesCounter] = Double_back_what;
								*ValidMovesCounter = *ValidMovesCounter + 1;

							} 

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

		if (Draculas_trail[*Trail_moves-1] == HIDE) {
	 		Valid_moves[*ValidMovesCounter] = DOUBLE_BACK_1;
			*ValidMovesCounter = *ValidMovesCounter + 1;
		}
	}
}

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

PlaceId *DvWhereCanDracGoByType(DraculaView dv,int *numLocs, bool road,bool rail,bool boat) {
	
	int numLocsMoveable = -1;
	PlaceId *Possible_moves = DvGetValidMovesbyType(dv, &numLocsMoveable, road, rail, boat);

	if (Possible_moves == NULL) {
		*numLocs = 0;
		return NULL;
	}

	int numLoc = -1;
	bool canfree = false;
	//int Trail_moves = -1;
	PlaceId *Location_History = GvGetLocationHistory(dv->gv, PLAYER_DRACULA, &numLoc, &canfree);
	//PlaceId *Draculas_trail = GvGetLastMoves(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &Trail_moves, &canfree);

	int *Hi_Db_moves = calloc (6, sizeof(*Hi_Db_moves));
	
	int HI_DB_move = 0;
	for (int i = 0; i < numLocsMoveable; i++) {
		if (HIDE_MOVE(Possible_moves[i]) || DB_MOVE(Possible_moves[i])) {
			Hi_Db_moves[HI_DB_move] = Possible_moves[i];
			HI_DB_move++;
		}
	}
	
	int j = 0;
	PlaceId *WhereCanIgo = calloc(numLocsMoveable, sizeof(*WhereCanIgo));
	
	int num_hide_Db = 0;
	
	while (num_hide_Db < HI_DB_move) {
		
		if (HIDE_MOVE(Hi_Db_moves[num_hide_Db])) {
			WhereCanIgo[j] = Location_History[numLoc-1];
			j++;
		}
		
		else if (DB_MOVE(Hi_Db_moves[num_hide_Db])) {
			int Db_value = Hi_Db_moves[num_hide_Db] - HIDE;
			int compare_index = numLoc - Db_value;
			if (compare_index != 0) {
				if (Location_History[compare_index] != Location_History[compare_index-1]) {
					WhereCanIgo[j] = Location_History[numLoc-Db_value];
					j++;
				}
			}
			
		}
		num_hide_Db++;
	}
	
	int k = j;
	
	for (int i = 0; i < numLocsMoveable; i++) {
		if(NOT_HI_DB_MOVE(Possible_moves[i])) {
			WhereCanIgo[k] = Possible_moves[i];
			k++;
		} 
		
	}

	// free(GetReachable);
	*numLocs = k;
	return WhereCanIgo;

}






// PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
// {
// 	// If we are on the first move return an empty array (NULL is satisfactory)
// 	int round = DvGetRound(dv);
// 	if (round == 0) {
// 		return NULL;
// 	}
	
// 	int DB_moves = 0;
// 	int hide_moves = 0;
// 	int Trail_moves = *numReturnedMoves;
	
// 	// Get the current location of Dracula
// 	PlaceId Last_loc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
// 	char *visited = calloc(NUM_REAL_PLACES, sizeof(*visited));
// 	// Get Draculas Trail
// 	PlaceId *Draculas_trail = GvGetLastMoves(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &Trail_moves, &canfree);
// 	// Create array to store valid moves/locations
// 	PlaceId* Valid_moves = calloc (Trail_moves, sizeof (*Valid_moves));
// 	// Get all the adjacent cities to current city
// 	int num_reachable = -1;
// 	PlaceId* Reachable_places = GvGetReachable(dv->gv, PLAYER_DRACULA, round, Last_loc, &num_reachable);	
	
// 	// Calculate (1) visited places (2) whether HI or DB is available
// 	for (int i = 0; i < Trail_moves; i++) {
// 		if (NOT_HI_DB_MOVE(Draculas_trail[i])) {
// 			visited[Draculas_trail[i]] = 1;
// 		}
// 		if (HIDE_MOVE(Draculas_trail[i])) {
// 			hide_moves++;
// 		}
// 		if (DB_MOVE(Draculas_trail[i])) {
// 			DB_moves++;
// 		}
// 	}

// 	int j = 0;
// 	// Copy cities which are (not in trail && adjacent) into Valid_moves array
// 	for(int i=0; i < num_reachable; i++) {
// 		if (visited[Reachable_places[i]] != 1 && Reachable_places[i != Last_loc]) {
// 			Valid_moves[j] = Reachable_places[i];
// 			j++;
// 		}
// 	}

// 	// DB
// 	// Count how many of the reacheable places are in the trail
// 	int reach_trail = 0;
// 	if (DB_moves == 0) {
// 		// Find which cities are both in trail and adjacent then copy the relating Dn to valid_moves
// 		for(int i=0; i < num_reachable; i++) {
// 			if (visited[Reachable_places[i]] == 1) {
				
// 			}
// 		}
// 	}

// 	// HI
// 	if (hide_moves == 0) {
// 		Valid_moves[j] = Last_loc;
// 		j++;
// 	}

// 	if (hide_moves ==1 && DB_moves == 1 && reach_trail == 6) {
// 		return NULL;
// 	}
	
// 	*numReturnedMoves = 0;
// 	return NULL;
// }
