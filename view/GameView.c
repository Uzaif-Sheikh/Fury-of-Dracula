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
#include <ctype.h>

#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"
#include "Queue.h"
#include <string.h>

void PastPlayAnalysis(GameView gv);
Player DeterminePlayerId(GameView gv, char PlayerAbbrev);
char DeterminePlayerAbr(Player player);
int LastPlay(GameView gv, char character); 
// PlaceId RevealHideLocation(GameView gv, int lastTurn);
// PlaceId RevealDoubleBackLocation(GameView gv, int PreviousTurn);
void RestCheck (PlaceId Loc, GameView gv, Player character, int player_round);
void AdjustHunterHealth(GameView gv, Player character, int player_round);
void DraculaLocation(PlaceId Loc, GameView gv, Player character, int player_round); 
void AdjustDraculaHealth(GameView gv, Player character);
void HunterEncounters(PlaceId Loc, GameView gv, Player character,char *play, int round);
void DraculaTraps(PlaceId Loc, GameView gv, char *play, int round, int mature);
int  RailRoutesFind (int max_rail_size, PlaceId *GetReachable, 
     int movable_places, TransportType* TransportConnections, Map Places);


#define NO_PLAYER 		10
#define MAX_ROUND_STRING 	39
#define HEALTH_UNKNOWN 		100
#define MAX_PLAY_LENGTH 	8
#define PLAYER_MOVES_ACTIONS 	7
#define NOT_PLAYED_YET 		-1024
#define START			0

#define HUNTER(id) (id != PLAYER_DRACULA)
#define NOT_HI_DB_MOVE(id) (id != HIDE && id != DOUBLE_BACK_1 \
				&& id != DOUBLE_BACK_2 && id != DOUBLE_BACK_3 \
				&& id != DOUBLE_BACK_4 && id != DOUBLE_BACK_5)

#define DB_MOVE(id) (id == DOUBLE_BACK_1 || id == DOUBLE_BACK_2 \
			|| id == DOUBLE_BACK_3 || id == DOUBLE_BACK_4 \
			|| id == DOUBLE_BACK_5)
			
#define PASTPLAYS_NOT_FINISHED(c) (c != NULL)

typedef int Score;
typedef int Health;
typedef int Trap_Encounter;
typedef int Player_Encounter;
typedef int Vampire_Encounter;
typedef int Rest;
typedef int Death;

struct game_Player {
	Health health;
	PlaceId Location;
	Trap_Encounter Trap_Encounter;
	Player_Encounter Player_Encounter;
	Vampire_Encounter Vampire_Encounter;
	Rest Rest;
	Death death;
	PlaceId * MoveHistory;
} ;

typedef struct game_Player *Game_Player;

struct gameView {
	
	Round curr_round;
	Game_Player Player[NUM_PLAYERS];
	Score curr_score;
	char* Game_State;
	PlaceId Vampire_Location;
	int mature;
	int Drac_Trap;
};

/*////////////////////////////////////////////////////////////////////////////////////////
//    			         NOTES FOR OUR PROGRAM					//
// * We consider one move/turn by a player to be called a "PLAY". So any variable  	//
// * which is named "play" is considered to refer to a turn. 				//
// * An example of a play: DMU....      						//
////////////////////////////////////////////////////////////////////////////////*/////////


////////////////////////////////////////////////////////////////////////
// 			  Constructor/Distructor		      //
////////////////////////////////////////////////////////////////////////

static Game_Player new_player (int i) {
	
	Game_Player play = malloc(sizeof(*play));
	if (play == NULL) {
		fprintf(stderr, "Couldn't allocate Player!\n");
		exit(EXIT_FAILURE);
	}
	
	play->health = GAME_START_HUNTER_LIFE_POINTS;
	play->Location = NOWHERE;
	play->Trap_Encounter = 0;
	play->Vampire_Encounter = 0;
	play->Player_Encounter = 0;
	play->death = 0;
	play->MoveHistory = calloc(i, sizeof(int));
	return play;
}

GameView GvNew(char *pastPlays, Message messages[]) {
	
	GameView gameView = malloc(sizeof(*gameView));

	if (gameView == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	 
	gameView->Game_State = strdup(pastPlays);
	gameView->curr_score = GAME_START_SCORE;
	int n = GvGetRound(gameView) + 1;
	for (int i = 0; i < NUM_PLAYERS; i++) {
	    gameView->Player[i] = new_player(n);
	}

	gameView->curr_round = 0;
	gameView->Player[4]->health = GAME_START_BLOOD_POINTS;										
	gameView->Vampire_Location = NOWHERE;
	gameView->mature = 0;

	PastPlayAnalysis(gameView);
	
	return gameView;
}

void GvFree(GameView gv) {
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// 			  Game State Infomration		      //
////////////////////////////////////////////////////////////////////////



Round GvGetRound(GameView gv) 
{	
	int size_Game_State = strlen(gv->Game_State);
	int Round_number = (size_Game_State)/MAX_ROUND_STRING;
	gv->curr_round = Round_number;

	return gv->curr_round;
}

Player GvGetPlayer(GameView gv) {
    
	int curr_player = NO_PLAYER;
	int index = strlen(gv->Game_State) - PLAYER_MOVES_ACTIONS;

	switch(gv->Game_State[index]){
		
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
			curr_player = PLAYER_LORD_GODALMING;
			break;
	}

	return curr_player;
}


int GvGetHealth(GameView gv, Player player)
{
	return gv->Player[player]->health;
}


PlaceId GvGetPlayerLocation(GameView gv, Player player)
{	
	// Call reveal HIDE and Doubleback functions
	return gv->Player[player]->Location;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	return gv->Vampire_Location;
}

// PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
// {
	// char *copy_string = strdup(gv->Game_State);

	// PlaceId* trap = calloc(gv->Drac_Trap,sizeof(PlaceId));
	// PlaceId* trap_encounter_player = calloc(gv->Drac_Trap,sizeof(PlaceId));
	// int count = 0;
	// int count1 = 0;
	// char city[2];
	// char *tok = strtok(copy_string," ");
	// while(tok != NULL){

	// 	Player curr_player = DeterminePlayerId(gv, tok[0]);
		
	// 	strncpy(city,&tok[1],2);

	// 	PlaceId Loc = placeAbbrevToId(city);
		
	// 	if(curr_player == PLAYER_DRACULA && tok[3] == 'T'){
	// 		if(Loc == HIDE){
	// 			Loc = RevealHideLocation(gv, LastPlay(gv,tok[0])); 
	// 		}
	// 		if(DB_MOVE(Loc)) {
	// 			Loc = RevealDoubleBackLocation(gv,LastPlay(gv,tok[0]));
	// 		}
	// 		trap[count] = Loc;
	// 		count++;
	// 	}
	// 	if(curr_player == PLAYER_DRACULA && tok[5] == 'M'){
	// 		trap[count-1] = 0;
	// 		count--;
	// 	}
	// 	if(curr_player != PLAYER_DRACULA){
	// 		for(int i = 3; i < strlen(copy_string);i++){
	// 			if(tok[i] == 'T'){
	// 				trap_encounter_player[count1] = Loc;
	// 				count1++;
	// 			}
	// 		}
	// 	}
	// 	tok = strtok(NULL," ");

	// } 

	// int total_trap_encounter = 0;
	// for(int l = 0;l < 4;l++) {
	// 	total_trap_encounter += gv->Player[l]->Trap_Encounter;
	// }

	// for(int j = 0;j < gv->Drac_Trap;j++){
	// 	for(int k = 0;k < gv->Drac_Trap;k++){
	// 		if(trap[j] == trap_encounter_player[k]){
	// 			trap[j] = 0;
	// 		}
	// 	}
	// }

	// int temp = 0;
	
	// for(int j = 0;j < gv->Drac_Trap;j++){
	// 	for (int k = j + 1; k < gv->Drac_Trap; k++){
	// 		if(trap[j] < trap[k]){
				
	// 			temp = trap[j];
	// 			trap[j] = trap[k];
	// 			trap[k] = temp; 

	// 		}
	// 	}
		
	// }
	// free(tok);
	// free(trap_encounter_player);
	// free(copy_string);

	// *numTraps = (count-total_trap_encounter);
	// return trap; 
//}

////////////////////////////////////////////////////////////////////////
// 			     Game History			      //
////////////////////////////////////////////////////////////////////////

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	int i = 0;
	while (gv->Player[player]->MoveHistory[i] != 0) {
		//printf("%d ", gv->Player[player]->MoveHistory[i]);
		i++;
	}
	//printf("\n%d\n", i);
	*numReturnedMoves = i;
	return gv->Player[player]->MoveHistory;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{
	PlaceId *Move_history = GvGetMoveHistory(gv,player,numReturnedMoves,canFree);
	printf("%d\n", *numReturnedMoves);
	PlaceId *Last_move = calloc(numMoves,sizeof(PlaceId));

	int count = 0;
	for(int i = (*numReturnedMoves-1); i >= (*(numReturnedMoves)-numMoves); i--) {		
		Last_move[count] = Move_history[i];
		printf("LOCATION -%d\n", Last_move[count]);
		count++;
	}
	*numReturnedMoves = count;
	*canFree = false;
	return Last_move;
	
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{	
	int numMoves = 0;
	
	PlaceId *Move_History = GvGetMoveHistory(gv, player, &numMoves, canFree);

	PlaceId *Loc_History = calloc(numMoves,sizeof(int));
	
	if (Loc_History == NULL) {
		fprintf(stderr, "Couldnt allocate memory");
		printf("\n");
	}
	
	for (int i = 0; i < numMoves; i++) {
		
		if (Move_History[i] == HIDE) {
			Loc_History[i] = Loc_History[i-1];
		} 
		else if (Move_History[i] == DOUBLE_BACK_1) {
			Loc_History[i] = Loc_History[i-1];
		} 
		else if (Move_History[i] == DOUBLE_BACK_2) {
			Loc_History[i] = Loc_History[i-2];
		} 
		else if (Move_History[i] == DOUBLE_BACK_3) {
			Loc_History[i] = Loc_History[i-3];
		} 
		else if (Move_History[i] == DOUBLE_BACK_4) {
			Loc_History[i] = Loc_History[i-4];
		} 
		else if (Move_History[i] == DOUBLE_BACK_5) {
			Loc_History[i] = Loc_History[i-5];
		} 
		else if (Move_History[i] == TELEPORT) {
			Loc_History[i] = CASTLE_DRACULA;
		} 
		else {
			Loc_History[i] = Move_History[i];
		}
	}
	
	*numReturnedLocs = numMoves;
	return Loc_History;
	
}

PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{
	PlaceId *Location_history = GvGetLocationHistory(gv, player, numReturnedLocs, canFree);
	PlaceId *Last_move = calloc(numLocs,sizeof(PlaceId));

	int count = 0;
	for(int i = (*numReturnedLocs-1); i >= (*(numReturnedLocs)-numLocs); i--) {		
		Last_move[count] = Location_history[i];
		count++;
	}
	*numReturnedLocs = count;
	*canFree = false;
	return Last_move;
}

////////////////////////////////////////////////////////////////////////
// 			    Making a Move			      //
////////////////////////////////////////////////////////////////////////

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	assert(placeIsReal(from));
	Map Places = MapNew();
	
	PlaceId *GetReachable = calloc(NUM_REAL_PLACES, sizeof(*GetReachable));
    	TransportType *TransportConnections = calloc(NUM_REAL_PLACES, sizeof(*GetReachable));
	
	int movable_places = 0;
	GetReachable[movable_places] = from;	
	
	ConnList Reachable_Places = MapGetConnections(Places, from);
	ConnList Places_Movable = Reachable_Places;
	TransportConnections[movable_places] = Reachable_Places->type;

	if (player != PLAYER_DRACULA) {
		
		int max_rail_size = (round + player)%(4);
		while (Places_Movable) {
			
			movable_places++;
			GetReachable[movable_places] = Places_Movable->p;
			// if (Places_Movable->type == RAIL) {
			// 	RailRoutesFind (max_rail_size, GetReachable, from)
			// }
			TransportConnections[movable_places] = Places_Movable->type;
			Places_Movable = Places_Movable->next;
		}

		movable_places = RailRoutesFind (max_rail_size, GetReachable, 
				 movable_places, TransportConnections, Places);
		
	}

	else {
		
		while (Places_Movable) {
			
			if (Places_Movable->p != HOSPITAL_PLACE && 
			Places_Movable->type != RAIL) {
				movable_places++;
				GetReachable[movable_places] = Places_Movable->p;
				Places_Movable = Places_Movable->next;
			}
		}

	}
	
	*numReturnedLocs = movable_places;
	return GetReachable;
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int numLocs = -1;
	PlaceId *ReachablebyType = calloc(NUM_REAL_PLACES, sizeof(*ReachablebyType));
	PlaceId *Reachable_All_Type =  GvGetReachable(gv, player, round, from, &numLocs);
	
	int i = 0;
	while (i < numLocs) {
		if (Reachable_All_Type[i])
	}
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
//		   	  ADDITIONAL PROTOTYPES			      //
////////////////////////////////////////////////////////////////////////

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
			return NO_PLAYER;
	}

	return curr_Player_turn;
}

// Get the last turn of a  given player. The return is the position
// within the pastPlay string of the last appearance
// of the characters abbreviated name. If the player has not
// played yet, NOT_PLAYED_YET is returned
/*
int LastPlay(GameView gv, char character) 
{	
	int lastPlay = NOT_PLAYED_YET;
	for (lastPlay = strlen(gv->Game_State) - PLAYER_MOVES_ACTIONS ;
		lastPlay >= 0; lastPlay -= (PLAYER_MOVES_ACTIONS + 1)) {
				
		if (gv->Game_State[lastPlay] == character) 
			return lastPlay;
	}

	return lastPlay;
}

// Given that a HIDE move is revealed, find the LOCATION
PlaceId RevealHideLocation(GameView gv, int lastTurn) {
	
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

*/
int GvGetScore(GameView gv)
{
    int total_death = 0;
	for(int i = 0; i < 4;i++){
		total_death += gv->Player[i]->death;
	}
	gv->curr_score -=  ((SCORE_LOSS_VAMPIRE_MATURES*gv->mature) + GvGetRound(gv)
				+ (SCORE_LOSS_HUNTER_HOSPITAL*total_death));
	return gv->curr_score;
}

	

void PastPlayAnalysis(GameView gv) {
	int currG_round = START;
	int currS_round = START;
	int currH_round = START;
	int currM_round = START;
	int Drac_round = START;
	int round = START;
	int mature = START;
	char *copy_string = strdup (gv->Game_State);
	char *play = strtok(copy_string, " ");

	while (play != NULL) {
		
		char *city;
		city = strndup(&play[1],2);
		PlaceId Loc = placeAbbrevToId(city);
		Player character = DeterminePlayerId (gv, play[0]);
		
		int player_round = 0;
		switch (character) {
			case PLAYER_LORD_GODALMING:
					player_round = currG_round;
					currG_round++;
				break;

			case PLAYER_DR_SEWARD:
					player_round = currS_round;
					currS_round++;
				break;

			case PLAYER_VAN_HELSING:
				player_round = currH_round;
				currH_round++;
				break;

			case PLAYER_MINA_HARKER:
				player_round = currM_round;
				currM_round++;
				break;

			case PLAYER_DRACULA:
				player_round = Drac_round;
				Drac_round++;
				break;

		}
		//printf("PLAYER ROUND : %d\n", player_round);
		if (HUNTER(character)) {
			
			RestCheck(Loc, gv, character, player_round);

			HunterEncounters(Loc, gv, character, play, round);

			AdjustHunterHealth(gv, character, player_round);
			
			/*
			PlaceId *loc = gv->Player[PLAYER_LORD_GODALMING]->MoveHistory;
			int i = 0;
			while (loc[i] != 0) {
				printf("%d ", loc[i]);
				i++;
			}
			printf("\n");
			*/
		} else {
			
			round ++;
			
			DraculaLocation(Loc, gv, character, player_round);

			AdjustDraculaHealth(gv, character); 

			DraculaTraps(Loc, gv, play, round, mature); 			
		}

		free(city);
		play = strtok(NULL, " ");
	}
}

void RestCheck(PlaceId Loc, GameView gv, Player character, int player_round) {

	if (Loc == gv->Player[character]->Location) {
		gv->Player[character]->Rest++;
		gv->Player[character]->MoveHistory[player_round] = Loc;
	} else {
		gv->Player[character]->Location = Loc;
		gv->Player[character]->MoveHistory[player_round] = Loc;
	}

}

void HunterEncounters(PlaceId Loc, GameView gv, Player character,
				char *play, int round) 
{
	for (int i = 3;i < strlen(play); i++) {
		if (play[i] == 'V') {
			if (round != 6 && gv->Player[PLAYER_DRACULA]->Vampire_Encounter != 0) {
				gv->Vampire_Location = NOWHERE;
			}
		} else if (play[i] == 'T') {
			gv->Player[character]->Trap_Encounter++;
		} else if (play[i] == 'D' && (placeIdToType(Loc) != SEA)) {
			gv->Player[character]->Player_Encounter++;
			gv->Player[PLAYER_DRACULA]->Player_Encounter++;
		}
	}

	
}

void AdjustHunterHealth(GameView gv, Player character, int player_round) 
{
	int num_traps = gv->Player[character]->Trap_Encounter;
	int num_encount = gv->Player[character]->Player_Encounter;
	int num_rest = gv->Player[character]->Rest;
	int dracula_num_encount = gv->Player[PLAYER_DRACULA]->Player_Encounter;
	
	int HP =  GAME_START_HUNTER_LIFE_POINTS - (LIFE_LOSS_TRAP_ENCOUNTER * num_traps) 
							- (LIFE_LOSS_DRACULA_ENCOUNTER * num_encount) 
							+ (LIFE_GAIN_REST * num_rest);

	gv->Player[PLAYER_DRACULA]->health = gv->Player[PLAYER_DRACULA]->health 
						- (LIFE_LOSS_HUNTER_ENCOUNTER * dracula_num_encount);

	if (HP > GAME_START_HUNTER_LIFE_POINTS) {
		HP = GAME_START_HUNTER_LIFE_POINTS;
		gv->Player[character]->Rest = 0;
		gv->Player[character]->health = HP;
	} else if (HP <= 0) {
		gv->Player[character]->death++;
		gv->Player[character]->Location = HOSPITAL_PLACE;
		gv->Player[character]->Trap_Encounter = 0;
		gv->Player[character]->Player_Encounter = 0;
		gv->Player[character]->health = 0;
	} else {
		gv->Player[character]->health = HP;
	}
}

void DraculaLocation(PlaceId Loc, GameView gv, Player character, int player_round) 
{
	if (Loc == DOUBLE_BACK_1) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-1] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-2];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-1];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_2) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-2] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-3];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-2];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_3) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-3] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-4];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-3];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_4) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-4] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-5];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-4];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_5) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-5] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-6];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-5];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == CASTLE_DRACULA) {
		gv->Player[PLAYER_DRACULA]->Location = Loc;
		gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
	} 
	else if (placeIdToType(Loc) == SEA || Loc == SEA_UNKNOWN) {
		gv->Player[PLAYER_DRACULA]->Location = Loc;
		gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
	} 
	else if (Loc == TELEPORT) {
		gv->Player[PLAYER_DRACULA]->Location = CASTLE_DRACULA;
		gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
	} 
	else {
		gv->Player[PLAYER_DRACULA]->Location = Loc;
		gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
	} 
}

void AdjustDraculaHealth(GameView gv, Player character) 
{	
	// Dracula is at Castle Dracula and regains some health
	
	if (gv->Player[PLAYER_DRACULA]->Location == CASTLE_DRACULA) {
		gv->Player[PLAYER_DRACULA]->Rest++; 						
	} 
	
	//Trap_enconter for Dracula keeps track of Dracula being at sea
	
	else if (placeIdToType(gv->Player[PLAYER_DRACULA]->Location) == SEA || 
	gv->Player[PLAYER_DRACULA]->Location == SEA_UNKNOWN) {
		gv->Player[character]->Trap_Encounter++; 					
	} 

	int num_traps = gv->Player[character]->Trap_Encounter;
	int num_rest = gv->Player[character]->Rest;
	//printf("NUMTRAPS : %d\n", num_traps);
	//printf("NUMREST : %d\n", num_rest);
	int HP_D = GAME_START_BLOOD_POINTS - (LIFE_LOSS_SEA * num_traps) + (LIFE_GAIN_CASTLE_DRACULA * num_rest);
	//printf("HP : %d\n", HP_D);
	gv->Player[PLAYER_DRACULA]->health = HP_D;
}

void DraculaTraps(PlaceId Loc, GameView gv, char *play, int round, int mature) 
{	
	// TRAP ANALYSIS. This won't work for the dracula count of DXY.... where the 
	// first .. is for moves while the second .. is action phases. INCLUDE DOUBLEBACK
	for (int i = 3;i < strlen(play); i++) {
		if (play[i] == 'V') {
			gv->Player[PLAYER_DRACULA]->Vampire_Encounter++;
			gv->Vampire_Location = Loc;

			if(round == 7 && gv->Player[PLAYER_DRACULA]->Location == gv->Vampire_Location) {
				gv->Vampire_Location = NOWHERE;
				mature++; 
				gv->Player[PLAYER_DRACULA]->Vampire_Encounter = 0;
				gv->mature = mature;
				round = START;
			}

		} else if (play[i] == 'T') {
			gv->Drac_Trap++;
		}
	}
}

int RailRoutesFind (int max_rail_size, PlaceId *GetReachable, int movable_places, 
TransportType* TransportConnections, Map places) {

	Queue q = newQueue();
	
	for (int i = 0; i < movable_places; i++) {
		if (TransportConnections[i] == RAIL) {
			QueueJoin(q, GetReachable[i]);
		}
		
	}
	
	int num_rail_moves = 1;
	
	while (!QueueIsEmpty(q) && num_rail_moves < max_rail_size) {

		int num_curr_level_rail = QueueSize(q);

		while (num_curr_level_rail--) {
			
			PlaceId Rail_connecting = QueueLeave(q);
			ConnList Rail_route = MapGetConnections(places, Rail_connecting);
			
			ConnList connecting_rail = Rail_route;

			while (connecting_rail) {
				if (connecting_rail->type == RAIL) {
					movable_places++;
					GetReachable[movable_places] = connecting_rail->p;
					QueueJoin(q, connecting_rail->p);
				}
				connecting_rail = connecting_rail->next;
			}
		}

		num_rail_moves++;
	}
	
	return movable_places;
}