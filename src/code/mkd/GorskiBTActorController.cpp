#include "pch.h"
#include "GorskiBTActorController.h"


GorskiBTActorController::GorskiBTActorController(ActorAI* ai) : IActorController(ai)
{
	m_bb = new BT::BlackBoard();
	m_bb->init();
}

GorskiBTActorController::~GorskiBTActorController()
{
	delete m_bb;
	delete m_root;
}

void GorskiBTActorController::onCreate()
{
	getAI()->lookAt(mkVec3::ZERO);
	getAI()->setSpeed(0.0f);

	BT::Parser parser;
	parser.parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
	parser.parseAliases(m_bb);

	// set constants in the black board
	m_bb->setStateFloat("ShootingRange", getAI()->getShootingRange());
	m_bb->setStateFloat("MeleeRange", getAI()->getMeleeRange());
	m_bb->setStateFloat("MeleeConeSize", getAI()->getMeleeConeSize());

	// exploding barrels timer
	m_bb->setStateFloat("BarrelTimer", 0.0f);
}

void GorskiBTActorController::onTakeDamage(const SDamageInfo& dmgInfo)
{
}

void GorskiBTActorController::onUpdate(float dt)
{
	updateWorldState(dt);
	m_root->tick(m_bb);
}

void GorskiBTActorController::onDebugDraw()
{
	getAI()->drawSensesInfo();
}

void GorskiBTActorController::onDie()
{
}

void GorskiBTActorController::updateWorldState(float dt)
{
	// update black board variables used in the BT

	// is enemy visible?
	Character* target = getAI()->findClosestEnemyInSight();
	if (target)
	{
		m_bb->setStateBool("IsEnemyVisible", true);
	}
	else
	{
		m_bb->setStateBool("IsEnemyVisible", false);
	}

	// are barrels available?
	std::vector<ModelObject*> barrels = getAI()->getBarrels();
	bool isBarrelAvailable = barrels.size() > 0;
	m_bb->setStateBool("IsBarrelAvailable", isBarrelAvailable);

	// is buff available?
	m_bb->setStateBool("IsBuffAvailable", getAI()->isBuffAvailable());

	// distance to the closest barrel
	float distanceToBarrels = 1000000.0f;
	ModelObject* closest = nullptr;
	if (isBarrelAvailable)
	{
		distanceToBarrels = (barrels[0]->getWorldPosition() - getAI()->getSimPos()).length();
		closest = barrels[0];
		for (int i = 1; i < barrels.size(); i++)
		{
			float currentBarrelDistance = (barrels[i]->getWorldPosition() - getAI()->getSimPos()).length();
			if (currentBarrelDistance < distanceToBarrels)
			{
				distanceToBarrels = currentBarrelDistance;
				closest = barrels[i];
			}
		}
	}
	m_bb->setStateFloat("DistanceToBarrels", distanceToBarrels);
	if (closest)
	{
		m_bb->setStateVec3("BarrelPos", closest->getWorldPosition());
	}

	// distance to the buff
	float distanceToBuff = 1000000.0f;
	if (getAI()->isBuffAvailable())
	{
		distanceToBuff = (getAI()->getBuffPosition() - getAI()->getSimPos()).length();
	}
	m_bb->setStateFloat("DistanceToBuff", distanceToBuff);

	// health percentage
	float healthPercentage = (getAI()->getHealth() / getAI()->getMaxHealth()) * 100.0f;
	m_bb->setStateFloat("HealthPercentage", healthPercentage);

	if (target)
	{
		updateTargetState(target);
	}

	// adjust barrel timer
	bool isValid;
	float timer = m_bb->getStateFloat("BarrelTimer", isValid);
	timer -= dt;
	if (timer < 0.0f)
	{
		timer = 0.0f;
	}
	m_bb->setStateFloat("BarrelTimer", timer);
}

void GorskiBTActorController::updateTargetState(Character* target)
{
	// enemy position
	m_bb->setStateVec3("EnemyPos", target->getSimPos());

	// distance to the enemy
	float distanceToEnemy = (target->getSimPos() - getAI()->getSimPos()).length();
	m_bb->setStateFloat("DistanceToEnemy", distanceToEnemy);

	// enemy distance to the closest barrel
	std::vector<ModelObject*> barrels = getAI()->getBarrels();
	float enemyDistanceToBarrels = 1000000.0f;
	if (barrels.size() > 0)
	{
		enemyDistanceToBarrels = (barrels[0]->getWorldPosition() - target->getSimPos()).length();
		for (int i = 1; i < barrels.size(); i++)
		{
			float currentBarrelDistance = (barrels[i]->getWorldPosition() - target->getSimPos()).length();

			if (currentBarrelDistance < enemyDistanceToBarrels)
			{
				enemyDistanceToBarrels = currentBarrelDistance;
			}
		}
	}
	m_bb->setStateFloat("EnemyDistanceToBarrels", enemyDistanceToBarrels);

	// enemy distance to the buff
	float enemyDistanceToBuff = 1000000.0f;
	if (getAI()->isBuffAvailable())
	{
		enemyDistanceToBuff = (getAI()->getBuffPosition() - target->getSimPos()).length();
	}
	m_bb->setStateFloat("EnemyDistanceToBuff", enemyDistanceToBuff);

	// angle difference to the enemy
	float angleDifference = getAI()->getCharToEnemyAngle(target->getSimPos()).valueDegrees();
	m_bb->setStateFloat("AngleDifference", angleDifference);
}