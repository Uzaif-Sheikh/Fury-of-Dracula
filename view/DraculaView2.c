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

// Function Prototypes
//PlaceId* GetDraculasTrail (DraculaView dv, Player player, int *num_moves);
PlaceId* DvGetValidMovesbyType(DraculaView dv,int *numReturnedMoves, bool road, bool rail, bool boat);
PlaceId *DvWhereCanDracGoByType(DraculaView dv,int *numLocs, bool road,bool rail,bool boat);

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
	printf ("%d\n", health);
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
	
	int round = DvGetRound(dv);
	
	//need to update this to if there is no turn of the player yet...
	if (round == 0){
		*numReturnedMoves = 0;
		return NULL;
	}
	
	int DB_moves = 0;
	int hide_moves = 0;
	int Trail_moves = *numReturnedMoves;
	bool canfree = false;
	int visited_places = 0;
	
	char *visited = calloc(NUM_REAL_PLACES, sizeof(*visited));
	int *Double_back = calloc(10, sizeof(*Double_back));
	
	PlaceId *Draculas_trail = GvGetLastMoves(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &Trail_moves, &canfree);
	PlaceId *Valid_moves = calloc (Trail_moves, sizeof (*Valid_moves));
	
	PlaceId Last_loc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
	for (int i = 0; i < Trail_moves; i++) {
		if (NOT_HI_DB_MOVE(Draculas_trail[i]) && Draculas_trail[i] != TELEPORT) {
			visited[Draculas_trail[i]] = 1;
			visited_places++;
		}
		if (HIDE_MOVE(Draculas_trail[i])) {
			hide_moves++;
		}
		if (DB_MOVE(Draculas_trail[i])) {
			DB_moves++;
		}
	}
	
	int num_reachable = -1;
	int teleport_check = 0;
	PlaceId double_back = DOUBLE_BACK_1;
	
	PlaceId* Reachable_places = GvGetReachableByType(dv->gv, PLAYER_DRACULA, round, Last_loc, road, rail, boat, &num_reachable);	
	
	if (num_reachable == 1 && Reachable_places[0] == Last_loc) {
		*numReturnedMoves = 0;
		return NULL;
	}							
	
	for (int i = 0; i < num_reachable; i++) {

		if (visited[Reachable_places[i]]) {
			Double_back[teleport_check] = double_back;
			teleport_check++;
			double_back++;
		}
		
	} 
	
	int num_real_reachable = num_reachable - 1;
	int visited_adjacent = teleport_check - 1;
	
	int j = 0;
	
	for (int i = 0; i < num_reachable; i++) {
		if (!visited[Reachable_places[i]] && Reachable_places[i] != Last_loc) {
			Valid_moves[j] = Reachable_places[i];
			j++;
		}
		
	}
	

	int k = 0;
    
	if (hide_moves == 0) {
		Valid_moves[j] = HIDE;
		j++;
	}
	
	if (DB_moves == 0) {

		while (Double_back[k] != 0) {
			Valid_moves[j] = Double_back[k];
			j++;
			k++;
		}

	}

	if (hide_moves + DB_moves == 2 && visited_adjacent == num_real_reachable) {

	 	Valid_moves = NULL;
	 	j = 0;
	}
	
	free (Draculas_trail);
	*numReturnedMoves = j;
	return Valid_moves;

}
PlaceId *DvWhereCanDracGoByType(DraculaView dv,int *numLocs, bool road,bool rail,bool boat) {
	
	int numLocsMoveable = -1;
	PlaceId *Possible_moves = DvGetValidMovesbyType(dv, &numLocsMoveable, road, rail, boat);
	for (int i = 0; i < numLocsMoveable; i++) {
		printf ("%s\n", placeIdToName(Possible_moves[i]));
	}

	if (Possible_moves == NULL) {
		*numLocs = 0;
		return NULL;
	}

	int numLoc = -1;
	bool canfree = false;
	//int Trail_moves = -1;
	PlaceId *Move_History = GvGetMoveHistory(dv->gv, PLAYER_DRACULA, &numLoc, &canfree);
	//PlaceId *Draculas_trail = GvGetLastMoves(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &Trail_moves, &canfree);
	
	int *Hi_Db_moves = calloc (6, sizeof(*Hi_Db_moves));
	int HI_DB_move = 0;
	for (int i = 0; i < numLocsMoveable; i++) {
		if (HIDE_MOVE(Possible_moves[i]) || DB_MOVE(Possible_moves[i])) {
			Hi_Db_moves[HI_DB_move] = Possible_moves[i];
			HI_DB_move++;
		}
	}
    
	
	PlaceId *WhereCanIgo = calloc(numLocsMoveable, sizeof(*WhereCanIgo));

	int j = 0;
	
	PlaceId Loc = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA);
	
	int i = numLoc-1;
	while(Move_History[i] != Loc) {
		i--;
	}

	// PlaceId Loc = Move_History[i];

	while (HI_DB_move > 0)  {
		
		while (HIDE_MOVE(Loc) || DB_MOVE(Loc)) {
			if (DB_MOVE(Loc)) {
				int DB_value = Loc - HIDE;
				i = i - DB_value;
			} 
			else if (HIDE_MOVE(Loc)) {
				i = i - 1;
			}
			Loc = Move_History[i];

		}

		WhereCanIgo[j] = Loc;
		j++;
		Loc = Hi_Db_moves[j];	
		HI_DB_move--;
	}
	
	int k = j;
	
	for (int i = 0; i < numLocsMoveable; i++) {
		if(NOT_HI_DB_MOVE(Possible_moves[i])) {
			WhereCanIgo[k] = Possible_moves[i];
			k++;
		} 
		
	}
    
	for (int i = 0; i < k; i++){
		printf ("%s", placeIdToName(WhereCanIgo[i]));
	}
	*numLocs = k;
	return WhereCanIgo;

}


