#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include "GraphObject.h"
#include <time.h>
#include <string>
#include <sstream>
#include <list>
#include <algorithm>
using namespace std;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class Player;
class Goodie;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir), m_jewels(0)
	{
		dirOrders[0][0] = GraphObject::up;
		dirOrders[0][1] = GraphObject::down;
		dirOrders[0][2] = GraphObject::left;
		dirOrders[0][3] = GraphObject::right;
		for (int i = 1; i < 24; i++)
		{
			for (int j = 0; j < 4; j++)
				dirOrders[i][j] = dirOrders[i-1][j];
			next_permutation(dirOrders[i], dirOrders[i] + 4);
		}
        srand(static_cast<unsigned int>(time(0)));
	}
	virtual ~StudentWorld();

	virtual int init();
	virtual int move();
	virtual void cleanUp();

	Actor* getContentOf(int x, int y, Actor* self = nullptr);
	Goodie* getGoodie(int x, int y);
	bool playerIsOn(int x, int y);
	bool exitIsOn(int x, int y) { return (exitX == x && exitY == y); }
	Player* getPlayer() { return m_player; }
	void addActor(Actor* ptr);
	int getNumOfJewels() { return m_jewels; }
	void decJewel() { m_jewels--; }
	void removeDeadGameObjects();
	void setDisplayText();
	void restorePlayerHealth(); 
	void increasePlayerAmmo();
    GraphObject::Direction* getDirections(int i) { return dirOrders[i]; }

private:
	list<Actor*> m_actors;
	Player* m_player;
	int m_jewels, exitX, exitY, m_bonus;
    GraphObject::Direction dirOrders[24][4];
};

#endif // STUDENTWORLD_H_