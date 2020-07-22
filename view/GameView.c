////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
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
#include "Map.h"
#include "Places.h"
// add your own #includes here
#include <string.h>

#define NO_PLAYER 10;
#define MAX_ROUND_STRING 40;
#define HEALTH_UNKNOWN 100;
#define MAX_PLAY_LENGTH 8;
#define PLAYER_MOVES_ACTIONS 7;

typedef int Score;
typedef int Health;
typedef int Trap_Encounter;
typedef int Player_Encounter;
typedef int Vampire_Encounter;
typedef int Rest;

struct game_Player {	
	Health health;
	PlaceId Location;
	Trap_Encounter Trap_Encounter;
	Player_Encounter Player_Encounter;
	Vampire_Encounter Vampire_Encounter;
	Rest Rest;
	PlaceId Trail[TRAIL_SIZE];
	// PlaceId *Player_Locations;
} ; 
// TODO: ADD YOUR OWN STRUCTS HERE

typedef struct game_Player *Game_Player;

struct gameView {
	// TODO: ADD FIELDS HERE
	Round curr_round;
	Game_Player Player[NUM_PLAYERS];
    	Score curr_score;
	char* Game_State;
};


////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

static Game_Player new_player () {
	
	Game_Player play = malloc(sizeof(struct game_Player));
	if (play == NULL) {
		fprintf(stderr, "Couldn't allocate Player!\n");
		exit(EXIT_FAILURE);
	}
	
	play->health = HEALTH_UNKNOWN;
	play->Location = UNKNOWN_PLACE;
	return play;

}

GameView GvNew(char *pastPlays, Message messages[]) {
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GameView gameView = malloc(sizeof(gameView));
	
	if (gameView == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	
	strcpy(gameView->Game_State, pastPlays);
    	gameView->curr_score = GAME_START_SCORE;
	for (int i = 0; i < NUM_PLAYERS; i++) {
	    gameView->Player[i] = new_player();
	}
	return gameView;
}

void GvFree(GameView gv) {
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv) {
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    	int size_Game_State = strlen(gv->Game_State);
	int Round_number = (size_Game_State)/MAX_ROUND_STRING;
    	gv->curr_round = Round_number;

	return gv->curr_round;
}

Player GvGetPlayer(GameView gv) {
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    
	int index = strlen(gv->Game_State) - PLAYER_MOVES_ACTIONS;

	char pre_player = gv->Game_State[index];
	int curr_player = NO_PLAYER;

	switch(pre_player){
		
		case 'D':
			curr_player = PLAYER_LORD_GODALMING;
			break;
		case 'G':
			curr_player = PLAYER_DR_SEWARD;
			break;
		case 'S':
			curr_player = PLAYER_VAN_HELSING;
			break;
		case 'H':
			curr_player = PLAYER_MINA_HARKER;
			break;
		case 'M':
			curr_player = PLAYER_DRACULA;
			break;
		default:
			curr_player = NO_PLAYER;
			break;
	}

	

	return curr_player;
}

int GvGetScore(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int GetGameSCore = GAME_START_SCORE;
	GetGameSCore -= (gv->curr_round)*(SCORE_LOSS_DRACULA_TURN);
	

	return 0;
}

int GvGetHealth(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int GetHealthPlayer = HEALTH_UNKNOWN;

	if (player == PLAYER_DRACULA) {
		
		GetHealthPlayer = GAME_START_BLOOD_POINTS;
		
		GetHealthPlayer -= (gv->Player[PLAYER_DRACULA]->
		Player_Encounter)*(LIFE_LOSS_HUNTER_ENCOUNTER);
		
		if (gv->Player[PLAYER_DRACULA]->Trail[TRAIL_SIZE-1] == SEA) {
			GetHealthPlayer -= LIFE_LOSS_SEA;
		}
		
		if (gv->Player[PLAYER_DRACULA]->Trail[TRAIL_SIZE-1] 
		== CASTLE_DRACULA) {
			GetHealthPlayer += LIFE_GAIN_CASTLE_DRACULA;	
		}
	}

	else {
		GetHealthPlayer = GAME_START_HUNTER_LIFE_POINTS;
		
		GetHealthPlayer -= (gv->Player[player]->
		Trap_Encounter)*(LIFE_LOSS_TRAP_ENCOUNTER);
		
		GetHealthPlayer -= (gv->Player[player]->
		Player_Encounter)*(LIFE_LOSS_DRACULA_ENCOUNTER);

	}
	
	return GetHealthPlayer;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return NOWHERE;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return NOWHERE;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Game History

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	*canFree = false;
	return NULL;
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	PlaceId *lastNumLoc = calloc(gv->curr_round,sizeof(*lastNumLoc));
	char location[2];
	char character = DeterminePlayerAb(player);

	int i;
	for(i = strlen(gv->Game_State); i >= 0; i--) {
		if (gv->Game_State[i] == character) {
			break;
		}
	}
	
	int countLocs = 0;
	for(int j = i; j >= 0 ; j = j - MAX_ROUND_STRING) {
		if(countLocs == numLocs) {
			break;
		}

		strncpy(location, gv->Game_State[j + 1], 2);
		lastNumLoc[countLocs] = placeAbbrevToId(location);
		countLocs++;
	}
	
	*numReturnedLocs = countLocs;
	*canFree = true;
	return lastNumLoc;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

void PastPlayAnalysis(GameView gv) {
	
	char *play = malloc(8);
	
	play = strtok(gv->Game_State, " ");
	PlaceId dracTrailNum = 0;
    	Player character = DeterminePlayer(gv, play[0]);

	while (play != '\0') {
		// Adjust Dracula's Trail
		// PlayerLocationsAdd(gv, character, play);
		if (character == PLAYER_DRACULA) { 
			adjDraculasTrail(gv, play, dracTrailNum);
		}
        	Encounters(gv, play, character);

		if (character != PLAYER_DRACULA) {
			RestCheck (gv, character, play);
		}

		play = strtok(NULL, " ");
	}
}

void adjDraculasTrail(GameView gv, char *playString, PlaceId trailNum) 
{
	char *location = strncpy(location, playString[1], 2);
	for(int i = 0; playString[i] != '\0'; i++) {
		gv->Player[PLAYER_DRACULA]->Trail[trailNum] = placeAbbrevToId(location);
		trailNum++;

		if (trailNum == TRAIL_SIZE) {
			trailNum = 0;
		}

	}
}

void Encounters (GameView gv, char *playString, Player Character) 
{
     	char *Encounter_Check;
	strncpy(Encounter_Check, playString[3], 4);
	
	for (int i = 0; Encounter_Check[i] != '\0'; i++) {
		
		switch (Encounter_Check[i]) {
			case 'V':
				gv->Player[Character]->Vampire_Encounter++;
				break;
			
			case 'D':
				gv->Player[Character]->Player_Encounter++;
				break;
			
			case 'T':
				gv->Player[Character]->Trap_Encounter++;
			
			default:
				break;
		}

	}


}

Player DeterminePlayer (GameView gv, char PlayerAbbrev) {
	
    Player curr_Player_turn;
	
	switch (PlayerAbbrev) {
		
		case 'D':
	        	curr_Player_turn = PLAYER_DRACULA;
			break;
		case 'G':
			curr_Player_turn = PLAYER_LORD_GODALMING;
			break;
		case 'S':
			curr_Player_turn = PLAYER_DR_SEWARD;
			break;
		case 'H':
			curr_Player_turn = PLAYER_VAN_HELSING;
			break;
		case 'M':
			curr_Player_turn = PLAYER_MINA_HARKER;
			break;
		default:
			curr_Player_turn = NO_PLAYER;
			break;
	}

	return curr_Player_turn;
			    
}

void RestCheck (gv, character, play) 
{

}

// void PlayerLocationsAdd (GameView gv, Player character, char *play) 
// {
// 	char *location = strncpy(location, play[1], 2);
// 	laceAbbrevToId(location);
// 	gv->Player[character]->Player_Locations[0]

// }

Player DeterminePlayerAb (Player player) {
	
    char curr_Player_turn;
	
	switch (player) {
		
		case PLAYER_DRACULA:
	        	curr_Player_turn = 'D';
			break;
		case PLAYER_LORD_GODALMING:
			curr_Player_turn = 'G';
			break;
		case PLAYER_DR_SEWARD:
			curr_Player_turn = "S";
			break;
		case PLAYER_VAN_HELSING:
			curr_Player_turn = 'H';
			break;
		case PLAYER_MINA_HARKER:
			curr_Player_turn = 'M';
			break;
		default:
			return;
	}

	return curr_Player_turn;
			    
}