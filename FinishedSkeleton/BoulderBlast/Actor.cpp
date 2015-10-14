#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp
void Actor::move(int& x, int& y, Direction dir)
{
	switch (dir)
	{
	case up:
		y++;
		break;
	case down:
		y--;
		break;
	case left:
		x--;
		break;
	case right:
		x++;
		break;
	}
}

void Player::checkMove(Direction dir)
{
	int x = getX(), y = getY();
	move(x, y, dir);
	StudentWorld* ptr = getWorld();
	Actor* content = ptr->getContentOf(x, y);
	if (content == nullptr)
	{
		moveTo(x, y);
		return;
	}
	if (!content->moveable()) return;
	int xx = x, yy = y;
	move(xx, yy, dir);
	if (ptr->exitIsOn(xx, yy) && ptr->getNumOfJewels() == 0) return;
	Actor* content2 = ptr->getContentOf(xx, yy);
	if (content2 == nullptr || content2->imAHole())
	{
		moveTo(x, y);
		content->moveTo(xx, yy);
	}
}


void Player::doSomething()
{
	int ch;
	StudentWorld* ptr = getWorld();
	if (ptr->getKey(ch))
	{
		switch (ch)
		{
		case KEY_PRESS_LEFT:
			setDirection(GraphObject::left);
			checkMove(getDirection());
			break;
		case KEY_PRESS_RIGHT:
			setDirection(GraphObject::right);
			checkMove(getDirection());
			break;
		case KEY_PRESS_UP:
			setDirection(GraphObject::up);
			checkMove(getDirection());
			break;
		case KEY_PRESS_DOWN:
			setDirection(GraphObject::down);
			checkMove(getDirection());
			break;
		case KEY_PRESS_SPACE:
		{
			if (m_ammo == 0) return;
			int x = getX(), y = getY();
			move(x, y, getDirection());
			ptr->playSound(SOUND_PLAYER_FIRE);
			ptr->addActor(new Bullet(ptr, x, y, getDirection()));
			m_ammo--;
			break;
		}
		case KEY_PRESS_ESCAPE:
			setDead();
			break;
		}
	}
}

void Player::doDamage()
{ 
	m_hitpts -= 2; 
	if (m_hitpts <= 0) setDead();
	else getWorld()->playSound(SOUND_PLAYER_IMPACT);
}


void Bullet::hit(int x, int y)
{
	StudentWorld* ptr = getWorld();
	Actor* content = ptr->getContentOf(x, y, this);
	if (content == nullptr) return;
	content->doDamage();
	if (content->blockBullet()) setDead();
}

void Bullet::doSomething()
{
	if (!checkAlive()) return;
	int x = getX(), y = getY();
	hit(x, y);
	if (!checkAlive()) return;
	move(x, y, getDirection());
	moveTo(x, y);
	hit(x, y);
}

void Hole::doSomething()
{
	if (!checkAlive()) return;
	StudentWorld* ptr = getWorld();
	int x = getX(), y = getY();
	Actor* content = ptr->getContentOf(x, y, this);
	if (content == nullptr) return;
	if (content->moveable())
	{
		setDead();
		content->setDead();
	}
}

void Goodie::doSomething()
{
	if (!checkAlive()) return;
	StudentWorld* ptr = getWorld();
	if (ptr->playerIsOn(getX(), getY()) && pickable)
	{
		increasePlayerPoints();
		setDead();
		ptr->playSound(SOUND_GOT_GOODIE);
	}
}

void Jewel::increasePlayerPoints() 
{ 
	getWorld()->increaseScore(50); getWorld()->decJewel(); 
}

void ExtraLife::increasePlayerPoints()
{ 
	getWorld()->increaseScore(1000); getWorld()->incLives();
}

void RestoreHealth::increasePlayerPoints() 
{ 
	getWorld()->increaseScore(500); getWorld()->restorePlayerHealth(); 
}

void Ammo::increasePlayerPoints() 
{ 
	getWorld()->increaseScore(100); getWorld()->increasePlayerAmmo(); 
}

void TheExit::doSomething()
{
	StudentWorld* ptr = getWorld();
	if (ptr->getNumOfJewels() > 0) return;
	if (!revealed)
	{
		revealed = true;
		setVisible(true);
		ptr->playSound(SOUND_REVEAL_EXIT);
	}
}

Robot::Robot(StudentWorld* ptr, int IID, int startX, int startY, int hitpts, Direction dir) : Actor(ptr, IID, startX, startY, dir), m_hitpts(hitpts)
{
	ticks = (28 - getWorld()->getLevel()) / 4;
	if (ticks < 3) ticks = 3;
	currentTick = 1;
}

void Robot::doDamage() 
{
	m_hitpts -= 2; 
	if (m_hitpts > 0)
		getWorld()->playSound(SOUND_ROBOT_IMPACT);
	else
	{
		setDead();
		increasePlayerPoints();
		getWorld()->playSound(SOUND_ROBOT_DIE);
		Goodie* goodie = getGoodieCarrying();
        if (goodie != nullptr)
        {
			goodie->moveTo(getX(), getY());
            goodie->setVisible(true);
            goodie->setPickability(true);
        }
	}
}

bool Robot::playerInSight()
{
	StudentWorld* ptr = getWorld();
	int x = getX(), y = getY(), playerX = ptr->getPlayer()->getX(), playerY = ptr->getPlayer()->getY();
	if (getDirection() == up && (playerX != x || playerY < y)) return false;
	if (getDirection() == down && (playerX != x || playerY > y)) return false;
	if (getDirection() == left && (playerY != y || playerX > x)) return false;
	if (getDirection() == right && (playerY != y || playerX < x)) return false;
	Actor* content;
	do
	{
		move(x, y, getDirection());
		if (x < 0 || y < 0 || x >= VIEW_WIDTH || y >= VIEW_HEIGHT) break;
		if (ptr->playerIsOn(x, y)) return true;
		content = ptr->getContentOf(x, y);
	} while (content == nullptr || !content->blockActor() || content->imAHole());
	return false;
}

void Robot::fireGun()
{
	StudentWorld* ptr = getWorld();
	int x = getX(), y = getY();
	move(x, y, getDirection());
	ptr->playSound(SOUND_ENEMY_FIRE);
	ptr->addActor(new Bullet(ptr, x, y, getDirection()));
}

void SnarlBot::increasePlayerPoints()
{
	getWorld()->increaseScore(100);
}


bool SnarlBot::checkMove(Direction dir)
{
	int x = getX(), y = getY();
	move(x, y, dir);
	StudentWorld* ptr = getWorld();
	Actor* content = ptr->getContentOf(x, y);
	if (content != nullptr && content->blockActor())
	{
		switch (dir)
		{
		case up:
			setDirection(down);
			break;
		case down:
			setDirection(up);
			break;
		case left:
			setDirection(right);
			break;
		case right:
			setDirection(left);
			break;
		}
		return false;
	}
	else
	{
		moveTo(x, y);
		return true;
	}
}

void SnarlBot::doSomething()
{
	increaseCurrentTick();
	if (getCurrentTick() != getTicks() || !checkAlive()) return;
	if (playerInSight()) fireGun();
	else checkMove(getDirection());
}

void KleptoBot::increasePlayerPoints() 
{ 
	getWorld()->increaseScore(10); 
}

bool KleptoBot::checkMove(Direction dir)
{
	int x = getX(), y = getY();
	move(x, y, dir);
	StudentWorld* ptr = getWorld();
	Actor* content = ptr->getContentOf(x, y);
	if (content != nullptr && content->blockActor())
		return false;
	else
	{
		moveTo(x, y);
		return true;
	}
}

void KleptoBot::kleptobotMove()
{
	StudentWorld* ptr = getWorld();
	int x = getX(), y = getY();
	Goodie* goodie = ptr->getGoodie(x, y);
	if (goodie != nullptr && rand() % 10 == 0 && goodieCarrying == nullptr && goodie->checkPickable())
	{
		goodie->setVisible(false);
		goodie->setPickability(false);
		goodieCarrying = goodie;
		ptr->playSound(SOUND_ROBOT_MUNCH);
	}
	else if (distanceMoved < distanceBeforeTurning && checkMove(getDirection()))
		distanceMoved++;
	else
	{
		randomizeDistance();
		Direction* a = ptr->getDirections(rand() % 24);
		for (int i = 0; i < 4; i++)
			if (checkMove(a[i]))
			{
				setDirection(a[i]);
				distanceMoved++;
				return;
			}
		setDirection(a[0]);
	}
}

void KleptoBot::doSomething()
{
	if (!checkAlive()) return;
	increaseCurrentTick();
	if (getCurrentTick() == getTicks())
		kleptobotMove();
}

void AngryKleptoBot::increasePlayerPoints()
{ 
	getWorld()->increaseScore(20); 
}

void AngryKleptoBot::doSomething()
{
	increaseCurrentTick();
    if (!checkAlive() || getCurrentTick() != getTicks()) return;
	if (playerInSight()) fireGun();
	else kleptobotMove();
}

bool Factory::canCreate()
{
	Actor* content = getWorld()->getContentOf(getX(), getY());
	if (content != nullptr && content->imAKleptobot()) return false;
	int x1 = (getX() > 3) ? getX() - 3 : 0,
		y1 = (getY() > 3) ? getY() - 3 : 0,
		x2 = (getX() + 3 < VIEW_WIDTH) ? getX() + 3 : VIEW_WIDTH - 1,
		y2 = (getY() + 3 < VIEW_HEIGHT) ? getY() + 3 : VIEW_HEIGHT - 1,
		sum = 0;
	for (int i = x1; i <= x2; i++)
		for (int j = y1; j <= y2; j++)
		{
			content = getWorld()->getContentOf(i, j);
			if (content != nullptr && content->imAKleptobot())
				sum++;
		}
	if (sum >= 3) return false;
	else return true;
}

void KleptoBot_Factory::doSomething()
{
	if ((rand() % 50 == 0) && canCreate())
	{
		StudentWorld* ptr = getWorld();
		ptr->playSound(SOUND_ROBOT_BORN);
		ptr->addActor(new KleptoBot(ptr, getX(), getY()));
	}
}

void AngryKleptoBot_Factory::doSomething()
{
    if ((rand() % 50 == 0) && canCreate())
    {
        StudentWorld* ptr = getWorld();
        ptr->playSound(SOUND_ROBOT_BORN);
        ptr->addActor(new AngryKleptoBot(ptr, getX(), getY()));
    }
}