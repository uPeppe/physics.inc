#include <a_samp>
#define COLANDREAS
#include <physics>
#include <zcmd>
#include <sscanf>


main() { }

public OnGameModeInit()
{
	SetGameModeText("Blank Script");
	AddPlayerClass(0, 1958.3783, 1343.1572, 15.3746, 269.1425, 0, 0, 0, 0, 0, 0);
	
	/*PHY_CreateWall(1961.4270, 1339.6151, 1961.4269, 1346.3175);
	PHY_CreateWall(1961.4269, 1346.3175, 1951.4360, 1345.8835);
	PHY_CreateWall(1951.4360, 1345.8835, 1951.5145, 1340.2163);
	PHY_CreateWall(1951.5145, 1340.2163, 1961.4270, 1339.6151);*/
	
	SetTimer("DogTimer", 400, 1);
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

new dog = -1;
new Float:destangle;
new jumpstr;

forward DogTimer();
public DogTimer()
{
	new Float:vx, Float:vy, Float:vz;
	new Float:x, Float:y, Float:z;
	GetObjectPos(dog, x, y, z);
	PHY_GetObjectVelocity(dog, vx, vy, vz);
	if(z <= PHY_Object[dog][PHY_LowZBound] + 0.1)
	{
	    PHY_SetObjectVelocity(dog, vx, vy, 1.2);
	}
	return 1;
}

public OnPlayerSpawn(playerid)
{
	SetPlayerSkin(playerid, random(300));
	SetPlayerPos(playerid, 1723.3232,-1867.1775,13.5705);
	return 1;
}

public PHY_OnObjectCollideWithSAWorld(objectid)
{
	if(objectid == dog)
	{
		new Float:vx, Float:vy, Float:vz;
		PHY_GetObjectVelocity(objectid, vx, vy, vz);
		vx *= 0.1;
		vy *= 0.1;
		PHY_SetObjectVelocity(objectid, vx, vy, vz);
	}
	return 1;
}

public PHY_OnObjectUpdate(objectid)
{
	if(objectid == dog)
	{
	    new Float:x, Float:y, Float:z;
	    GetObjectRot(dog, x, y, z);
		if(destangle < 0.0)
		    destangle += 360.0;
		else if(destangle > 360.0)
		    destangle -= 360.0;
        if(z < 0.0)
		    z += 360.0;
		else if(z > 360.0)
		    z -= 360.0;
	    if(z != destangle)
	    {
	        if(destangle > z)
	        {
	            if(destangle - z < 180.0)
	            {
	                z += 10.0;
	                if(z > destangle)
	                    z = destangle;
				}
				else
				{
	                z -= 10.0;
	                if(z < destangle)
	                    z = destangle;
				}
	        }
	        else
	        {
	            if(z - destangle < 180.0)
	            {
	                z -= 10.0;
	                if(z < destangle)
	                    z = destangle;
				}
				else
				{
	                z += 10.0;
	                if(z > destangle)
	                    z = destangle;
				}
	        }
	        SetObjectRot(dog, x, y, z);
	    }
	}
	return 1;
}

public OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
	/*if(newkeys & KEY_HANDBRAKE)
	{
	    new Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz;
	    GetObjectPos(object, x, y, z);
	    CA_RayCastLineAngle(x, y, z, x, y, z - 1000.0, x, y, z, rx, ry, rz);
		new string[128];
		format(string, sizeof string, "%f %f %f", rx, ry, rz);
		SendClientMessage(playerid, -1, string);
	}*/
	
	if(newkeys & KEY_HANDBRAKE && !(oldkeys & KEY_HANDBRAKE))
	{
		new Float:x, Float:y, Float:z, Float:a;
		GetPlayerPos(playerid, x, y, z);
		GetPlayerFacingAngle(playerid, a);
		new Float:ox, Float:oy, Float:oz;
		GetObjectPos(dog, ox, oy, oz);
		new Float:vx, Float:vy, Float:vz, Float:vel;
		GetPlayerCameraFrontVector(playerid, vx, vy, vz);
		if((vx == 0.0 && vy == 0.0 && vz == 0.0) || (vx != vx || vy != vy || vz != vz) || newkeys & KEY_SPRINT)
		{
		    
		}
		else
		{
		    new Float:cx, Float:cy, Float:cz;
		    GetPlayerCameraPos(playerid, cx, cy, cz);
		    CA_RayCastLine(cx, cy, cz, cx + 100.0 * vx, cy + 100.0 * vy, cz + 100.0 * vz, x, y, z);
		}
		PHY_GetObjectVelocity(dog, vx, vy, vz);
		vx = x - ox;
		vy = y - oy;
		vel = 7.0 / floatsqroot(vx * vx + vy * vy);
		PHY_SetObjectVelocity(dog, vx * vel, vy * vel, vz);
		destangle = atan2(vy, vx);
		
		PHY_SetObjectAirResistance(dog, 0.0);
		PHY_SetObjectFriction(dog, 0.0);
	}
	else if(oldkeys & KEY_HANDBRAKE && !(newkeys & KEY_HANDBRAKE))
	{
		PHY_SetObjectAirResistance(dog, 20.0);
		PHY_SetObjectFriction(dog, 10.0);
	}
	
	if(oldkeys & KEY_FIRE && !(newkeys & KEY_FIRE))
	{
	    new str = GetTickCount() - jumpstr;
	    if(str > 500)
	        str = 500;
		new Float:x, Float:y, Float:z;
		GetObjectPos(dog, x, y, z);
		new Float:vx, Float:vy, Float:vz;
		PHY_GetObjectVelocity(dog, vx, vy, vz);
		if(z <= PHY_Object[dog][PHY_LowZBound] + 0.1)
		    PHY_SetObjectVelocity(dog, vx, vy, 5.0);
	}
	return 1;
}

command(dog, playerid, params[])
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	dog = CreateObject(19315, x, y, z, 0.0, 0.0, 0.0);
	PHY_InitObject(dog, 19315, 2.0, 0.3);
	PHY_SetObjectGravity(dog, 12);
	PHY_ToggleObjectPlayerColls(dog, 1, 0.1, 0.5);
	PHY_UseColAndreas(dog);
	return 1;
}

command(go, playerid, params[])
{
	new a = strval(params);
	new Float:x, Float:y, Float:z;
	GetObjectPos(a, x, y, z);
	SetPlayerPos(playerid, x, y, z);
	return 1;
}

command(ball, playerid, params[])
{
    new Float:x, Float:y, Float:z, Float:ang, Float:vz, Float:mass;
    sscanf(params, "F(0.0)F(1.0)", vz, mass);
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, ang);
	new a = CreateObject(1598, x, y, z-0.69, 0, 0, 0);
	PHY_InitObject(a, 1598, mass, _, PHY_MODE_3D);
	PHY_SetObjectVelocity(a, 6.0 * floatsin(-ang, degrees), 6.0 * floatcos(-ang, degrees), vz);
	PHY_RollObject(a);
	PHY_SetObjectFriction(a, 0.25);
	PHY_SetObjectAirResistance(a, 0.1);
	PHY_SetObjectGravity(a, 7.1);
	PHY_SetObjectZBound(a, _, _, 0.5);
	PHY_UseColAndreas(a);
	PHY_ToggleObjectPlayerColls(playerid);
	return 1;
}

command(stopped, playerid, params[])
{
    new Float:x, Float:y, Float:z, Float:ang;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, ang);
	new a = CreateObject(1598, x, y, z-0.69, 0, 0, 0);
	PHY_InitObject(a, 1598, 1.0, _, PHY_MODE_3D);
	//PHY_SetObjectVelocity(a, 5.0 * floatsin(-ang, degrees), 5.0 * floatcos(-ang, degrees));
	PHY_RollObject(a);
	PHY_SetObjectFriction(a, 1.3);
	PHY_SetObjectAirResistance(a, 0.1);
	PHY_SetObjectGravity(a, 7.1);
	PHY_SetObjectZBound(a, _, _, 0.5);
	PHY_UseColAndreas(a);
	
	new string[128];
	format(string, sizeof string, "RX %f RY %f", PHY_Object[a][PHY_LowZRX], PHY_Object[a][PHY_LowZRY]);
	SendClientMessage(playerid, -1, string);
	return 1;
}

command(jetpack, playerid, params[])
{
	SetPlayerSpecialAction(playerid, SPECIAL_ACTION_USEJETPACK);
	return 1;
}

command(veh, playerid, params[])
{
	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	CreateVehicle(411, x + 2.0, y+ 2.0, z, 0, 0, 0, 1000000);
	return 1;
}

command(nocol, playerid, params[])
{
    new Float:x, Float:y, Float:z, Float:ang, Float:vz, Float:mass;
    sscanf(params, "F(0.0)F(1.0)", vz, mass);
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, ang);
	new a = CreateObject(1598, x, y, z-0.69, 0, 0, 0);
	PHY_InitObject(a, 1598, mass, _, PHY_MODE_3D);
	PHY_SetObjectVelocity(a, 6.0 * floatsin(-ang, degrees), 6.0 * floatcos(-ang, degrees), vz);
	PHY_RollObject(a);
	PHY_SetObjectFriction(a, 0.25);
	PHY_SetObjectAirResistance(a, 0.1);
	PHY_SetObjectGravity(a, 7.1);
	PHY_SetObjectZBound(a, _, _, 0.5);
	PHY_UseColAndreas(a, 3);
	return 1;
}

