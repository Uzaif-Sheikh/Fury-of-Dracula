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

static bool adjacent(Map m,PlaceId p1,PlaceId p2);
// TODO: ADD YOUR OWN STRUCTS HERE

struct hunter{
	PlaceId Loction;
	Health health;

};


struct hunterView {
	
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
	GameView gv = GvNew(pastPlays,messages);

	for(int i = 0;i < MAX_HUNTERS;i++){
		hunter_state->hunters[i] = new_player(gv,i);
	}

	hunter_state->Dracula_health = GvGetHealth(gv,PLAYER_DRACULA);
	hunter_state->curr_score = GvGetScore(gv);
	hunter_state->curr_round = GvGetRound(gv);
	hunter_state->Dracula_loc = GvGetPlayerLocation(gv,PLAYER_DRACULA);
	hunter_state->last_know_loc_dracula = NOWHERE;
	hunter_state->vampire_loc = GvGetVampireLocation(gv);
	hunter_state->curr_player_turn = GvGetPlayer(gv);

	GvFree(gv);

	return hunter_state;
}

void HvFree(HunterView hv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
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

PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength)
{
	Map m = MapNew();
	int num_places = MapNumPlaces(m);
	int* distance = malloc(num_places*sizeof(int));
	PlaceId* parent = calloc(num_places,sizeof(PlaceId));

	for(PlaceId i = 0;i < num_places;i++){
		distance[i] = INF_NUM;
	}

	PlaceId from = hv->hunters[hunter]->Loction;
	distance[from] = 0;

	Queue q = newQueue();
	QueueJoin(q,from);
	parent[from] = from;

	while(!QueueIsEmpty(q)){
		PlaceId p = QueueLeave(q);

		if(p == dest) break;

		for(PlaceId j = 0;j < num_places;j++){
			
			if(adjacent(m,p,j) && distance[p] + 1 < distance[j] && parent[j] == 0){
				QueueJoin(q,j);
				distance[j] = distance[p] + 1;
				parent[j] = p;
			}
		}

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

PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
