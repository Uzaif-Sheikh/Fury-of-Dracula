PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
PlaceId *GetReachable = calloc(NUM_REAL_PLACES, sizeof(*GetReachable));
PlaceId *GetReachable_type = calloc(NUM_REAL_PLACES, sizeof(*GetReachable));

Map m = MapNew();

ConnList node = MapGetConnections(m, from);
int j = 0;
int count = 0;

GetReachable[count] = from;
count++;

if (rail == true) {
int k = round + player;
int l = k % 4;
PlaceId *top = GetReachable;
int i = count;
int t = 0;
int a = 0;
while (a < l) {
while (t < i) {
node = MapGetConnections(m, *top);
while (node != NULL) {
if (node->type == RAIL) {
GetReachable[count] = node->p;
count++;
}
node = node->next;
}
printf("%s %d\n", placeIdToName(*top), count);
top++;
t++;
}
t = i;
i = count;
printf("\n");
a++;
}
}

if (road == true) {
node = MapGetConnections(m, from);
while (node != NULL) {
if (node->type == ROAD) {
GetReachable[count] = node->p;
printf("%s   ", placeIdToName(GetReachable[count]));
count++;
}
node = node->next;
}
}

if (boat == true) {
node = MapGetConnections(m, from);
while (node != NULL) {
if (node->type == BOAT) {
GetReachable[count] = node->p;
printf("%s   ", placeIdToName(GetReachable[count]));
count++;
}
node = node->next;
}
printf("\n");
}



sortPlaces(GetReachable, count);
for (int i = 0; i < count; i++) {
if (i != 0) {
if (GetReachable[i] != GetReachable[i-1]) {
//printf("%s\n", placeIdToName(GetReachable[i]));
GetReachable_type[j] = GetReachable[i];
j++;
}
} else {
GetReachable_type[j] = GetReachable[i];
j++;
}
}


free(GetReachable);

*numReturnedLocs = j;
return GetReachable_type;
}

