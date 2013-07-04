#include <a_samp>
#include <physics>
#include <zcmd>
#include <sscanf>

new PoolBall[16];

main() { }

public OnGameModeInit()
{
	SetGameModeText("Objects Physics - Pool Demo");
	AddPlayerClass(0, 2442.1621,2059.5051,10.8203, 269.1425, 0, 0, 0, 0, 0, 0);

	PoolBall[0] = CreateObject(3003, 510.11218261719, -84.40771484375, 998.86785888672, 0, 0, 0);
    PoolBall[1] = CreateObject(3002, 510.10882568359, -85.166389465332, 998.86749267578, 0, 0, 0);
    PoolBall[2] = CreateObject(3101, 510.14270019531, -85.232612609863, 998.86749267578, 0, 0, 0);
    PoolBall[3] = CreateObject(2995, 510.0676574707, -85.232200622559, 998.86749267578, 0, 0, 0);
    PoolBall[4] = CreateObject(2996, 510.18600463867, -85.295257568359, 998.86749267578, 0, 0, 0);
    PoolBall[5] = CreateObject(3106, 510.11242675781, -85.297294616699, 998.86749267578, 0, 0, 0);
    PoolBall[6] = CreateObject(3105, 510.03665161133, -85.299163818359, 998.86749267578, 0, 0, 0);
    PoolBall[7] = CreateObject(3103, 510.22308349609, -85.362342834473, 998.86749267578, 0, 0, 0);
    PoolBall[8] = CreateObject(3001, 510.14828491211, -85.365989685059, 998.86749267578, 0, 0, 0);
    PoolBall[9] = CreateObject(3100, 510.07455444336, -85.365234375, 998.86749267578, 0, 0, 0);
    PoolBall[10] = CreateObject(2997, 510.00054931641, -85.363563537598, 998.86749267578, 0, 0, 0);
    PoolBall[11] = CreateObject(3000, 510.25915527344, -85.431137084961, 998.86749267578, 0, 0, 0);
    PoolBall[12] = CreateObject(3102, 510.18399047852, -85.430549621582, 998.86749267578, 0, 0, 0);
    PoolBall[13] = CreateObject(2999, 510.10900878906, -85.43196105957, 998.86749267578, 0, 0, 0);
    PoolBall[14] = CreateObject(2998, 510.03570556641, -85.432624816895, 998.86749267578, 0, 0, 0);
    PoolBall[15] = CreateObject(3104, 509.96197509766, -85.427406311035, 998.86749267578, 0, 0, 0);

    for(new i; i < sizeof PoolBall; i++)
    {
        PHY_InitObject(PoolBall[i], 3003, _, _, PHY_MODE_2D); // Notice that I typed modelid 3003 because all the balls are equal.
        PHY_SetObjectFriction(PoolBall[i], 0.08);
        PHY_RollObject(PoolBall[i]);
	}

	PHY_CreateWall(509.627 - 0.038, -85.780, 510.598 + 0.038, -85.780);
	PHY_CreateWall(510.598 + 0.038, -85.780, 510.598 + 0.038, -83.907);
	PHY_CreateWall(510.598 + 0.038, -83.907, 509.627 - 0.038, -83.907);
	PHY_CreateWall(509.627 - 0.038, -83.907, 509.627 - 0.038, -85.780);

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

public PHY_OnObjectUpdate(objectid)
{
	if(IsObjectInSphere(objectid,509.61123657,-85.79737091,998.86785889,0.08) ||
	IsObjectInSphere(objectid,510.67373657,-84.84423065,998.86785889,0.08) ||
	IsObjectInSphere(objectid,510.61914062,-83.88769531,998.86785889,0.08) ||
    IsObjectInSphere(objectid,509.61077881,-83.89227295,998.86785889,0.08) ||
	IsObjectInSphere(objectid,510.61825562,-85.80107880,998.86785889,0.08) ||
	IsObjectInSphere(objectid,509.55642700,-84.84602356,998.86785889,0.08))
	{
		DestroyObject(objectid);
		PHY_DeleteObject(objectid);
	}
	return 1;
}

stock IsObjectInSphere(objectid,Float:x,Float:y,Float:z,Float:radius2)
{
    new Float:x1,Float:y1,Float:z1,Float:tmpdis;
    GetObjectPos(objectid,x1,y1,z1);
    tmpdis = floatsqroot(floatpower(floatabs(floatsub(x,x1)),2)+ floatpower(floatabs(floatsub(y,y1)),2)+ floatpower(floatabs(floatsub(z,z1)),2));
    if(tmpdis < radius2) return 1;
    return 0;
}

command(movefirst, playerid, params[])
{
	new
	    Float:speed;
	if(sscanf(params, "f", speed)) return SendClientMessage(playerid, -1, "Use: /movefirst <speed>");
    PHY_MoveObject(PoolBall[0], 0.0, -speed);
	return 1;
}

command(move, playerid, params[])
{
	new
		id,
	    Float:speed,
		Float:angle;
	if(sscanf(params, "dff", id, speed, angle)) return SendClientMessage(playerid, -1, "Use: /move <id> <speed> <angle>");
    PHY_MoveObject(PoolBall[id], speed * floatsin(-angle, degrees), speed * floatcos(-angle, degrees));
	return 1;
}
