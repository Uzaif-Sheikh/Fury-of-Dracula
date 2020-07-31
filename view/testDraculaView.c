////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testDraculaView.c: test the DraculaView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-02	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DraculaView.h"
#include "Game.h"
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for basic functions, "
			   "just before Dracula's first move\n");

		char *trail =
			"GST.... SAO.... HZU.... MBB....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "..."
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 0);
		assert(DvGetScore(dv) == GAME_START_SCORE);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == ZURICH);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 0);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for encountering Dracula\n");

		char *trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD.. SAO.... HCD.... MAO....";
		
		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahah",
			"Aha!", "", "", ""
		};
		
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 1);
		assert(DvGetScore(dv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 5);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GENEVA);
		assert(DvGetVampireLocation(dv) == NOWHERE);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula leaving minions 1\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DED.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GST.... SST.... HST.... MST....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 4);
		assert(DvGetVampireLocation(dv) == EDINBURGH);
		int numTraps = -1;
		PlaceId *traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 3);
		sortPlaces(traps, numTraps);
		assert(traps[0] == LONDON);
		assert(traps[1] == LONDON);
		assert(traps[2] == MANCHESTER);
		free(traps);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves in round 0, should return a NULL array\n");

		char *trail =
			"GGE.... SGE.... HGE.... MGE....";
			
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		assert (numMoves == 0);
		assert (moves == NULL);
		printf("Test passed!\n");
		DvFree(dv);
	}
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves with one hide and the possible locations that can be visited in the next turn.\n");

		char *trail = 
			"GST.... SGE.... HGE.... MGE.... DPL.V.."
			"GST.... SST.... HST.... MST.... DEC...." 
			"GST.... SST.... HST.... MST.... DLET..."
			"GST.... SST.... HST.... MST.... DHIT..." 
			"GST.... SST.... HST.... MST....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);

		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		
		assert(numMoves == 6);
		sortPlaces(moves, numMoves);

		assert(moves[0] == BRUSSELS);
		assert(moves[1] == NANTES);
		assert(moves[2] == PARIS);
		assert(moves[3] == DOUBLE_BACK_1);
		assert(moves[4] == DOUBLE_BACK_2);
		assert(moves[5] == DOUBLE_BACK_3);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		sortPlaces(locs, numLocs);

		assert (numLocs == 5);
		assert(locs[0] == BRUSSELS);
		assert(locs[1] == ENGLISH_CHANNEL);
		assert(locs[2] == LE_HAVRE);
		assert(locs[3] == NANTES);
		assert(locs[4] == PARIS);
		
		free(locs);
		free(moves);
		printf("Test passed!\n");
		DvFree(dv);
	}
	
	// {///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves with one hide and double back in the trail\n");
		
		char *trail ="GNA.... SAO.... HEC.... MAO.... DMNT... "
		"GBO.... SIR.... HLE.... MEC.... DD1T... "
		"GSR.... SDU.... HBU.... MPL.... DHIT... "
		"GSN.... SIR.... HAM.... MLO.... DTPT... "
		"GAL.... SAO.... HCO.... MEC.... DCDT... "
		"GMS.... SMS.... HFR.... MLE.... DKLT.V. ";
			
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		
		assert(numMoves == 5);
		sortPlaces(moves, numMoves);

		assert(moves[0] == BELGRADE);
		assert(moves[1] == BUCHAREST);
		assert(moves[2] == BUDAPEST);
		assert(moves[3] == GALATZ);
		assert(moves[4] == SZEGED);
		free(moves);
		
		printf("Test passed!\n");
		DvFree(dv);
	// }

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's valid moves after round 1 at Castle Dracula\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);

		assert(numMoves == 4);
		sortPlaces(moves, numMoves);
		assert(moves[0] == GALATZ);
		assert(moves[1] == KLAUSENBURG);
		assert(moves[2] == HIDE);
		assert(moves[3] == DOUBLE_BACK_1);
		free(moves);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for Dracula's Teleport condition should return a NULL array\n");
		
		char *trail = "GMS.... SIO.... HTS.... MIO.... DAO..M. "
		"GAO.... STS.... HMS.... MTS.... DNS.... "
		"GBB.... SMS.... HAO.... MMS.... DED.V.. "
		"GNA.... SAO.... HEC.... MAO.... DMNT... "
		"GBO.... SIR.... HLE.... MEC.... DD2T... "
		"GSR.... SDU.... HBU.... MPL.... DHIT... ";
		
		Message messages[9] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		//for (int i = 0; i < )
		
		assert(numMoves == 0);
		assert (moves == NULL);
		free(moves);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	
	
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanIGo with Hide and double Back moves\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == CONSTANTA);
		assert(locs[2] == GALATZ);
		assert(locs[3] == SOFIA);
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanIGo with the valid moves on the next turn" 
		"when Hospital place is adjacent and Hide move is available\n");
		
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DBET... "
			"GGE.... SGE.... HGE.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		
		sortPlaces(moves, numMoves);
		assert (numMoves == 4);
		assert(moves[0] == SARAJEVO);
		assert(moves[1] == SOFIA);
		assert(moves[2] == SZEGED);
		assert(moves[3] == HIDE);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGo(dv, &numLocs);
		
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);

		assert(locs[0] == BELGRADE);
		assert(locs[1] == SARAJEVO);
		assert(locs[2] == SOFIA);
		assert(locs[3] == SZEGED);
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	{///////////////////////////////////////////////////////////////////
	
		printf("Test for DvWhereCanIGobytype with the set of moves possible and the possible connections with road\n");
		
		char *trail = "GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
		"GLO.... SAL.... HCO.... MBR.... DBET... "
		"GED.... SBO.... HLI.... MPR.... DKLT... "
		"GLV.... SNA.... HNU.... MBD.... DCDT... "
		"GIR.... SPA.... HPR.... MKLT... DHIT... "
		"GAO.... SST.... HSZ.... MCDTTD. DGAT... "
		"GMS.... SFL.... HKL.... MSZ.... DCNT.V. "
		"GTS.... SRO.... HBC.... MCNTD.. DBS..M. "
		"GIO.... SBI.... HCN.... MCN.... DIO.... "
		"GIO.... SAS.... HBS.... MCN.... DTS.... "
		"GTS.... SAS.... HIO.... MBS.... DMS.... "
		"GMS.... SIO.... HTS.... MIO.... DAO..M. "
		"GAO.... STS.... HMS.... MTS.... DNS.... "
		"GBB.... SMS.... HAO.... MMS.... DED.V.. "
		"GNA.... SAO.... HEC.... MAO.... DMNT... "
		"GBO.... SIR.... HLE.... MEC.... DD2T... "
		"GSR.... SDU.... HBU.... MPL.... DHIT... "
		"GSN.... SIR.... HAM.... MLO.... DTPT... "
		"GAL.... SAO.... HCO.... MEC.... DCDT... "
		"GMS.... SMS.... HFR.... MLE.... DKLT.V. "
		"GTS.... STS.... HBR.... MCO.... DGAT.M. "
		"GIO.... SIO.... HBD.... MLI.... DD3T.M. "
		"GBS.... SBS.... HKLT... MBR.... DHI..M. "
		"GCN.... SCN.... HCDTTTD MVI.... DTPT... "
		"GGAT... SGA.... HSZ.... MBC.... DCDT... "
		"GCDTTD. SCDD... HKL.... MGA.... DKLT... "
		"GSZ.... SKLTD.. HKLD... MKLD... DBC.V.. "
		"GBD.... SBE.... HGA.... MBCVD.. DSOT... "
		"GSZ.... SSOTD.. HBC.... MSOD...";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numMoves = -1;
		PlaceId *moves = DvGetValidMoves(dv, &numMoves);
		
		assert(numMoves == 7);
		sortPlaces(moves, numMoves);

		assert(moves[0] == BELGRADE);
		assert(moves[1] == SALONICA);
		assert(moves[2] == SARAJEVO);
		assert(moves[3] == VALONA);
		assert(moves[4] == VARNA);
		assert(moves[5] == DOUBLE_BACK_1);
		assert(moves[6] == DOUBLE_BACK_2);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, true, false, &numLocs);
		
		assert(numLocs == 7);
		sortPlaces(locs, numLocs);

		assert(locs[0] == BELGRADE);
		assert(locs[1] == BUCHAREST);
		assert(locs[2] == SALONICA);
		assert(locs[3] == SARAJEVO);
		assert(locs[4] == SOFIA);
		assert(locs[5] == VALONA);
		assert(locs[6] == VARNA);
		
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}

	{	
		printf("Test for DvWhereCanIGobytype with the set of moves possible and the possible connections with boat,"
		"Since, it has no possible moves so it returns NULL.\n");
    	
		char *trail ="GMS.... SIO.... HTS.... MIO.... DAO..M. "
		"GAO.... STS.... HMS.... MTS.... DNS.... "
		"GBB.... SMS.... HAO.... MMS.... DED.V.. "
		"GNA.... SAO.... HEC.... MAO.... DMNT... "
		"GBO.... SIR.... HLE.... MEC.... DD2T... "
		"GSR.... SDU.... HBU.... MPL.... DHIT... ";

		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		
		int numLocs = -1;
		PlaceId *locs = DvWhereCanIGoByType(dv, false, true, &numLocs);
		assert(numLocs == 0);
		assert (locs == NULL);
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	{	
		char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HIO.... MGE....";
		
		Message messages[24] = {};
		DraculaView dv = DvNew(trail, messages);
		int numLocs = -1;
		PlaceId *locs = DvWhereCanTheyGo(dv, PLAYER_LORD_GODALMING, &numLocs);

		assert(numLocs == 6);
	    sortPlaces(locs, numLocs);
		assert(locs[0] == CLERMONT_FERRAND);
		assert(locs[1] == GENEVA);
		assert(locs[2] == MARSEILLES);
		assert(locs[3] == PARIS);
		assert(locs[4] == STRASBOURG);
		assert(locs[5] == ZURICH);
		free(locs);
		
		printf("Test passed!\n");
		DvFree(dv);
	}
	
	{
			printf("Checking Ionian Sea boat connections "
			       "(Lord Godalming, Round 1)\n");
			char *trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HIO.... MGE....";
			Message messages[24] = {};
			DraculaView dv = DvNew(trail, messages);
			int numLocs = -1;
			PlaceId *locs = DvWhereCanTheyGo(dv, PLAYER_VAN_HELSING, &numLocs);
			
			sortPlaces(locs, numLocs);
			assert(numLocs == 7);
			
			assert(locs[0] == ADRIATIC_SEA);
			assert(locs[1] == ATHENS);
			assert(locs[2] == BLACK_SEA);
			assert(locs[3] == IONIAN_SEA);
			assert(locs[4] == SALONICA);
			assert(locs[5] == TYRRHENIAN_SEA);
			assert(locs[6] == VALONA);
			free(locs);
			printf("Test passed!\n");
			DvFree(dv);
	}

	{		printf("Testing connections\n");
			char *trail =
				"GTS.... SRO.... HBC.... MCNTD.. DBS..M. "
				"GIO.... SBI.... HCN.... MCN.... DIO.... "
				"GIO.... SAS.... HBS.... MCN.... DTS.... "
				"GTS.... SAS.... HIO.... MBS.... DMS.... "
				"GMS.... SIO.... HTS.... MIO.... DAO..M. ";
			
		
			Message messages[32] = {};
			DraculaView dv = DvNew(trail, messages);
			
			printf("\nTesting move/location history\n\n");
			int numLocs = 0; 
			PlaceId* LastMoves =  DvWhereCanTheyGoByType(dv, PLAYER_VAN_HELSING, true, true, false, &numLocs);
			printf ("%d", numLocs);
			for (int i = 0; i < numLocs; i++) {
				printf ("%s\n", placeIdToName(LastMoves[i]));
			}

			free(LastMoves);
			DvFree(dv);
			printf("Test passed!\n");
			
	}
	return EXIT_SUCCESS;
}