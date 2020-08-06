////////////////////////////////////////////////////////////////////////
// Runs a player's game turn ...
//
// Can  produce  either a hunter player or a Dracula player depending on
// the setting of the I_AM_DRACULA #define
//
// This  is  a  dummy  version of the real player.c used when you submit
// your AIs. It is provided so that you can test whether  your  code  is
// likely to compile ...
//
// Note that this is used to drive both hunter and Dracula AIs. It first
// creates an appropriate view, and then invokes the relevant decideMove
// function,  which  should  use the registerBestPlay() function to send
// the move back.
//
// The real player.c applies a timeout, and will halt your  AI  after  a
// fixed  amount of time if it doesn 't finish first. The last move that
// your AI registers (using the registerBestPlay() function) will be the
// one used by the game engine. This version of player.c won't stop your
// decideMove function if it goes into an infinite loop. Sort  that  out
// before you submit.
//
// Based on the program by David Collien, written in 2012
//
// 2017-12-04	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v1.2	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v1.3	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#ifdef I_AM_DRACULA
# include "dracula.h"
# include "DraculaView.h"
# include "GameView.h"
#else
# include "hunter.h"
# include "HunterView.h"

#endif

// Moves given by registerBestPlay are this long (including terminator)
#define MOVE_SIZE 3

// The minimum static globals I can get away with
static char latestPlay[MOVE_SIZE] = "";
static char latestMessage[MESSAGE_SIZE] = "";

// A pseudo-generic interface, which defines
// - a type `View',
// - functions `ViewNew', `decideMove', `ViewFree',
// - a trail `xtrail', and a message buffer `xmsgs'.
#ifdef I_AM_DRACULA

typedef DraculaView View;

# define ViewNew DvNew
# define decideMove decideDraculaMove
# define ViewFree DvFree

# define xPastPlays 
//"GMA.... SMN.... HBC.... MPA.... DST.V.. GAL.... SED.... HBE.... MBU.... DZUT... GAL.... SED.... HBE.... MAM.... DMIT... GAL.... SED.... HBE.... MAM.... DVET... GAL.... SED.... HBE.... MAM.... DGOT... GAL.... SED.... HBE.... MAM.... DTS.... GAL.... SNS.... HSJ.... MBU.... DROT.V. GBO.... SAM.... HZA.... MST.... DNPT.M. GCF.... SBU.... HMU.... MCO.... DBIT.M. GCF.... SBU.... HMU.... MCO.... DAS..M. GGE.... SMR.... HVE.... MFR.... DIO..M. GFL.... SGO.... HMI.... MBR.... DVAT... GFL.... SGO.... HMI.... MBR.... DSOT.M. GRO.... SFL.... HNPT... MLI.... DVR.V.. GBIT... SRO.... HBI.... MFR...."

//"GMN.... SVR.... HGR.... MBR.... DST.V.. GLO.... SBE.... HAL.... MPR.... DZUT... GEC.... SBD.... HSR.... MVI.... DMIT... GLE.... SZA.... HTO.... MBD.... DGOT... GBU.... SMU.... HBO.... MSO.... DFLT... GSTV... SZA.... HBU.... MSJ.... DROT... GZUT... SMU.... HST.... MZA.... DTS.... GGE.... SMU.... HST.... MMU.... DNPT... GGE.... SMU.... HPA.... MVI.... DBIT.M. GMR.... SMU.... HGE.... MMU.... DAS..M. GMS.... SMU.... HGE.... MMU.... DIO..M. GMR.... SMU.... HGE.... MMU.... DVAT.M. GTO.... SVI.... HMI.... MFR.... DSOT... GMR.... SMU.... HMU.... MST.... DVR.VM. GGE.... SMI.... HZU.... MST.... DCNT.M. GPA.... SMI.... HMI.... MLI...."

//"GST.... SSR.... HBC.... MCD.... DST.V.. GNU.... SPA.... HVI.... MKL.... DZUT... GSTV... SST.... HMU.... MBD.... DMRT... GGE.... SNU.... HST.... MPR.... DTOT... GST.... SST.... HBU.... MFR.... DSRT... GNU.... SFR.... HST.... MST.... DSNT... GST.... SST.... HZUT... MZU.... DMAT... GZU.... SZU.... HMU.... MMI.... DLST... GZU.... SZU.... HMU.... MMI.... DCAT.M. GZU.... SMR.... HMI.... MMR.... DGRT.M. GZU.... SMR.... HMI.... MMR.... DHIT.M. GZU.... SMS.... HMR.... MBO.... DD2T.M. GZU.... SMS.... HMR.... MBO.... DALT.M. GZU.... SALTD.. HMS.... MSR.... DSR.VM. GMR.... SSRVD.. HAL.... MTO.... DSNT.M. GSR.... SZA.... HSR.... MSR.... DBB..M. GAL.... SMU.... HAL.... MTO.... DAO..M. GSR.... SMI.... HSR.... MSR.... DNS..M. GSNT... SMR.... HSN.... MSN.... DHAT... GSR.... SMS.... HMA.... MLS.... DLIT... GSN.... SAL.... HSN.... MSN.... DNUT... GSN.... SAL.... HSN.... MSN.... DPRT... GSN.... SLS.... HBB.... MBB.... DVIT... GSN.... SLS.... HBB.... MBB.... DZAT... GSN.... SAO.... HAO.... MAO.... DSJT.M. GSN.... SAO.... HAO.... MAO.... DBET.M. GSN.... SNS.... HNS.... MNS.... DSO.VM. GSN.... SNS.... HNS.... MNS.... DVRT.M. GSN.... SHA.... HHA.... MHA.... DBS..M. GSN.... SHA.... HHA.... MHA.... DIO..M. GSN.... SVI.... HBR.... MBR.... DVAT.M. GSN.... SVI.... HBR.... MBR.... DSAT.M. GSN.... SBD.... HVI.... MBD.... DHIT.V. GSN.... SBD.... HVI.... MBD.... DD3T.M. GSN.... SCN.... HBD.... MSZ.... DATT... GSN.... SCN.... HBD.... MSZ...."

//"GST.... SSR.... HBC.... MCD.... DLV.V.."
//"GST.... SSR.... HBC.... MCD.... DLV.V.. GBU.... SAL.... HBE.... MGA.... DSWT..." 
//"GLS.... SBD.... HSJ.... MGE.... DSZ.V.. GSN.... SSZVD.. HSJ.... MPA.... DBET... GMA.... SSZ.... HBETD.. MLE.... DKLT... GCA.... SBE.... HBE.... MNA.... DCDT..."
//"GLV.... SSO.... HIO.... MIR.... DST.V.. GLV.... SSO.... HIO.... MIR.... DZUT... GLV.... SSO.... HIO.... MIR.... DMUT... GLV.... SSO.... HIO.... MIR.... DZAT... GLV.... SSO.... HIO.... MIR.... DVIT... GLV.... SSO.... HIO.... MIR.... DBDT... GLV.... SSO.... HIO.... MIR.... DKLT.V. GLV.... SSO.... HIO.... MIR.... DCDT.M. GLV.... SSO.... HIO.... MIR.... DGAT.M. GLV.... SSO.... HIO.... MIR.... DHIT.M. GLV.... SSO.... HIO.... MIR.... DD4T.M. GLV.... SSO.... HIO.... MIR.... DSZT.M. GLV.... SSO.... HIO.... MIR.... DZAT.M. GLV.... SSO.... HIO.... MIR.... DVI.VM. GLV.... SSO.... HIO.... MIR.... DPRT.M. GLV.... SSO.... HIO.... MIR.... DNUT.M. GLV.... SSO.... HIO.... MIR.... DSTT.M. GLV.... SSO.... HIO.... MIR.... DZUT.M. GLV.... SSO.... HIO.... MIR.... DMUT.M. GLV.... SSO.... HIO.... MIR.... DZAT.V. GLV.... SSO.... HIO.... MIR.... DBDT.M. GLV.... SSO.... HIO.... MIR.... DKLT.M. GLV.... SSO.... HIO.... MIR.... DCDT.M. GLV.... SSO.... HIO.... MIR.... DGAT.M. GLV.... SSO.... HIO.... MIR.... DHIT.M. GLV.... SSO.... HIO.... MIR.... DD4T.M. GLV.... SSO.... HIO.... MIR.... DSZ.VM. GLV.... SSO.... HIO.... MIR.... DZAT.M. GLV.... SSO.... HIO.... MIR.... DMUT.M. GLV.... SSO.... HIO.... MIR.... DVIT.M. GLV.... SSO.... HIO.... MIR.... DBDT.M. GLV.... SSO.... HIO.... MIR.... DKLT.M. GLV.... SSO.... HIO.... MIR.... DGAT.V. GLV.... SSO.... HIO.... MIR.... DCDT.M. GLV.... SSO.... HIO.... MIR.... DHIT.M. GLV.... SSO.... HIO.... MIR.... DD3T.M. GLV.... SSO.... HIO.... MIR.... DCNT.M. GLV.... SSO.... HIO.... MIR.... DVRT.M. GLV.... SSO.... HIO.... MIR.... DSOT.M. GLV.... SSOTD.. HIO.... MIR.... DVA.VM. GLV.... SSO.... HIO.... MIR.... DIO..M. GLV.... SSO.... HIO.... MIR...."
//"GMN.... SVR.... HGR.... MBR.... DST.V.. GLO.... SBE.... HAL.... MPR.... DZUT... GEC.... SBD.... HSR.... MVI.... DMIT... GLE.... SZA.... HTO.... MBD.... DGOT... GBU.... SMU.... HBO.... MSO.... DFLT... GSTV... SZA.... HBU.... MSJ.... DROT... GZUT... SMU.... HST.... MZA.... DTS.... GGE.... SMU.... HST.... MMU.... DNPT... GGE.... SMU.... HPA.... MVI.... DBIT.M. GMR.... SMU.... HGE.... MMU.... DAS..M. GMS.... SMU.... HGE.... MMU.... DIO..M. GMR.... SMU.... HGE.... MMU.... DVAT.M. GTO.... SVI.... HMI.... MFR.... DSOT... GMR.... SMU.... HMU.... MST.... DVR.VM. GGE.... SMI.... HZU.... MST.... DCNT.M. GPA.... SMI.... HMI.... MLI...."
//"GZA.... SED.... HZU.... MZU...." 
//"GST.... SSR.... HBC.... MCD.... DST.V.. GNU.... SPA.... HVI.... MKL.... DZUT... GSTV... SST.... HMU.... MBD.... DMRT... GGE.... SNU.... HST.... MPR.... DTOT... GST.... SST.... HBU.... MFR.... DSRT... GNU.... SFR.... HST.... MST.... DSNT... GST.... SST.... HZUT... MZU.... DMAT... GZU.... SZU.... HMU.... MMI.... DLST... GZU.... SZU.... HMU.... MMI.... DCAT.M. GZU.... SMR.... HMI.... MMR.... DGRT.M. GZU.... SMR.... HMI.... MMR.... DHIT.M. GZU.... SMS.... HMR.... MBO.... DD2T.M. GZU.... SMS.... HMR.... MBO.... DALT.M. GZU.... SALTD.. HMS.... MSR.... DSR.VM. GMR.... SSRVD.. HAL.... MTO.... DSNT.M. GSR.... SZA.... HSR.... MSR.... DBB..M. GAL.... SMU.... HAL.... MTO.... DAO..M. GSR.... SMI.... HSR.... MSR.... DNS..M. GSNT... SMR.... HSN.... MSN.... DHAT... GSR.... SMS.... HMA.... MLS.... DLIT... GSN.... SAL.... HSN.... MSN.... DNUT... GSN.... SAL.... HSN.... MSN.... DPRT... GSN.... SLS.... HBB.... MBB.... DVIT... GSN.... SLS.... HBB.... MBB.... DZAT... GSN.... SAO.... HAO.... MAO.... DSJT.M. GSN.... SAO.... HAO.... MAO.... DBET.M. GSN.... SNS.... HNS.... MNS.... DSO.VM. GSN.... SNS.... HNS.... MNS.... DVRT.M. GSN.... SHA.... HHA.... MHA.... DBS..M. GSN.... SHA.... HHA.... MHA.... DIO..M. GSN.... SVI.... HBR.... MBR.... DVAT.M. GSN.... SVI.... HBR.... MBR.... DSAT.M. GSN.... SBD.... HVI.... MBD.... DHIT.V. GSN.... SBD.... HVI.... MBD.... DD3T.M. GSN.... SCN.... HBD.... MSZ.... DATT... GSN.... SCN.... HBD.... MSZ...."
//"GMA.... SMN.... HBC.... MPA.... DST.V.. GAL.... SED.... HBE.... MBU.... DZUT... GAL.... SED.... HBE.... MAM.... DMIT... GAL.... SED.... HBE.... MAM.... DVET... GAL.... SED.... HBE.... MAM.... DGOT... GAL.... SED.... HBE.... MAM.... DTS.... GAL.... SNS.... HSJ.... MBU.... DROT.V. GBO.... SAM.... HZA.... MST.... DNPT.M. GCF.... SBU.... HMU.... MCO.... DBIT.M. GCF.... SBU.... HMU.... MCO.... DAS..M. GGE.... SMR.... HVE.... MFR.... DIO..M. GFL.... SGO.... HMI.... MBR.... DVAT... GFL.... SGO.... HMI.... MBR.... DSOT.M. GRO.... SFL.... HNPT... MLI.... DVR.V.. GBIT... SRO.... HBI.... MFR...."




// "GSW.... SLS.... HMR.... MHA.... DSJ.V.. GLO.... SAL.... HCO.... MBR.... DBET... GED.... SBO.... HLI.... MPR.... DKLT... GLV.... SNA.... HNU.... MBD.... DCDT... GIR.... SPA.... HPR.... MKLT... DHIT... GAO.... SST.... HSZ.... MCDTTD. DGAT... GMS.... SFL.... HKL.... MSZ.... DCNT.V. GTS.... SRO.... HBC.... MCNTD.. DBS..M. GIO.... SBI.... HCN.... MCN.... DIO.... GIO.... SAS.... HBS.... MCN.... DTS.... GTS.... SAS.... HIO.... MBS.... DMS.... GMS.... SIO.... HTS.... MIO.... DAO..M. GAO.... STS.... HMS.... MTS.... DNS.... GBB.... SMS.... HAO.... MMS.... DED.V.. GNA.... SAO.... HEC.... MAO.... DMNT... GBO.... SIR.... HLE.... MEC.... DD2T... GSR.... SDU.... HBU.... MPL.... DHIT... GSN.... SIR.... HAM.... MLO.... DTPT... GAL.... SAO.... HCO.... MEC.... DCDT... GMS.... SMS.... HFR.... MLE.... DKLT.V. GTS.... STS.... HBR.... MCO.... DGAT.M. GIO.... SIO.... HBD.... MLI.... DD3T.M. GBS.... SBS.... HKLT... MBR.... DHI..M. GCN.... SCN.... HCDTTTD MVI.... DTPT... GGAT... SGA.... HSZ.... MBC.... DCDT... GCDTTD. SCDD... HKL.... MGA.... DKLT... GSZ.... SKLTD.. HKLD... MKLD... DBC.V.. GBD.... SBE.... HGA.... MBCVD.. DSOT... GSZ.... SSOTD.. HBC.... MSOD..."
# define xMsgs { "", "", "", "" }

#else

typedef HunterView View;

# define ViewNew HvNew
# define decideMove decideHunterMove
# define ViewFree HvFree

# define xPastPlays "GZA.... SED.... HZU.... MZU.... DLO...."
# define xMsgs { "", "", "" }

#endif

int main(void)
{
	char *pastPlays = xPastPlays;
	Message msgs[] = xMsgs;

	View state = ViewNew(pastPlays, msgs);
	// int numMoves = -1;
	// PlaceId* Valid_moves = DvGetValidMoves(state, &numMoves);
	
	// for (int i = 0; i < numMoves; i++) {
	// 	printf ("%s\n", placeIdToAbbrev(Valid_moves[i]));
	// }
	
	decideMove(state);
	ViewFree(state);

	printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
	return EXIT_SUCCESS;
}

// Saves characters from play (and appends a terminator)
// and saves characters from message (and appends a terminator)
void registerBestPlay(char *play, Message message)
{
	strncpy(latestPlay, play, MOVE_SIZE - 1);
	latestPlay[MOVE_SIZE - 1] = '\0';

	strncpy(latestMessage, message, MESSAGE_SIZE - 1);
	latestMessage[MESSAGE_SIZE - 1] = '\0';
}
