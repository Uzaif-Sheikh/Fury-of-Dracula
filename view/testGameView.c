////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testGameView.c: test the GameView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v1.2	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#include "GameView.h"
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////
	
		printf("Basic initialisation\n");

		char *trail = "";
		Message messages[] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 0);
		assert(GvGetPlayer(gv) == PLAYER_LORD_GODALMING);
		assert(GvGetScore(gv) == GAME_START_SCORE);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == NOWHERE);
		assert(GvGetVampireLocation(gv) == NOWHERE);

		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("After Lord Godalming's turn\n");

		char *trail =
			"GST....";
		
		Message messages[1] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 0);
		assert(GvGetPlayer(gv) == PLAYER_DR_SEWARD);
		assert(GvGetScore(gv) == GAME_START_SCORE);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(GvGetPlayerLocation(gv, PLAYER_DR_SEWARD) == NOWHERE);

		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");

	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("After Mina Harker's turn\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB....";
		
		Message messages[4] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 0);
		assert(GvGetPlayer(gv) == PLAYER_DRACULA);
		assert(GvGetScore(gv) == GAME_START_SCORE);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(GvGetPlayerLocation(gv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(GvGetPlayerLocation(gv, PLAYER_VAN_HELSING) == ZURICH);
		assert(GvGetPlayerLocation(gv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == NOWHERE);

		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("After Dracula's turn\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB.... DC?.V..";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahahaha"
		};
		
		GameView gv = GvNew(trail, messages);
		assert(GvGetRound(gv) == 1);
		assert(GvGetPlayer(gv) == PLAYER_LORD_GODALMING);
		assert(GvGetScore(gv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS);
		assert(GvGetHealth(gv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(GvGetPlayerLocation(gv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(GvGetPlayerLocation(gv, PLAYER_VAN_HELSING) == ZURICH);
		assert(GvGetPlayerLocation(gv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);
		assert(GvGetVampireLocation(gv) == CITY_UNKNOWN);

		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Encountering Dracula\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD..";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahahaha",
			"Aha!"
		};
		
		GameView gv = GvNew(trail, messages);
		
		
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) ==
				GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_DRACULA_ENCOUNTER);
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS - LIFE_LOSS_HUNTER_ENCOUNTER);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == GENEVA);
		assert(GvGetVampireLocation(gv) == NOWHERE);

		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
		
		printf("Test for Dracula doubling back at sea, "
		       "and losing blood points (Hunter View)\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DS?.... "
			"GST.... SST.... HST.... MST.... DCD.... "
			"GST.... SST.... HST.... MST.... DD2... ";
		
		Message messages[] = {
			"Party at Geneva", "Okay", "Sure", "Let's go", "Mwahahahaha",
			"", "", "", "", "Back I go"
		};
		
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 3);
		assert(GvGetPlayer(gv) == PLAYER_LORD_GODALMING);
		assert(GvGetScore(gv) == GAME_START_SCORE - 3 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == SEA_UNKNOWN);
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS -  2 * LIFE_LOSS_SEA + LIFE_GAIN_CASTLE_DRACULA);

		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
		
	}

	{/////////////////////////////////////////////////
		printf("Testing for hunter dying and teleporting to hospital\n");
		
		char *trail =
		"GED.... SGE.... HZU.... MCA.... DCF.V.. "
		"GMN.... SCFVD.. HGE.... MLS..... DC?T... "
		"GLO.... SMR.... HCF.... MMA.... DC?T... "
		"GPL.... SMS.... HMR.... MGR.... DBAT... "
		"GEC.... SBATD.. HGO....";
		
		Message messages[21] = {};
		GameView gv = GvNew(trail, messages);

		assert(GvGetPlayer(gv) == PLAYER_MINA_HARKER);
		assert(GvGetRound(gv) == 4);
		assert(GvGetHealth(gv , PLAYER_DR_SEWARD) == 0);
		assert(GvGetHealth(gv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS - 20);
		assert(GvGetScore(gv) ==  GAME_START_SCORE - 6 - 4);
		assert(GvGetPlayerLocation(gv, PLAYER_DR_SEWARD) ==  HOSPITAL_PLACE);
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) == 0);

		GvFree(gv);
		
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}

	/////////////////////////////////////////////////////////////////////
	{
		printf("Test for Dracula's health loss at sea and health gain at Castle Dracula\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DS?.... "
			"GST.... SST.... HST.... MST.... DCD.... "
			"GST.... SST.... HST.... MST.... DD2... ";
		
		Message messages[] = {
			"Party at Geneva", "Okay", "Sure", "Let's go", "Mwahahahaha",
			"", "", "", "", "Back I go"
		};
		
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 3);
		assert(GvGetPlayer(gv) == PLAYER_LORD_GODALMING);
		assert(GvGetScore(gv) == GAME_START_SCORE - 3 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == SEA_UNKNOWN);
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS -  2 * LIFE_LOSS_SEA + LIFE_GAIN_CASTLE_DRACULA);

		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
		
	}


	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula doubling back at sea, "
			   "and losing blood points (Dracula View)\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DEC.... "
			"GST.... SST.... HST.... MST.... DD1.... "
			"GFR.... SFR.... HFR.... MFR....";
		
		Message messages[14] = {
			"Hello", "Rubbish", "Stuff", "", "Mwahahah",
			"Aha!", "", "", "", "Back I go"};
		
		GameView gv = GvNew(trail, messages);

		assert(GvGetRound(gv) == 2);
		assert(GvGetPlayer(gv) == PLAYER_DRACULA);
		assert(GvGetScore(gv) == GAME_START_SCORE - 2 * SCORE_LOSS_DRACULA_TURN);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == ENGLISH_CHANNEL);
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS - (2 * LIFE_LOSS_SEA));
	
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
		
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Checking that hunters' health points are capped\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DC?.V.. "
			"GGE....";
	
		Message messages[6] = {};
		GameView gv = GvNew(trail, messages);
	
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS);
		
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Testing a hunter 'dying'\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DC?.V.. "
			"GGE.... SGE.... HGE.... MGE.... DSTT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GSTTTTD SGE.... HGE.... MGE.... DSTT... " ;
		
		Message messages[21] = {};
		GameView gv = GvNew(trail, messages);
		
		assert(GvGetScore(gv) == GAME_START_SCORE
		                         - 5 * SCORE_LOSS_DRACULA_TURN
		                         - SCORE_LOSS_HUNTER_HOSPITAL);
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) == 0);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == HOSPITAL_PLACE);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == STRASBOURG);
		int numMoves = 0;
		bool canFree = false;
		PlaceId *moves = GvGetLocationHistory(gv, PLAYER_LORD_GODALMING,
			                                  &numMoves, &canFree);
			assert(numMoves == 5);
			assert(moves[0] == GENEVA);
			assert(moves[1] == GENEVA);
			assert(moves[2] == GENEVA);
			assert(moves[3] == GENEVA);
			assert(moves[4] == STRASBOURG);
			if (canFree) free(moves);
		
		int numTraps = 0;
		PlaceId *traps = GvGetTrapLocations(gv, &numTraps);
		assert(numTraps == 1);
		assert(traps[0] == STRASBOURG);
		
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
		
		printf("Testing Dracula doubling back to Castle Dracula\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T...";
		
		Message messages[10] = {};
		GameView gv = GvNew(trail, messages);
		
		assert(GvGetHealth(gv, PLAYER_DRACULA) ==
				GAME_START_BLOOD_POINTS + (2 * LIFE_GAIN_CASTLE_DRACULA));
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CASTLE_DRACULA);
		
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Testing vampire/trap locations\n");
		
		char *trail =
			"GVI.... SGE.... HGE.... MGE.... DCD.V.. "
			"GBD.... SGE.... HGE.... MGE.... DKLT... "
			"GSZ.... SGE.... HGE.... MGE.... DGAT... "
			"GSZ.... SGE.... HGE.... MGE....";
		
		Message messages[19] = {};
		GameView gv = GvNew(trail, messages);
		
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == GALATZ);
		assert(GvGetVampireLocation(gv) == CASTLE_DRACULA);
		int numTraps = 0;
		PlaceId *traps = GvGetTrapLocations(gv, &numTraps);
		assert(numTraps == 2);
		sortPlaces(traps, numTraps);
		assert(traps[0] == GALATZ && traps[1] == KLAUSENBURG);
		
		free(traps);
		
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Testing trap locations after one is destroyed\n");
		
		char *trail =
			"GVI.... SGE.... HGE.... MGE.... DBC.V.. "
			"GBD.... SGE.... HGE.... MGE.... DKLT... "
			"GSZ.... SGE.... HGE.... MGE.... DGAT... "
			"GBE.... SGE.... HGE.... MGE.... DCNT... "
			"GKLT... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		GameView gv = GvNew(trail, messages);
		
		assert(GvGetHealth(gv, PLAYER_LORD_GODALMING) ==
				GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_TRAP_ENCOUNTER);
		assert(GvGetPlayerLocation(gv, PLAYER_LORD_GODALMING) == KLAUSENBURG);
		assert(GvGetVampireLocation(gv) == BUCHAREST);
		
		int numTraps = 0;
		PlaceId *traps = GvGetTrapLocations(gv, &numTraps);
		assert(numTraps == 2);
		sortPlaces(traps, numTraps);
		assert(traps[0] == CONSTANTA && traps[1] == GALATZ);
		
		free(traps);
		
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Testing a vampire maturing\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DC?.V.. "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?..V. "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?.V.. "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGEV... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?T... "
			"GGE.... SGE.... HGE.... MGE.... DC?....";
		
		Message messages[35] = {};
		GameView gv = GvNew(trail, messages);
		

		assert(GvGetScore(gv) == GAME_START_SCORE
		                         - 19 * SCORE_LOSS_DRACULA_TURN
		                         - SCORE_LOSS_VAMPIRE_MATURES);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);
		assert(GvGetVampireLocation(gv) == NOWHERE);
		
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Testing move/location history\n");
		
		char *trail =
			"GLS.... SGE.... HGE.... MGE.... DST.V.. "
			"GCA.... SGE.... HGE.... MGE.... DC?T... "
			"GGR.... SGE.... HGE.... MGE.... DC?T... "
			"GAL.... SGE.... HGE.... MGE.... DD3T... "
			"GSR.... SGE.... HGE.... MGE.... DHIT... "
			"GSN.... SGE.... HGE.... MGE.... DC?T... "
			"GMA.... SSTTTV.";
		
		Message messages[32] = {};
		GameView gv = GvNew(trail, messages);
		
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) ==
				GAME_START_HUNTER_LIFE_POINTS - 2 * LIFE_LOSS_TRAP_ENCOUNTER);
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CITY_UNKNOWN);
		assert(GvGetVampireLocation(gv) == NOWHERE);
		
		// Lord Godalming's move/location history
		{
			int numMoves = 0; bool canFree = false;
			PlaceId *moves = GvGetMoveHistory(gv, PLAYER_LORD_GODALMING,
			                                  &numMoves, &canFree);
			assert(numMoves == 7);
			assert(moves[0] == LISBON);
			assert(moves[1] == CADIZ);
			assert(moves[2] == GRANADA);
			assert(moves[3] == ALICANTE);
			assert(moves[4] == SARAGOSSA);
			assert(moves[5] == SANTANDER);
			assert(moves[6] == MADRID);
			if (canFree) free(moves);
		}
		
		// Dracula's move/location history
		{
			int numMoves = 0; bool canFree = false;
			PlaceId *moves = GvGetMoveHistory(gv, PLAYER_DRACULA,
			                                  &numMoves, &canFree);
			assert(numMoves == 6);
			assert(moves[0] == STRASBOURG);
			assert(moves[1] == CITY_UNKNOWN);
			assert(moves[2] == CITY_UNKNOWN);
			assert(moves[3] == DOUBLE_BACK_3);
			assert(moves[4] == HIDE);
			assert(moves[5] == CITY_UNKNOWN);
			if (canFree) free(moves);
		}
		
		{
			int numLocs = 0; bool canFree = false;
			PlaceId *locs = GvGetLocationHistory(gv, PLAYER_DRACULA,
			                                     &numLocs, &canFree);
			assert(numLocs == 6);
			assert(locs[0] == STRASBOURG);
			assert(locs[1] == CITY_UNKNOWN);
			assert(locs[2] == CITY_UNKNOWN);
			assert(locs[3] == STRASBOURG);
			assert(locs[4] == STRASBOURG);
			assert(locs[5] == CITY_UNKNOWN);
			if (canFree) free(locs);
		}
		
		GvFree(gv);
		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	}

	// {///////////////////////////////////////////////////////////////////
	
	// 	printf("Testing connections\n");
		
	// 	char *trail = "GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
	// 	"GLO.... SAL.... HCO.... MBR.... DBET... "
	// 	"GED.... SBO.... HLI.... MPR.... DKLT... "
	// 	"GLV.... SNA.... HNU.... MBD.... DCDT... "
	// 	"GIR.... SPA.... HPR.... MKLT... DHIT... "
	// 	"GAO.... SST.... HSZ.... MCDTTD. DGAT... "
	// 	"GMS.... SFL.... HKL.... MSZ.... DCNT.V. "
	// 	"GTS.... SRO.... HBC.... MCNTD.. DBS..M. "
	// 	"GIO.... SBI.... HCN.... MCN.... DIO.... "
	// 	"GIO.... SAS.... HBS.... MCN.... DTS.... "
	// 	"GTS.... SAS.... HIO.... MBS.... DMS.... "
	// 	"GMS.... SIO.... HTS.... MIO.... DAO..M. "
	// 	"GAO.... STS.... HMS.... MTS.... DNS.... "
	// 	"GBB.... SMS.... HAO.... MMS.... DED.V.. "
	// 	"GNA.... SAO.... HEC.... MAO.... DMNT... "
	// 	"GBO.... SIR.... HLE.... MEC.... DD2T... "
	// 	"GSR.... SDU.... HBU.... MPL.... DHIT... "
	// 	"GSN.... SIR.... HAM.... MLO.... DTPT... "
	// 	"GAL.... SAO.... HCO.... MEC.... DCDT... "
	// 	"GMS.... SMS.... HFR.... MLE.... DKLT.V. "
	// 	"GTS.... STS.... HBR.... MCO.... DGAT.M. "
	// 	"GIO.... SIO.... HBD.... MLI.... DD3T.M. "
	// 	"GBS.... SBS.... HKLT... MBR.... DHI..M. "
	// 	"GCN.... SCN.... HCDTTTD MVI.... DTPT... "
	// 	"GGAT... SGA.... HSZ.... MBC.... DCDT... "
	// 	"GCDTTD. SCDD... HKL.... MGA.... DKLT... "
	// 	"GSZ.... SKLTD.. HKLD... MKLD... DBC.V.. "
	// 	"GBD.... SBE.... HGA.... MBCVD.. DSOT... "
	// 	"GSZ.... SSOTD.. HBC.... MSOD...";
	// 	Message messages[] = {};
	// 	GameView gv = GvNew(trail, messages);

	// 	{
	// 		printf("Checking Galatz road connections "
	// 		       "(Lord Godalming, Round 1)\n");
	// 		int numLocs = -1;
	// 		PlaceId *locs = GvGetReachable(gv, PLAYER_LORD_GODALMING,
	// 		                                     1, GALATZ, &numLocs);

	// 		assert(numLocs == 5);
	// 		sortPlaces(locs, numLocs);
			
	// 		assert(locs[0] == BUCHAREST);
	// 		assert(locs[1] == CASTLE_DRACULA);
	// 		assert(locs[2] == CONSTANTA);
	// 		assert(locs[3] == GALATZ);
	// 		assert(locs[4] == KLAUSENBURG);
	// 		free(locs);
	// 		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	// 	}

	// 	{
	// 		printf("Checking Ionian Sea boat connections "
	// 		       "(Lord Godalming, Round 1)\n");
			
	// 		int numLocs = -1;
	// 		PlaceId *locs = GvGetReachable(gv, PLAYER_LORD_GODALMING,
	// 		                                     1, IONIAN_SEA, &numLocs);
			
	// 		sortPlaces(locs, numLocs);
	// 		assert(numLocs == 7);
			
	// 		assert(locs[0] == ADRIATIC_SEA);
	// 		assert(locs[1] == ATHENS);
	// 		assert(locs[2] == BLACK_SEA);
	// 		assert(locs[3] == IONIAN_SEA);
	// 		assert(locs[4] == SALONICA);
	// 		assert(locs[5] == TYRRHENIAN_SEA);
	// 		assert(locs[6] == VALONA);
	// 		free(locs);
	// 		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	// 	}

	// 	{
	// 		printf("Checking Paris rail connections "
	// 		       "(Lord Godalming, Round 2)\n");
	// 		int numLocs = -1;
	// 		PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
	// 		                                     2, PARIS, false, true,
	// 		                                     false, &numLocs);
	// 		assert(numLocs == 7);
			
	// 		sortPlaces(locs, numLocs);
	// 		assert(locs[0] == BORDEAUX);
	// 		assert(locs[1] == BRUSSELS);
	// 		assert(locs[2] == COLOGNE);
	// 		assert(locs[3] == LE_HAVRE);
	// 		assert(locs[4] == MARSEILLES);
	// 		assert(locs[5] == PARIS);
	// 		assert(locs[6] == SARAGOSSA);
	// 		free(locs);
	// 		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	// 	}
		
	// 	{
	// 		printf("Checking Athens rail connections (none)\n");
	// 		int numLocs = -1;
	// 		PlaceId *locs = GvGetReachableByType(gv, PLAYER_LORD_GODALMING,
	// 		                                     1, ATHENS, false, true,
	// 		                                     false, &numLocs);
			
	// 		assert(numLocs == 1);
	// 		assert(locs[0] == ATHENS);
	// 		free(locs);
	// 		printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	// 	}

	// 	{
	// 		printf("Checking reachable places for dracula\n");
			
	// 		int numMoves = 0; bool canFree = false;
	// 		int numLocs = 0;
			
	// 		PlaceId* LastMoves = GvGetLastMoves(gv, PLAYER_DRACULA, 12, &numMoves, &canFree);
	// 		PlaceId* LastLocs = GvGetLastLocations(gv, PLAYER_DRACULA, 12, &numLocs, &canFree);
			
	// 		//asserting and checking the moves in the order of what they come without sorting.
	// 		assert (numMoves == 12);
	// 		assert (LastMoves[0] == HIDE);
	// 		assert (LastMoves[1] == TELEPORT);
	// 		assert (LastMoves[2] == CASTLE_DRACULA);
	// 		assert (LastMoves[3] == KLAUSENBURG);
	// 		assert (LastMoves[4] == GALATZ);
	// 		assert (LastMoves[5] == DOUBLE_BACK_3);
	// 		assert (LastMoves[6] == HIDE);
	// 		assert (LastMoves[7] == TELEPORT);
	// 		assert (LastMoves[8] == CASTLE_DRACULA);
	// 		assert (LastMoves[9] == KLAUSENBURG);
	// 		assert (LastMoves[10] == BUCHAREST);
	// 		assert (LastMoves[11] == SOFIA);
			
	// 		//asserting and checking the locations in the order of what they come without sorting.
	// 		assert (numLocs == 12);
	// 		assert (LastLocs[0] == EDINBURGH);
	// 		assert (LastLocs[1] == CASTLE_DRACULA);
	// 		assert (LastLocs[2] == CASTLE_DRACULA);
	// 		assert (LastLocs[3] == KLAUSENBURG);
	// 		assert (LastLocs[4] == GALATZ);
	// 		assert (LastLocs[5] == CASTLE_DRACULA);
	// 		assert (LastLocs[6] == CASTLE_DRACULA);
	// 		assert (LastLocs[7] == CASTLE_DRACULA);
	// 		assert (LastLocs[8] == CASTLE_DRACULA);
	// 		assert (LastLocs[9] == KLAUSENBURG);
	// 		assert (LastLocs[10] == BUCHAREST);
	// 		assert (LastLocs[11] == SOFIA);
			
	// 		PlaceId from = GvGetPlayerLocation(gv, PLAYER_DRACULA);
	// 		assert (from == SOFIA);
	// 		int round = GvGetRound(gv);
			
	// 		assert (round == 29);
	// 		PlaceId *locs = GvGetReachable(gv, PLAYER_DRACULA, round, from, &numLocs);
			
	// 		sortPlaces(locs, numLocs);
			
	// 		assert (numLocs == 7);
	// 		assert (locs[0] == BELGRADE);
	// 		assert (locs[1] == BUCHAREST);
	// 		assert (locs[2] == SALONICA);
	// 		assert (locs[3] == SARAJEVO);
	// 		assert (locs[4] == SOFIA);
	// 		assert (locs[5] == VALONA);
	// 		assert (locs[6] == VARNA);
	// 		free(locs);
	// 		free (LastMoves);
	// 	}


	// 	GvFree(gv);
	// 	printf("\033[32m" "Test Passed! :)" "\033[0m\n");
	// }


	{///////////////////////////////////////////////////////////////////////

		printf("Checking for the Max health of Hunter\n");

		char *trail =
			"GZU.... SVI.... HVR.... MTS.... DCG.V.. GST.... SSZ.... HVR.... MTS.... DD1T... GFR.... SSA.... HBS.... MTS.... DTS.... GLI.... SIO.... HBS.... MTS.... DMS.... GLI.... SVA.... HBS.... MTS.... DMRT... GLI.... SSO.... HBS.... MTS.... DTOT... GCO.... SBC.... HBS.... MTS.... DSRT.V. GHA.... SKL.... HBS.... MTS.... DSNT.M. GHA.... SSZ.... HBS.... MTS.... DMAT... GHA.... SBD.... HBS.... MTS.... DGRT... GNS.... SGA.... HBS.... MTS.... DCAT.M. GHA.... SCN.... HBS.... MTS.... DLST.M. GHA.... SCN.... HBS.... MTS.... DHIT.M. GHA.... SSZ.... HBS.... MTS.... DSN.VM. GNS.... SSA.... HBS.... MTS.... DSRT.M. GHA.... SIO.... HBS.... MTS.... DTOT.M. GHA.... SVA.... HBS.... MTS.... DCFT.M. GHA.... SSO.... HBS.... MTS.... DNAT.M. GNS.... SBC.... HBS.... MTS.... DLET.M. GHA.... SKL.... HBS.... MTS.... DPAT.V. GHA.... SSZ.... HBS.... MTS.... DSTT.M. GHA.... SBD.... HBS.... MTS.... DNUT.M. GNS.... SGA.... HBS.... MTS.... DPRT.M. GHA.... SCN.... HBS.... MTS.... DBRT.M. GHA.... SGA.... HCN.... MGO.... DD2T.M. GLI.... SSZ.... HGA.... MVE.... DHIT.M. GFR.... SSO.... HGA.... MFL.... DVI.VM. GLI.... SBC.... HGA.... MRO.... DBDT.M. GNU.... SBC.... HSZ.... MGE.... DZAT.M. GST.... SKL.... HSO.... MZU.... DSZT.M. GST.... SBC.... HBC.... MZU.... DD2T.M. GBU.... SSO.... HBC.... MGE.... DHIT.M. GST.... SBC.... HKL.... MGO.... DSJT.V. GFR.... SKL.... HBC.... MVE.... DVAT.M. GNU.... SBC.... HSO.... MFL.... DATT.M. GFR.... SSO.... HBC.... MRO.... DIO..M. GNU.... SBC.... HKL.... MGE.... DSAT.M. GST.... SKL.... HBC.... MZU.... DHIT.M. GST.... SBC.... HSO.... MZU.... DD3..M. GBU.... SSO.... HBC.... MGE.... DBS..M. GST.... SBC.... HKL.... MGO.... DVRT.M. GFR.... SKL.... HBC.... MVE.... DSOT... GNU.... SBC.... HSOTD.. MFL.... DSJT.M. GFR.... SSO.... HBC.... MRO.... DZAT.M. GNU.... SBC.... HKL.... MGE.... DVIT... GST.... SKL.... HBC.... MZU.... DPRT... GST.... SBC.... HSO.... MZU.... DBRT.M. GBU.... SSO.... HBC.... MGE.... DHAT... GST.... SBC.... HKL.... MGO.... DNS..M. GFR.... SKL.... HBC.... MVE.... DEC..M. GNU.... SBC.... HSO.... MFL.... DPLT.M. GFR.... SSO.... HBC.... MRO.... DLOT.M. GNU.... SBC.... HKL.... MGE.... DSW.VM. GST.... SKL.... HBC.... MZU.... DLVT.M. GST.... SBC.... HSO.... MZU.... DMNT... GBU.... SSO.... HBC.... MGE.... DHIT... GST.... SBC.... HKL.... MGO.... DD2T.M. GFR.... SKL.... HBC.... MVE.... DLOT.M. GNU.... SBC.... HSO.... MFL.... DSWT.V. GFR.... SSO.... HBC.... MRO.... DLVT.M. GNU.... SBC.... HKL.... MGE.... DMNT.M. GST.... SKL.... HBC.... MZU.... DHIT.M. GBU.... SCD.... HBC.... MMR.... DD2T.M. GAM.... SGA.... HGA.... MMS.... DLOT.M. GNS.... SGA.... HCD.... MCG.... DSWT.M. GAO.... SCD.... HGA.... MMS.... DLV.VM. GGW.... SGA.... HGA.... MCG.... DMNT.M. GDU.... SGA.... HGA.... MMS.... DHIT.M. GGW.... SGA.... HCD.... MCG.... DD2T.M. GDU.... SCD.... HGA.... MMS.... DLOT.M. GGW.... SGA.... HGA.... MCG.... DSWT.M. GDU.... SGA.... HGA.... MMS.... DLVT.V. GGW.... SGA.... HCD.... MCG.... DMNT.M. GDU.... SCD.... HGA.... MMS.... DHIT.M. GGW.... SGA.... HGA.... MCG.... DD2T.M. GDU.... SGA.... HGA.... MMS.... DLOT.M. GGW.... SGA.... HCD.... MCG.... DSWT.M. GDU.... SCD.... HGA.... MMS.... DLVT.M. GGW.... SGA.... HGA.... MCG.... DMN.VM. GDU.... SGA.... HGA.... MMS.... DHIT.M. GGW.... SGA.... HCD.... MCG.... DD2T.M. GDU.... SCD.... HGA.... MMS.... DLOT.M. GGW.... SGA.... HGA.... MCG.... DSWT.M. GDU.... SGA.... HGA.... MMS.... DLVT.M. GGW.... SGA.... HCD.... MCG.... DMNT.V. GDU.... SCD.... HGA.... MMS.... DHIT.M. GGW.... SGA.... HGA.... MCG.... DD2T.M. GDU.... SGA.... HGA.... MMS.... DLOT.M. GGW.... SGA.... HCD.... MCG.... DSWT.M. GDU.... SCD.... HGA.... MMS.... DLVT.M. GDU.... SCD.... HBC.... MAL.... DMNT.M. GDU.... SCD.... HBE.... MGR.... DHI.VM. GDU.... SCD.... HBE.... MGR.... DD2T.M. GDU.... SCD.... HSA.... MGR.... DLOT.M. GDU.... SCD.... HSO.... MGR.... DSWT.M. GDU.... SCD.... HBE.... MGR.... DLVT.M. GDU.... SCD.... HBE.... MGR.... DMNT.M. GDU.... SCD.... HSA.... MGR.... DHIT.V. GDU.... SCD.... HSO.... MGR.... DD2T.M. GDU.... SCD.... HBE.... MGR.... DLOT.M. GDU.... SCD.... HBE.... MGR.... DSWT.M. GDU.... SCD.... HSA.... MGR.... DLVT.M. GDU.... SCD.... HSO.... MGR.... DMNT.M. GDU.... SCD.... HBE.... MGR.... DHIT.M. GDU.... SCD.... HBE.... MGR.... DD2.VM. GDU.... SCD.... HSA.... MGR.... DLOT.M. GDU.... SCD.... HSO.... MGR.... DSWT.M. GDU.... SCD.... HBE.... MGR.... DLVT.M. GDU.... SCD.... HBE.... MGR.... DMNT.M. GDU.... SCD.... HSA.... MAL.... DHIT.M. GIR.... SKL.... HIO.... MGR.... DD2T.V. GSWT... SBE.... HAS.... MAL.... DEDT.M. GIR.... SBE.... HAS.... MMA.... DNS.... GSW.... SSA.... HAS.... MAL.... DHAT.M. GSW.... SBE.... HAS.... MGR.... DLIT.M. GLV.... SBE.... HAS.... MAL.... DNUT.M. GIR.... SBE.... HAS.... MMA.... DPRT.M. GSW.... SSA.... HAS.... MAL.... DVI.VM. GSW.... SBE.... HAS.... MGR.... DZAT... GLV.... SBE.... HAS.... MAL.... DBDT.M. GIR.... SBE.... HAS.... MMA.... DHIT.M. GSW.... SSA.... HAS.... MAL.... DD1T.M. GSW.... SBE.... HAS.... MGR.... DKLT.M. GLV.... SBE.... HAS.... MAL.... DCDT.V. GIR.... SBE.... HAS.... MMA.... DGAT.M. GSW.... SSA.... HAS.... MAL.... DBCT.M. GSW.... SBE.... HAS.... MGR.... DBET.M. GLV.... SBETD.. HAS.... MAL.... DD1T.M. GIR.... SBETD.. HAS.... MMA.... DSZT.M. GLV....";
		
		Message messages[32] = {};
		GameView gv = GvNew(trail, messages);
		
		assert(GvGetHealth(gv, PLAYER_DR_SEWARD) == 0);
		//assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == MADRID);		

		GvFree(gv);

		printf("\033[32m" "Test Passed! :)" "\033[0m\n");

	}

	{///////////////////////////////////////////

		printf("Checking for the health of Dracula\n");

		char *trail =
			"GLS.... SBD.... HSJ.... MGE.... DSZ.V.. "
			"GSN.... SSZVD.. HSJ.... MPA.... DBET... "
			"GMA.... SSZ.... HBETD.. MLE.... DKLT... "
			"GCA.... SBE.... HBE.... MNA.... DCDT...";
		
		Message messages[32] = {};
		GameView gv = GvNew(trail, messages);
		
		assert(GvGetHealth(gv, PLAYER_VAN_HELSING) == (GAME_START_HUNTER_LIFE_POINTS - (LIFE_LOSS_TRAP_ENCOUNTER + LIFE_LOSS_DRACULA_ENCOUNTER) + LIFE_GAIN_REST));
		assert(GvGetPlayerLocation(gv, PLAYER_DRACULA) == CASTLE_DRACULA);
		assert(GvGetHealth(gv,PLAYER_DRACULA) == 30);		

		GvFree(gv);

		printf("\033[32m" "Test Passed! :)" "\033[0m\n");

	}

	{///////////////////////////////////////////

		printf("Checking for the trap location\n");

		char *trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT... "
			"GIR.... SPA.... HPR.... MKLT... DHIT... "
			"GAO.... SST.... HSZ.... MCDTTD. DGAT... "
			"GMS.... SFL.... HKL.... MSZ.... DCNT.V. "
			"GTS.... SRO.... HBC.... MCNTD.. DBS..M.";
		
		Message messages[32] = {};
		GameView gv = GvNew(trail, messages);
		
		int numTraps = 0;
		PlaceId *traps = GvGetTrapLocations(gv, &numTraps);
		assert(numTraps == 1);
		sortPlaces(traps, numTraps);
		assert(traps[0] == GALATZ);		

		GvFree(gv);

		printf("\033[32m" "Test Passed! :)" "\033[0m\n");

	}
	
	return EXIT_SUCCESS;
}
