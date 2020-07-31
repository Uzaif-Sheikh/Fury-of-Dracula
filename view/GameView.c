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
void PlayerRoundAdjust(int *currG_round, int *currS_round, int *currH_round, 
			int *currM_round, int *Drac_round, 
			int *player_round, Player character);

void ByBoat(bool boat, PlaceId *GetReachable, int *num_places_type, PlaceId from, Map Places);
void ByRoad(bool road, PlaceId *GetReachable, int *num_places_type, PlaceId from, Map Places, Player player);
void ByRail(bool rail, Player player, Round round, int *num_places_type, PlaceId *GetReachable, Map Places, PlaceId from);

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

// Create new player, and intialize all attributes
static Game_Player new_player (int i) {
	
	Game_Player play = malloc(sizeof(*play));
	
	// Check whether play has been correctly allocated and initialised
	if (play == NULL) {
		fprintf(stderr, "Couldn't allocate Player!\n");
		exit(EXIT_FAILURE);
	}
	
	play->Rest = START;
	play->health = GAME_START_HUNTER_LIFE_POINTS;
	play->Location = NOWHERE;
	play->Trap_Encounter = START;
	play->trap_player = START;
	play->Vampire_Encounter = START;
	play->Player_Encounter = START;
	play->death = START;
	play->MoveHistory = calloc(i, sizeof(int));
	return play;
}

// Creates a new view to summarise the current state of the game.
GameView GvNew(char *pastPlays, Message messages[]) {
	
	GameView gameView = malloc(sizeof(*gameView));

	// Check whether gameView has been correctly allocated and initialised
	if (gameView == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	
	gameView->Game_State = strdup(pastPlays);
	gameView->curr_score = GAME_START_SCORE;

	// Finding the current round to assess how much 
	// to calloc for MoveHistory
	int n = GvGetRound(gameView) + 1;
	for (int i = 0; i < NUM_PLAYERS; i++) {
	    gameView->Player[i] = new_player(n);
	}

	gameView->curr_round = START;
	gameView->Player[4]->health = GAME_START_BLOOD_POINTS;										
	gameView->Vampire_Location = NOWHERE;
	gameView->mature = START;

	// Update the Game data according to the last play
	PastPlayAnalysis(gameView);
	
	return gameView;
}

// Free the GameView and all its attributes
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

// Get the current round of play for any given player
Round GvGetRound(GameView gv) 
{	
	int size_Game_State = strlen(gv->Game_State);
	int Round_number = (size_Game_State)/MAX_ROUND_STRING;
	gv->curr_round = Round_number;

	return gv->curr_round;
}

// Gets the current player; i.e., whose turn is it?
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

// Gets the current health points for the given player; 
// a value between 0 and 9 for a hunter, or >= 0 for Dracula.
int GvGetHealth(GameView gv, Player player)
{
	return gv->Player[player]->health;
}

// Gets the current location of a given player.
PlaceId GvGetPlayerLocation(GameView gv, Player player)
{	
	return gv->Player[player]->Location;
}

// Gets the location of the sleeping immature vampire.
PlaceId GvGetVampireLocation(GameView gv)
{
	return gv->Vampire_Location;
}

// Gets the locations of all active traps.
PlaceId *GvGetTrapLocations(GameView gv, int *numTraps)
{
	// Copy the the past plays string
	char *pastPlays = strdup(gv->Game_State);
	
	// Get Dracula's location history in a dynamic array
	int numReturnedLocs = START;
	bool canfree = false;
	PlaceId *loc_his = GvGetLocationHistory(gv,PLAYER_DRACULA,
					&numReturnedLocs,&canfree);

	// Create array to store locaation of active traps dropped by Dracula
	// Initialise all to NOWHERE
	PlaceId *trap = calloc(gv->Drac_Trap,sizeof(PlaceId));
	for(int i = 0;i < gv->Drac_Trap;i++){
		trap[i] = NOWHERE;
	}

	// Create array to store the numbe of trap encounters for a given player
	PlaceId *trap_encounter_player = calloc(gv->Drac_Trap,sizeof(PlaceId));

	int trap_count = START;
	char *city;
	int trap_encounter_count = START;
	int round = START - 1;
	
	// Tokenize the past play string into individual player moves
	char *play = strtok(pastPlays," ");

	while (play != NULL) {

		// Determine player 
		Player curr_player = DeterminePlayerId(gv, play[0]);
		
		// Increment round at the end of this turn if
		// Dracula is playing, as Dracula is the last play
		if(curr_player == PLAYER_DRACULA) round = round + 1;

		// Copy the city/move into city variable
		city = strndup(&play[1],2);

		// Convert city/move into placeId
		PlaceId Loc = placeAbbrevToId(city);
		
		// If Dracula drops a trap, increase trap count
		if (curr_player == PLAYER_DRACULA && play[3] == 'T') {
			trap[trap_count] = loc_his[round];
			trap_count = trap_count + 1;
		}
		
		// If Hunters encounter a trap, increase trap encounter count
		if (HUNTER(curr_player)) {
			for(int i = 3; i < strlen(pastPlays); i++){
				if(play[i] == 'T'){
					trap_encounter_player[trap_encounter_count] = Loc;
					trap_encounter_count = trap_encounter_count + 1;
				}
			}
		}
		
		free(city);
		play = strtok(NULL," ");

	} 

	// Calculate sum of trap encounters encountered
	// by all four hunters
	int total_trap_encounter = 0;
	for (int i = 0; i < 4; i++) {
		total_trap_encounter += gv->Player[i]->Trap_Encounter;
		
	}

	// If the location of the traps dropped by dracula is equal to
	// the location of the trap encountered by a hunter, this trap
	// is not active anymore; remove from active trap array
	for (int i = 0; i < gv->Drac_Trap; i++){
		if (trap[i] == trap_encounter_player[i]){
			trap[i] = NOWHERE;
		}
		
	}

	// Sort Places in trap array to be in ascending order
	// of PlaceId
	int temp = 0;
	for (int i = 0; i < gv->Drac_Trap; i++){
		for (int j = i + 1; j < gv->Drac_Trap; j++){
			if(trap[i] < trap[j]){
				temp = trap[i];
				trap[i] = trap[j];
				trap[j] = temp; 

			}
		}
		
	}

	// Free all arrays created in this function
	free(play);
	free(trap_encounter_player);
	free(pastPlays);
	free(loc_his);

	*numTraps = (trap_count-total_trap_encounter);
	return trap;
	 
}

////////////////////////////////////////////////////////////////////////
// 			     Game History			      //
////////////////////////////////////////////////////////////////////////

// Gets  the complete move history, in chronological order, of the given
// player as shown in the play string.
PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	// Calculate the numReturnedMoves
	int numMoves = 0;
	while (gv->Player[player]->MoveHistory[numMoves]) {
		numMoves++;
	}
	
	*numReturnedMoves = numMoves;
	*canFree = false;
	return gv->Player[player]->MoveHistory;
}

// Gets  the given player's last `numMoves` moves in chronological order
// as shown in the play string.
PlaceId *GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int *numReturnedMoves, bool *canFree)
{	
	int LastNumMovesRet = *numReturnedMoves;
	PlaceId *Move_history = GvGetMoveHistory (gv, player, &LastNumMovesRet, canFree);
	PlaceId *Last_move = calloc(numMoves+1,sizeof(*Last_move));

	int count = 0;
	int round = GvGetRound(gv);
	
	if (numMoves >= round) {
		numMoves = round;
	}

	int move_history_needed = LastNumMovesRet - numMoves;
	
	for(int i = move_history_needed; i < LastNumMovesRet; i++) {		
		Last_move[count] = Move_history[i];
		count++;
	}
	
	*numReturnedMoves = count;
	*canFree = true;
	return Last_move;
	
}

// Gets  the  complete  location history, in chronological order, of the
//  given player.
PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{	
	int numMoves = 0;
	PlaceId *Move_History = GvGetMoveHistory(gv, player, &numMoves, canFree);
	PlaceId *Loc_History = calloc(numMoves,sizeof(int));
	
	// Check that location history array has been correctly initialized
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

// Gets  the  given   player's last `numLocs` locations
// in chronological order.
PlaceId *GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int *numReturnedLocs, bool *canFree)
{	
	int lastLocs = *numReturnedLocs;
	PlaceId *Location_history = GvGetLocationHistory(gv, player, &lastLocs, canFree);
	PlaceId *Last_move = calloc(numLocs,sizeof(*Last_move));

	int count = 0;
	int round = GvGetRound(gv);
	
	if (numLocs >= round) {
		numLocs = round;
	}
	
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

	ByRail(rail, player, round, &num_places_type, GetReachable, Places, from);

	ByRoad(road, GetReachable, &num_places_type, from, Places, player);

	ByBoat(boat, GetReachable, &num_places_type, from, Places); 
	
	int num_places_unique = 0;
	
	// Sort the Reachable Places
	sortPlaces(GetReachable, num_places_type);
	
	// Remove duplicate reachable place i.e. places
	// which are reacheable in more than one way (e.g. if 
	// D1 and HI give the same location, the location must appear
	// only once)
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

void ByRail(bool rail, Player player, Round round, int *num_places_type, PlaceId *GetReachable, Map Places, PlaceId from)
{
	if (player != PLAYER_DRACULA) {
		if (rail == true) {
			int max_rail_size = (round + player)%4;
			*num_places_type = RailRoutesFind(max_rail_size, 
						GetReachable, Places, from); 
		}
	}
}

void ByBoat(bool boat, PlaceId *GetReachable, int *num_places_type, PlaceId from, Map Places) 
{
	if (boat == true) {
		ConnList Reachable_places = MapGetConnections(Places, from);
		while (Reachable_places) {
			if (Reachable_places->type == BOAT) {
				GetReachable[*num_places_type] = Reachable_places->p;
				*num_places_type = *num_places_type + 1;
			}
			Reachable_places = Reachable_places->next;
		}
		
	} 
}

void ByRoad(bool road, PlaceId *GetReachable, int *num_places_type, PlaceId from, Map Places, Player player) 
{
	if (road == true) {
		ConnList Reachable_places = MapGetConnections(Places, from);
		while (Reachable_places) {
			if (Reachable_places->type == ROAD) {
				if (player == PLAYER_DRACULA) {
					if (Reachable_places->p != HOSPITAL_PLACE) {
						GetReachable[*num_places_type] = Reachable_places->p;
						*num_places_type = *num_places_type + 1;
					}
				}
				else {
					GetReachable[*num_places_type] = Reachable_places->p;
					*num_places_type = *num_places_type + 1;
				}
				
			}

			Reachable_places = Reachable_places->next;
		}
	}
}

////////////////////////////////////////////////////////////////////////
//		   	  ADDITIONAL PROTOTYPES			      //
////////////////////////////////////////////////////////////////////////

// The main analysis of past plays; used to update
// struct.
void PastPlayAnalysis(GameView gv) {
	
	int currG_round = START;
	int currS_round = START;
	int currH_round = START;
	int currM_round = START;
	int Drac_round = START;
	int mature = START;
	char *pastPlays = strdup (gv->Game_State);
	char *play = strtok(pastPlays, " ");

	while (play != NULL) {
		
		char *city;
		city = strndup(&play[1],2);
		PlaceId Loc = placeAbbrevToId(city);
		Player character = DeterminePlayerId (gv, play[0]);
		
		int player_round = START;
		
		PlayerRoundAdjust(&currG_round, &currS_round, &currH_round, 
						&currM_round, &Drac_round, 
						&player_round, character);
		
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
	for(int i = 0; i < 4; i++) {
		total_death += gv->Player[i]->death;
	}
	gv->curr_score -=  ((SCORE_LOSS_VAMPIRE_MATURES*gv->mature) + GvGetRound(gv)
				+ (SCORE_LOSS_HUNTER_HOSPITAL*total_death));
	return gv->curr_score;
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
	for (int i = 3; i < strlen(play); i++) {
		
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
	int HP =  GAME_START_HUNTER_LIFE_POINTS - (LIFE_LOSS_TRAP_ENCOUNTER * num_traps) 
							- (LIFE_LOSS_DRACULA_ENCOUNTER * num_encount) 
							+ (LIFE_GAIN_REST * num_rest);

	gv->Player[PLAYER_DRACULA]->health = gv->Player[PLAYER_DRACULA]->health 
						- (LIFE_LOSS_HUNTER_ENCOUNTER * dracula_num_encount);
	gv->Player[PLAYER_DRACULA]->Player_Encounter = 0;
	
	if (HP > GAME_START_HUNTER_LIFE_POINTS) {
		HP = GAME_START_HUNTER_LIFE_POINTS;
		gv->Player[character]->Rest = 0;
		gv->Player[character]->health = HP;
	} 
	
	else if (HP <= 0) {
		gv->Player[character]->death++;
		gv->Player[character]->Location = HOSPITAL_PLACE;
		gv->Player[character]->trap_player = START;
		gv->Player[character]->Player_Encounter = START;
		gv->Player[character]->health = START;
	} 
	
	else {
		gv->Player[character]->health = HP;
	}
}

void DraculaLocation(PlaceId Loc, GameView gv, Player character, int player_round) 
{
	if (DB_MOVE(Loc)) {
		
		int DB_value = Loc - HIDE;
		
		if (gv->Player[PLAYER_DRACULA]->MoveHistory
				[player_round-DB_value] == HIDE) {
			
			gv->Player[PLAYER_DRACULA]->Location = 
				gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-DB_value-1];
			
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		} 
		
		else {
			gv->Player[PLAYER_DRACULA]->Location = 
				gv->Player[PLAYER_DRACULA]->MoveHistory[player_round-DB_value];
			
			gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
		}
	
	} 
	
	else if (Loc == TELEPORT) {
		gv->Player[PLAYER_DRACULA]->Location = CASTLE_DRACULA;
		gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
	} 
	
	else if (HIDE_MOVE(Loc)) {
		gv->Player[PLAYER_DRACULA]->MoveHistory[player_round] = Loc;
	} 
	
	else if (!DB_MOVE(Loc)) {
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
	
	gv->Player[PLAYER_DRACULA]->health = gv->Player[PLAYER_DRACULA]->health 
				- (LIFE_LOSS_SEA * num_traps) 
				+ (LIFE_GAIN_CASTLE_DRACULA * num_rest);
				
	//gv->Player[PLAYER_DRACULA]->health = HP_D;
	gv->Player[PLAYER_DRACULA]->Trap_Encounter = 0;
	gv->Player[PLAYER_DRACULA]->Rest = 0;
}

void DraculaTraps(PlaceId Loc, GameView gv, char *play, int *mature) 
{	
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
					rail_type_places = rail_type_places + 1;
					QueueJoin(q, Rail_route->p);
				}

				Rail_route = Rail_route->next;
			}

			num_curr_level_rail = num_curr_level_rail - 1;
		}	
		
		num_rail_moves++;
		
	}
	
	dropQueue (q);
	return rail_type_places;
}

void PlayerRoundAdjust(int *currG_round, int *currS_round, int *currH_round, 
			int *currM_round, int *Drac_round, 
			int *player_round, Player character)
{
	switch (character) {
		case PLAYER_LORD_GODALMING:
			*player_round = *currG_round;
			*currG_round = *currG_round + 1;
			break;

		case PLAYER_DR_SEWARD:
			*player_round = *currS_round;
			*currS_round = *currS_round + 1;
			break;

		case PLAYER_VAN_HELSING:
			*player_round = *currH_round;
			*currH_round = *currH_round + 1;
			break;

		case PLAYER_MINA_HARKER:
			*player_round = *currM_round;
			*currM_round = *currM_round + 1;
			break;

		case PLAYER_DRACULA:
			*player_round = *Drac_round;
			*Drac_round = *Drac_round + 1;
			break;

	}
}
