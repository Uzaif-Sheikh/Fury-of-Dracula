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
// TODO: ADD YOUR OWN STRUCTS HERE

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

HunterView HvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
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

void HvFree(HunterView hv)
{
	for(int i = 0;i < MAX_HUNTERS;i++){
		free(hv->hunters[i]);
	}
	free(hv->Hunter_string);
	GvFree(hv->gv);
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv)
{
	return hv->curr_round;
}

Player HvGetPlayer(HunterView hv)
{
	return hv->curr_player_turn;
}

int HvGetScore(HunterView hv)
{
	return hv->curr_score;
}

int HvGetHealth(HunterView hv, Player player)
{
	if(player == PLAYER_DRACULA){
		return hv->Dracula_health;
	}
	return hv->hunters[player]->health;
}

PlaceId HvGetPlayerLocation(HunterView hv, Player player)
{
	if(player == PLAYER_DRACULA){
		return hv->Dracula_loc;
	}
	return hv->hunters[player]->Loction;
}

PlaceId HvGetVampireLocation(HunterView hv)
{

	return hv->vampire_loc;
}

////////////////////////////////////////////////////////////////////////
// Utility Functions

// Gets the Last location  of Dracula known to hunters
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


// Finds the shortest path using Breadth first Search algorithm (implemented
// using queues) from the hunter's current  position to the provided
// destination . Function reads the reachable cities from the hunter's current 
// position including the maximum number of rail moves, pushing each entry into
// a queue which hasn't already been visited and storing the immediate parent 
// of the city in a parent array. Shortest path is determined by retracing the parent
// array from destination to source.
PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength)
{
	Map m = MapNew();

	// round keeps track of round since the hunter last moved
	int round = 0;

	int num_places = MapNumPlaces(m);
	
	// Allocating space for parent array
	PlaceId* parent = malloc(num_places*sizeof(PlaceId));

	for(PlaceId i = 0;i < num_places;i++){
		parent[i] = NOWHERE;
	}

	PlaceId from = hv->hunters[hunter]->Loction;
	
	// Getting the new queue
	Queue q = newQueue();
	QueueJoin(q,from);
	parent[from] = from;

	// flag keeps track of whether the destination has been found or not
	int flag = 0;

	// while Queue is empty and destination not found
	while(!QueueIsEmpty(q) && flag != 1){
		PlaceId p ;
		int size_q = QueueSize(q);
		while(size_q > 0 && flag != 1) {
			p = QueueLeave(q);
			size_q--;
			int numlocs = 0;

			// Reachable helper function called
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
			free(reach);
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
	
	free(parent);
	*pathLength = count;
	return shortest_path;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

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

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs)
{
	if(HvGetPlayerLocation(hv,player) == NOWHERE || HvGetPlayerLocation(hv, PLAYER_DRACULA) == CITY_UNKNOWN) return NULL;
	int round = GvGetRound(hv->gv);
	if (HvGetPlayer(hv) != PLAYER_LORD_GODALMING) {
		round += 1;
	}
	int num_max = 0;
	PlaceId* reachable = GvGetReachable(hv->gv,player,round,HvGetPlayerLocation(hv,player),&num_max);
	*numReturnedLocs = num_max;
	return reachable;
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	if(HvGetPlayerLocation(hv,player) == NOWHERE || HvGetPlayerLocation(hv, PLAYER_DRACULA) == CITY_UNKNOWN) return NULL;
	int round = GvGetRound(hv->gv);
	if (HvGetPlayer(hv) != PLAYER_LORD_GODALMING) {
		round += 1;
	}
	int num_max = 0;
	PlaceId* reachable = GvGetReachableByType(hv->gv,player,round,HvGetPlayerLocation(hv,player),
	road,rail,boat,&num_max);
	*numReturnedLocs = num_max;
	return reachable;
}

////////////////////////////////////////////////////////////////////////

// Reachable function is a helper function for shortest path which uses the current round 
//from the hunter's provided round and location  and provides the reachable cities using
// GvGetReachable() function from GameView
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


