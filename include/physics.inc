/*  <SA-MP Objects Physics - Handle collisions and more.>
    Copyright (C) <2013>  <Peppe>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <modelsizes>

#define PHY_TIMER_INTERVAL (20)
#define PHY_MAX_WALLS (512)

#define PHY_MODE_3D (0)
#define PHY_MODE_2D (1)

#define FLOAT_INFINITY   (Float:0x7F800000)
#define FLOAT_NEG_INFINITY (Float:0xFF800000)
#define FLOAT_NAN     (Float:0xFFFFFFFF)

/* Callbacks */
forward PHY_OnObjectUpdate(objectid);
forward PHY_OnObjectCollideWithObject(object1, object2);
forward PHY_OnObjectCollideWithWall(objectid, wallid);


enum (<<= 1)
{
	PHY_OBJECT_USED = 0b01,
	PHY_OBJECT_MODE,
	PHY_OBJECT_ROLL,
	PHY_OBJECT_GHOST_OBJECTS,
	PHY_OBJECT_GHOST_WALLS,
	PHY_OBJECT_GHOST_PLAYERS
}

enum E_PHY_OBJECT
{
	PHY_Properties,
	PHY_World,
	Float:PHY_Size,
	Float:PHY_Mass,
	Float:PHY_VX,
	Float:PHY_VY,
	Float:PHY_VZ,
	Float:PHY_Friction,
	Float:PHY_Gravity,
	Float:PHY_LowZBound,
	Float:PHY_HighZBound,
	Float:PHY_BoundConst
}

new
	PHY_Object[MAX_OBJECTS][E_PHY_OBJECT];


enum E_PHY_WALL
{
	PHY_Created,
	PHY_World,
	Float:PHY_X1,
	Float:PHY_Y1,
	Float:PHY_X2,
	Float:PHY_Y2,
	Float:PHY_Z1,
	Float:PHY_Z2,
	Float:PHY_A,
	Float:PHY_B,
	Float:PHY_C,
	Float:PHY_M,
	Float:PHY_Q
}

new
	PHY_Wall[PHY_MAX_WALLS][E_PHY_WALL];

/* Macros are self-explanatory */
#define PHY_IsObjectUsingPhysics(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_USED)
#define PHY_IsObjectUsing3D(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_MODE)
#define PHY_IsObjectGhostWithObjects(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_OBJECTS)
#define PHY_IsObjectGhostWithWalls(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_WALLS)
#define PHY_IsObjectGhostWithPlayers(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_PLAYERS)
#define PHY_IsObjectMoving(%1) (PHY_Object[%1][PHY_VX] != 0 || PHY_Object[%1][PHY_VY] != 0 || PHY_Object[%1][PHY_VZ] != 0)
#define PHY_IsObjectRolling(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_ROLL)
#define PHY_GetObjectFriction(%1) (PHY_Object[%1][PHY_Friction])
#define PHY_GetObjectGravity(%1) (PHY_Object[%1][PHY_Gravity])
#define PHY_GetObjectMode(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_MODE)

public OnGameModeInit()
{
	SetTimer("PHY_CoreTimer", PHY_TIMER_INTERVAL, true);
	
	CallLocalFunction("PHY_OnGameModeInit", "");
	return 1;
}

forward PHY_CoreTimer();
public PHY_CoreTimer()
{
	new
		Float:x1,
	    Float:y1,
	    Float:z1,
	    Float:x2,
	    Float:y2,
	    Float:z2,
	    Float:rx,
	    Float:ry,
	    Float:rz,
	    Float:speed,
	    Float:dx,
	    Float:dy,
	    Float:dz,
		Float:dist,
		Float:maxdist,
		Float:angle,
		Float:moveangle,
		Float:dvx,
		Float:dvy,
		Float:mag,
		/*Float:vxp,
		Float:vyp,
		Float:vzp,*/
		Float:tmpvx1,
		Float:tmpvx2,
		Float:newvy1,
		Float:newvy2,
		Float:newvx1,
		Float:newvx2;
	for(new a; a < sizeof PHY_Object; a++)
	{
	    if(PHY_Object[a][PHY_Properties] & PHY_OBJECT_USED)
	    {
            GetObjectPos(a, x1, y1, z1);
            x1 += (PHY_Object[a][PHY_VX] * PHY_TIMER_INTERVAL)/1000;
			y1 += (PHY_Object[a][PHY_VY] * PHY_TIMER_INTERVAL)/1000;
			if(PHY_GetObjectMode(a) == PHY_MODE_3D)
				z1 += (PHY_Object[a][PHY_VZ] * PHY_TIMER_INTERVAL)/1000;
			if(PHY_IsObjectMoving(a))
	        {
                moveangle = atan2(PHY_Object[a][PHY_VY], PHY_Object[a][PHY_VX]) - 90.0;
                if(moveangle < 0.0)
					moveangle += 360.0;
				speed = floatsqroot(PHY_Object[a][PHY_VX] * PHY_Object[a][PHY_VX] + PHY_Object[a][PHY_VY] * PHY_Object[a][PHY_VY]);
                if(PHY_GetObjectMode(a) == PHY_MODE_3D)
				{
					if(z1 > PHY_Object[a][PHY_HighZBound] && PHY_Object[a][PHY_VZ] > 0)
					{
						PHY_Object[a][PHY_VZ] = -PHY_Object[a][PHY_VZ] * PHY_Object[a][PHY_BoundConst];
						z1 = PHY_Object[a][PHY_HighZBound];
					}
					else if(z1 < PHY_Object[a][PHY_LowZBound] && PHY_Object[a][PHY_VZ] < 0)
					{
					    PHY_Object[a][PHY_VZ] = -PHY_Object[a][PHY_VZ] * PHY_Object[a][PHY_BoundConst];
					    z1 = PHY_Object[a][PHY_LowZBound];
					}
					if(PHY_GetObjectGravity(a) != 0)
	                {
					    if(PHY_Object[a][PHY_VZ] > 0)
						{
							PHY_Object[a][PHY_VZ] -= (PHY_Object[a][PHY_Gravity] * PHY_TIMER_INTERVAL)/1000;
							if(PHY_Object[a][PHY_VZ] < 0)
							    PHY_Object[a][PHY_VZ] = 0;
						}
						else
							PHY_Object[a][PHY_VZ] -= (PHY_Object[a][PHY_Gravity] * PHY_TIMER_INTERVAL)/1000;
					}
				}
				if(PHY_GetObjectFriction(a) != 0 && z1 == PHY_Object[a][PHY_LowZBound])
				{
				    speed -= (PHY_Object[a][PHY_Friction] * PHY_TIMER_INTERVAL)/1000;
					if(speed < 0)
					    speed = 0;
				    PHY_Object[a][PHY_VX] = speed * floatsin(-moveangle, degrees);
				    PHY_Object[a][PHY_VY] = speed * floatcos(-moveangle, degrees);
				}
				if(PHY_IsObjectRolling(a))
				{
				    GetObjectRot(a, rx, ry, rz);
					rx -= ((speed * PHY_TIMER_INTERVAL)/1000) * (180/3.14159) / PHY_Object[a][PHY_Size];
					if(rx < 0.0)
					    rx += 360.0;
		            rz = moveangle;
		            SetObjectRot(a, rx, ry, rz);
				}
				if(!PHY_IsObjectGhostWithObjects(a))
				{
					for(new b; b < sizeof PHY_Object; b++)
					{
					    if(PHY_Object[b][PHY_Properties] & PHY_OBJECT_USED && !PHY_IsObjectGhostWithObjects(b) && (!PHY_Object[a][PHY_World] || !PHY_Object[b][PHY_World] || PHY_Object[a][PHY_World] == PHY_Object[b][PHY_World]))
					    {
					        GetObjectPos(b, x2, y2, z2);
					        dx = x1 - x2;
					        dy = y1 - y2;
					        dz = (PHY_GetObjectMode(a) == PHY_MODE_3D || PHY_GetObjectMode(b) == PHY_MODE_3D) ? (z1 - z2) : (0.0);
					        dist = (dx * dx) + (dy * dy) + (dz * dz);
					        maxdist = PHY_Object[a][PHY_Size] + PHY_Object[b][PHY_Size];
					        if(dist <= (maxdist * maxdist))
					        {
								dvx = PHY_Object[a][PHY_VX] - PHY_Object[b][PHY_VX];
		            	        dvy = PHY_Object[a][PHY_VY] - PHY_Object[b][PHY_VX];
		            	        mag = dvx * dx + dvy * dy;

		            	        if(mag < 0.0)
		            	        {
                           			angle = -atan2(dy, dx);
		            	            tmpvx1 = PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees);
		            	            newvy1 = PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees);
		            	            tmpvx2 = PHY_Object[b][PHY_VX] * floatcos(angle, degrees) - PHY_Object[b][PHY_VY] * floatsin(angle, degrees);
		            	            newvy2 = PHY_Object[b][PHY_VX] * floatsin(angle, degrees) + PHY_Object[b][PHY_VY] * floatcos(angle, degrees);

                           			newvx1 = ((PHY_Object[a][PHY_Mass] - PHY_Object[b][PHY_Mass]) * tmpvx1 + 2 * PHY_Object[b][PHY_Mass] * tmpvx2) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
		            	            newvx2 = ((PHY_Object[b][PHY_Mass] - PHY_Object[a][PHY_Mass]) * tmpvx2 + 2 * PHY_Object[a][PHY_Mass] * tmpvx1) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);

									angle = -angle;
                                    PHY_Object[a][PHY_VX] = newvx1 * floatcos(angle, degrees) - newvy1 * floatsin(angle, degrees);
		            	            PHY_Object[a][PHY_VY] = newvx1 * floatsin(angle, degrees) + newvy1 * floatcos(angle, degrees);
                           			PHY_Object[b][PHY_VX] = newvx2 * floatcos(angle, degrees) - newvy2 * floatsin(angle, degrees);
		            	            PHY_Object[b][PHY_VY] = newvx2 * floatsin(angle, degrees) + newvy2 * floatcos(angle, degrees);

                                    CallLocalFunction("PHY_OnObjectCollideWithObject", "dd", a, b);
								}
					        }
					    }
					}
				}
				if(!PHY_IsObjectGhostWithWalls(a))
				{
					for(new w; w < sizeof PHY_Wall; w++)
					{
					    if(PHY_Wall[w][PHY_Created] && (!PHY_Object[a][PHY_World] || !PHY_Wall[w][PHY_World] || PHY_Object[a][PHY_World] == PHY_Wall[w][PHY_World]))
					    {
					        //dist = (y1 - PHY_Wall[w][PHY_M] * x1 - PHY_Wall[w][PHY_Q])/floatsqroot(1 + PHY_Wall[w][PHY_M] * PHY_Wall[w][PHY_M]);
							dist = (PHY_Wall[w][PHY_A] * x1 + PHY_Wall[w][PHY_B] * y1 + PHY_Wall[w][PHY_C])/floatsqroot(PHY_Wall[w][PHY_A] * PHY_Wall[w][PHY_A] + PHY_Wall[w][PHY_B] * PHY_Wall[w][PHY_B]);
							angle = (PHY_Wall[w][PHY_M] == PHY_Wall[w][PHY_M]) ? -atan(PHY_Wall[w][PHY_M]) : 90.0;
							if(floatabs(dist) <= PHY_Object[a][PHY_Size] && PHY_Wall[w][PHY_X1] - PHY_Object[a][PHY_Size] <= x1 <= PHY_Wall[w][PHY_X2] + PHY_Object[a][PHY_Size] && PHY_Wall[w][PHY_Z1] - PHY_Object[a][PHY_Size] <= z1 <= PHY_Wall[w][PHY_Z2] + PHY_Object[a][PHY_Size])
					        {
					            //mag = y1 + PHY_Object[a][PHY_Size] * floatcos(-moveangle, degrees) - (x1 + PHY_Object[a][PHY_Size] * floatsin(-moveangle, degrees)) * PHY_Wall[w][PHY_M] - PHY_Wall[w][PHY_Q];
					            mag = PHY_Wall[w][PHY_A] * (x1 + PHY_Object[a][PHY_Size] * floatsin(-moveangle, degrees)) + PHY_Wall[w][PHY_B] * (y1 + PHY_Object[a][PHY_Size] * floatcos(-moveangle, degrees)) + PHY_Wall[w][PHY_C];
					            if((dist >= 0) ? (mag <= 0) : (mag > 0))
					            {
	       	         				newvx1 = PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees);
			            	        newvy1 = PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees);
									newvy1 = -newvy1;
									angle = -angle;
	                                PHY_Object[a][PHY_VX] = newvx1 * floatcos(angle, degrees) - newvy1 * floatsin(angle, degrees);
	        						PHY_Object[a][PHY_VY] = newvx1 * floatsin(angle, degrees) + newvy1 * floatcos(angle, degrees);

                                    CallLocalFunction("PHY_OnObjectCollideWithWall", "dd", a, w);
								}
					        }
					    }
					}
				}
	        }
	        SetObjectPos(a, x1, y1, z1);
	        CallLocalFunction("PHY_OnObjectUpdate", "d", a);
	    }
	}
	return 1;
}


/* Starts using physics for objectid.
modelid - object's modelid, used to get its size with modelsizes include.
mass - object's mass, it is like its weight and is used in collisions.
size - object's sphere radius, taken from modelsizes.inc by default.
mode - PHY_MODE_3D or PHY_MODE_2D. */
stock PHY_InitObject(objectid, modelid = 0, Float:mass = 1.0, Float:size = 0.0, mode = PHY_MODE_3D)
{
	if(IsValidObject(objectid))
	{
	    PHY_Object[objectid][PHY_Properties] = PHY_OBJECT_USED | (mode ? PHY_OBJECT_MODE : 0);
	    PHY_Object[objectid][PHY_Mass] = mass;
	    PHY_Object[objectid][PHY_World] = 0;
	    PHY_Object[objectid][PHY_VX] = 0;
	    PHY_Object[objectid][PHY_VY] = 0;
	    PHY_Object[objectid][PHY_Gravity] = 0;
	    new
	        Float:unused;
	    GetObjectPos(objectid, unused, unused, PHY_Object[objectid][PHY_LowZBound]);
		PHY_Object[objectid][PHY_HighZBound] = FLOAT_INFINITY;
		PHY_Object[objectid][PHY_BoundConst] = 0;
	    
	    if(size == 0.0)
		{
			if(modelid)
			    PHY_Object[objectid][PHY_Size] = GetColSphereRadius(modelid);
		}
		else
			PHY_Object[objectid][PHY_Size] = size;
	    return 1;
	}
	return 0;
}

/* Stops using physics for objectid (doesn't destroy it). */
stock PHY_ObjectDelete(objectid)
{
	PHY_Object[objectid][PHY_Properties] = 0;
	return 1;
}

/* Moves the object with vx, vy, vz velocities. */
stock PHY_MoveObject(objectid, Float:vx, Float:vy, Float:vz = 0.0)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
	    PHY_Object[objectid][PHY_VX] = vx;
	    PHY_Object[objectid][PHY_VY] = vy;
	    PHY_Object[objectid][PHY_VZ] = vz;
	    return 1;
	}
	return 0;
}

/* Self-explanatory */
stock PHY_GetObjectVelocity(objectid, &Float:vx, &Float:vy, &Float:vz)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
	    vx = PHY_Object[objectid][PHY_VX];
	    vy = PHY_Object[objectid][PHY_VY];
	    vz = PHY_Object[objectid][PHY_VZ];
	    return 1;
	}
	return 0;
}

/* Starts rolling the object when it moves of toggle = 1 or stops if toggle = 0. */
stock PHY_RollObject(objectid, toggle = 1)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
	    if(toggle)
	        PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_ROLL;
	    else
	        PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_ROLL;
	    return 1;
	}
	return 0;
}

/* Applies friction to the object, that decelerates of 'acceleration' automatically when it moves. */
stock PHY_SetObjectFriction(objectid, Float:acceleration)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
	    PHY_Object[objectid][PHY_Friction] = acceleration;
	    return 1;
	}
	return 0;
}

/* Limits the object's Z position.
low - The lowest Z that the object can have (you can use FLOAT_NEG_INFINITY). If it is set to NaN it doesn't change.
high - The highest Z that the object can have (you can use FLOAT_INFINITY). If it is set to NaN it doesn't change.
(When you use PHY_InitObject lowest Z is set to the current object's Z and highest Z to FLOAT_INFINITY.
constant - It should be from 0.0 to 1.0. If it is 1.0 the object doesn't lose velocity,
if it is 0.0 the object stops when it bounces. It could be a middle ground.*/
stock PHY_SetObjectZBound(objectid, Float:low = FLOAT_NAN, Float:high = FLOAT_NAN, Float:constant = 0.0)
{
    if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		if(low == low)
		    PHY_Object[objectid][PHY_LowZBound] = low;
		if(high == high)
			PHY_Object[objectid][PHY_HighZBound] = high;
		PHY_Object[objectid][PHY_BoundConst] = constant;
		return 1;
	}
	return 0;
}

/* Sets the gravity's acceleration that the object is subjected to. */
stock PHY_SetObjectGravity(objectid, Float:gravity)
{
    if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		PHY_Object[objectid][PHY_Gravity] = gravity;
		return 1;
	}
	return 0;
}

/* Object and walls collide only if the are in the same world or one of them is in the world 0 (default). */
stock PHY_SetObjectWorld(objectid, world)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
	    PHY_Object[objectid][PHY_World] = world;
	    return 1;
	}
	return 0;
}

/* Creates a collision wall (straight line) from A(x1, y1) to B(x2, y2).
low is the lowest wall's Z, high is the highest. If they're set to default the wall is like infinitely high.*/
stock PHY_CreateWall(Float:x1, Float:y1, Float:x2, Float:y2, Float:low = FLOAT_NEG_INFINITY, Float:high = FLOAT_INFINITY)
{
	for(new i; i < sizeof PHY_Wall; i++)
	{
	    if(!PHY_Wall[i][PHY_Created])
	    {
	        PHY_Wall[i][PHY_Created] = 1;
	        PHY_Wall[i][PHY_World] = 0;
	        if(x2 > x1)
	        {
				PHY_Wall[i][PHY_X1] = x1;
		        PHY_Wall[i][PHY_Y1] = y1;
		        PHY_Wall[i][PHY_X2] = x2;
		        PHY_Wall[i][PHY_Y2] = y2;
	        }
	        else
	        {
				PHY_Wall[i][PHY_X1] = x2;
		        PHY_Wall[i][PHY_Y1] = y2;
		        PHY_Wall[i][PHY_X2] = x1;
		        PHY_Wall[i][PHY_Y2] = y1;
	        }
	        PHY_Wall[i][PHY_Z1] = low;
	        PHY_Wall[i][PHY_Z2] = high;
	        PHY_Wall[i][PHY_A] = -(y2 - y1);
			PHY_Wall[i][PHY_B] = (x2 - x1);
			PHY_Wall[i][PHY_C] = (y2 - y1) * x1 - (x2 - x1) * y1;
	        PHY_Wall[i][PHY_M] = (x2 - x1) != 0 ? (y2 - y1)/(x2 - x1) : FLOAT_NAN;
			PHY_Wall[i][PHY_Q] = -((y2 - y1) * x1)/(x2 - x1) + y1;
	        return i;
	    }
	}
	return -1;
}

/* Self-explanatory */
stock PHY_DestroyWall(wallid)
{
	PHY_Wall[wallid][PHY_Created] = 0;
	return 1;
}

/* Se PHY_SetObjectWorld(objectid, world). */
stock PHY_SetWallWorld(id, world)
{
	if(PHY_Wall[wallid][PHY_Created])
	{
	    PHY_Wall[wallid][PHY_World] = world;
	    return 1;
	}
	return 0;
}


#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit PHY_OnGameModeInit

forward OnGameModeInit();
