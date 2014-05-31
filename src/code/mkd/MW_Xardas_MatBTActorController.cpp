#include "pch.h"
#include "MW_Xardas_MatBTActorController.h"

MW_XardasActorController::MW_XardasActorController(ActorAI* ai) : IActorController(ai)
{
	m_bb = new BT::BlackBoard();

	m_attackDir = mkVec3::ZERO;
	m_prevEnemyDistTime = 0.f;
	m_prevEnemyDist = 10000.f;
	m_prevDistSum = 0.f;
	m_resetAngerMode = false;

	m_minSpeed = 1.0f;

	m_lastHitDt = 1500.0f;
	m_lastHit = g_game->getTimeMs();

	m_bb->init();

	m_parser = new BT::Parser();
}

MW_XardasActorController::~MW_XardasActorController()
{
	delete m_bb;
	delete m_root;
	delete m_parser;
}

//////////////////////////////////////////////////////////////////////////

void MW_XardasActorController::onCreate()
{
	getAI()->lookAt(mkVec3::ZERO);
	m_bb->setStateFloat("rngFbMax", getAI()->getShootingRange());
	m_bb->setStateFloat("rngMelee", getAI()->getMeleeRange());
	m_bb->setStateFloat("rng2Melee", getAI()->getMeleeRange()*1.7f);
	m_bb->setStateFloat("rngRingMinMelee", getAI()->getMeleeRange()*1.3f);
	m_bb->setStateFloat("rngRingMaxMelee", getAI()->getMeleeRange()*1.5f);
	m_bb->setStateFloat("HealthAMLimit", getAI()->getMaxHealth() / 2);
	m_bb->setStateFloat("dmgConeSize", getAI()->getMeleeConeSize());
	m_bb->setStateFloat("rngMinFbMax", getAI()->getShootingRange()*0.75);
	m_bb->setStateBool("ProbabDmg", false);
	m_bb->setStateBool("ProbabFutureDmg", false);

	//Ogre::LogManager::getSingleton().logMessage(getAI()->getBtTreePath());
	m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
	m_parser->parseAliases(m_bb);

	Ogre::LogManager::getSingleton().logMessage("BT controller created!");
}

void MW_XardasActorController::onTakeDamage(const SDamageInfo& dmg_info)
{
	m_attackDir = mkVec3::ZERO - dmg_info.dir;

	m_bb->setStateFloat("lastHitDt", g_game->getTimeMs() - m_lastHit);
	m_bb->setStateFloat("lastHit", g_game->getTimeMs());

	m_lastHitDt = g_game->getTimeMs() - m_lastHit;
	m_lastHit = g_game->getTimeMs();

	m_probabHit = m_lastHit + m_lastHitDt;

	std::stringstream ss;
	ss << "##hit " << m_lastHit << "::" << m_lastHitDt << "::" << m_probabHit << std::endl;

	Ogre::LogManager::getSingleton().logMessage(ss.str());
}

void MW_XardasActorController::onUpdate(float dt)
{
	updateWorldState(dt);
	BT::Status s = m_root->tick(m_bb);

	if (m_parser->isFileModified(getAI()->getBtTreePath()) && m_root->isTerminated()){
		m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
		m_parser->parseAliases(m_bb);
	}
}

void MW_XardasActorController::onDebugDraw()
{
	getAI()->drawSensesInfo();
	DebugDrawer::getSingleton().drawCircle(m_enemySpot, 0.2f, 30, Ogre::ColourValue::Black, true);
}

void MW_XardasActorController::onDie()
{
	m_resetAngerMode = true;
}

//////////////////////////////////////////////////////////////////////////

void MW_XardasActorController::updateWorldState(float dt)
{
	

	if (m_resetAngerMode){
		m_bb->setStateBool("IsActorAM", false);
		m_resetAngerMode = false;
	}

	if (m_attackDir != mkVec3::ZERO){
		m_bb->setStateBool("IsEnemyAttack", true);
		m_bb->setStateVec3("AttackDir", m_attackDir);
		m_attackDir = mkVec3::ZERO;
	}

	// wall behind
	mkVec3 oldDir = getAI()->getSimDir();
	RayCastResult ray_result = getAI()->raycast(-oldDir, 0.0f, 0.5f);
	RayCastResult ray_result2 = getAI()->raycast(-oldDir, 1.0f, 0.5f);

	if (ray_result.hit || ray_result2.hit)
	{
		//Beep(1000, 100);
		m_bb->setStateBool("IsWallBehind", true);
	}
	else
		m_bb->setStateBool("IsWallBehind", false);


	//m_lastHitDt = 100;
	if (m_probabHit + 50 < g_game->getTimeMs())
		m_probabHit += m_lastHitDt;

	if (abs(m_probabHit - g_game->getTimeMs()) < 500)
	{
		m_bb->setStateBool("ProbabFutureDmg", true);
	}
	else
	{
		m_bb->setStateBool("ProbabFutureDmg", false);
	}

	if (abs(m_probabHit - g_game->getTimeMs()) < 150)
	{
		m_bb->setStateBool("ProbabDmg", true);
	}
	else
	{
		m_bb->setStateBool("ProbabDmg", false);
	}


	m_bb->setStateBool("EnemyCloseToBarrel", false);

	m_bb->setStateFloat("ActorHealth", getAI()->getHealth());
	m_target = getAI()->findClosestEnemyInSight();
	if (m_target){


		float angleDiff = (float)(getAI()->getCharToEnemyAngle(m_target->getSimPos()).valueDegrees());
		m_bb->setStateFloat("EnemyDgrDiff", angleDiff);
		m_bb->setStateBool("IsEnemySeen", true);
		m_bb->setStateVec3("LastEnemySpot", m_target->getSimPos());

		m_enemySpeed = (m_target->getSimPos() - m_enemySpot) / dt;

		lastSpeeds.push_back(m_enemySpeed);
		speedSum += m_enemySpeed;

		while (lastSpeeds.size() > avgSize)
		{
			speedSum -= lastSpeeds.front();
			lastSpeeds.pop_front();
		}

		m_enemySpot = m_target->getSimPos();
		
		std::vector<ModelObject*> barrels = getAI()->getBarrels();

		for (int i = 0; i < barrels.size(); ++i)
		{
			if (barrels[i]->getWorldPosition().squaredDistance(m_enemySpot) < 8 * 8 && barrels[i]->getWorldPosition().distance(getAI()->getSimPos()) < getAI()->getShootingRange())
			{
				m_bb->setStateBool("EnemyCloseToBarrel", true);
				m_bb->setStateVec3("BarrelOfDeath", barrels[i]->getWorldPosition());
				break;
			}
		}
		

		m_bb->setStateVec3("EnemyPos", m_target->getSimPos());
		m_bb->setStateBool("IsEnemyVisible", true);

		m_bb->setStateVec3("EnemySpeed", m_enemySpeed);
		m_bb->setStateVec3("EnemyAvgSpeed", speedSum / avgSize);
		m_bb->setStateVec3("EnemyFeet", m_target->getPosForHeight(0) * 2 - m_target->getPosForHeight(1));

		if (m_enemySpeed.squaredLength() > m_minSpeed)
			m_bb->setStateBool("IsEnemyMoving", true);
		else
			m_bb->setStateBool("IsEnemyMoving", false);

		float enemyDistance = (float)(m_target->getSimPos() - getAI()->getSimPos()).length();
		m_bb->setStateFloat("EnemyDistance", enemyDistance);

		m_prevDistSum += enemyDistance - m_prevEnemyDist;
		m_prevEnemyDist = enemyDistance;

		m_prevEnemyDistTime = m_prevEnemyDistTime - dt;
		if (m_prevEnemyDistTime <= 0.f){
			if (m_prevDistSum > 1.f)
				m_bb->setStateBool("IsEnemyRunningAway", true);
			else
				m_bb->setStateBool("IsEnemyRunningAway", false);
			m_prevDistSum = 0.f;
			m_prevEnemyDistTime = 0.5f;
		}
	}
	else {
		m_bb->setStateVec3("EnemyPos", mkVec3::ZERO);
		m_bb->setStateBool("IsEnemyVisible", false);
		m_bb->setStateFloat("EnemyDistance", 10000.f);
		m_bb->setStateFloat("EnemyDgrDiff", 0.f);
	}
}
