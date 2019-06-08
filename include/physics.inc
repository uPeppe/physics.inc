/*  <SA-MP physics.inc - Handle collisions and more.>
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

#pragma compress 1

#if !defined _inc_y_iterate
	#include <foreach>
#endif

#if defined COLANDREAS
	#include <colandreas>
#endif

#if !defined PHY_TIMER_INTERVAL
	#define PHY_TIMER_INTERVAL (20)
#endif
#if !defined PHY_MAX_WALLS	
	#define PHY_MAX_WALLS (512)
#endif
#if !defined PHY_MAX_CYLINDERS
	#define PHY_MAX_CYLINDERS (512)
#endif		

#define PHY_MODE_3D (0)
#define PHY_MODE_2D (1)

#define PHY_ROLLING_MODE_DEFAULT (0)
#define PHY_ROLLING_MODE_ADVANCED (1)

#if !defined FLOAT_INFINITY
	#define FLOAT_INFINITY   (Float:0x7F800000)
#endif
#if !defined FLOAT_NEG_INFINITY
	#define FLOAT_NEG_INFINITY (Float:0xFF800000)
#endif
#if !defined FLOAT_NAN
	#define FLOAT_NAN	 (Float:0xFFFFFFFF)
#endif

/* Callbacks */
forward PHY_OnObjectUpdate(objectid);
forward PHY_OnObjectCollideWithObject(object1, object2);
forward PHY_OnObjectCollideWithZBound(objectid, lowhigh); // low bound = 0, high bound = 1
forward PHY_OnObjectCollideWithSAWorld(objectid, Float:cx, Float:cy, Float:cz);
forward PHY_OnObjectCollideWithWall(objectid, wallid);
forward PHY_OnObjectCollideWithCylinder(objectid, cylinderid);
forward PHY_OnObjectCollideWithPlayer(objectid, playerid);


enum (<<= 1)
{
	PHY_OBJECT_USED = 0b01,
	PHY_OBJECT_MODE,
	PHY_OBJECT_ROLL,
	PHY_OBJECT_ROLLING_MODE,
	PHY_OBJECT_COLANDREAS_BOUNDS,
	PHY_OBJECT_COLANDREAS_COLLS,
	PHY_OBJECT_GHOST_OBJECTS,
	PHY_OBJECT_GHOST_WALLS,
	PHY_OBJECT_GHOST_CYLINDERS,
	PHY_OBJECT_PLAYER_COLLISIONS
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
	Float:PHY_AX,
	Float:PHY_AY,
	Float:PHY_AZ,
	Float:PHY_AngularVelocity,
	Float:PHY_AxisX,
	Float:PHY_AxisY,
	Float:PHY_AxisZ,
	Float:PHY_QW,
	Float:PHY_QX,
	Float:PHY_QY,
	Float:PHY_QZ,
	Float:PHY_Friction,
	Float:PHY_AirResistance,
	Float:PHY_Gravity,
	Float:PHY_LowZBound,
	/*Float:PHY_LowZRX,
	Float:PHY_LowZRY,*/
	Float:PHY_HighZBound,
	Float:PHY_BoundConst,
	Float:PHY_PlayerConst,
	Float:PHY_PlayerDist,
	Float:PHY_PlayerLowZ,
	Float:PHY_PlayerHighZ
}

new
	PHY_Object[MAX_OBJECTS][E_PHY_OBJECT],
	Iterator:ITER_Object<MAX_OBJECTS>;


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
	Float:PHY_BounceConst,
	Float:PHY_ANG
}

new
	PHY_Wall[PHY_MAX_WALLS][E_PHY_WALL],
	Iterator:ITER_Wall<PHY_MAX_WALLS>;


enum E_PHY_CYLINDER
{
	PHY_Created,
	PHY_World,
	Float:PHY_X,
	Float:PHY_Y,
	Float:PHY_Z1,
	Float:PHY_Z2,
	Float:PHY_Size,
	Float:PHY_BounceConst
}

new
	PHY_Cylinder[PHY_MAX_CYLINDERS][E_PHY_CYLINDER],
	Iterator:ITER_Cylinder<PHY_MAX_CYLINDERS>;


enum E_PHY_PLAYER
{
	PHY_World
}

new
	PHY_Player[MAX_PLAYERS][E_PHY_PLAYER];
	

/* Macros are self-explanatory */
#define PHY_IsObjectUsingPhysics(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_USED)
#define PHY_IsObjectUsing3D(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_MODE)
#define PHY_IsObjectUsingColAndreasCollisions(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_COLANDREAS_COLLS)
#define PHY_IsObjectUsingColAndreasBounds(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_COLANDREAS_BOUNDS)
#define PHY_IsObjectGhostWithObjects(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_OBJECTS)
#define PHY_IsObjectGhostWithWalls(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_WALLS)
#define PHY_IsObjectGhostWithCylinders(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_CYLINDERS)
#define PHY_IsObjectCollidingWithPlayers(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_PLAYER_COLLISIONS)
#define PHY_IsObjectMoving(%1) (PHY_Object[%1][PHY_VX] != 0 || PHY_Object[%1][PHY_VY] != 0 || PHY_Object[%1][PHY_VZ] != 0)
#define PHY_IsObjectRolling(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_ROLL)
#define PHY_GetObjectFriction(%1) (PHY_Object[%1][PHY_Friction])
#define PHY_GetObjectAirResistance(%1) (PHY_Object[%1][PHY_AirResistance])
#define PHY_GetObjectGravity(%1) (PHY_Object[%1][PHY_Gravity])
#define PHY_GetObjectMode(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_MODE)
#define PHY_IsObjectRollingModeAdvanced(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_ROLLING_MODE)

static
	cb_Connect;

public OnGameModeInit()
{
	SetTimer("PHY_CoreTimer", PHY_TIMER_INTERVAL, true);
	
	#if defined COLANDREAS
	CA_Init();
	#endif

	cb_Connect = funcidx("PHY_OnPlayerConnect") != -1;
	if(funcidx("PHY_OnGameModeInit") != -1)
		return CallLocalFunction("PHY_OnGameModeInit", "");
	return 1;
}

public OnPlayerConnect(playerid)
{
	PHY_Player[playerid][PHY_World] = 0;

	if(cb_Connect)
		return CallLocalFunction("PHY_OnPlayerConnect", "i", playerid);
	return 1;
}

forward PHY_CoreTimer();
public PHY_CoreTimer()
{
	new
	    #if defined COLANDREAS
		Float:mod,
		Float:crx,
		Float:cry,
		Float:crz,
		Float:cx,
		Float:cy,
		Float:cz,
		#endif
		
		Float:x,
		Float:y,
		Float:z,
		Float:x1,
		Float:y1,
		Float:z1,
		Float:xclosest,
		Float:yclosest,
		Float:x2,
		Float:y2,
		Float:z2,
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
		Float:dvz,
		Float:mag,
		Float:tmpvx,
		Float:tmpvy,
		Float:tmpvz,
		Float:newvy1,
		Float:newvy2,
		Float:newvx1,
		Float:newvx2,
		Float:newvz1,
		Float:newvz2
		/*Float:sx,
		Float:sy*/;
 	foreach(new a : ITER_Object)
	{
		if(PHY_Object[a][PHY_Properties] & PHY_OBJECT_USED)
		{
			GetObjectPos(a, x, y, z);
			
			x1 = x + PHY_Object[a][PHY_VX] * (PHY_TIMER_INTERVAL/1000.0);
			y1 = y + PHY_Object[a][PHY_VY] * (PHY_TIMER_INTERVAL/1000.0);
			if(PHY_GetObjectMode(a) == PHY_MODE_3D)
			{
				z1 = z + PHY_Object[a][PHY_VZ] * (PHY_TIMER_INTERVAL/1000.0);
				
				if(z1 > PHY_Object[a][PHY_HighZBound])
				{
					if(PHY_Object[a][PHY_VZ] > 0)
					{
						PHY_Object[a][PHY_VZ] = -PHY_Object[a][PHY_VZ] * PHY_Object[a][PHY_BoundConst];
						CallLocalFunction("PHY_OnObjectCollideWithZBound", "dd", a, 1);
					}
					z1 = PHY_Object[a][PHY_HighZBound];
				}
				else if(z1 < PHY_Object[a][PHY_LowZBound])
				{
					if(PHY_Object[a][PHY_VZ] < 0)
					{
						PHY_Object[a][PHY_VZ] = -PHY_Object[a][PHY_VZ] * PHY_Object[a][PHY_BoundConst];
						CallLocalFunction("PHY_OnObjectCollideWithZBound", "dd", a, 0);
					}
					z1 = PHY_Object[a][PHY_LowZBound];
				}
				if(PHY_GetObjectGravity(a) != 0)
				{
					// ColAndreas Incline testing
					/*if(z1 == PHY_Object[a][PHY_LowZBound])
					{
						if(PHY_Object[a][PHY_LowZRX] != 0.0 || PHY_Object[a][PHY_LowZRY] != 0.0)
						{
							sx = floatsin(PHY_Object[a][PHY_LowZRY], degrees);
							cx = floatcos(PHY_Object[a][PHY_LowZRY], degrees);
							sy = floatsin(-PHY_Object[a][PHY_LowZRX], degrees);
							cy = floatcos(-PHY_Object[a][PHY_LowZRX], degrees);
							PHY_Object[a][PHY_VZ] -= PHY_Object[a][PHY_Gravity] * (PHY_TIMER_INTERVAL/1000.0) * (sx * sx + sy * sy);
							PHY_Object[a][PHY_VX] += PHY_Object[a][PHY_Gravity] * (PHY_TIMER_INTERVAL/1000.0) * cx * sx;
							PHY_Object[a][PHY_VY] += PHY_Object[a][PHY_Gravity] * (PHY_TIMER_INTERVAL/1000.0) * cy * sy;

						}
					}*/
					
					if(PHY_Object[a][PHY_VZ] > 0)
					{
						PHY_Object[a][PHY_VZ] -= PHY_Object[a][PHY_Gravity] * (PHY_TIMER_INTERVAL/1000.0);
						if(PHY_Object[a][PHY_VZ] < 0)
							PHY_Object[a][PHY_VZ] = 0;
					}
					else
						PHY_Object[a][PHY_VZ] -= PHY_Object[a][PHY_Gravity] * (PHY_TIMER_INTERVAL/1000.0);
				}
			}
			else
				z1 = z;
			if(PHY_IsObjectMoving(a))
			{
				if(!PHY_IsObjectGhostWithObjects(a))
				{
					foreach(new b : ITER_Object)
					{
						if(a != b && PHY_Object[b][PHY_Properties] & PHY_OBJECT_USED && !PHY_IsObjectGhostWithObjects(b) && (!PHY_Object[a][PHY_World] || !PHY_Object[b][PHY_World] || PHY_Object[a][PHY_World] == PHY_Object[b][PHY_World]))
						{
							GetObjectPos(b, x2, y2, z2);
							dx = x1 - x2;
							dy = y1 - y2;
							dz = (PHY_GetObjectMode(a) == PHY_MODE_3D && PHY_GetObjectMode(b) == PHY_MODE_3D) ? (z1 - z2) : (0.0);
							dist = (dx * dx) + (dy * dy) + (dz * dz);
							maxdist = PHY_Object[a][PHY_Size] + PHY_Object[b][PHY_Size];
							if(dist < (maxdist * maxdist))
							{
								dvx = PHY_Object[a][PHY_VX] - PHY_Object[b][PHY_VX];
								dvy = PHY_Object[a][PHY_VY] - PHY_Object[b][PHY_VY];
								dvz = PHY_Object[a][PHY_VZ] - PHY_Object[b][PHY_VZ];
								mag = dvx * dx + dvy * dy + dvz * dz;

								if(mag < 0.0)
								{
									newvx1 = PHY_Object[a][PHY_VX];
									newvy1 = PHY_Object[a][PHY_VY];
									newvz1 = PHY_Object[a][PHY_VZ];
									newvx2 = PHY_Object[b][PHY_VX];
									newvy2 = PHY_Object[b][PHY_VY];
									newvz2 = PHY_Object[b][PHY_VZ];

									projectVonU(PHY_Object[a][PHY_VX], PHY_Object[a][PHY_VY], PHY_Object[a][PHY_VZ], dx, dy, dz, tmpvx, tmpvy, tmpvz);
									newvx1 -= tmpvx;
									newvy1 -= tmpvy;
									newvz1 -= tmpvz;
									projectVonU(PHY_Object[b][PHY_VX], PHY_Object[b][PHY_VY], PHY_Object[b][PHY_VZ], -dx, -dy, -dz, tmpvx, tmpvy, tmpvz);
									tmpvx = ((PHY_Object[a][PHY_Mass] - PHY_Object[b][PHY_Mass]) * PHY_Object[a][PHY_VX] + 2 * PHY_Object[b][PHY_Mass] * tmpvx) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
									tmpvy = ((PHY_Object[a][PHY_Mass] - PHY_Object[b][PHY_Mass]) * PHY_Object[a][PHY_VY] + 2 * PHY_Object[b][PHY_Mass] * tmpvy) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
									tmpvz = ((PHY_Object[a][PHY_Mass] - PHY_Object[b][PHY_Mass]) * PHY_Object[a][PHY_VZ] + 2 * PHY_Object[b][PHY_Mass] * tmpvz) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
									newvx1 += tmpvx;
									newvy1 += tmpvy;
									newvz1 += tmpvz;

									projectVonU(PHY_Object[b][PHY_VX], PHY_Object[b][PHY_VY], PHY_Object[b][PHY_VZ], dx, dy, dz, tmpvx, tmpvy, tmpvz);
									newvx2 -= tmpvx;
									newvy2 -= tmpvy;
									newvz2 -= tmpvz;
									projectVonU(PHY_Object[a][PHY_VX], PHY_Object[a][PHY_VY], PHY_Object[a][PHY_VZ], -dx, -dy, -dz, tmpvx, tmpvy, tmpvz);
									tmpvx = ((PHY_Object[b][PHY_Mass] - PHY_Object[a][PHY_Mass]) * PHY_Object[b][PHY_VX] + 2 * PHY_Object[a][PHY_Mass] * tmpvx) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
									tmpvy = ((PHY_Object[b][PHY_Mass] - PHY_Object[a][PHY_Mass]) * PHY_Object[b][PHY_VY] + 2 * PHY_Object[a][PHY_Mass] * tmpvy) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
									tmpvz = ((PHY_Object[b][PHY_Mass] - PHY_Object[a][PHY_Mass]) * PHY_Object[b][PHY_VZ] + 2 * PHY_Object[a][PHY_Mass] * tmpvz) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
									newvx2 += tmpvx;
									newvy2 += tmpvy;
									newvz2 += tmpvz;

									PHY_Object[a][PHY_VX] = newvx1;
									PHY_Object[a][PHY_VY] = newvy1;
									PHY_Object[a][PHY_VZ] = newvz1;

									PHY_Object[b][PHY_VX] = newvx2;
									PHY_Object[b][PHY_VY] = newvy2;
									PHY_Object[b][PHY_VZ] = newvz2;

									CallLocalFunction("PHY_OnObjectCollideWithObject", "dd", a, b);
								}
								
								/*
								// Old algorithm, works good only in 2D
								dvx = PHY_Object[a][PHY_VX] - PHY_Object[b][PHY_VX];
								dvy = PHY_Object[a][PHY_VY] - PHY_Object[b][PHY_VY];
								mag = dvx * dx + dvy * dy;
								
								if(mag < 0.0)
								{
						   			angle = -atan2(dy, dx);
									tmpvx1 = PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees);
									newvy1 = PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees);
									tmpvx2 = PHY_Object[b][PHY_VX] * floatcos(angle, degrees) - PHY_Object[b][PHY_VY] * floatsin(angle, degrees);
									newvy2 = PHY_Object[b][PHY_VX] * floatsin(angle, degrees) + PHY_Object[b][PHY_VY] * floatcos(angle, degrees);

									if(PHY_Object[a][PHY_Mass] == FLOAT_INFINITY)
										newvx2 = -tmpvx2;
									else if(PHY_Object[b][PHY_Mass] == FLOAT_INFINITY)
										newvx1 = -tmpvx1;
						   			else
							   		{
							   			newvx1 = ((PHY_Object[a][PHY_Mass] - PHY_Object[b][PHY_Mass]) * tmpvx1 + 2 * PHY_Object[b][PHY_Mass] * tmpvx2) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
										newvx2 = ((PHY_Object[b][PHY_Mass] - PHY_Object[a][PHY_Mass]) * tmpvx2 + 2 * PHY_Object[a][PHY_Mass] * tmpvx1) / (PHY_Object[a][PHY_Mass] + PHY_Object[b][PHY_Mass]);
									}

									angle = -angle;
									PHY_Object[a][PHY_VX] = newvx1 * floatcos(angle, degrees) - newvy1 * floatsin(angle, degrees);
									PHY_Object[a][PHY_VY] = newvx1 * floatsin(angle, degrees) + newvy1 * floatcos(angle, degrees);
						   			PHY_Object[b][PHY_VX] = newvx2 * floatcos(angle, degrees) - newvy2 * floatsin(angle, degrees);
									PHY_Object[b][PHY_VY] = newvx2 * floatsin(angle, degrees) + newvy2 * floatcos(angle, degrees);

									CallLocalFunction("PHY_OnObjectCollideWithObject", "dd", a, b);
								}
								*/
							}
						}
					}
				}

				#if defined COLANDREAS
				if(PHY_IsObjectUsingColAndreasBounds(a))
				{
					if(PHY_GetObjectMode(a) == PHY_MODE_3D)
						PHY_UpdateBounds(a, x1, y1, z1);
				}
				
				if(PHY_IsObjectUsingColAndreasCollisions(a))
				{
					mod = floatsqroot(PHY_Object[a][PHY_VX] * PHY_Object[a][PHY_VX] + PHY_Object[a][PHY_VY] * PHY_Object[a][PHY_VY]);
					cx = 0.0;
					cy = 0.0;
					cz = 0.0;
					crx = 0.0;
					cry = 0.0;
					crz = 0.0;
					CA_RayCastLineAngle(x1, y1, z1, x1 + PHY_Object[a][PHY_Size] * (PHY_Object[a][PHY_VX] / mod), y1 + PHY_Object[a][PHY_Size] * (PHY_Object[a][PHY_VY] / mod), z1, cx, cy, cz, crx, cry, crz);
					if((cx != 0.0 || cy != 0.0 || cz != 0.0) && cx == cx && cy == cy && cz == cz && (crx != 0.0 || cry != 0.0) && crx == crx && cry == cry && crz == crz)
					{
						angle = atan2(-cry, crx);
						newvx1 = (PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees));
						newvy1 = -(PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees));
						angle = -angle;
						PHY_Object[a][PHY_VX] = newvx1 * floatcos(angle, degrees) - newvy1 * floatsin(angle, degrees);
						PHY_Object[a][PHY_VY] = newvx1 * floatsin(angle, degrees) + newvy1 * floatcos(angle, degrees);
						
						angle = angle + (newvy1 > 0 ? 90.0 : -90.0);
						x1 = cx + (PHY_Object[a][PHY_Size] + 0.001) * floatcos(angle, degrees);
						y1 = cy + (PHY_Object[a][PHY_Size] + 0.001) * floatsin(angle, degrees);
						
						CallLocalFunction("PHY_OnObjectCollideWithSAWorld", "dfff", a, cx, cy, cz);
					}
				}
				#endif
				
				if(!PHY_IsObjectGhostWithWalls(a))
				{
					foreach(new w : ITER_Wall)
					{
						if(PHY_Wall[w][PHY_Created])
						{
							if((!PHY_Object[a][PHY_World] || !PHY_Wall[w][PHY_World] || PHY_Object[a][PHY_World] == PHY_Wall[w][PHY_World]))
							{
								//dist = (y1 - PHY_Wall[w][PHY_M] * x1 - PHY_Wall[w][PHY_Q])/floatsqroot(1 + PHY_Wall[w][PHY_M] * PHY_Wall[w][PHY_M]);
								//dist = (PHY_Wall[w][PHY_A] * x1 + PHY_Wall[w][PHY_B] * y1 + PHY_Wall[w][PHY_C])/floatsqroot(PHY_Wall[w][PHY_A] * PHY_Wall[w][PHY_A] + PHY_Wall[w][PHY_B] * PHY_Wall[w][PHY_B]);
								angle = PHY_Wall[w][PHY_ANG];
								if(PHY_Wall[w][PHY_Z1] - PHY_Object[a][PHY_Size] < z1 < PHY_Wall[w][PHY_Z2] + PHY_Object[a][PHY_Size] &&
								(check_segment_intersection(PHY_Wall[w][PHY_X1], PHY_Wall[w][PHY_Y1], PHY_Wall[w][PHY_X2], PHY_Wall[w][PHY_Y2], x1, y1, PHY_Object[a][PHY_Size], xclosest, yclosest) || /* && floatabs(dist) < PHY_Object[a][PHY_Size]*/
								check_segment_intersection(PHY_Wall[w][PHY_X1], PHY_Wall[w][PHY_Y1], PHY_Wall[w][PHY_X2], PHY_Wall[w][PHY_Y2], (x + x1)/2, (y + y1)/2, PHY_Object[a][PHY_Size], xclosest, yclosest)))
								{
									//mag = y1 + PHY_Object[a][PHY_Size] * floatcos(-moveangle, degrees) - (x1 + PHY_Object[a][PHY_Size] * floatsin(-moveangle, degrees)) * PHY_Wall[w][PHY_M] - PHY_Wall[w][PHY_Q];
									//mag = PHY_Wall[w][PHY_A] * (x1 + PHY_Object[a][PHY_Size] * floatsin(-moveangle, degrees)) + PHY_Wall[w][PHY_B] * (y1 + PHY_Object[a][PHY_Size] * floatcos(-moveangle, degrees)) + PHY_Wall[w][PHY_C];
									//if((dist >= 0) ? (mag <= 0) : (mag >= 0))
				   			 		newvx1 = PHY_Wall[w][PHY_BounceConst] * (PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees));
									newvy1 = -PHY_Wall[w][PHY_BounceConst] * (PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees));
									angle = -angle;
									PHY_Object[a][PHY_VX] = newvx1 * floatcos(angle, degrees) - newvy1 * floatsin(angle, degrees);
									PHY_Object[a][PHY_VY] = newvx1 * floatsin(angle, degrees) + newvy1 * floatcos(angle, degrees);

									angle = angle + (newvy1 > 0 ? 90.0 : -90.0);
									x1 = xclosest + (PHY_Object[a][PHY_Size] + 0.001) * floatcos(angle, degrees);
									y1 = yclosest + (PHY_Object[a][PHY_Size] + 0.001) * floatsin(angle, degrees);

									CallLocalFunction("PHY_OnObjectCollideWithWall", "dd", a, w);
								}
							}
						}
						else
							Iter_SafeRemove(ITER_Wall, w, w);
					}
				}
				if(!PHY_IsObjectGhostWithCylinders(a))
				{
					foreach(new c : ITER_Cylinder)
					{
						if(PHY_Cylinder[c][PHY_Created])
						{
							if((!PHY_Object[a][PHY_World] || !PHY_Cylinder[c][PHY_World] || PHY_Object[a][PHY_World] == PHY_Cylinder[c][PHY_World]))
							{
								if(PHY_Cylinder[c][PHY_Z1] - PHY_Object[a][PHY_Size] < z1 < PHY_Cylinder[c][PHY_Z2] + PHY_Object[a][PHY_Size])
								{
									x2 = PHY_Cylinder[c][PHY_X];
									y2 = PHY_Cylinder[c][PHY_Y];
									dx = x1 - x2;
									dy = y1 - y2;
									dist = (dx * dx) + (dy * dy);
									maxdist = PHY_Object[a][PHY_Size] + PHY_Cylinder[c][PHY_Size];
									if(dist < (maxdist * maxdist))
									{
										mag = PHY_Object[a][PHY_VX] * dx + PHY_Object[a][PHY_VY] * dy;

										if(mag < 0.0)
										{
								   			angle = -atan2(dy, dx);
											newvx1 = -PHY_Cylinder[c][PHY_BounceConst] * (PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees));
											newvy1 = PHY_Cylinder[c][PHY_BounceConst] * (PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees));

											angle = -angle;
											PHY_Object[a][PHY_VX] = newvx1 * floatcos(angle, degrees) - newvy1 * floatsin(angle, degrees);
											PHY_Object[a][PHY_VY] = newvx1 * floatsin(angle, degrees) + newvy1 * floatcos(angle, degrees);

											CallLocalFunction("PHY_OnObjectCollideWithCylinder", "dd", a, c);
										}
									}
								}
							}
						}
						else
							Iter_SafeRemove(ITER_Cylinder, c, c);
					}
				}
				if(PHY_IsObjectCollidingWithPlayers(a))
				{
					foreach(new i : Player)
					{
						if((!PHY_Object[a][PHY_World] || !PHY_Player[i][PHY_World] || PHY_Object[a][PHY_World] == PHY_Player[i][PHY_World]))
						{
							GetPlayerPos(i, x2, y2, z2);
								
							if(z2 - PHY_Object[a][PHY_PlayerLowZ] - PHY_Object[a][PHY_Size] < z1 < z2 + PHY_Object[a][PHY_PlayerHighZ] + PHY_Object[a][PHY_Size])
							{
								dx = x1 - x2;
								dy = y1 - y2;
								dist = (dx * dx) + (dy * dy);
								maxdist = PHY_Object[a][PHY_Size] + PHY_Object[a][PHY_PlayerDist];
								if(dist < (maxdist * maxdist))
								{
									mag = PHY_Object[a][PHY_VX] * dx + PHY_Object[a][PHY_VY] * dy;

									if(mag < 0.0)
									{
							   			angle = -atan2(dy, dx);
										newvx1 = -PHY_Object[a][PHY_PlayerConst] * (PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees));
										newvy1 = PHY_Object[a][PHY_PlayerConst] * (PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees));

										angle = -angle;
										PHY_Object[a][PHY_VX] = newvx1 * floatcos(angle, degrees) - newvy1 * floatsin(angle, degrees);
										PHY_Object[a][PHY_VY] = newvx1 * floatsin(angle, degrees) + newvy1 * floatcos(angle, degrees);

										CallLocalFunction("PHY_OnObjectCollideWithPlayer", "dd", a, i);
									}
								}
							}
						}
					}
				}
				moveangle = atan2(PHY_Object[a][PHY_VY], PHY_Object[a][PHY_VX]) - 90.0;
				speed = floatsqroot(PHY_Object[a][PHY_VX] * PHY_Object[a][PHY_VX] + PHY_Object[a][PHY_VY] * PHY_Object[a][PHY_VY]);
				if(PHY_GetObjectFriction(a) != 0 && z1 == PHY_Object[a][PHY_LowZBound])
				{
					speed -= PHY_Object[a][PHY_Friction] * (PHY_TIMER_INTERVAL/1000.0);
					if(speed < 0.001)
						speed = 0;
					PHY_Object[a][PHY_VX] = speed * floatsin(-moveangle, degrees);
					PHY_Object[a][PHY_VY] = speed * floatcos(-moveangle, degrees);
				}
				if(PHY_GetObjectAirResistance(a) != 0)
				{
					PHY_Object[a][PHY_VX] -= PHY_Object[a][PHY_VX] * PHY_Object[a][PHY_AirResistance] * (PHY_TIMER_INTERVAL/1000.0);
					PHY_Object[a][PHY_VY] -= PHY_Object[a][PHY_VY] * PHY_Object[a][PHY_AirResistance] * (PHY_TIMER_INTERVAL/1000.0);
					PHY_Object[a][PHY_VZ] -= PHY_Object[a][PHY_VZ] * PHY_Object[a][PHY_AirResistance] * (PHY_TIMER_INTERVAL/1000.0);
				}
				if(PHY_IsObjectRolling(a) && speed > 0.0)
				    if(PHY_IsObjectRollingModeAdvanced(a))
				        PHY_ApplyQuaternionsRotation(a, speed, moveangle);
				    else
						PHY_ApplyRotation(a, speed, moveangle);
			}
			
			PHY_Object[a][PHY_VX] += PHY_Object[a][PHY_AX];
			PHY_Object[a][PHY_VY] += PHY_Object[a][PHY_AY];
			PHY_Object[a][PHY_VZ] += PHY_Object[a][PHY_AZ];
			
			SetObjectPos(a, x1, y1, z1);
			CallLocalFunction("PHY_OnObjectUpdate", "d", a);
		}
		else
			Iter_SafeRemove(ITER_Object, a, a);
	}
	return 1;
}


/* Starts using physics for objectid.
modelid - object's modelid, used to get its size with modelsizes include.
mass - object's mass, it is like its weight and is used in collisions.
size - object's sphere radius, taken from modelsizes.inc by default.
mode - PHY_MODE_3D or PHY_MODE_2D. */
stock PHY_InitObject(objectid, modelid = 0, Float:mass = 1.0, Float:size = FLOAT_NAN, mode = PHY_MODE_3D)
{
	if(IsValidObject(objectid))
	{
		PHY_Object[objectid][PHY_Properties] = PHY_OBJECT_USED | (mode ? PHY_OBJECT_MODE : 0);
		PHY_Object[objectid][PHY_Mass] = mass;
		PHY_Object[objectid][PHY_World] = 0;
		PHY_Object[objectid][PHY_VX] = 0;
		PHY_Object[objectid][PHY_VY] = 0;
		PHY_Object[objectid][PHY_VZ] = 0;
		PHY_Object[objectid][PHY_Gravity] = 0;

		new
			Float:x, Float:y, Float:z;
		GetObjectPos(objectid, x, y, z);
		PHY_Object[objectid][PHY_LowZBound] = z;
		PHY_Object[objectid][PHY_HighZBound] = FLOAT_INFINITY;
		PHY_Object[objectid][PHY_BoundConst] = 0;
		
		if(size != size)
		{
			if(modelid)
				PHY_Object[objectid][PHY_Size] = GetColSphereRadius(modelid);
		}
		else
			PHY_Object[objectid][PHY_Size] = size;
			
		Iter_Add(ITER_Object, objectid);
				
		return 1;
	}
	return 0;
}

/* Stops using physics for objectid (doesn't destroy it). */
stock PHY_DeleteObject(objectid)
{
	PHY_Object[objectid][PHY_Properties] = 0;
	// Iter_Remove(ITER_Object, objectid);
	return 1;
}

/* Moves the object with vx, vy, vz velocities. */
stock PHY_SetObjectVelocity(objectid, Float:vx, Float:vy, Float:vz = 0.0)
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

/* Sets the object's acceleration. */
stock PHY_SetObjectAcceleration(objectid, Float:ax, Float:ay, Float:az = 0.0)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		PHY_Object[objectid][PHY_AX] = ax;
		PHY_Object[objectid][PHY_AY] = ay;
		PHY_Object[objectid][PHY_AZ] = az;
		return 1;
	}
	return 0;
}

/* Self-explanatory */
stock PHY_GetObjectAcceleration(objectid, &Float:ax, &Float:ay, &Float:az)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		ax = PHY_Object[objectid][PHY_AX];
		ay = PHY_Object[objectid][PHY_AY];
		az = PHY_Object[objectid][PHY_AZ];
		return 1;
	}
	return 0;
}

/* Self-explanatory */
stock PHY_GetObjectSpeed(objectid, &Float:speed, _3D = 0)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		speed = floatsqroot(PHY_Object[objectid][PHY_VX] * PHY_Object[objectid][PHY_VX] + PHY_Object[objectid][PHY_VY] * PHY_Object[objectid][PHY_VY] + _3D ? (PHY_Object[objectid][PHY_VZ] * PHY_Object[objectid][PHY_VZ]) : 0.0);
		return 1;
	}
	return 0;
}

/* Self-explanatory */
stock PHY_GetObjectMoveAngle(objectid, &Float:moveangle)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		moveangle = atan2(PHY_Object[objectid][PHY_VY], PHY_Object[objectid][PHY_VX]) - 90.0;
		return 1;
	}
	return 0;
}

/* Sets ColAndreas mode for the object. Modes: 0 none, 1 collisions + z bounds, 2 collisions only, 3 z bounds only*/
stock PHY_UseColAndreas(objectid, mode = 1)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		new Float:x, Float:y, Float:z;
		GetObjectPos(objectid, x, y, z);
		
		switch(mode)
		{
			case 0:
			{
				PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_COLANDREAS_BOUNDS;
				PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_COLANDREAS_COLLS;
			}
			case 1:
			{
				PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_COLANDREAS_BOUNDS;
				PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_COLANDREAS_COLLS;
				PHY_UpdateBounds(objectid, x, y, z);
			}
			case 2:
			{
				PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_COLANDREAS_BOUNDS;
				PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_COLANDREAS_COLLS;
			}
			case 3:
			{
				PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_COLANDREAS_BOUNDS;
				PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_COLANDREAS_COLLS;
				PHY_UpdateBounds(objectid, x, y, z);
			}
		}
		return 1;
	}
	return 0;
}

/* Starts rolling the object when it moves of toggle = 1 or stops if toggle = 0.
rollingmode = PHY_ROLLING_MODE_DEFAULT (Euler angles) or PHY_ROLLING_MODE_ADVANCED (Quaternions)
When using advanced rolling mode, if you manually use SetObjectRot in your script, it is adviced to
call PHY_RollObject again, in order to recalculate its quaternion angles.*/
stock PHY_RollObject(objectid, toggle = 1, rollingmode = PHY_ROLLING_MODE_DEFAULT)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		if(toggle)
			PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_ROLL;
		else
			PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_ROLL;
			
        if(rollingmode == PHY_ROLLING_MODE_ADVANCED){
			PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_ROLLING_MODE;
			
			new
			    Float:rx, Float:ry, Float:rz;
			GetObjectRot(objectid, rx, ry, rz);
			PHY_GetRotationQuaternion(rx, ry, rz, PHY_Object[objectid][PHY_QW], PHY_Object[objectid][PHY_QX], PHY_Object[objectid][PHY_QY], PHY_Object[objectid][PHY_QZ]);
        }
		else
			PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_ROLLING_MODE;
		return 1;
	}
	return 0;
}

/* Applies friction to the object when it moves on the floor. */
stock PHY_SetObjectFriction(objectid, Float:friction)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		if(friction >= 0.0)
			PHY_Object[objectid][PHY_Friction] = friction;
		return 1;
	}
	return 0;
}

/* Applies air resistance to the object when it moves. */
stock PHY_SetObjectAirResistance(objectid, Float:resistance)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		if(0.0 <= resistance <= 1.0)
			PHY_Object[objectid][PHY_AirResistance] = resistance;
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

/* Toggles object's collisions with players.
- constant - It should be from 0.0 to 1.0. If it is 1.0 the object doesn't lose velocity,
if it is 0.0 the object stops when it bounces. It could be a middle ground.
- distoffset - The distance at which the object collides with the player.
- zoffsetlow/zoffsethigh - The max Z distance (downward/upward) at which the object collides with the player. */
stock PHY_ToggleObjectPlayerColls(objectid, toggle = 1, Float:constant = 1.0, Float:distoffset = 0.8, Float:zoffsetlow = 1.0, Float:zoffsethigh = 1.0)
{
	if(PHY_Object[objectid][PHY_Properties] & PHY_OBJECT_USED)
	{
		if(toggle)
		{
			PHY_Object[objectid][PHY_Properties] |= PHY_OBJECT_PLAYER_COLLISIONS;
			PHY_Object[objectid][PHY_PlayerConst] = constant;
			PHY_Object[objectid][PHY_PlayerDist] = distoffset;
			PHY_Object[objectid][PHY_PlayerLowZ] = zoffsetlow;
			PHY_Object[objectid][PHY_PlayerHighZ] = zoffsethigh;
		}
		else
			PHY_Object[objectid][PHY_Properties] &= ~PHY_OBJECT_PLAYER_COLLISIONS;
		return 1;
	}
	return 0;
}

/* Used internally to rotate the objects */
stock PHY_ApplyRotation(objectid, Float:speed, Float:moveangle)
{
	new
		Float:rx, Float:ry, Float:rz;

	GetObjectRot(objectid, rx, ry, rz);
	rx -= speed * (PHY_TIMER_INTERVAL/1000.0) * (180.0/3.14159) / PHY_Object[objectid][PHY_Size];
	if(rx < 0.0)
		rx += 360.0;
	rz = moveangle;
		
	SetObjectRot(objectid, rx, ry, rz);

	return 1;
}

/* New mode of rolling, based on Quaternions */
stock PHY_ApplyQuaternionsRotation(objectid, Float:speed, Float:moveangle)
{
	new
		Float:rx, Float:ry, Float:rz,
		Float:dx, Float:dy,
		Float:q_roll[4],
		Float:vx, Float:vy, Float:vz,
		Float:quat[4];
		
    dx = floatsin(-moveangle, degrees);
	dy = floatcos(-moveangle, degrees);

	PHY_vectorcrossp(dx, dy, 0.0, 0.0, 0.0, 1.0, vx, vy, vz);
	PHY_QuatFromAxisAngle(vx, vy, vz, speed * (PHY_TIMER_INTERVAL/1000.0) * (180.0/3.14159) / PHY_Object[objectid][PHY_Size], q_roll[0], q_roll[1], q_roll[2], q_roll[3]);
	
	quat[0] = PHY_Object[objectid][PHY_QW];
	quat[1] = PHY_Object[objectid][PHY_QX];
	quat[2] = PHY_Object[objectid][PHY_QY];
	quat[3] = PHY_Object[objectid][PHY_QZ];
	PHY_GetQuatProduct(quat, q_roll, quat);
	
	PHY_Object[objectid][PHY_QW] = quat[0];
	PHY_Object[objectid][PHY_QX] = quat[1];
	PHY_Object[objectid][PHY_QY] = quat[2];
	PHY_Object[objectid][PHY_QZ] = quat[3];
	
	PHY_GetQuaternionAngles(quat[0], quat[1], quat[2], quat[3], rx, ry, rz);
    
	SetObjectRot(objectid, rx, ry, rz);
	return 1;
}

/* Creates a collision wall (straight line) from A(x1, y1) to B(x2, y2).
constant should be from 0.0 to 1.0. If it is 1.0 the object doesn't lose velocity,
if it is 0.0 the object stops when it collides.
low is the lowest wall's Z, high is the highest. If they're set to default the wall is like infinitely high.*/
stock PHY_CreateWall(Float:x1, Float:y1, Float:x2, Float:y2, Float:constant = 1.0, Float:low = FLOAT_NEG_INFINITY, Float:high = FLOAT_INFINITY)
{
	new
		i = Iter_Free(ITER_Wall);
	if(i != -1)
	{
		if(!PHY_Wall[i][PHY_Created])
		{
			PHY_Wall[i][PHY_Created] = 1;
			PHY_Wall[i][PHY_World] = 0;
			PHY_Wall[i][PHY_X1] = x1;
			PHY_Wall[i][PHY_Y1] = y1;
			PHY_Wall[i][PHY_X2] = x2;
			PHY_Wall[i][PHY_Y2] = y2;
			PHY_Wall[i][PHY_Z1] = low;
			PHY_Wall[i][PHY_Z2] = high;
			PHY_Wall[i][PHY_BounceConst] = constant;
			PHY_Wall[i][PHY_ANG] = atan2(y1 - y2, x1 - x2);
			/*PHY_Wall[i][PHY_A] = -(y2 - y1);
			PHY_Wall[i][PHY_B] = (x2 - x1);
			PHY_Wall[i][PHY_C] = (y2 - y1) * x1 - (x2 - x1) * y1;*/
			//PHY_Wall[i][PHY_Q] = -((y2 - y1) * x1)/(x2 - x1) + y1;
			
			Iter_Add(ITER_Wall, i);
			return i;
		}
	}
	return -1;
}

/* Creates four walls that form an area. Works like IsPlayerInArea. */
stock PHY_CreateArea(Float:minX, Float:minY, Float:maxX, Float:maxY, Float:constant = 1.0, Float:low = FLOAT_NEG_INFINITY, Float:high = FLOAT_INFINITY)
{
	PHY_CreateWall(minX, minY, minX, maxY, constant, low, high);
	PHY_CreateWall(minX, maxY, maxX, maxY, constant, low, high);
	PHY_CreateWall(maxX, maxY, maxX, minY, constant, low, high);
	PHY_CreateWall(maxX, minY, minX, minY, constant, low, high);
}

/* Self-explanatory */
stock PHY_DestroyWall(wallid)
{
	PHY_Wall[wallid][PHY_Created] = 0;
	// Iter_Remove(ITER_Wall, wallid);
	return 1;
}

/* See PHY_SetObjectWorld(objectid, world). */
stock PHY_SetWallWorld(wallid, world)
{
	if(PHY_Wall[wallid][PHY_Created])
	{
		PHY_Wall[wallid][PHY_World] = world;
		return 1;
	}
	return 0;
}

/* Creates a collision cylinder at position x, y.
constant should be from 0.0 to 1.0. If it is 1.0 the object doesn't lose velocity,
if it is 0.0 the object stops when it collides.
low is the lowest cylinder's Z, high is the highest. If they're set to default the cylinder is like infinitely high.*/
stock PHY_CreateCylinder(Float:x, Float:y, Float:size, Float:constant = 1.0, Float:low = FLOAT_NEG_INFINITY, Float:high = FLOAT_INFINITY)
{
	new
		i = Iter_Free(ITER_Cylinder);
	if(i != -1)
	{
		if(!PHY_Cylinder[i][PHY_Created])
		{
			PHY_Cylinder[i][PHY_Created] = 1;
			PHY_Cylinder[i][PHY_World] = 0;
			PHY_Cylinder[i][PHY_X] = x;
			PHY_Cylinder[i][PHY_Y] = y;
			PHY_Cylinder[i][PHY_Size] = size;
			PHY_Cylinder[i][PHY_Z1] = low;
			PHY_Cylinder[i][PHY_Z2] = high;
			PHY_Cylinder[i][PHY_BounceConst] = constant;
			
			Iter_Add(ITER_Cylinder, i);
			return i;
		}
	}
	return -1;
}

/* Self-explanatory */
stock PHY_DestroyCylinder(cylinderid)
{
	PHY_Cylinder[cylinderid][PHY_Created] = 0;
	// Iter_Remove(ITER_Cylinder, cylinderid);
	return 1;
}

/* See PHY_SetObjectWorld(objectid, world). */
stock PHY_SetCylinderWorld(cylinderid, world)
{
	if(PHY_Cylinder[cylinderid][PHY_Created])
	{
		PHY_Cylinder[cylinderid][PHY_World] = world;
		return 1;
	}
	return 0;
}

/* See PHY_SetObjectWorld(objectid, world). */
stock PHY_SetPlayerWorld(playerid, world)
{
	PHY_Player[playerid][PHY_World] = world;
	return 1;
}

/* Privates */

stock Float:vectordotp(Float:v1x, Float:v1y, Float:v1z, Float:v2x, Float:v2y, Float:v2z)
	return (v1x * v2x + v1y * v2y + v1z * v2z);

stock projectVonU(Float:vx, Float:vy, Float:vz, Float:ux, Float:uy, Float:uz, &Float:x, &Float:y, &Float:z)
{
	new Float:fac = vectordotp(vx, vy, vz, ux, uy, uz) / vectordotp(ux, uy, uz, ux, uy, uz);
	x = ux * fac;
	y = uy * fac;
	z = uz * fac;
}

stock check_segment_intersection(Float:x1, Float:y1, Float:x2, Float:y2, Float:xc, Float:yc, Float:r, &Float:x, &Float:y)
{
	new Float:v1[2];
	new Float:v2[2];
	v1[0] = x2 - x1;
	v1[1] = y2 - y1;
	v2[0] = xc - x1;
	v2[1] = yc - y1;
	new Float:v1_len = floatsqroot(v1[0] * v1[0] + v1[1] * v1[1]);
	v1[0] /= v1_len;
	v1[1] /= v1_len;
	new Float:proj = vectordotp(v2[0], v2[1], 0.0, v1[0], v1[1], 0.0);
	x = proj * v1[0] + x1;
	y = proj * v1[1] + y1;
	return ((0 - r < proj < v1_len + r) && ((x - xc) * (x - xc) + (y - yc) * (y - yc) < (r * r)));
}

#if defined COLANDREAS
stock PHY_UpdateBounds(objectid, Float:x, Float:y, Float:z)
{
	new Float:unused, Float:bound;
	// CA_RayCastLineAngle(x, y, z, x, y, z - 1000.0, unused, unused, bound, PHY_Object[objectid][PHY_LowZRX], PHY_Object[objectid][PHY_LowZRY], unused);
	CA_RayCastLineAngle(x, y, z, x, y, z - 1000.0, unused, unused, bound);
	if(bound != 0.0 && bound == bound)
		PHY_Object[objectid][PHY_LowZBound] = bound + PHY_Object[objectid][PHY_Size];
	bound = 0.0;
	CA_RayCastLine(x, y, z, x, y, z + 1000.0, unused, unused, bound);
	if(bound > PHY_Object[objectid][PHY_LowZBound] && bound == bound)
		PHY_Object[objectid][PHY_HighZBound] = bound - PHY_Object[objectid][PHY_Size];
	else
		PHY_Object[objectid][PHY_HighZBound] = FLOAT_INFINITY;
}
#endif

stock FixRot(&Float:x, &Float:y, &Float:z)
{
	FixAngle(x);
	FixAngle(y);
	FixAngle(z);
}

stock FixAngle(&Float:x)
{
	while(x < 0.0)
		x += 360.0;
	while(x > 360.0)
		x -= 360.0;
	if(x < 0.01)
		x = 0.01;
	else if(89.99 < x <= 90.0)
		x = 89.99;
	else if(90.0 < x < 90.01)
		x = 90.01;
	else if(179.99 < x <= 180.0)
		x = 179.99;
	else if(180.0 < x < 180.01)
		x = 180.01;
	else if(269.99 < x <= 270.0)
		x = 269.99;
	else if(270.0 < x < 270.01)
		x = 270.01;
	else if(x > 359.99)
		x = 359.99;
}

stock PHY_vectorcrossp(Float:v1x, Float:v1y, Float:v1z, Float:v2x, Float:v2y, Float:v2z, &Float:c1, &Float:c2, &Float:c3)
{
	c1 = (v1y * v2z) - (v1z * v2y),
	c2 = (v1z * v2x) - (v1x * v2z),
	c3 = (v1x * v2y) - (v1y * v2x);
}

stock PHY_QuatFromAxisAngle(Float:x, Float:y, Float:z, Float:angle, &Float:q_w, &Float:q_x, &Float:q_y, &Float:q_z)
{
	new
		Float:omega,
		Float:s;

	s = floatsqroot(x*x + y*y + z*z);

	if (s > 0.01)
	{

		x /= s;
		y /= s;
		z /= s;

		omega = 0.5 * angle;
		s = floatsin(omega, degrees);

		q_x = s*x;
		q_y = s*y;
		q_z = s*z;
		q_w = floatcos(omega, degrees);
	}
	else
	{
		q_x = 0.0;
		q_y = 0.0;
		q_z = 0.0;
		q_w = 1.0;
	}

	PHY_QuatNormalize(q_w, q_x, q_y, q_z);
}

stock PHY_QuatNormalize(&Float:q_w, &Float:q_x, &Float:q_y, &Float:q_z)
{
	new Float:magnitude = floatsqroot(q_w*q_w + q_x*q_x + q_y*q_y + q_z*q_z);
	if (magnitude == 0.0)
	{
		q_w = 1.0;
		q_x = 0.0;
		q_y = 0.0;
		q_z = 0.0;
	}
	else
	{
		q_w /= magnitude;
		q_x /= magnitude;
		q_y /= magnitude;
		q_z /= magnitude;
	}
}


/* The following are from i_quat.inc, made by IllidanS4 (spacemud)
https://github.com/IllidanS4/i_quat/blob/master/i_quat.inc */

static stock Float:PHY_asin_limit(Float:value)
{
    if(value > 1.0) value = 1.0;
    else if(value < -1.0) value = -1.0;
    return asin(value);
}

static stock Float:PHY_atan2_limit(Float:x, Float:y)
{
    if(x > 1.0) x = 1.0;
    else if(x < -1.0) x = -1.0;
    if(y > 1.0) y = 1.0;
    else if(y < -1.0) y = -1.0;
    return atan2(x, y);
}

stock PHY_GetQuatProduct(const Float:q1[4], const Float:q2[4], Float:q3[4])
{
    new Float:w = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    new Float:x = q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2];
    new Float:y = q1[0]*q2[2] - q1[1]*q2[3] + q1[2]*q2[0] + q1[3]*q2[1];
    new Float:z = q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1] + q1[3]*q2[0];
    q3[0] = w, q3[1] = x, q3[2] = y, q3[3] = z;
}

stock PHY_GetQuaternionAngles(Float:w, Float:x, Float:y, Float:z, &Float:xa, &Float:ya, &Float:za)
{
    #if defined QUAT_FLOAT_EPSILON
    static const Float:epsilon = QUAT_FLOAT_EPSILON;
    #else
    static const Float:epsilon = 0.00000202655792236328125;
    #endif

    new Float:temp = 2*y*z - 2*x*w;

    if(temp >= 1-epsilon)
    {
        xa = 90.0;
        ya = -PHY_atan2_limit(y, w);
        za = -PHY_atan2_limit(z, w);
    }else if(-temp >= 1-epsilon)
    {
        xa = -90.0;
        ya = -PHY_atan2_limit(y, w);
        za = -PHY_atan2_limit(z, w);
    }else{
        xa = PHY_asin_limit(temp);
        ya = -PHY_atan2_limit(x*z + y*w, 0.5 - x*x - y*y);
        za = -PHY_atan2_limit(x*y + z*w, 0.5 - x*x - z*z);
    }
}

stock PHY_GetRotationQuaternion(Float:x, Float:y, Float:z, &Float:qw, &Float:qx, &Float:qy, &Float:qz)
{
    new Float:cx = floatcos(-0.5*x, degrees);
    new Float:sx = floatsin(-0.5*x, degrees);
    new Float:cy = floatcos(-0.5*y, degrees);
    new Float:sy = floatsin(-0.5*y, degrees);
    new Float:cz = floatcos(-0.5*z, degrees);
    new Float:sz = floatsin(-0.5*z, degrees);
    qw = cx * cy * cz + sx * sy * sz;
    qx = cx * sy * sz + sx * cy * cz;
    qy = cx * sy * cz - sx * cy * sz;
    qz = cx * cy * sz - sx * sy * cz;
}


#if defined _ALS_OnGameModeInit
	#undef OnGameModeInit
#else
	#define _ALS_OnGameModeInit
#endif
#define OnGameModeInit PHY_OnGameModeInit

forward OnGameModeInit();


#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect PHY_OnPlayerConnect

forward OnPlayerConnect(playerid);
