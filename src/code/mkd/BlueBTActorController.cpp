#include "pch.h"
#include "BlueBTActorController.h"

BlueBTActorController::BlueBTActorController( ActorAI* ai ) : IActorController(ai)
{
    m_bb = new BT::BlackBoard();
    m_attackDir = mkVec3::ZERO;
    m_bb->init();
    m_parser = new BT::Parser();
}

BlueBTActorController::~BlueBTActorController()
{
    delete m_bb;
    delete m_root;
    delete m_parser;
}

//////////////////////////////////////////////////////////////////////////

void BlueBTActorController::onCreate()
{
    getAI()->lookAt(mkVec3::ZERO);
    m_bb->setStateFloat("ShootingRange", getAI()->getShootingRange());
    m_bb->setStateFloat("MeleeRange", getAI()->getMeleeRange());

	m_bb->setStateBool("IsHealing", false);
	m_bb->setStateFloat("HealthLimit", getAI()->getMaxHealth() / 2);
	m_bb->setStateBool("InLake", false);

	if(m_attackDir != mkVec3::ZERO){
        m_bb->setStateBool("IsEnemyAttack",true);
        m_bb->setStateVec3("AttackDir",m_attackDir);
        m_attackDir = mkVec3::ZERO;
    }

    m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
    m_parser->parseAliases(m_bb);
}

void BlueBTActorController::onTakeDamage( const SDamageInfo& dmg_info )
{
    m_attackDir = mkVec3::ZERO-dmg_info.dir;
}

void BlueBTActorController::onUpdate( float dt )
{
    updateWorldState(dt);
    BT::Status s = m_root->tick(m_bb);

    if(m_parser->isFileModified(getAI()->getBtTreePath()) && m_root->isTerminated()){
        m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
        m_parser->parseAliases(m_bb);
    }
}

void BlueBTActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
}

void BlueBTActorController::onDie()
{
    
}

//////////////////////////////////////////////////////////////////////////

void BlueBTActorController::updateWorldState(float dt)
{
	m_bb->setStateBool("IsEnemyAttack", false);
	m_bb->setStateBool("IsEnemyVisible", false);

	m_bb->setStateFloat("CurrentHealth", getAI()->getHealth());

	bool isValid = true;
	//bool isHealing = m_bb->getStateBool("IsHealing", isValid);

	m_bb->setStateBool("BuffAvailable", getAI()->isBuffAvailable());
	m_bb->setStateBool("MedkitAvailable", getAI()->isMedkitAvailable());

	//jesli jestesmy w jeziorze, to strzelamy
	m_bb->setStateBool("InLake", (getAI()->getSimPos() - getAI()->getPowerLakePosition()).length() <= 10.0f);

	m_target = getAI()->findClosestEnemyInSight();

	//wyleczono
	if(m_bb->getStateFloat("CurrentHealth", isValid) == getAI()->getMaxHealth()) {
		m_bb->setStateBool("IsHealing", false);
	}

	if(m_target) { //znaleziono cel - gonimy
		m_bb->setStateBool("IsEnemyVisible", true);
		m_bb->setStateVec3("EnemyPos", m_target->getSimPos());

		//odleglosc od celu
		float enemyDistance = (float)(m_target->getSimPos() - getAI()->getSimPos()).length();
        m_bb->setStateFloat("EnemyDistance", enemyDistance);
	} else {
		m_bb->setStateBool("IsEnemyVisible", false);
	}
}