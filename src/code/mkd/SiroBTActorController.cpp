#include "pch.h"
#include "SiroBTActorController.h"

SiroBTActorController::SiroBTActorController( ActorAI* ai ) : IActorController(ai)
{
    m_bb = new BT::BlackBoard();

	m_prevEnemyPos = mkVec3::ZERO;

    m_bb->init();

    m_parser = new BT::Parser();
}

SiroBTActorController::~SiroBTActorController()
{
    delete m_bb;
    delete m_root;
    delete m_parser;
}

//////////////////////////////////////////////////////////////////////////

void SiroBTActorController::onCreate()
{
    getAI()->setDirection(mkVec3::UNIT_Z);

	m_bb->setStateFloat("rngFbMax", getAI()->getShootingRange());
    m_bb->setStateFloat("rngMelee", getAI()->getMeleeRange());
	 m_bb->setStateFloat("dmgConeSize", getAI()->getMeleeConeSize());
	 m_bb->setStateFloat("EnemySpeed", 0.f);
	 m_bb->setStateVec3("DiffEnemyPos", mkVec3::ZERO);
	 m_bb->setStateVec3("EnemyLastSeenPos", mkVec3::ZERO);
	 m_bb->setStateBool("FindEnemy", false);
	 m_bb->setStateBool("BarrelsInRange", false);

    m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
    m_parser->parseAliases(m_bb);

	m_timeEnemyLastSeen = 0.f;

    Ogre::LogManager::getSingleton().logMessage("BT controller created!");
}

void SiroBTActorController::onTakeDamage( const SDamageInfo& dmg_info )
{

}

void SiroBTActorController::onUpdate( float dt )
{
    updateWorldState(dt);
    BT::Status s = m_root->tick(m_bb);

    if(m_parser->isFileModified(getAI()->getBtTreePath()) && m_root->isTerminated()){
        m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
        m_parser->parseAliases(m_bb);
    }
}

void SiroBTActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
}

void SiroBTActorController::onDie()
{
}

//////////////////////////////////////////////////////////////////////////

void SiroBTActorController::updateWorldState(float dt)
{
	Ogre::LogManager::getSingleton().logMessage("Update");
    Character* target = getAI()->findClosestEnemyInSight();
    if(target){
        m_bb->setStateBool("IsEnemyVisible", true);

		float enemyDistance = (float)(target->getSimPos() - getAI()->getSimPos()).length();
        m_bb->setStateFloat("EnemyDistance", enemyDistance);

		float angleDiff = (float)(getAI()->getCharToEnemyAngle(target->getSimPos()).valueDegrees());
        m_bb->setStateFloat("EnemyDgrDiff", angleDiff);

		m_bb->setStateVec3("EnemyPos", target->getSimPos());

		if((float)(target->getSimPos() - getAI()->getBuffPosition()).length() < 7.f && getAI()->getBarrels().size() > 0){
			m_bb->setStateBool("IsEnemyInBarrelsRange", true);
		}else{
			m_bb->setStateBool("IsEnemyInBarrelsRange", false);
		}

		if(getAI()->getBarrels().size() > 0){
			bool found = false;
			for(int i = 0; i < getAI()->getBarrels().size(); i++){
				mkVec3 barrel_pos = getAI()->getBarrels()[i]->getWorldPosition();
				if((float)(barrel_pos - getAI()->getSimPos()).length() < getAI()->getShootingRange()){
					m_bb->setStateBool("BarrelsInRange", true);
					found = true;
				}
			}
			if(!found){
				m_bb->setStateBool("BarrelsInRange", false);
			}
		}else{
			m_bb->setStateBool("BarrelsInRange", false);
		}

		 float difference= (float)(getAI()->getCharToEnemyAngle(target->getSimDir()).valueDegrees());
		std::string s = boost::lexical_cast<std::string>(difference);
		Ogre::LogManager::getSingleton().logMessage(s);
		if(difference < 3.f){
			m_bb->setStateBool("IsEnemyGoingForward", true);
		}else{
			m_bb->setStateBool("IsEnemyGoingForward", false);
		}
		
		m_bb->setStateVec3("EnemyDirection", target->getSimDir());


	//	m_bb->setStateVec3("DiffEnemyPos", target->getSimPos() - m_prevEnemyPos);
		//m_prevEnemyPos = target->getSimPos();

		m_bb->setStateFloat("EnemySpeed", target->getRealSpeed());

		m_bb->setStateVec3("EnemyLastSeenPos", target->getSimPos());

		if(enemyDistance <= getAI()->getShootingRange()){
			m_timeEnemyLastSeen = getAI()->getTimeMs();
		}

    } else {
        m_bb->setStateBool("IsEnemyVisible", false);
		m_bb->setStateFloat("EnemyDistance", 10000.f);
		m_bb->setStateVec3("EnemyPos", mkVec3::ZERO);
		m_bb->setStateFloat("EnemyDgrDiff", 0.f);
		m_bb->setStateFloat("EnemySpeed", 0.f);
		m_prevEnemyPos = mkVec3::ZERO;
    } 
	
	if(getAI()->getTimeMs() - m_timeEnemyLastSeen > 30000.f){
		m_bb->setStateBool("FindEnemy", true);
	}


	if((float)(getAI()->getPowerLakePosition() - getAI()->getSimPos()).length() < 7.f){
		m_bb->setStateBool("IsSpotReached", true);
	}else{
		m_bb->setStateBool("IsSpotReached", false);
	}
	//m_bb->setStateBool("boolLol", true);
}

