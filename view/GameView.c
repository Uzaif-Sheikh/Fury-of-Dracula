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
#include <ctype.h>


void PastPlayAnalysis(GameView gv);
void adjDraculasTrail(GameView gv, char *playString, PlaceId trailNum);
void Encounters(GameView gv, char *playString, Player Character);
Player DeterminePlayerId(GameView gv, char PlayerAbbrev);
char DeterminePlayerAbr(Player player);
int LastPlay(GameView gv, char character); 
PlaceId RevealHideLocation(GameView gv, int lastTurn);
PlaceId RevealDoubleBackLocation(GameView gv, int PreviousTurn);
void RestCheck (PlaceId Loc, GameView gv, Player character);

#define NO_PLAYER 		10
#define MAX_ROUND_STRING 	39
#define HEALTH_UNKNOWN 		100
#define MAX_PLAY_LENGTH 	8
#define PLAYER_MOVES_ACTIONS 	7
#define NOT_PLAYED_YET 		-1024
#define DOUBLE_BACK(loc) (loc == DOUBLE_BACK_1 || loc == DOUBLE_BACK_2 || loc == DOUBLE_BACK_3 || loc == DOUBLE_BACK_4 || loc == DOUBLE_BACK_5 )

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
	
	Game_Player play = malloc(sizeof(*play));
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
	GameView gameView = malloc(sizeof(*gameView));
	
	if (gameView == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	

	gameView->Game_State = strdup(pastPlays);
    	gameView->curr_score = GAME_START_SCORE;
	for (int i = 0; i < NUM_PLAYERS; i++) {
	    gameView->Player[i] = new_player();
	}
	PastPlayAnalysis(gameView);
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
	int GetGameSCore = GAME_START_SCORE;
	GetGameSCore -= (gv->curr_round)*(SCORE_LOSS_DRACULA_TURN);

	int health_zero = 0;

	for (int i = 0; i < 4; i++) {
		if (GvGetHealth(gv,i) == 0) health_zero++;
	}
	
	GetGameSCore -= (health_zero)*(SCORE_LOSS_HUNTER_HOSPITAL);

	int num_of_vampire = (GvGetRound(gv)/13) + 1;

	int vampire_left = -1;
	int total_vampire_encounter = 0;
	for (int j = 0;j < 4;j++) {
		total_vampire_encounter += gv->Player[j]->Vampire_Encounter;
	}

	vampire_left = num_of_vampire - total_vampire_encounter;

	GetGameSCore -= (vampire_left)*(SCORE_LOSS_VAMPIRE_MATURES);

	return GetGameSCore;
}

int GvGetHealth(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int GetHealthPlayer = HEALTH_UNKNOWN;

	if (player == PLAYER_DRACULA) {
		
		GetHealthPlayer = GAME_START_BLOOD_POINTS;
		
		GetHealthPlayer -= (gv->Player[PLAYER_DRACULA]->
		Player_Encounter)*(LIFE_LOSS_HUNTER_ENCOUNTER);
		
		if (gv->Player[PLAYER_DRACULA]->Trail[TRAIL_SIZE-1] == SEA_UNKNOWN || placeIdToType(gv->Player[PLAYER_DRACULA]->Trail[TRAIL_SIZE-1]) == SEA){
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
                
		GetHealthPlayer += (LIFE_GAIN_REST)*(gv->Player[player]->Rest);
	}
	
	return GetHealthPlayer;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{       

	int PlayerLocation;
	char character;
	
	character = DeterminePlayerAbr(player);
	int lastTurn = LastPlay(gv, character);
	
	// Check if player has played a turn
	if (gv->curr_round == 0 && lastTurn == NOT_PLAYED_YET) return NOWHERE;
	
	char location[2];
	if (player != PLAYER_DRACULA) {
		
		if (GvGetHealth(gv,player) == 0) {
			PlayerLocation = ST_JOSEPH_AND_ST_MARY;
		} 
		else {
			strncpy(location,&gv->Game_State[lastTurn+1],2);
			PlayerLocation = placeAbbrevToId(location);
		}
	}
	else {

		strncpy(location, &gv->Game_State[lastTurn + 1], 2);
		
		if (strcmp(location,"HI") == 0)
			PlayerLocation = RevealHideLocation(gv, lastTurn);

		else if (location[0] == 'D' && isdigit(location[1]))	
			PlayerLocation = RevealDoubleBackLocation(gv, lastTurn);
		
		else if (strcmp(location,"CD") == 0 || strcmp(location,"TP") == 0)
			PlayerLocation = CASTLE_DRACULA;
		
		else if (strcmp(location,"C?") == 0) 
			PlayerLocation = CITY_UNKNOWN;
		
		else if (strcmp(location,"S?") == 0) 
		 	PlayerLocation = SEA_UNKNOWN;
	
	}

	return PlayerLocation;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION

	return NOWHERE;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{
	char *copy_string = strdup(gv->Game_State);

	PlaceId* trap = calloc(gv->Drac_Trap,sizeof(PlaceId));
	PlaceId* trap_encounter_player = calloc(gv->Drac_Trap,sizeof(PlaceId));
	int count = 0;
	int count1 = 0;
	char city[2];
	char *tok = strtok(copy_string," ");
	while(tok != NULL){

		Player curr_player = DeterminePlayerId(gv, tok[0]);
		
		strncpy(city,&tok[1],2);

		PlaceId Loc = placeAbbrevToId(city);
		
		if(curr_player == PLAYER_DRACULA && tok[3] == 'T'){
			if(Loc == HIDE){
				Loc = RevealHideLocation(gv, LastPlay(gv,tok[0])); 
			}
			if(DOUBLE_BACK(Loc)){
				Loc = RevealDoubleBackLocation(gv,LastPlay(gv,tok[0]));
			}
			trap[count] = Loc;
			count++;
		}
		if(curr_player != PLAYER_DRACULA){
			for(int i = 3; i < strlen(copy_string);i++){
				if(tok[i] == 'T'){
					trap_encounter_player[count1] = Loc;
					count1++;
				}
			}
		}
		tok = strtok(NULL," ");

	}

	int total_trap_encounter = 0;
	for(int l = 0;l < 4;l++){
		total_trap_encounter += gv->Player[l]->Trap_Encounter;
	}

	for(int j = 0;j < gv->Drac_Trap;j++){
		for(int k = 0;k < gv->Drac_Trap;k++){
			if(trap[j] == trap_encounter_player[k]){
				trap[j] = 0;
			}
		}
	}

	int temp = 0;
	
	for(int j = 0;j < gv->Drac_Trap;j++){
		for (int k = j + 1; k < gv->Drac_Trap; k++){
			if(trap[j] < trap[k]){
				
				temp = trap[j];
				trap[j] = trap[k];
				trap[k] = temp; 

			}
		}
		
	}
	free(tok);
	free(trap_encounter_player);
	free(copy_string);

	*numTraps = (count-total_trap_encounter);
	return trap;

}

////////////////////////////////////////////////////////////////////////
// 			     Game History			      //
////////////////////////////////////////////////////////////////////////

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	PlaceId* Move_history = calloc((GvGetRound(gv) + 1),sizeof(PlaceId));
	char player_place[3]; 

	player_place[0] = DeterminePlayerAbr(player);
	strncpy(&player_place[1],placeIdToAbbrev(gv->Player[player]->Location),2);

	char *point = malloc(4);
	int i;
	for(i = 0;i < strlen(gv->Game_State);i++){
		if(strncmp(player_place,&point[i],3) == 0) break;
	}

	char* copy_string;
	copy_string = strdup(gv->Game_State);
	char* tok = strtok(copy_string," ");
	char city[2];
	int count = 0;
	int flag = 0;

	while(flag == 0 && tok != NULL){

		if(player == DeterminePlayerId(gv,tok[0])){

			for(int j = 0;j < i; j += MAX_ROUND_STRING){

				flag = 1;
				strncpy(city,&gv->Game_State[j+1],2);
				PlaceId Loc = placeAbbrevToId(city);
				Move_history[count++] = Loc;

			}
		}

		tok = strtok(NULL," ");
	}

	free(tok);
	free(copy_string);
	free(point);

	*numReturnedMoves = count;
	*canFree = false;
	return Move_history;
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
	PlaceId *Loc_History = calloc(gv->curr_round,sizeof(*Loc_History));
	char location[2];
	char character;
	character = DeterminePlayerAbr(player);

	int lastLoc = LastPlay(gv, character);

	assert(lastLoc > 0);
	
	int countLocs = 0;
	for(int prevLocs = lastLoc; prevLocs >= 0; 
		prevLocs = prevLocs - MAX_ROUND_STRING) {

		strncpy(location, &gv->Game_State[prevLocs + 1], 2);
		Loc_History[countLocs] = placeAbbrevToId(location);
		countLocs++;
	}
	
	*numReturnedLocs = countLocs;
	*canFree = true;
	return Loc_History;
}

// Get the last numLocs locations of a particular player
PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	PlaceId *lastNumLoc = calloc(gv->curr_round,sizeof(*lastNumLoc));
	char location[2];
	char character;
	character = DeterminePlayerAbr(player);
	
	int lastLoc = LastPlay(gv, character);
	
	assert(lastLoc > 0);
	
	int countLocs = 0;
	for(int prevLocs = lastLoc; prevLocs >= 0 ; 
	prevLocs = prevLocs - MAX_ROUND_STRING) {
		
		if(countLocs == numLocs) break;

		strncpy(location, &gv->Game_State[prevLocs + 1], 2);
		lastNumLoc[countLocs] = placeAbbrevToId(location);
		countLocs++;
	}
	
	*numReturnedLocs = countLocs;
	*canFree = true;
	return lastNumLoc;
}

////////////////////////////////////////////////////////////////////////
// 			    Making a Move			      //
////////////////////////////////////////////////////////////////////////

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
//		   	  ADDITIONAL PROTOTYPES			      //
////////////////////////////////////////////////////////////////////////

// Analyse all game past game moves
void PastPlayAnalysis(GameView gv) 
{	
	char *play = malloc(8);
	char curr_loc[2];

	play = strtok(gv->Game_State, " ");
	//PlaceId dracTrailNum = 0;
    	

	while (play != NULL) {
		// Adjust Dracula's Trail
		// PlayerLocationsAdd(gv, character, play);
		// if (character == PLAYER_DRACULA) { 
		// 	adjDraculasTrail(gv, play, dracTrailNum);
		// }
		Player character = DeterminePlayerId(gv, play[0]);
		
		strncpy(curr_loc, &play[1], 2);
		PlaceId Loc = placeAbbrevToId(curr_loc);

		if (character != PLAYER_DRACULA) {
			RestCheck (Loc, gv, character);
		}
		
        	Encounters(gv, play, character);
		
		play = strtok(NULL, " "); 
	}
}

// Update Draculas trail
// void adjDraculasTrail(GameView gv, char *playString, PlaceId trailNum) 
// {
// 	char *location;
// 	strncpy(location, &playString[1], 2);
// 	for(int i = 0; playString[i] != '\0'; i++) {
// 		gv->Player[PLAYER_DRACULA]->Trail[trailNum] = placeAbbrevToId(location);
// 		trailNum++;

// 		if (trailNum == TRAIL_SIZE) trailNum = 0;
// 	}
// }

// Count the encounters of immature Vampires, Traps, and speciffically
// for hunters, the number of Dracula encounters
void Encounters(GameView gv, char *playString, Player Character) 
{
     	char Encounter_Check[4];
	strncpy(Encounter_Check, &playString[3], 4);
	
	/*char *location;
	strncpy(location,playString[1], 2);
	PlaceId encounter_loc = placeAbbrevToId(location);*/
	
	for (int i = 0; Encounter_Check[i] != '\0'; i++) {
		
		switch (Encounter_Check[i]) {
			case 'V':
				if (i < 2) {
					gv->Player[Character]->Vampire_Encounter++;
				}
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

// Given abbreviation, find playerId
Player DeterminePlayerId(GameView gv, char PlayerAbbrev) 
{
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

// void PlayerLocationsAdd (GameView gv, Player character, char *play) 
// {
// 	char *location = strncpy(location, play[1], 2);
// 	laceAbbrevToId(location);
// 	gv->Player[character]->Player_Locations[0]

// }

// Given playerId, find player abbreviation
char DeterminePlayerAbr(Player player)
{	
    char curr_Player_turn;
	
	switch (player) {
		
		case PLAYER_DRACULA:
	        	curr_Player_turn = 'D';
			break;
		case PLAYER_LORD_GODALMING:
			curr_Player_turn = 'G';
			break;
		case PLAYER_DR_SEWARD:
			curr_Player_turn = 'S';
			break;
		case PLAYER_VAN_HELSING:
			curr_Player_turn = 'H';
			break;
		case PLAYER_MINA_HARKER:
			curr_Player_turn = 'M';
			break;
		default:
			break;
	}

	return curr_Player_turn;
}

// Retrieve a character's last turn
// char playerLastTurn(GameView gv, char character) {
	
// 	for(int i = strlen(gv->Game_State); i >= 0; i--) {
// 		if (gv->Game_State[i] == character) {
// 			return gv->Game_State[i];
// 		}
// 	}
// 	return NULL;
// }

// Get the last turn of a  given player. The return is the position
// within the pastPlay string of the last appearance
// of the characters abbreviated name. If the player has not
// played yet, NOT_PLAYED_YET is returned
int LastPlay(GameView gv, char character) 
{
	int lastPlay = NOT_PLAYED_YET;
	for (lastPlay = strlen(gv->Game_State - PLAYER_MOVES_ACTIONS); 
	    lastPlay >= 0; lastPlay -= (PLAYER_MOVES_ACTIONS + 1)) {
				
		if (gv->Game_State[lastPlay] == character) 
			return lastPlay;
	}
	return 0;
}

// Given that a HIDE move is revealed, find the LOCATION
PlaceId RevealHideLocation(GameView gv, int lastTurn) {
	
	// TO DO:
	int PreviousTurn = lastTurn - MAX_ROUND_STRING;
	char location[2];
	
	strncpy(location, &gv->Game_State[PreviousTurn+1], 2);
	
	if (((strcmp(location, "HI") != 0) ||  
	!(location[0] == 'D') )
	&& isdigit(location[1])){

		return placeAbbrevToId(location);
	}
        
	PlaceId Loc;

	if(strcmp (location,"HI") == 0) {
		Loc = RevealHideLocation(gv, PreviousTurn);
	}

	else if ((location[0] == 'D') 
	&& isdigit(location[1])) {

		Loc = RevealDoubleBackLocation(gv, PreviousTurn);
		
	}

	return Loc; 
}

// Given that a DOUBLEBACK move is revealed, find the LOCATION								
PlaceId RevealDoubleBackLocation(GameView gv, int PreviousTurn) {
	// TO DO:
	char location[2];
	
	strncpy(location, &gv->Game_State[PreviousTurn+1], 2);

	if (((strcmp(location, "HI") != 0) ||  
	!(location[0] == 'D') )
	&& isdigit(location[1])) {

		return placeAbbrevToId(location);
	}

	PlaceId Loc;

	if ((location[0] == 'D') 
	&& isdigit(location[1])) {
                
		int DB_move = gv->Game_State[PreviousTurn+1];
		Loc = RevealDoubleBackLocation(gv, PreviousTurn - DB_move);
	}

	else if (strcmp (location,"HI") == 0) {
		Loc = RevealHideLocation(gv, PreviousTurn);
	}

	return Loc;
}

void RestCheck (PlaceId Loc, GameView gv, Player character) {

	if (Loc == gv->Player[character]->Location) {
		gv->Player[character]->Rest++;
	} 
	else {
		gv->Player[character]->Location = Loc;
	}

}