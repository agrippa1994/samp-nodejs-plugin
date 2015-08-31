#include <a_samp>
#include <core>
#include <float>

#pragma tabsize 0

main()
{
	print("\n----------------------------------");
	print("  Bare Script\n");
	print("----------------------------------\n");
}

public OnPlayerSpawn(playerid)
{
	SetPlayerInterior(playerid,0);
	TogglePlayerClock(playerid,0);
	return 1;
}

SetupPlayerForClassSelection(playerid)
{
 	SetPlayerInterior(playerid,14);
	SetPlayerPos(playerid,258.4893,-41.4008,1002.0234);
	SetPlayerFacingAngle(playerid, 270.0);
	SetPlayerCameraPos(playerid,256.0815,-43.0475,1004.0234);
	SetPlayerCameraLookAt(playerid,258.4893,-41.4008,1002.0234);
}

public OnPlayerRequestClass(playerid, classid)
{
	SetupPlayerForClassSelection(playerid);
	return 1;
}

forward Timer();
public Timer() {
	printf("AddIntsInJS returned: %d", CallLocalFunction("AddIntsInJS", "dd", 5, 30));
	printf("AddIntsInJS returned: %d", CallLocalFunction("AddIntsInJS", "dd", 5, 30));
	
	printf("Strlen in JS %d", CallLocalFunction("StrlenInJS", "s", "This is a test string"));
}
public OnGameModeInit()
{
	printf("ONGAMEMODEINIT!");
 	SetTimerEx("Timer", 250, true, "df", 5, 12.5);

	SetGameModeText("Bare Script");
	ShowPlayerMarkers(1);
	ShowNameTags(1);
	AllowAdminTeleport(1);

	AddPlayerClass(265,1958.3783,1343.1572,15.3746,270.1425,0,0,0,0,-1,-1);
	return 1;
}

