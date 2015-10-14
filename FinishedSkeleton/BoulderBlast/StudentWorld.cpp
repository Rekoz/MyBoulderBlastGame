#include "StudentWorld.h"
#include "Actor.h"
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::~StudentWorld()
{
	list<Actor*>::iterator i;
	while (i != m_actors.end())
	{
		delete *i;
		list<Actor*>::iterator j = i;
		i++;
		m_actors.erase(j);
	}
	delete m_player;
}

int StudentWorld::init()
{
	m_bonus = 1000; m_jewels = 0;
	ostringstream oss;
	oss << "level0" << getLevel() << ".dat";
	string curLevel = oss.str();
	Level lev(assetDirectory());
	Level::LoadResult result = lev.loadLevel(curLevel);

	if (result == Level::load_fail_file_not_found || getLevel() == 100)
		return GWSTATUS_PLAYER_WON;
	else if (result == Level::load_fail_bad_format)
		return GWSTATUS_LEVEL_ERROR;
	else if (result == Level::load_success)
	{
		for (int x = 0; x < VIEW_WIDTH; x++)
			for (int y = 0; y < VIEW_HEIGHT; y++)
			{
				Level::MazeEntry me = lev.getContentsOf(x, y);
				switch (me)
				{
				case Level::empty:
					break;
				case Level::player:
					m_player = new Player(this, x, y);
					break;
				case Level::wall:
					m_actors.push_front(new Wall(this, x, y));
					break;
				case Level::boulder:
					m_actors.push_back(new Boulder(this, x, y));
					break;
				case Level::hole:
					m_actors.push_back(new Hole(this, x, y));
					break;
				case Level::jewel:
					m_actors.push_back(new Jewel(this, x, y));
					m_jewels++;
					break;
				case Level::exit:
					m_actors.push_back(new TheExit(this, x, y));
					exitX = x; exitY = y;
					break;
				case Level::extra_life:
					m_actors.push_back(new ExtraLife(this, x, y));
					break;
				case Level::restore_health:
					m_actors.push_back(new RestoreHealth(this, x, y));
					break;
				case Level::ammo:
					m_actors.push_back(new Ammo(this, x, y));
					break;
				case Level::horiz_snarlbot:
					m_actors.push_back(new SnarlBot(this, x, y, Actor::right));
					break;
				case Level::vert_snarlbot:
					m_actors.push_back(new SnarlBot(this, x, y, Actor::down));
					break;
				case Level::kleptobot_factory:
					m_actors.push_back(new KleptoBot_Factory(this, x, y));
					break;
                case Level::angry_kleptobot_factory:
	                m_actors.push_back(new AngryKleptoBot_Factory(this, x, y));
                    break;
				default:
					break;
				}
			}
	}
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::setDisplayText()
{
	int healthPercentage = m_player->getHealth() * 100 / 20;
	ostringstream oss;
	oss << "Score: ";
	oss.fill('0');
	oss << setw(7) << getScore() << "  Level: " << setw(2) << getLevel() << "  Lives: ";
	oss.fill(' ');
	oss << setw(2) << getLives() << "  Health: " << setw(3) << healthPercentage << "%  Ammo: " << setw(3) << m_player->getAmmo() << "  Bonus: " << setw(4) << m_bonus;
	setGameStatText(oss.str());
}

int StudentWorld::move()
{
	setDisplayText();

	for (list<Actor*>::iterator i = m_actors.begin(); i != m_actors.end(); i++)
		if ((*i)->checkAlive())
		{
			(*i)->doSomething();
			if (!m_player->checkAlive())
			{
				playSound(SOUND_PLAYER_DIE);
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}
			if (m_jewels == 0 && m_player->getX() == exitX && m_player->getY() == exitY)
			{
				playSound(SOUND_FINISHED_LEVEL);
				increaseScore(2000 + m_bonus);
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
	m_player->doSomething();

	removeDeadGameObjects();
	if (m_bonus > 0) m_bonus--;

	if (!m_player->checkAlive())
	{
		playSound(SOUND_PLAYER_DIE);
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	if (m_jewels == 0 && m_player->getX() == exitX && m_player->getY() == exitY)
	{
		playSound(SOUND_FINISHED_LEVEL);
		increaseScore(2000 + m_bonus);
		return GWSTATUS_FINISHED_LEVEL;
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	list<Actor*>::iterator i = m_actors.begin();
	while (i != m_actors.end())
	{
		delete *i;
		list<Actor*>::iterator j = i;
		i++;
		m_actors.erase(j);
	}
	delete m_player;
}

Actor* StudentWorld::getContentOf(int x, int y, Actor* self)
{
	if (m_player->getX() == x && m_player->getY() == y) return m_player;
	for (list<Actor*>::iterator i = m_actors.begin(); i != m_actors.end(); i++)
		if ((*i)->getX() == x && (*i)->getY() == y && (*i)->blockActor() && (*i) != self)
			return (*i);
	return nullptr;
}

Goodie* StudentWorld::getGoodie(int x, int y)
{
	for (list<Actor*>::iterator i = m_actors.begin(); i != m_actors.end(); i++)
		if ((*i)->getX() == x && (*i)->getY() == y && (*i)->imAGoodie())
            return dynamic_cast<Goodie*>(*i);
	return nullptr;
}

void StudentWorld::addActor(Actor* ptr)
{
	if (ptr->blockActor())
		m_actors.push_front(ptr);
	else
		m_actors.push_back(ptr);
}

void StudentWorld::removeDeadGameObjects()
{
	list<Actor*>::iterator i = m_actors.begin();
	while (i != m_actors.end())
	{
		if (!(*i)->checkAlive())
		{
			delete *i;
			list<Actor*>::iterator j = i;
			i++;
			m_actors.erase(j);
		}
		else
			i++;
	}
}

bool StudentWorld::playerIsOn(int x, int y)
{
	return (m_player->getX() == x && m_player->getY() == y);
}

void StudentWorld::restorePlayerHealth()
{
	m_player->restoreHealth(); 
}

void StudentWorld::increasePlayerAmmo() 
{ 
	m_player->increaseAmmo(); 
}