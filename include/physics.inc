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
#pragma compress 1
#include <foreach>

#if !defined PHY_TIMER_INTERVAL
	#define PHY_TIMER_INTERVAL (20)
#endif
#if !defined PHY_MAX_WALLS    
	#define PHY_MAX_WALLS (512)
#endif
#if !defined PHY_MAX_CYLINDERS
	#define PHY_MAX_CYLINDERS (128)
#endif		

#define PHY_MODE_3D (0)
#define PHY_MODE_2D (1)

#if !defined FLOAT_INFINITY
	#define FLOAT_INFINITY   (Float:0x7F800000)
#endif
#if !defined FLOAT_NEG_INFINITY
	#define FLOAT_NEG_INFINITY (Float:0xFF800000)
#endif
#if !defined FLOAT_NAN
	#define FLOAT_NAN     (Float:0xFFFFFFFF)
#endif

/* Callbacks */
forward PHY_OnObjectUpdate(objectid);
forward PHY_OnObjectCollideWithObject(object1, object2);
forward PHY_OnObjectCollideWithWall(objectid, wallid);
forward PHY_OnObjectCollideWithCylinder(objectid, cylinderid);


enum (<<= 1)
{
	PHY_OBJECT_USED = 0b01,
	PHY_OBJECT_MODE,
	PHY_OBJECT_ROLL,
	PHY_OBJECT_GHOST_OBJECTS,
	PHY_OBJECT_GHOST_WALLS,
	PHY_OBJECT_GHOST_CYLINDERS,
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
	Float:PHY_AX,
	Float:PHY_AY,
	Float:PHY_AZ,
	Float:PHY_Friction,
	Float:PHY_AirResistance,
	Float:PHY_Gravity,
	Float:PHY_LowZBound,
	Float:PHY_HighZBound,
	Float:PHY_BoundConst
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

/* Macros are self-explanatory */
#define PHY_IsObjectUsingPhysics(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_USED)
#define PHY_IsObjectUsing3D(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_MODE)
#define PHY_IsObjectGhostWithObjects(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_OBJECTS)
#define PHY_IsObjectGhostWithWalls(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_WALLS)
#define PHY_IsObjectGhostWithCylinders(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_CYLINDERS)
#define PHY_IsObjectGhostWithPlayers(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_GHOST_PLAYERS)
#define PHY_IsObjectMoving(%1) (PHY_Object[%1][PHY_VX] != 0 || PHY_Object[%1][PHY_VY] != 0 || PHY_Object[%1][PHY_VZ] != 0)
#define PHY_IsObjectRolling(%1) (PHY_Object[%1][PHY_Properties] & PHY_OBJECT_ROLL)
#define PHY_GetObjectFriction(%1) (PHY_Object[%1][PHY_Friction])
#define PHY_GetObjectAirResistance(%1) (PHY_Object[%1][PHY_AirResistance])
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
		Float:mag,
		Float:tmpvx1,
		Float:tmpvx2,
		Float:newvy1,
		Float:newvy2,
		Float:newvx1,
		Float:newvx2;
	foreach(ITER_Object, a)
	{
	    if(PHY_Object[a][PHY_Properties] & PHY_OBJECT_USED)
	    {
            GetObjectPos(a, x1, y1, z1);
            x1 += PHY_Object[a][PHY_VX] * (PHY_TIMER_INTERVAL/1000.0);
			y1 += PHY_Object[a][PHY_VY] * (PHY_TIMER_INTERVAL/1000.0);
			if(PHY_GetObjectMode(a) == PHY_MODE_3D)
			{
			    z1 += PHY_Object[a][PHY_VZ] * (PHY_TIMER_INTERVAL/1000.0);
			    
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
						PHY_Object[a][PHY_VZ] -= PHY_Object[a][PHY_Gravity] * (PHY_TIMER_INTERVAL/1000.0);
						if(PHY_Object[a][PHY_VZ] < 0)
							PHY_Object[a][PHY_VZ] = 0;
					}
					else
						PHY_Object[a][PHY_VZ] -= PHY_Object[a][PHY_Gravity] * (PHY_TIMER_INTERVAL/1000.0);
				}
			}
			if(PHY_IsObjectMoving(a))
	        {
				if(!PHY_IsObjectGhostWithObjects(a))
				{
					foreach(ITER_Object, b)
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
		            	        dvy = PHY_Object[a][PHY_VY] - PHY_Object[b][PHY_VX];
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
					        }
					    }
					}
				}
				if(!PHY_IsObjectGhostWithWalls(a))
				{
					foreach(ITER_Wall, w)
					{
						if(PHY_Wall[w][PHY_Created])
						{
						    if((!PHY_Object[a][PHY_World] || !PHY_Wall[w][PHY_World] || PHY_Object[a][PHY_World] == PHY_Wall[w][PHY_World]))
						    {
						        //dist = (y1 - PHY_Wall[w][PHY_M] * x1 - PHY_Wall[w][PHY_Q])/floatsqroot(1 + PHY_Wall[w][PHY_M] * PHY_Wall[w][PHY_M]);
								//dist = (PHY_Wall[w][PHY_A] * x1 + PHY_Wall[w][PHY_B] * y1 + PHY_Wall[w][PHY_C])/floatsqroot(PHY_Wall[w][PHY_A] * PHY_Wall[w][PHY_A] + PHY_Wall[w][PHY_B] * PHY_Wall[w][PHY_B]);
								angle = PHY_Wall[w][PHY_ANG];
								if(PHY_Wall[w][PHY_Z1] + PHY_Object[a][PHY_Size] < z1 < PHY_Wall[w][PHY_Z2] - PHY_Object[a][PHY_Size] && check_segment_intersection(PHY_Wall[w][PHY_X1], PHY_Wall[w][PHY_Y1], PHY_Wall[w][PHY_X2], PHY_Wall[w][PHY_Y2], x1, y1, PHY_Object[a][PHY_Size], xclosest, yclosest) /* && floatabs(dist) < PHY_Object[a][PHY_Size]*/)
						        {
						            //mag = y1 + PHY_Object[a][PHY_Size] * floatcos(-moveangle, degrees) - (x1 + PHY_Object[a][PHY_Size] * floatsin(-moveangle, degrees)) * PHY_Wall[w][PHY_M] - PHY_Wall[w][PHY_Q];
									//mag = PHY_Wall[w][PHY_A] * (x1 + PHY_Object[a][PHY_Size] * floatsin(-moveangle, degrees)) + PHY_Wall[w][PHY_B] * (y1 + PHY_Object[a][PHY_Size] * floatcos(-moveangle, degrees)) + PHY_Wall[w][PHY_C];
									//if((dist >= 0) ? (mag <= 0) : (mag >= 0))
			       	         		newvx1 = PHY_Object[a][PHY_VX] * floatcos(angle, degrees) - PHY_Object[a][PHY_VY] * floatsin(angle, degrees);
					            	newvy1 = PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees);
									newvy1 = -newvy1 * PHY_Wall[w][PHY_BounceConst];
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
					foreach(ITER_Cylinder, c)
					{
					    if(PHY_Cylinder[c][PHY_Created])
						{
						    if((!PHY_Object[a][PHY_World] || !PHY_Cylinder[c][PHY_World] || PHY_Object[a][PHY_World] == PHY_Cylinder[c][PHY_World]))
						    {
								if(PHY_Cylinder[c][PHY_Z1] + PHY_Object[a][PHY_Size] < z1 < PHY_Cylinder[c][PHY_Z2] - PHY_Object[a][PHY_Size])
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
				            	            newvy1 = PHY_Object[a][PHY_VX] * floatsin(angle, degrees) + PHY_Object[a][PHY_VY] * floatcos(angle, degrees);

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
	        Float:unused;
	    GetObjectPos(objectid, unused, unused, PHY_Object[objectid][PHY_LowZBound]);
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
	//PHY_Roll(rx, ry, rz, PHY_Object[a][PHY_VX], PHY_Object[a][PHY_VY], ((speed * PHY_TIMER_INTERVAL)/1000) * (180/3.14159) / PHY_Object[a][PHY_Size]);
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


/* Privates */

stock Float:vectordotp(Float:v1x, Float:v1y, Float:v2x, Float:v2y)
    return (v1x * v2x + v1y * v2y);

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
	new Float:proj = vectordotp(v2[0], v2[1], v1[0], v1[1]);
	x = proj * v1[0] + x1;
	y = proj * v1[1] + y1;
    return ((0 - r < proj < v1_len + r) && ((x - xc) * (x - xc) + (y - yc) * (y - yc) < (r * r)));
}

/* Function to make a better ball rolling, not used because it doesn't work well. */
stock PHY_Roll(&Float:x, &Float:y, &Float:z, Float:dx, Float:dy, Float:speed)
{
	new Float:q_roll[4];
	new Float:vx, Float:vy, Float:vz;
	vectorcrossp(dx, dy, 0.0, 0.0, 0.0, 1.0, vx, vy, vz);
	QuatFromAxisAngle(vx, vy, vz, -speed, q_roll[0], q_roll[1], q_roll[2], q_roll[3]);

	new Float:q_rot[4];
	EulerToQuaternion(x, y, z, q_rot[0], q_rot[1], q_rot[2], q_rot[3]);

    new Float:q_res[4];
    QuatMultiply(q_roll[0], q_roll[1], q_roll[2], q_roll[3], q_rot[0], q_rot[1], q_rot[2], q_rot[3], q_res[0], q_res[1], q_res[2], q_res[3]);

	QuaternionToEuler(q_res[0], q_res[1], q_res[2], q_res[3], x, y, z);
}

stock QuatMultiply(Float:w1, Float:x1, Float:y1, Float:z1, Float:w2, Float:x2, Float:y2, Float:z2, &Float:q_w, &Float:q_x, &Float:q_y, &Float:q_z)
{
	q_w = w1*w2 - x1*x2 - y1*y2 - z1*z2;
	q_x = w1*x2 + x1*w2 + y1*z2 - z1*y2;
	q_y = w1*y2 - x1*z2 + y1*w2 + z1*x2;
	q_z = w1*z2 + x1*y2 - y1*x2 + z1*w2;
	
	QuatNormalize(q_w, q_x, q_y, q_z);
}

stock QuatFromAxisAngle(Float:x, Float:y, Float:z, Float:angle, &Float:q_w, &Float:q_x, &Float:q_y, &Float:q_z)
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
    
    QuatNormalize(q_w, q_x, q_y, q_z);
}
  
stock QuatNormalize(&Float:q_w, &Float:q_x, &Float:q_y, &Float:q_z)
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

stock EulerToQuaternion(Float:z, Float:x, Float:y, &Float:q_w, &Float:q_x, &Float:q_y, &Float:q_z)
{
    new Float:c1 = floatcos(x, degrees);
    new Float:s1 = floatsin(x, degrees);
    new Float:c2 = floatcos(y, degrees);
    new Float:s2 = floatsin(y, degrees);
    new Float:c3 = floatcos(z, degrees);
    new Float:s3 = floatsin(z, degrees);
    q_w = floatsqroot(1.0 + c1 * c2 + c1*c3 - s1 * s2 * s3 + c2*c3) / 2.0;
    new Float:w4 = (4.0 * q_w);
    q_x = (c2 * s3 + c1 * s3 + s1 * s2 * c3) / w4 ;
    q_y = (s1 * c2 + s1 * c3 + c1 * s2 * s3) / w4 ;
    q_z = (-s1 * s3 + c1 * s2 * c3 +s2) / w4 ;
    
    QuatNormalize(q_w, q_x, q_y, q_z);
}

stock QuaternionToEuler(Float:q_w, Float:q_x, Float:q_y, Float:q_z, &Float:z, &Float:x, &Float:y)
{
	new Float:test = q_x*q_y + q_z*q_w;
	if (test > 0.499) { // singularity at north pole
		x = 2 * atan2(q_x,q_w);
		y = 90.0;
		z = 0;
		return;
	}
	if (test < -0.499) { // singularity at south pole
		x = -2 * atan2(q_x,q_w);
		y = -90.0;
		z = 0;
		return;
	}
    new Float:sqx = q_x*q_x;
    new Float:sqy = q_y*q_y;
    new Float:sqz = q_z*q_z;
    x = atan2(2*q_y*q_w-2*q_x*q_z , 1 - 2*sqy - 2*sqz);
	y = asin(2*test);
	z = atan2(2*q_x*q_w-2*q_y*q_z , 1 - 2*sqx - 2*sqz);
}

stock vectorcrossp(Float:v1x, Float:v1y, Float:v1z, Float:v2x, Float:v2y, Float:v2z, &Float:c1, &Float:c2, &Float:c3)
{
	c1 = (v1y * v2z) - (v1z * v2y),
    c2 = (v1z * v2x) - (v1x * v2z),
    c3 = (v1x * v2y) - (v1y * v2x);
}


#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit PHY_OnGameModeInit

forward OnGameModeInit();
