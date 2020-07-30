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
#include <string.h>

#include "Game.h"
#include "GameView.h"
#include "Map.h"
#include "Places.h"
#include "testUtils.h"
#include "Queue.h"

void PastPlayAnalysis(GameView gv);
Player DeterminePlayerId(GameView gv, char PlayerAbbrev);
char DeterminePlayerAbr(Player player);
int LastPlay(GameView gv, char character); 
PlaceId RevealHideLocation(GameView gv, int lastTurn);
PlaceId RevealDoubleBackLocation(GameView gv, int PreviousTurn);
void RestCheck (PlaceId Loc, GameView gv, Player character, int player_round);
void AdjustHunterHealth(GameView gv, Player character, int player_round);
void DraculaLocation(PlaceId Loc, GameView gv, Player character, int player_round); 
void AdjustDraculaHealth(GameView gv, Player character);
void HunterEncounters(PlaceId Loc, GameView gv, Player character,char *play);
void DraculaTraps(PlaceId Loc, GameView gv, char *play, int *mature);
int RailRoutesFind (int max_rail_size, PlaceId *GetReachable,Map places, PlaceId from);
//TransportType* PlaceIdtoTransportType (PlaceId *Locations_Reachable, int movable_places);

#define NO_PLAYER 		10
#define MAX_ROUND_STRING 	39
#define HEALTH_UNKNOWN 		100
#define MAX_PLAY_LENGTH 	8
#define PLAYER_MOVES_ACTIONS 	7
#define NOT_PLAYED_YET 		-1024
#define START			0

#define HIDE_MOVE(id) (id == HIDE)
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
	int trap_player;
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
	
	play->Rest = 0;
	play->health = GAME_START_HUNTER_LIFE_POINTS;
	play->Location = NOWHERE;
	play->Trap_Encounter = 0;
	play->trap_player = 0;
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
	
	free(gv->Game_State);
	for (int i = 0; i < NUM_PLAYERS; i++) {
		free(gv->Player[i]->MoveHistory);
	}
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
	//printf ("%s", placeIdToName(gv->Player[player]->Location));
	return gv->Player[player]->Location;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	return gv->Vampire_Location;
}

PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{
	char *copy_string = strdup(gv->Game_State);

	int num = 0;
	int round = -1;
	bool canfree = false;

	PlaceId* loc_his = GvGetLocationHistory(gv,PLAYER_DRACULA,&num,&canfree);

	PlaceId* trap = calloc(gv->Drac_Trap,sizeof(PlaceId));
	for(int v = 0;v < gv->Drac_Trap;v++){
		trap[v] = NOWHERE;
	}
	PlaceId* trap_encounter_player = calloc(gv->Drac_Trap,sizeof(PlaceId));
	int count = 0;
	int count1 = 0;
	char *city;
	char *tok = strtok(copy_string," ");
	while (tok != NULL) {

		Player curr_player = DeterminePlayerId(gv, tok[0]);
		
		if(curr_player == PLAYER_DRACULA) round++;

		city = strndup(&tok[1],2);

		

		PlaceId Loc = placeAbbrevToId(city);
		
		if (curr_player == PLAYER_DRACULA && tok[3] == 'T') {
			printf("%s city \n",city);
			trap[count] = loc_his[round];
			printf("%d loc his\n\n",loc_his[round]);
			count++;
		}
		
		if (curr_player != PLAYER_DRACULA) {
			for(int i = 3; i < strlen(copy_string);i++){
				if(tok[i] == 'T'){
					trap_encounter_player[count1] = Loc;

					count1++;
				}
			}
		}
		
		free(city);
		tok = strtok(NULL," ");

	} 

	int total_trap_encounter = 0;
	for (int l = 0;l < 4;l++) {
		total_trap_encounter += gv->Player[l]->Trap_Encounter;
		
	}


	for (int j = 0;j < gv->Drac_Trap;j++){
		
			//printf("%d hunter trap loc \n\n",trap_encounter_player[k]);
		if (trap[j] == trap_encounter_player[j]){
			trap[j] = NOWHERE;
		}
		
	}

	//sortPlaces(trap, count-total_trap_encounter);
	int temp = 0;
	for (int j = 0;j < gv->Drac_Trap;j++){
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
	free(loc_his);

	*numTraps = (count-total_trap_encounter);
	return trap;
	 
}

////////////////////////////////////////////////////////////////////////
// 			     Game History			      //
////////////////////////////////////////////////////////////////////////

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	int i = 0;
	while (gv->Player[player]->MoveHistory[i]) {
		//printf("%d ", gv->Player[player]->MoveHistory[i]);
		i++;
	}
	//printf("\n%d\n", i);
	*numReturnedMoves = i;
	*canFree = false;
	return gv->Player[player]->MoveHistory;
}

PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{	
	int LastMoves = *numReturnedMoves;
	PlaceId *Move_history = GvGetMoveHistory (gv, player, &LastMoves, canFree);
	
	//printf("%d\n", LastMoves);
	PlaceId *Last_move = calloc(numMoves+1,sizeof(*Last_move));

	int count = 0;
	int move_history_needed = LastMoves - numMoves;
	
	for(int i = move_history_needed; i < LastMoves; i++) {		
		Last_move[count] = Move_history[i];
		//printf("%s\n", placeIdToName(Last_move[count]));
		count++;
	}
	
	*numReturnedMoves = count;
	*canFree = true;
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
		
		if (HIDE_MOVE(Move_History[i])) {
			Loc_History[i] = Loc_History[i-1];
		} 
		else if (DB_MOVE(Move_History[i])) {
			int DB_value = Move_History[i] - HIDE;
			Loc_History[i] = Loc_History[i-DB_value];
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
	int lastLocs = *numReturnedLocs;
	PlaceId *Location_history = GvGetLocationHistory(gv, player, &lastLocs, canFree);
	PlaceId *Last_move = calloc(numLocs,sizeof(*Last_move));

	int count = 0;
	int Loc_hist_needed = lastLocs - numLocs;
	for(int i = Loc_hist_needed; i < lastLocs; i++) {		
		Last_move[count] = Location_history[i];
		count++;
	}
	
	*numReturnedLocs = count;
	*canFree = true;
	return Last_move;
}

////////////////////////////////////////////////////////////////////////
// 			    Making a Move			      //
////////////////////////////////////////////////////////////////////////

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	//assert(placeIsReal(from));
	int numLocs = 0;
	PlaceId *Reachable = GvGetReachableByType(gv, player, round, from, true, true, true, &numLocs);
	
	*numReturnedLocs = numLocs;
	return Reachable;
	
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{	
	//assert(placeIsReal(from));
	PlaceId *GetReachable = calloc(NUM_REAL_PLACES, sizeof(*GetReachable));
	PlaceId *GetReachable_type = calloc(NUM_REAL_PLACES, sizeof(*GetReachable_type));

	Map Places = MapNew();
	int num_places_type = 0;

	GetReachable[num_places_type] = from;
	num_places_type++;

	if (player != PLAYER_DRACULA) {
		if (rail == true) {
			// if (player == PLAYER_DRACULA) {
			// 	fprintf(stderr, "No rail route available for Dracula\n");
			// 	exit(EXIT_FAILURE);
			// }
			int max_rail_size = (round + player)%4;
			num_places_type = RailRoutesFind (max_rail_size, GetReachable, Places, from); 
		}
	}


	if (road == true) {
		ConnList Reachable_places = MapGetConnections(Places, from);
		while (Reachable_places) {
			if (Reachable_places->type == ROAD) {
				if (player == PLAYER_DRACULA) {
					if (Reachable_places->p != HOSPITAL_PLACE) {
						GetReachable[num_places_type] = Reachable_places->p;
						num_places_type++;
					}
				}
				else {
					GetReachable[num_places_type] = Reachable_places->p;
					num_places_type++;
				}
				
			}
			Reachable_places = Reachable_places->next;
		}
	}

	if (boat == true) {
		ConnList Reachable_places = MapGetConnections(Places, from);
		while (Reachable_places) {
			if (Reachable_places->type == BOAT) {
				GetReachable[num_places_type] = Reachable_places->p;
				num_places_type++;
			}
			Reachable_places = Reachable_places->next;
		}
		
	} 
	
	int num_places_unique = 0;
	
	sortPlaces(GetReachable, num_places_type);
	
	for (int i = 0; i < num_places_type; i++) {
		
		if (i != 0) {
			if (GetReachable[i] != GetReachable[i-1]) {
				GetReachable_type[num_places_unique] = GetReachable[i];
				num_places_unique++;
			}
		} 
		
		else {
			GetReachable_type[num_places_unique] = GetReachable[i];
			num_places_unique++;
		}
	}

	free(GetReachable);

	*numReturnedLocs = num_places_unique;
	return GetReachable_type;
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
		
		if (HUNTER(character)) {
			
			RestCheck(Loc, gv, character, player_round);

			HunterEncounters(Loc, gv, character, play);

			AdjustHunterHealth(gv, character, player_round);
			
		} 
		
		else {
			
			DraculaLocation(Loc, gv, character, player_round);

			AdjustDraculaHealth(gv, character); 

			DraculaTraps(Loc, gv, play, &mature); 			
		}

		free(city);
		play = strtok(NULL, " ");
	}
}

void RestCheck(PlaceId Loc, GameView gv, Player character, int player_round) {

	if (Loc == gv->Player[character]->Location) {
		gv->Player[character]->Rest++;
		gv->Player[character]->MoveHistory[player_round] = Loc;
	} 
	
	else {
		gv->Player[character]->Location = Loc;
		gv->Player[character]->MoveHistory[player_round] = Loc;
	}

}

void HunterEncounters(PlaceId Loc, GameView gv, Player character,
				char *play) 
{
	for (int i = 3;i < strlen(play); i++) {
		
		if (play[i] == 'V') {
			gv->Vampire_Location = NOWHERE;
		} 
		
		else if (play[i] == 'T') {
			gv->Player[character]->trap_player++;
			gv->Player[character]->Trap_Encounter++;
		} 
		
		else if (play[i] == 'D' && (placeIdToType(Loc) != SEA)) {
			gv->Player[character]->Player_Encounter++;
			gv->Player[PLAYER_DRACULA]->Player_Encounter++;
		}
	}

	
}

void AdjustHunterHealth(GameView gv, Player character, int player_round) 
{
	int num_traps = gv->Player[character]->trap_player;
	int num_encount = gv->Player[character]->Player_Encounter;
	int num_rest = gv->Player[character]->Rest;
	int dracula_num_encount = gv->Player[PLAYER_DRACULA]->Player_Encounter;
	//printf("%d\n", dracula_num_ecount);
	int HP =  GAME_START_HUNTER_LIFE_POINTS - (LIFE_LOSS_TRAP_ENCOUNTER * num_traps) 
							- (LIFE_LOSS_DRACULA_ENCOUNTER * num_encount) 
							+ (LIFE_GAIN_REST * num_rest);

	//printf("HP1 : %d\n", HP);
	gv->Player[PLAYER_DRACULA]->health = gv->Player[PLAYER_DRACULA]->health 
						- (LIFE_LOSS_HUNTER_ENCOUNTER * dracula_num_encount);
	gv->Player[PLAYER_DRACULA]->Player_Encounter = 0;
	//printf("Dracula HP : %d\n", gv->Player[PLAYER_DRACULA]->health);
	if (HP > GAME_START_HUNTER_LIFE_POINTS) {
		HP = GAME_START_HUNTER_LIFE_POINTS;
		gv->Player[character]->Rest = 0;
		gv->Player[character]->health = HP;
	} 
	
	else if (HP <= 0) {
		gv->Player[character]->death++;
		gv->Player[character]->Location = HOSPITAL_PLACE;
		gv->Player[character]->trap_player = 0;
		gv->Player[character]->Player_Encounter = 0;
		gv->Player[character]->health = 0;
	} 
	
	else {
		gv->Player[character]->health = HP;
	}
}

void DraculaLocation(PlaceId Loc, GameView gv, Player character, int player_round) 
{
	if (Loc == DOUBLE_BACK_1) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-1] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-2];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} 
		else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-1];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_2) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-2] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-3];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} 
		else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-2];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_3) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-3] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-4];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} 
		else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-3];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_4) {
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-4] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-5];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} 
		else {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-4];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	} 
	else if (Loc == DOUBLE_BACK_5) {
		
		if (gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-5] == HIDE) {
			gv->Player[PLAYER_DRACULA]->Location = gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-6];
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} 
		else {
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
	else if (Loc == HIDE) {
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
	else if (placeIdToType(gv->Player[PLAYER_DRACULA]->Location) == SEA 
	|| gv->Player[PLAYER_DRACULA]->Location == SEA_UNKNOWN) {
		gv->Player[character]->Trap_Encounter++; 					
	} 

	int num_traps = gv->Player[character]->Trap_Encounter;
	int num_rest = gv->Player[character]->Rest;
	//printf("NUMTRAPS : %d\n", num_traps);
	//printf("NUMREST : %d\n", num_rest);
	gv->Player[PLAYER_DRACULA]->health = gv->Player[PLAYER_DRACULA]->health - (LIFE_LOSS_SEA * num_traps) + (LIFE_GAIN_CASTLE_DRACULA * num_rest);
	//printf("HP : %d\n", gv->Player[PLAYER_DRACULA]->health);
	//gv->Player[PLAYER_DRACULA]->health = HP_D;
	gv->Player[PLAYER_DRACULA]->Trap_Encounter = 0;
	gv->Player[PLAYER_DRACULA]->Rest = 0;
}

void DraculaTraps(PlaceId Loc, GameView gv, char *play, int *mature) 
{	
	// TRAP ANALYSIS. This won't work for the dracula count of DXY.... where the 
	// first .. is for moves while the second .. is action phases. INCLUDE DOUBLEBACK
	if (play[3] == 'V' || play[4] == 'V') {
		gv->Player[PLAYER_DRACULA]->Vampire_Encounter++;
		gv->Vampire_Location = Loc;
	
	} else if (play[5] == 'V' || play[6] == 'V') {
		*mature = *mature + 1;
		gv->mature = *mature;
		gv->Vampire_Location = NOWHERE;

	} else if (play[3] == 'T' || play[4] == 'T') {
		gv->Drac_Trap++;
	}
}


int RailRoutesFind (int max_rail_size, PlaceId *GetReachable,Map places, PlaceId from) {

	Queue q = newQueue();
	QueueJoin(q, from);
	
	
	int num_rail_moves = 1;
	int rail_type_places = 1;
	
	while (!QueueIsEmpty(q) && num_rail_moves <= max_rail_size) {

		int num_curr_level_rail = QueueSize(q);
		
		while (num_curr_level_rail > 0) {
			
			PlaceId Rail_connecting = QueueLeave(q);
			
			ConnList Rail_route = MapGetConnections(places, Rail_connecting);
			
			while (Rail_route) {
				if (Rail_route->type == RAIL) {
					
					GetReachable[rail_type_places] = Rail_route->p;
					rail_type_places++;
					QueueJoin(q, Rail_route->p);
				}
				Rail_route = Rail_route->next;
			}
			num_curr_level_rail--;
		}
		
		num_rail_moves++;
		
	}
	
	dropQueue (q);
	return rail_type_places;
}
