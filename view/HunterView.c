////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
#include "Map.h"
#include "Places.h"

#define NOT_HI_DB_MOVE(id) (id != HIDE && id != DOUBLE_BACK_1 \
				&& id != DOUBLE_BACK_2 && id != DOUBLE_BACK_3 \
				&& id != DOUBLE_BACK_4 && id != DOUBLE_BACK_5)
#define MAX_HUNTERS 4
#define INF_NUM 99

typedef int Trap_Encounter;
typedef int Player_Encounter;
typedef int Vampire_Encounter;
typedef int Rest;
typedef int Score;
typedef int Death;
typedef int Health;
typedef struct hunter* Hunter;

#include "Queue.h"

static PlaceId* Reachable(HunterView hv,Player hunter,int round,PlaceId p,int *numReturnedLocs);

struct hunter{
	PlaceId Loction;
	Health health;

};


struct hunterView {
	GameView gv;
	Hunter hunters[MAX_HUNTERS];
	Round curr_round;
	Health Dracula_health;
	Score curr_score;
	Player curr_player_turn;
	PlaceId Dracula_loc;
	PlaceId last_know_loc_dracula;
	PlaceId vampire_loc;
	char* Hunter_string;

};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

static Hunter new_player(GameView gv,Player p){

	Hunter new_hunter = malloc(sizeof(*new_hunter));

	if(new_hunter == NULL){
		fprintf(stderr, "Couldn't allocate Player!\n");
		exit(EXIT_FAILURE);
	}

	new_hunter->Loction = GvGetPlayerLocation(gv,p);
	new_hunter->health = GvGetHealth(gv,p);

	return new_hunter;
}

// Creates a new view to summarise the current state of the game.
HunterView HvNew(char *pastPlays, Message messages[])
{
	HunterView hunter_state = malloc(sizeof(*hunter_state));
	if (hunter_state == NULL) {
		fprintf(stderr, "Couldn't allocate HunterView!\n");
		exit(EXIT_FAILURE);
	}

	hunter_state->Hunter_string = strdup(pastPlays);
	hunter_state->gv = GvNew(pastPlays,messages);

	for(int i = 0;i < MAX_HUNTERS;i++){
		hunter_state->hunters[i] = new_player(hunter_state->gv,i);
	}

	hunter_state->Dracula_health = GvGetHealth(hunter_state->gv,PLAYER_DRACULA);
	hunter_state->curr_score = GvGetScore(hunter_state->gv);
	hunter_state->curr_round = GvGetRound(hunter_state->gv);
	hunter_state->Dracula_loc = GvGetPlayerLocation(hunter_state->gv,PLAYER_DRACULA);
	hunter_state->last_know_loc_dracula = NOWHERE;
	hunter_state->vampire_loc = GvGetVampireLocation(hunter_state->gv);
	hunter_state->curr_player_turn = GvGetPlayer(hunter_state->gv);

	return hunter_state;
}

// Frees all memory allocated for `hv`.
// After this has been called, `hv` should not be accessed.
void HvFree(HunterView hv)
{
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

// Get the current round number
Round HvGetRound(HunterView hv)
{
	return hv->curr_round;
}

// Get the current player; i.e., whose turn is it?
Player HvGetPlayer(HunterView hv)
{
	return hv->curr_player_turn;
}

// Gets the current game score; a positive integer between 0 and 366.
int HvGetScore(HunterView hv)
{
	return hv->curr_score;
}

// Gets the current health points for the given player; an value between
// 0 and 9 for a hunter, or >= 0 for Dracula.
int HvGetHealth(HunterView hv, Player player)
{
	if(player == PLAYER_DRACULA){
		return hv->Dracula_health;
	}

	return hv->hunters[player]->health;
}

// Gets the current location of a given player.
PlaceId HvGetPlayerLocation(HunterView hv, Player player)
{
	if(player == PLAYER_DRACULA){
		return hv->Dracula_loc;
	}
	return hv->hunters[player]->Loction;
}

// Gets the location of the sleeping immature vampire.
PlaceId HvGetVampireLocation(HunterView hv)
{
	return hv->vampire_loc;
}

////////////////////////////////////////////////////////////////////////
// Utility Functions

// Gets  Dracula's  last  known  real  location  as revealed in the play
// string and sets *round to the number of the  latest  round  in  which
// Dracula moved there.
PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round)
{
	Message messages[4] = {};
	GameView gv = GvNew(hv->Hunter_string,messages);
	int round_known = 0;
	int num_move = 0;
	int count = -1;
	bool can_free = true;

	PlaceId* move_his = GvGetLocationHistory(gv,PLAYER_DRACULA,&num_move,&can_free);

	PlaceId known_loc = NOWHERE;
	
	for(int i = 0;i < num_move;i++){
		count++;
		if(move_his[i] != CITY_UNKNOWN && move_his[i] != SEA_UNKNOWN && NOT_HI_DB_MOVE(move_his[i])){
			known_loc = move_his[i];
			round_known = count;
		}
	}
	
	free(move_his);
	GvFree(gv);

	*round = round_known;
	return known_loc;
}

// Gets  the  shortest  path from the given hunter's current location to
// the given location, taking into account all connection types and  the
// fact that hunters can only move once per round.
PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength)
{
	Map m = MapNew();
	
	int round = 0;

	int num_places = MapNumPlaces(m);
	PlaceId* parent = malloc(num_places*sizeof(PlaceId));

	for(PlaceId i = 0;i < num_places;i++){
		parent[i] = NOWHERE;
	}

	PlaceId from = hv->hunters[hunter]->Loction;

	Queue q = newQueue();
	QueueJoin(q,from);
	parent[from] = from;
	int flag = 0;
	while(!QueueIsEmpty(q) && flag != 1){
		PlaceId p ;
		int size_q = QueueSize(q);
		while(size_q > 0 && flag != 1) {
			p = QueueLeave(q);
			size_q--;
			int numlocs = 0;
			PlaceId *reach = Reachable(hv, hunter, round, p, &numlocs);
			for (int j = 0; j < numlocs ; j++) {
				if (parent[reach[j]] == NOWHERE && reach[j] != p) {
					if (reach[j] == dest) {
						flag++;
						parent[dest] = p;
						break;
					}
					QueueJoin(q, reach[j]);
					parent[reach[j]] = p;
				}
			}
		}
		round++;
	}
	PlaceId temp[50];
	int index = 0;
	for(PlaceId i = dest;i != from;i = parent[i]){
		temp[index++] = i;
	}

	PlaceId* shortest_path = calloc(num_places,sizeof(PlaceId));
	int count = 0;
	for(PlaceId j = index-1;j >= 0;j--){
		shortest_path[count++] = temp[j];
	}

	*pathLength = count;
	return shortest_path;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

// Gets  all  the  locations  that the current hunter player can move to
// this turn.
PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs)
{
	if(HvGetPlayer(hv) == PLAYER_DRACULA){
		fprintf(stderr, "Invaild Hunter :( \n");
		exit(EXIT_FAILURE);
	}
	if(HvGetPlayerLocation(hv,HvGetPlayer(hv)) == NOWHERE) return NULL;
	int num_max = 0;
	PlaceId* reachable = GvGetReachable(hv->gv,HvGetPlayer(hv),HvGetRound(hv),HvGetPlayerLocation(hv,HvGetPlayer(hv)),&num_max);
	*numReturnedLocs = num_max;
	return reachable;
}

// Similar  to  HvWhereCanIGo, but the caller can restrict the transport
// types to be considered. For example, if road and rail are  true,  but
// boat is false, boat connections will be ignored.
PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs)
{
	if(HvGetPlayer(hv) == PLAYER_DRACULA){
		fprintf(stderr, "Invaild Hunter :( \n");
		exit(EXIT_FAILURE);
	}
	if(HvGetPlayerLocation(hv,HvGetPlayer(hv)) == NOWHERE) return NULL;
	int num_max = 0;
	PlaceId* reachable = GvGetReachableByType(hv->gv,HvGetPlayer(hv),HvGetRound(hv),HvGetPlayerLocation(hv,HvGetPlayer(hv)),
	road,rail,boat,&num_max);
	*numReturnedLocs = num_max;
	return reachable;
}

// Gets  all  the  locations  that the given player can move to in their
// next move (for the current player that is this turn).
PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs)
{
	if(HvGetPlayerLocation(hv,player) == NOWHERE) return NULL;
	int round = GvGetRound(hv->gv);
	if (HvGetPlayer(hv) != PLAYER_LORD_GODALMING) {
		round += 1;
	}
	int num_max = 0;
	PlaceId* reachable = GvGetReachable(hv->gv,player,round,HvGetPlayerLocation(hv,player),&num_max);
	*numReturnedLocs = num_max;
	return reachable;
}

// Similar to HvWhereCanTheyGo but the caller can restrict the transport
// types  to  be considered. For example, if road and rail are true, but
// boat is false, boat connections will be ignored.
PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	if(HvGetPlayerLocation(hv,player) == NOWHERE) return NULL;
	int round = GvGetRound(hv->gv);
	if (HvGetPlayer(hv) != PLAYER_LORD_GODALMING) {
		round += 1;
	}
	if(HvGetPlayerLocation(hv,player) == NOWHERE) return NULL;
	int num_max = 0;
	PlaceId* reachable = GvGetReachableByType(hv->gv,player,round,HvGetPlayerLocation(hv,player),
	road,rail,boat,&num_max);
	*numReturnedLocs = num_max;
	return reachable;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// Function to get the reachable cities for the hunters
static PlaceId* Reachable(HunterView hv,Player hunter,int round,PlaceId p,int *numReturnedLocs){

	if(HvGetPlayerLocation(hv,hunter) == NOWHERE) return NULL;
	int turn = 0;
	turn = GvGetRound(hv->gv) + round;
	if (HvGetPlayer(hv) != PLAYER_LORD_GODALMING) {
		turn = 1 + turn;
	}
	int num_max = 0;
	PlaceId* reachable = GvGetReachable(hv->gv,hunter,turn,p,&num_max);
	*numReturnedLocs = num_max;
	return reachable;
}


