#include <a_samp>
#include <physics>
#include <zcmd>
#include <sscanf>

#define OBJ_SLOT_POOL (0)

enum poolBall
{
	bObject,
	bExisting
}

new
	PlayingPool[MAX_PLAYERS],
	PoolCamera[MAX_PLAYERS],
	UsingChalk[MAX_PLAYERS],
	PoolScore[MAX_PLAYERS],
	Float:AimAngle[MAX_PLAYERS][2],
	AimObject,
	PoolStarted,
	PoolAimer = -1,
	PoolLastShooter = -1,
	PoolLastScore,
	PoolBall[16][poolBall],
	Text:PoolTD[4],
	Float:PoolPower,
	PoolDir;

main() { }

public OnGameModeInit()
{
	SetGameModeText("Objects Physics - Pool Demo");
	AddPlayerClass(0, 2442.1621,2059.5051,10.8203, 269.1425, 0, 0, 0, 0, 0, 0);

	LoadPool();

	return 1;
}

public OnGameModeExit()
{
	return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
	SetPlayerPos(playerid, 1958.3783, 1343.1572, 15.3746);
	SetPlayerCameraPos(playerid, 1958.3783, 1343.1572, 15.3746);
	SetPlayerCameraLookAt(playerid, 1958.3783, 1343.1572, 15.3746);
	return 1;
}

command(play, playerid, params[])
{
	if(!PlayingPool[playerid])
	{
		PlayingPool[playerid] = 1;
		PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
		GivePlayerWeapon(playerid, 7, 1);
		PoolScore[playerid] = 0;
		if(!PoolStarted)
		{
			PoolStarted = 1;
			RespawnPoolBalls(1);
		}
	}
	else
	{
		if(PoolAimer != playerid)
		{
			PlayingPool[playerid] = 0;
			new
				count = GetPoolPlayersCount();
			if(count <= 0)
			{
				PoolStarted = 0;
				RespawnPoolBalls();
			}
		}
	}
	return 1;
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
	if(PoolStarted && PlayingPool[playerid])
	{
		if (IsKeyJustUp(KEY_SECONDARY_ATTACK, newkeys, oldkeys))
		{
			if(PlayingPool[playerid] && PoolAimer != playerid && !UsingChalk[playerid])
			{
				SetTimerEx("PlayPoolSound", 1400, 0, "d", 31807);
				SetPlayerArmedWeapon(playerid, 0);
				SetPlayerAttachedObject(playerid, OBJ_SLOT_POOL, 338, 6, 0, 0.07, -0.85, 0, 0, 0);
				ApplyAnimation(playerid, "POOL", "POOL_ChalkCue",3.0,0,0,0,0,0,1);
				UsingChalk[playerid] = 1;
				SetTimerEx("RestoreWeapon", 3500, 0, "d", playerid);
			}
		}
		if (IsKeyJustUp(KEY_JUMP, newkeys, oldkeys))
		{
			if(PoolAimer == playerid)
			{
				if(PoolCamera[playerid] < 2) PoolCamera[playerid]++;
				else PoolCamera[playerid] = 0;
				new
					Float:poolrot = AimAngle[playerid][0],
					Float:Xa,
					Float:Ya,
					Float:Za,
					Float:x,
					Float:y;
				GetObjectPos(PoolBall[0][bObject], Xa, Ya, Za);
				switch(PoolCamera[playerid])
				{
					case 0:
					{
						GetXYBehindObjectInAngle(PoolBall[0][bObject], poolrot, x, y, 0.675);
						SetPlayerCameraPos(playerid, x, y, 998.86785888672+0.28);
						SetPlayerCameraLookAt(playerid, Xa, Ya, Za+0.170);
					}
					case 1:
					{
						SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
						SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
					}
					case 2:
					{
						SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
						SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
					}
				}
			}
		}
		if (IsKeyJustUp(KEY_HANDBRAKE, newkeys, oldkeys))
		{
			if(AreAllBallsStopped())
			{
				if(PoolAimer != playerid)
				{
					if(!UsingChalk[playerid] && PoolAimer == -1 && PoolBall[0][bExisting])
					{
						new
							Float:poolrot,
							Float:X,
							Float:Y,
							Float:Z,
							Float:Xa,
							Float:Ya,
							Float:Za,
							Float:x,
							Float:y;
						GetPlayerPos(playerid, X, Y, Z);
						GetObjectPos(PoolBall[0][bObject], Xa, Ya, Za);
						if(Is2DPointInRangeOfPoint(X, Y, Xa, Ya, 1.5) && Z < 999.5)
						{
							TogglePlayerControllable(playerid, 0);
							GetAngleToXY(Xa, Ya, X, Y, poolrot);
							SetPlayerFacingAngle(playerid, poolrot);
							AimAngle[playerid][0] = poolrot;
							AimAngle[playerid][1] = poolrot;
							SetPlayerArmedWeapon(playerid, 0);
							GetXYInFrontOfPos(Xa, Ya, poolrot+180, x, y, 0.085);
							AimObject = CreateObject(3004, x, y, Za, 7.0, 0, poolrot+180);
							switch(PoolCamera[playerid])
							{
								case 0:
								{
									GetXYBehindObjectInAngle(PoolBall[0][bObject], poolrot, x, y, 0.675);
									SetPlayerCameraPos(playerid, x, y, 998.86785888672+0.28);
									SetPlayerCameraLookAt(playerid, Xa, Ya, Za+0.170);
								}
								case 1:
								{
									SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
									SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
								}
								case 2:
								{
									SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
									SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
								}
							}
							ApplyAnimation(playerid, "POOL", "POOL_Med_Start",50.0,0,0,0,1,1,1);
							PoolAimer = playerid;
							TextDrawShowForPlayer(playerid, PoolTD[0]);
							TextDrawShowForPlayer(playerid, PoolTD[1]);
							TextDrawTextSize(PoolTD[2], 501.0, 0.0);
							TextDrawShowForPlayer(playerid, PoolTD[2]);
							TextDrawShowForPlayer(playerid, PoolTD[3]);
							PoolPower = 1.0;
							PoolDir = 0;
						}
					}
				}
				else
				{
					TogglePlayerControllable(playerid, 1);
					GivePlayerWeapon(playerid, 7, 1);
					ApplyAnimation(playerid, "CARRY", "crry_prtial", 1.0, 0, 0, 0, 0, 0, 1);
					SetCameraBehindPlayer(playerid);
					PoolAimer = -1;
					DestroyObject(AimObject);
					TextDrawHideForPlayer(playerid, PoolTD[0]);
					TextDrawHideForPlayer(playerid, PoolTD[1]);
					TextDrawHideForPlayer(playerid, PoolTD[2]);
					TextDrawHideForPlayer(playerid, PoolTD[3]);
				}
			}
		}
		if (IsKeyJustUp(KEY_FIRE, newkeys, oldkeys))
		{
			if(PoolAimer == playerid)
			{
				new
					Float:speed;
				ApplyAnimation(playerid, "POOL", "POOL_Med_Shot",3.0,0,0,0,0,0,1);
				speed = 0.4 + (PoolPower * 2.0) / 100.0;
				PHY_SetObjectVelocity(PoolBall[0][bObject], speed * floatsin(-AimAngle[playerid][0], degrees), speed * floatcos(-AimAngle[playerid][0], degrees));
				if(PoolCamera[playerid] == 0)
				{
					switch(random(2))
					{
						case 0: SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
						case 1: SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
					}
					SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
				}
				PlayPoolSound(31810);
				PoolAimer = -1;
				DestroyObject(AimObject);
				GivePlayerWeapon(playerid, 7, 1);
				PoolLastShooter = playerid;
				PoolLastScore = 0;
				TextDrawHideForPlayer(playerid, PoolTD[0]);
				TextDrawHideForPlayer(playerid, PoolTD[1]);
				TextDrawHideForPlayer(playerid, PoolTD[2]);
				TextDrawHideForPlayer(playerid, PoolTD[3]);
			}
		}
	}
	return 1;
}

public PHY_OnObjectUpdate(objectid)
{
	if(PoolStarted)
	{
		for(new i; i < sizeof PoolBall; i++)
		{
			if(objectid == PoolBall[i][bObject] && PHY_IsObjectMoving(PoolBall[i][bObject]))
			{
				new
					hole = IsBallInHole(i);
				if(hole)
				{
					new
						Float:speed,
						Float:vx, Float:vy, Float:vz;
					PHY_GetObjectVelocity(PoolBall[i][bObject], vx, vy, vz);
					speed = floatsqroot(vx * vx + vy * vy) + 0.2;

					PoolBall[i][bExisting] = 0;
					PHY_DeleteObject(PoolBall[i][bObject]);

					PlayPoolSound(31803 + random(3));

					switch(hole)
					{
						case 1: MoveObject(PoolBall[i][bObject], 509.61123657,-85.79737091,998.86785889-0.25, speed);
						case 2: MoveObject(PoolBall[i][bObject], 510.67373657,-84.84423065,998.86785889-0.25, speed);
						case 3: MoveObject(PoolBall[i][bObject], 510.61914062,-83.88769531,998.86785889-0.25, speed);
						case 4: MoveObject(PoolBall[i][bObject], 509.61077881,-83.89227295,998.86785889-0.25, speed);
						case 5: MoveObject(PoolBall[i][bObject], 510.61825562,-85.80107880,998.86785889-0.25, speed);
						case 6: MoveObject(PoolBall[i][bObject], 509.55642700,-84.84602356,998.86785889-0.25, speed);
					}
					if(i)
					{
						PoolScore[PoolLastShooter] ++;
						PoolLastScore ++;
						new string[128];
						if(PoolLastScore > 0) format(string, 128, "~g~~h~+%d", PoolLastScore);
						else format(string, 128, "~r~~h~%d", PoolLastScore);
						GameTextForPlayer(PoolLastShooter, string, 100000, 4);
						PlayerPlaySound(PoolLastShooter, 1250, 0.0, 0.0, 0.0);
					}
					else
					{
						PoolScore[PoolLastShooter] --;
						PoolLastScore --;
						new string[128];
						if(PoolLastScore > 0) format(string, 128, "~g~~h~+~r~~h~%d", PoolLastScore);
						else format(string, 128, "~r~~h~%d", PoolLastScore);
						GameTextForPlayer(PoolLastShooter, string, 100000, 4);
						PlayerPlaySound(PoolLastShooter, 1250, 0.0, 0.0, 0.0);
					}
					if(GetPoolBallsCount() <= 1)
					{
						PoolStarted = 0;
						PoolAimer = -1;
						new
							winscore = GetMaxPoolScore(),
							name[MAX_PLAYER_NAME];
						RespawnPoolBalls();
						RestoreCamera(PoolLastShooter);
						PoolLastShooter = -1;
						foreach (new p : Player)
						{
							if(PlayingPool[p] && PoolScore[p] == winscore)
							{
								new
									string[128];
								GetPlayerName(p, name, sizeof name);
								format(string, 128, "{FFFF45}The winner is: {EBEBEB}%s {FFFF45}with {EBEBEB}%d {FFFF45}points.", name, winscore);
								SendPoolPlayersMessage(string);
							}
						}
						foreach (new p : Player)
						{
							if(PlayingPool[p])
							{
								PlayingPool[p] = 0;
							}
						}
					}
					else if(AreAllBallsStopped())
					{
						SetTimerEx("RestoreCamera", 800, 0, "d", PoolLastShooter);
						PoolLastShooter = -1;
					}
				}
				return 1;
			}
		}
	}
	return 1;
}

public PHY_OnObjectCollideWithObject(object1, object2)
{
	if(PoolStarted)
	{
		for(new i; i < sizeof PoolBall; i++)
		{
			if(object1 == PoolBall[i][bObject])
			{
				PlayPoolSound(31800 + random(3));
				return 1;
			}
		}
	}
	return 1;
}

public PHY_OnObjectCollideWithWall(objectid, wallid)
{
	if(PoolStarted)
	{
		for(new i; i < sizeof PoolBall; i++)
		{
			if(objectid == PoolBall[i][bObject])
			{
				PlayPoolSound(31808);
				return 1;
			}
		}
	}
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	if(PoolAimer == playerid)
	{
		PoolAimer = -1;
		TextDrawHideForPlayer(playerid, PoolTD[0]);
		TextDrawHideForPlayer(playerid, PoolTD[1]);
		TextDrawHideForPlayer(playerid, PoolTD[2]);
		TextDrawHideForPlayer(playerid, PoolTD[3]);
		DestroyObject(AimObject);
	}
	if(PlayingPool[playerid])
	{
		PlayingPool[playerid] = 0;
		new
			count = GetPoolPlayersCount();
		if(count <= 0)
		{
			PoolStarted = 0;
			RespawnPoolBalls();
		}
	}
	return 1;
}

public OnPlayerDeath(playerid, killerid, reason)
{
	if(PoolAimer == playerid)
	{
		PoolAimer = -1;
		TextDrawHideForPlayer(playerid, PoolTD[0]);
		TextDrawHideForPlayer(playerid, PoolTD[1]);
		TextDrawHideForPlayer(playerid, PoolTD[2]);
		TextDrawHideForPlayer(playerid, PoolTD[3]);
		DestroyObject(AimObject);
	}
	if(PlayingPool[playerid])
	{
		PlayingPool[playerid] = 0;
		new
			count = GetPoolPlayersCount();
		if(count <= 0)
		{
			PoolStarted = 0;
			RespawnPoolBalls();
		}
	}
	return 1;
}

public OnPlayerSpawn(playerid)
{
	PreloadAnimLib(playerid, "POOL");

	if(PoolAimer == playerid)
	{
		PoolAimer = -1;
		TextDrawHideForPlayer(playerid, PoolTD[0]);
		TextDrawHideForPlayer(playerid, PoolTD[1]);
		TextDrawHideForPlayer(playerid, PoolTD[2]);
		TextDrawHideForPlayer(playerid, PoolTD[3]);
		DestroyObject(AimObject);
	}
	if(PlayingPool[playerid])
	{
		PlayingPool[playerid] = 0;
		new
			count = GetPoolPlayersCount();
		if(count <= 0)
		{
			PoolStarted = 0;
			RespawnPoolBalls();
		}
	}
	return 1;
}

public OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
	if(PlayingPool[playerid])
	{
		PlayingPool[playerid] = 0;
		new
			count = GetPoolPlayersCount();
		if(count <= 0)
		{
			PoolStarted = 0;
			RespawnPoolBalls();
		}
	}
	return 1;
}

Float:GetPointDistanceToPoint(Float:x1,Float:y1,Float:x2,Float:y2)
{
	new Float:x, Float:y;
	x = x1-x2;
	y = y1-y2;
	return floatsqroot(x*x+y*y);
}


stock GetAngleToXY(Float:X, Float:Y, Float:CurrX, Float:CurrY, &Float:angle)
{
	angle = atan2(Y-CurrY, X-CurrX);
	angle = floatsub(angle, 90.0);
	if(angle < 0.0) angle = floatadd(angle, 360.0);
}


stock GetXYInFrontOfPos(Float:xx,Float:yy,Float:a, &Float:x2, &Float:y2, Float:distance)
{
	if(a>360)
	{
		a=a-360;
	}
	xx += (distance * floatsin(-a, degrees));
	yy += (distance * floatcos(-a, degrees));
	x2=xx;
	y2=yy;
}

stock IsPointFacingPoint(Float:dOffset, Float:X, Float:Y, Float:pA, Float:pX, Float:pY)
{
	new
		Float:ang;

	if( Y > pY ) ang = (-acos((X - pX) / floatsqroot((X - pX)*(X - pX) + (Y - pY)*(Y - pY))) - 90.0);
	else if( Y < pY && X < pX ) ang = (acos((X - pX) / floatsqroot((X - pX)*(X - pX) + (Y - pY)*(Y - pY))) - 450.0);
	else if( Y < pY ) ang = (acos((X - pX) / floatsqroot((X - pX)*(X - pX) + (Y - pY)*(Y - pY))) - 90.0);

	if(AngleInRangeOfAngle(-ang, pA, dOffset)) return true;

	return false;

}

stock Is2DPointInRangeOfPoint(Float:x, Float:y, Float:x2, Float:y2, Float:range)
{
	x2 -= x;
	y2 -= y;
	return ((x2 * x2) + (y2 * y2)) < (range * range);
}

stock IsObjectInSphere(objectid,Float:x,Float:y,Float:z,Float:radius2)
{
	new Float:x1,Float:y1,Float:z1,Float:tmpdis;
	GetObjectPos(objectid,x1,y1,z1);
	tmpdis = floatsqroot(floatpower(floatabs(floatsub(x,x1)),2)+ floatpower(floatabs(floatsub(y,y1)),2)+ floatpower(floatabs(floatsub(z,z1)),2));
	if(tmpdis < radius2) return 1;
	return 0;
}

GetBallModel(i)
{
	switch(i)
	{
		case 0: return 3003;
		case 1: return 3002;
		case 2: return 3001;
		case 3: return 2995;
		case 4: return 2996;
		case 5: return 3106;
		case 6: return 3105;
		case 7: return 3103;
		case 8: return 3001;
		case 9: return 3100;
		case 10: return 2997;
		case 11: return 3000;
		case 12: return 3102;
		case 13: return 2999;
		case 14: return 2998;
		case 15: return 3104;
	}
	return 0;
}

stock GetPoolBallsCount()
{
	new
		count;
	for(new i; i < 16; i++)
	{
		if(PoolBall[i][bExisting] || i == 0) count++;
	}
	return count;
}


stock GetMaxPoolScore()
{
	new
		scoremax = -1;
	foreach (new i : Player)
	{
		if(PlayingPool[i])
		{
			if(PoolScore[i] > scoremax)
			{
				scoremax = PoolScore[i];
			}
		}
	}
	return scoremax;
}

stock SendPoolPlayersMessage(string[])
{
	foreach (new i : Player)
	{
		if(PlayingPool[i])
		{
			SendClientMessage(i, -1, string);
		}
	}
}

stock AreAllBallsStopped()
{
	new
		Float:x, Float:y, Float:z;
	for(new i; i < 16; i++)
	{
		if(PoolBall[i][bExisting])
		{
			PHY_GetObjectVelocity(PoolBall[i][bObject], x, y, z);
			if(x != 0.0 || y != 0.0)
				return 0;
		}
	}
	return 1;
}

stock RespawnPoolBalls(init = 0)
{
	for(new i; i < 16; i++)
	{
		/*PoolBall[i][bMoving] = 0;
		PoolBall[i][bSpeed] = 0;*/
		//StopObject(PoolBall[i][bObject]);
		DestroyObject(PoolBall[i][bObject]);
		if(PoolBall[i][bExisting])
		{
			PHY_DeleteObject(PoolBall[i][bObject]);
			PoolBall[i][bExisting] = 0;
		}
	}

	if(PoolAimer != -1)
	{
		TogglePlayerControllable(PoolAimer, 1);
		ClearAnimations(PoolAimer);
		ApplyAnimation(PoolAimer, "CARRY", "crry_prtial", 1.0, 0, 0, 0, 0, 0);
		SetCameraBehindPlayer(PoolAimer);
		PoolAimer = -1;
		DestroyObject(AimObject);
		TextDrawHideForPlayer(PoolAimer, PoolTD[0]);
		TextDrawHideForPlayer(PoolAimer, PoolTD[1]);
		TextDrawHideForPlayer(PoolAimer, PoolTD[2]);
		TextDrawHideForPlayer(PoolAimer, PoolTD[3]);
	}

	CreateBalls();
	if(init)
	{
		for(new i; i < sizeof PoolBall; i++)
			InitBall(i);
	}
}

stock CreateBalls()
{
	PoolBall[0][bObject] = CreateObject(3003, 510.11218261719, -84.40771484375, 998.86785888672, 0, 0, 0);
	PoolBall[1][bObject] = CreateObject(3002, 510.10882568359, -85.166389465332, 998.86749267578, 0, 0, 0);
	PoolBall[2][bObject] = CreateObject(3101, 510.14270019531, -85.232612609863, 998.86749267578, 0, 0, 0);
	PoolBall[3][bObject] = CreateObject(2995, 510.0676574707, -85.232200622559, 998.86749267578, 0, 0, 0);
	PoolBall[4][bObject] = CreateObject(2996, 510.18600463867, -85.295257568359, 998.86749267578, 0, 0, 0);
	PoolBall[5][bObject] = CreateObject(3106, 510.11242675781, -85.297294616699, 998.86749267578, 0, 0, 0);
	PoolBall[6][bObject] = CreateObject(3105, 510.03665161133, -85.299163818359, 998.86749267578, 0, 0, 0);
	PoolBall[7][bObject] = CreateObject(3103, 510.22308349609, -85.362342834473, 998.86749267578, 0, 0, 0);
	PoolBall[8][bObject] = CreateObject(3001, 510.14828491211, -85.365989685059, 998.86749267578, 0, 0, 0);
	PoolBall[9][bObject] = CreateObject(3100, 510.07455444336, -85.365234375, 998.86749267578, 0, 0, 0);
	PoolBall[10][bObject] = CreateObject(2997, 510.00054931641, -85.363563537598, 998.86749267578, 0, 0, 0);
	PoolBall[11][bObject] = CreateObject(3000, 510.25915527344, -85.431137084961, 998.86749267578, 0, 0, 0);
	PoolBall[12][bObject] = CreateObject(3102, 510.18399047852, -85.430549621582, 998.86749267578, 0, 0, 0);
	PoolBall[13][bObject] = CreateObject(2999, 510.10900878906, -85.43196105957, 998.86749267578, 0, 0, 0);
	PoolBall[14][bObject] = CreateObject(2998, 510.03570556641, -85.432624816895, 998.86749267578, 0, 0, 0);
	PoolBall[15][bObject] = CreateObject(3104, 509.96197509766, -85.427406311035, 998.86749267578, 0, 0, 0);
}

stock InitBall(i)
{
	PHY_InitObject(PoolBall[i][bObject], 3003, _, _, PHY_MODE_2D);
	PHY_SetObjectFriction(PoolBall[i][bObject], 0.40);
	PHY_RollObject(PoolBall[i][bObject], _, PHY_ROLLING_MODE_ADVANCED);
	PHY_SetObjectWorld(PoolBall[i][bObject], 3);
	PoolBall[i][bExisting] = 1;
}

stock LoadPool()
{
	CreateBalls();

	SetTimer("PoolTimer", 21, 1);

	PHY_SetWallWorld(PHY_CreateWall(509.627 - 0.038, -85.780 - 0.038, 510.598 + 0.038, -85.780 - 0.038), 3);
	PHY_SetWallWorld(PHY_CreateWall(510.598 + 0.038, -85.780 - 0.038, 510.598 + 0.038, -83.907 + 0.038), 3);
	PHY_SetWallWorld(PHY_CreateWall(510.598 + 0.038, -83.907 + 0.038, 509.627 - 0.038, -83.907 + 0.038), 3);
	PHY_SetWallWorld(PHY_CreateWall(509.627 - 0.038, -83.907 + 0.038, 509.627 - 0.038, -85.780 - 0.038), 3);


	PoolTD[0] = TextDrawCreate(505.000000, 260.000000, "~n~~n~");
	TextDrawBackgroundColor(PoolTD[0], 255);
	TextDrawFont(PoolTD[0], 1);
	TextDrawLetterSize(PoolTD[0], 0.500000, 0.439999);
	TextDrawColor(PoolTD[0], -1);
	TextDrawSetOutline(PoolTD[0], 0);
	TextDrawSetProportional(PoolTD[0], 1);
	TextDrawSetShadow(PoolTD[0], 1);
	TextDrawUseBox(PoolTD[0], 1);
	TextDrawBoxColor(PoolTD[0], 255);
	TextDrawTextSize(PoolTD[0], 569.000000, -10.000000);

	PoolTD[1] = TextDrawCreate(506.000000, 261.000000, "~n~~n~");
	TextDrawBackgroundColor(PoolTD[1], 255);
	TextDrawFont(PoolTD[1], 1);
	TextDrawLetterSize(PoolTD[1], 0.500000, 0.300000);
	TextDrawColor(PoolTD[1], -1);
	TextDrawSetOutline(PoolTD[1], 0);
	TextDrawSetProportional(PoolTD[1], 1);
	TextDrawSetShadow(PoolTD[1], 1);
	TextDrawUseBox(PoolTD[1], 1);
	TextDrawBoxColor(PoolTD[1], 911303167);
	TextDrawTextSize(PoolTD[1], 568.000000, 0.000000);

	PoolTD[2] = TextDrawCreate(506.000000, 261.000000, "~n~~n~");
	TextDrawBackgroundColor(PoolTD[2], 255);
	TextDrawFont(PoolTD[2], 1);
	TextDrawLetterSize(PoolTD[2], 0.500000, 0.300000);
	TextDrawColor(PoolTD[2], -1);
	TextDrawSetOutline(PoolTD[2], 0);
	TextDrawSetProportional(PoolTD[2], 1);
	TextDrawSetShadow(PoolTD[2], 1);
	TextDrawUseBox(PoolTD[2], 1);
	TextDrawBoxColor(PoolTD[2], -1949699841);
	TextDrawTextSize(PoolTD[2], 501.000000, 0.000000);

	PoolTD[3] = TextDrawCreate(503.000000, 240.000000, "Power");
	TextDrawBackgroundColor(PoolTD[3], 255);
	TextDrawFont(PoolTD[3], 2);
	TextDrawLetterSize(PoolTD[3], 0.280000, 1.699999);
	TextDrawColor(PoolTD[3], -1);
	TextDrawSetOutline(PoolTD[3], 1);
	TextDrawSetProportional(PoolTD[3], 1);
}

stock GetPoolPlayersCount()
{
	new
		count;
	if(PoolStarted)
	{
		foreach (new i : Player)
		{
			if(PlayingPool[i])
			{
				count++;
			}
		}
	}
	return count;
}

forward PoolTimer();
public PoolTimer()
{
	if(!PoolStarted) return 0;
	if(PoolAimer != -1)
	{
		new
			playerid = PoolAimer,
			keys,
			ud,
			lr;
		GetPlayerKeys(playerid, keys, ud, lr);
		if(!(keys & KEY_FIRE))
		{
			if(lr)
			{
				new
					Float:X,
					Float:Y,
					Float:Z,
					Float:Xa,
					Float:Ya,
					Float:Za,
					Float:x,
					Float:y,
					Float:newrot,
					Float:dist;
				GetPlayerPos(playerid, X, Y ,Z);
				GetObjectPos(PoolBall[0][bObject], Xa, Ya, Za);
				newrot = AimAngle[playerid][0] + (lr > 0 ? 0.9 : -0.9);
				dist = GetPointDistanceToPoint(X, Y, Xa, Ya);
				if(AngleInRangeOfAngle(AimAngle[playerid][1], newrot, 30.0))
				{
					AimAngle[playerid][0] = newrot;
					switch(PoolCamera[playerid])
					{
						case 0:
						{
							GetXYBehindObjectInAngle(PoolBall[0][bObject], newrot, x, y, 0.675);
							SetPlayerCameraPos(playerid, x, y, 998.86785888672+0.28);
							SetPlayerCameraLookAt(playerid, Xa, Ya, Za+0.170);
						}
						case 1:
						{
							SetPlayerCameraPos(playerid, 511.84469604492, -84.831642150879, 1001.4904174805);
							SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
						}
						case 2:
						{
							SetPlayerCameraPos(playerid, 508.7971496582, -84.831642150879, 1001.4904174805);
							SetPlayerCameraLookAt(playerid,510.11267089844, -84.831642150879, 998.86785888672);
						}
					}
					GetXYInFrontOfPos(Xa, Ya, newrot+180, x, y, 0.085);
					SetObjectPos(AimObject, x, y, Za);
					SetObjectRot(AimObject, 7.0, 0, AimAngle[playerid][0]+180);
					GetXYInFrontOfPos(Xa, Ya, newrot+180, X, Y, dist);
					SetPlayerPos(playerid, X, Y, Z);
					SetPlayerFacingAngle(playerid, newrot);
				}
			}
		}
		else
		{
			if(PoolDir)
				PoolPower -= 2.0;
			else
				PoolPower += 2.0;
			if(PoolPower <= 0)
			{
				PoolDir = 0;
				PoolPower = 2.0;
			}
			else if(PoolPower > 100.0)
			{
				PoolDir = 1;
				PoolPower = 98.0;
			}
			TextDrawTextSize(PoolTD[2], 501.0 + ((67.0 * PoolPower)/100.0), 0.0);
			TextDrawShowForPlayer(playerid, PoolTD[2]);
		}
	}

	if(PoolLastShooter != -1 && AreAllBallsStopped())
	{
		SetTimerEx("RestoreCamera", 800, 0, "d", PoolLastShooter);
		PoolLastShooter = -1;
	}
	return 1;
}

forward RestoreCamera(playerid);
public RestoreCamera(playerid)
{
	if(!PoolBall[0][bExisting])
	{
		DestroyObject(PoolBall[0][bObject]);
		PoolBall[0][bObject] = CreateObject(GetBallModel(0) ,510.11218261719, -84.40771484375, 998.86785888672, 0, 0, 0);
		InitBall(0);
	}
	GameTextForPlayer(playerid, " ", 100000, 4);
	if(PoolAimer == playerid) return 0;
	TogglePlayerControllable(playerid, 1);
	return SetCameraBehindPlayer(playerid);
}

forward RestoreWeapon(playerid);
public RestoreWeapon(playerid)
{
	RemovePlayerAttachedObject(playerid, OBJ_SLOT_POOL);
	UsingChalk[playerid] = 0;
	GivePlayerWeapon(playerid, 7, 1);
	ApplyAnimation(playerid, "CARRY", "crry_prtial", 1.0, 0, 0, 0, 0, 0, 1);
	return 1;
}

stock IsBallInHole(i)
{
	if(IsObjectInSphere(PoolBall[i][bObject],509.61123657,-85.79737091,998.86785889,0.0825)) return 1;
	else if(IsObjectInSphere(PoolBall[i][bObject],510.67373657,-84.84423065,998.86785889,0.0825)) return 2;
	else if(IsObjectInSphere(PoolBall[i][bObject],510.61914062,-83.88769531,998.86785889,0.0825)) return 3;
	else if(IsObjectInSphere(PoolBall[i][bObject],509.61077881,-83.89227295,998.86785889,0.0825)) return 4;
	else if(IsObjectInSphere(PoolBall[i][bObject],510.61825562,-85.80107880,998.86785889,0.0825)) return 5;
	else if(IsObjectInSphere(PoolBall[i][bObject],509.55642700,-84.84602356,998.86785889,0.0825)) return 6;
	else return 0;
}

stock GetXYBehindObjectInAngle(objectid, Float:a, &Float:x2, &Float:y2, Float:distance)
{
	new Float:z;
	GetObjectPos(objectid, x2, y2, z);

	x2 += (distance * floatsin(-a+180, degrees));
	y2 += (distance * floatcos(-a+180, degrees));
}

forward PlayPoolSound(soundid);
public PlayPoolSound(soundid)
{
	foreach(new i : Player)
	{
		if(PlayingPool[i])
		{
			PlayerPlaySound(i, soundid, 0, 0, 0);
		}
	}
	return 1;
}

stock IsKeyJustDown(key, newkeys, oldkeys)
{
	if((newkeys & key) && !(oldkeys & key)) return 1;
	return 0;
}

stock IsKeyJustUp(key, newkeys, oldkeys)
{
	if(!(newkeys & key) && (oldkeys & key)) return 1;
	return 0;
}

stock PreloadAnimLib(playerid, animlib[])
{
	ApplyAnimation(playerid,animlib,"null",0.0,0,0,0,0,0);
}

stock AngleInRangeOfAngle(Float:a1, Float:a2, Float:range)
{
	a1 -= a2;
	if((a1 < range) && (a1 > -range)) return true;

	return false;
}
