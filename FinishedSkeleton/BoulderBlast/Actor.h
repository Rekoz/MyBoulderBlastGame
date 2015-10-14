#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor : public GraphObject
{
public:
	Actor(StudentWorld* ptr, int imageID, int startX, int startY, Direction startDirection = none) : GraphObject(imageID, startX, startY, startDirection), m_stdworld(ptr), alive(true)
	{
		setVisible(true);
	}
	virtual void doSomething() = 0;
	StudentWorld* getWorld() { return m_stdworld; }
	bool checkAlive() const { return alive; } 
	void setDead(){ alive = false; }
	void move(int& x, int& y, Direction dir);
	virtual void doDamage(){}
	virtual bool blockActor() { return false; };
	virtual bool moveable() { return false; }
	virtual bool imAHole() { return false; }
	virtual bool imAGoodie() { return false; }
	virtual bool imAKleptobot() { return false; }
	virtual bool blockBullet() { return false; }
private:
	StudentWorld* m_stdworld;
	bool alive;
};

class Player : public Actor
{
public:
	Player(StudentWorld* ptr, int startX, int startY) : Actor(ptr, IID_PLAYER, startX, startY, right), m_hitpts(20), m_ammo(20){}
	void checkMove(Direction dir);
	virtual void doSomething();
	virtual void doDamage();
	int getHealth() { return m_hitpts; }
	int getAmmo() { return m_ammo; }
	void restoreHealth() { m_hitpts = 20; }
	void increaseAmmo() { m_ammo += 20; }
	virtual bool blockActor() { return true; }
	virtual bool blockBullet() { return true; }

private:
	int m_hitpts;
	int m_ammo;
};

class Wall : public Actor
{
public:
	Wall(StudentWorld* ptr, int startX, int startY) : Actor(ptr, IID_WALL, startX, startY){}
	~Wall(){}
	virtual void doSomething(){}
	virtual bool blockActor() { return true; }
	virtual bool blockBullet() { return true; }
};

class Boulder : public Actor
{
public:
	Boulder(StudentWorld* ptr, int startX, int startY) : Actor(ptr, IID_BOULDER, startX, startY), m_hitpts(10){}
	virtual void doSomething(){}
	virtual void doDamage(){ m_hitpts -= 2; if (m_hitpts <= 0) setDead(); }
	virtual bool blockActor() { return true; }
	virtual bool moveable() { return true; }
	virtual bool blockBullet() { return true; }

private:
	int m_hitpts;
};

class Bullet : public Actor
{
public:
	Bullet(StudentWorld* ptr, int startX, int startY, Direction dir) : Actor(ptr, IID_BULLET, startX, startY, dir){}
	virtual void doSomething();
	void hit(int x, int y);
};

class Hole : public Actor
{
public:
	Hole(StudentWorld* ptr, int startX, int startY) : Actor(ptr, IID_HOLE, startX, startY){}
	virtual void doSomething();
	virtual bool blockActor() { return true; }
	virtual bool imAHole() { return true; }
};

class Goodie : public Actor
{
public:
	Goodie(StudentWorld* ptr, int IID, int startX, int startY) : Actor(ptr, IID, startX, startY), pickable(true){}
	virtual void doSomething();
	virtual void increasePlayerPoints() = 0;
	virtual bool imAGoodie() { return true; }
	void setPickability(bool pickability) { pickable = pickability; }
	bool checkPickable() { return pickable; }
private:
	bool pickable;
};

class Jewel : public Goodie
{
public:
	Jewel(StudentWorld* ptr, int startX, int startY) : Goodie(ptr, IID_JEWEL, startX, startY){}
	virtual void increasePlayerPoints();
	virtual bool imAGoodie() { return false; }
};

class TheExit : public Actor
{
public:
	TheExit(StudentWorld* ptr, int startX, int startY) : Actor(ptr, IID_EXIT, startX, startY), revealed(false)
	{
		setVisible(false);
	}
	virtual void doSomething();
private:
	bool revealed;
};

class ExtraLife : public Goodie
{
public:
	ExtraLife(StudentWorld* ptr, int startX, int startY) : Goodie(ptr, IID_EXTRA_LIFE, startX, startY){}
	virtual void increasePlayerPoints();
};

class RestoreHealth : public Goodie
{
public:
	RestoreHealth(StudentWorld* ptr, int startX, int startY) : Goodie(ptr, IID_RESTORE_HEALTH, startX, startY){}
	virtual void increasePlayerPoints();
};

class Ammo : public Goodie
{
public:
	Ammo(StudentWorld* ptr, int startX, int startY) : Goodie(ptr, IID_AMMO, startX, startY){}
	virtual void increasePlayerPoints();
};

class Robot : public Actor
{
public:
	Robot(StudentWorld* ptr, int IID, int startX, int startY, int hitpts, Direction dir);
	virtual bool checkMove(Direction dir) = 0;
	virtual bool blockActor() { return true; }
	virtual void doDamage();
	virtual bool blockBullet() { return true; }
	int getTicks() { return ticks; }
	int getCurrentTick() { return currentTick; }
	void increaseCurrentTick() { currentTick = (currentTick +1) % (ticks+1); }
	bool playerInSight();
    virtual Goodie* getGoodieCarrying() { return nullptr;}
	virtual void increasePlayerPoints() = 0;
	void fireGun();
private:
	int m_hitpts, ticks, currentTick;
};

class SnarlBot : public Robot
{
public:
	SnarlBot(StudentWorld* ptr, int startX, int startY, Direction dir) : Robot(ptr, IID_SNARLBOT, startX, startY, 10, dir){}
	virtual void doSomething();
	virtual void increasePlayerPoints();
	virtual bool checkMove(Direction dir);
};

class KleptoBot : public Robot
{
public:
    KleptoBot(StudentWorld* ptr, int startX, int startY, int IID = IID_KLEPTOBOT, int hitpts = 5) : Robot(ptr, IID, startX, startY, hitpts, right), goodieCarrying(nullptr)
	{
		randomizeDistance();
	}
	void randomizeDistance() 
	{
		distanceBeforeTurning = rand() % 6 + 1;
		distanceMoved = 0;
	}
	virtual bool imAKleptobot() { return true; }
	virtual bool checkMove(Direction dir);
	virtual void doSomething();
	virtual void increasePlayerPoints();
	virtual void setGoodieCarrying(Goodie* goodie) { goodieCarrying = goodie; }
	virtual Goodie* getGoodieCarrying() { return goodieCarrying; }
	void kleptobotMove();
private:
	int distanceBeforeTurning, distanceMoved;
    Goodie* goodieCarrying;
};

class AngryKleptoBot : public KleptoBot
{
public:
    AngryKleptoBot(StudentWorld* ptr, int startX, int startY) : KleptoBot(ptr, startX, startY, IID_ANGRY_KLEPTOBOT, 8){}
	virtual void doSomething();
	virtual void increasePlayerPoints();
};

class Factory : public Actor
{
public:
	Factory(StudentWorld* ptr, int startX, int startY) : Actor(ptr, IID_ROBOT_FACTORY, startX, startY){}
	bool canCreate();
    virtual bool blockActor() { return true; }
	virtual bool blockBullet() { return true; }
};

class KleptoBot_Factory : public Factory
{
public:
	KleptoBot_Factory(StudentWorld* ptr, int startX, int startY) : Factory(ptr, startX, startY){}
	virtual void doSomething();
};

class AngryKleptoBot_Factory : public Factory
{
public:
    AngryKleptoBot_Factory(StudentWorld* ptr, int startX, int startY) : Factory(ptr, startX, startY){}
    virtual void doSomething();
};

#endif // ACTOR_H_