#include "pch.h"
#include "MK_STIG_ActorController.h"

MK_STIG_ActorController::MK_STIG_ActorController( ActorAI* ai ) : IActorController(ai)
	, m_attackDir(mkVec3::ZERO)
	, m_lastAtackTIme(-1.f)
	, m_enemyLastSpoot(mkVec3::ZERO)
	, m_resetAngerMode(false)
	, m_isWaitForAvaibleHealth(false)
	, m_isDDAvaible(true)
{
    m_bb = new BT::BlackBoard();
    m_bb->init();

    m_parser = new BT::Parser();
}

MK_STIG_ActorController::~MK_STIG_ActorController()
{
    delete m_bb;
    delete m_root;
    delete m_parser;
}

//////////////////////////////////////////////////////////////////////////

void MK_STIG_ActorController::onCreate()
{
    getAI()->setDirection(mkVec3::UNIT_Z);

	m_bb->setStateFloat("rngFbMax", getAI()->getShootingRange());
    m_bb->setStateFloat("rngMelee", getAI()->getMeleeRange());
    m_bb->setStateFloat("HealthAMLimit", getAI()->getMaxHealth()/2);
    m_bb->setStateFloat("dmgConeSize", getAI()->getMeleeConeSize());
	m_bb->setStateVec3("AttackDir_LONG_TIME", mkVec3::ZERO);
	m_bb->setStateBool("isEnemyLastSpoot",	false);

	m_bb->setStateBool("IsEnemyAttack",									false );
	m_bb->setStateBool("M_IsEnemyAttack",								false);

	m_bb->setStateVec3("AttackDir",										mkVec3::ZERO);
	m_bb->setStateVec3("AttackDir_LONG_TIME",							mkVec3::ZERO);

	m_bb->setStateFloat("kiedyStrzelilesWBeczke",						-10000000000.f);

    m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
    m_parser->parseAliases(m_bb);

    Ogre::LogManager::getSingleton().logMessage("BT controller created!");
}

void MK_STIG_ActorController::onTakeDamage( const SDamageInfo& dmg_info )
{
	m_attackDir = mkVec3::ZERO-dmg_info.dir;
	m_lastAtackTIme = g_game->getTimeMs();
}

void MK_STIG_ActorController::onUpdate( float dt )
{
    updateWorldState(dt);
    BT::Status s = m_root->tick(m_bb);

    if(m_parser->isFileModified(getAI()->getBtTreePath()) && m_root->isTerminated()){
        m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
        m_parser->parseAliases(m_bb);
    }
}

void MK_STIG_ActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
}

void MK_STIG_ActorController::onDie()
{
	m_resetAngerMode = true;
}

//////////////////////////////////////////////////////////////////////////

void MK_STIG_ActorController::updateWorldState(float dt)
{
    Character* target = getAI()->findClosestEnemyInSight();
    bool isEnemyAttack = m_attackDir != mkVec3::ZERO;
	const float cur_time = g_game->getTimeMs();
	const float czasOdOstatniegoAtaku = cur_time - m_lastAtackTIme;
	bool isEnemyVisible = target ? true : false;
	
	mkVec3 myPos = getAI()->getSimPos();

	mkVec3 enemyPos = isEnemyVisible ? target->getSimPos() : mkVec3::ZERO;
	if (isEnemyVisible) m_enemyLastSpoot = enemyPos;
	bool isEnemySpot = m_enemyLastSpoot != mkVec3::ZERO;

	float angleDiff = 0.f;
	if (isEnemyVisible) angleDiff = (float)(getAI()->getCharToEnemyAngle(target->getSimPos()).valueDegrees());

	float enemyDistance = 0.f;
	if (isEnemyVisible) enemyDistance = (float)(target->getSimPos() - getAI()->getSimPos()).length();

	float meleeRange = getAI()->getMeleeRange();

	

	if(isHealthOnCriticalLevel())
		m_isWaitForAvaibleHealth = true;
	else if(m_isWaitForAvaibleHealth && isHealthOnAvailableLevel())
		m_isWaitForAvaibleHealth = false;
	//LastEnemySpot
	
	//=================================================================================================================

	bool czyBeczkaWZasieguWroga = false; 
	bool czyJakasBeczkaNaMapie = false;	
	bool czyJestemWZasieguBeczkiWroga = false; 
	bool czyJestemWZasieguJakiejkolwiekBeczki = false;	

	bool isValid = true;

	// Strza³ w beczke co 2s
	float ostatniStrzalWBeczke = m_bb->getStateFloat("kiedyStrzelilesWBeczke", isValid);
	float czas = g_game->getTimeMs();
	bool czyMoznaStrzelacWBeczke = (czas - ostatniStrzalWBeczke) >= (1000.f * 2.f) && isValid;

	// Pozycja najblizszej beczki wzgledem wroga
	mkVec3 pozycjaNajblizszejBeczki = mkVec3::ZERO;	
	float odlegloscDoNajblizszejBeczki = 10000.f;
	
	char nazwyBeczek[4][10] = {"eo_b1", "eo_b2", "eo_b3", "eo_b4"};
	
	for(int i = 0; i < 4;i++)
	{	// po wszystkich beczkach
		if(getAI()->isObjectAvailable(nazwyBeczek[i]))
		{ 
			mkVec3 pozycjaBeczki = getAI()->getObjectPosition(nazwyBeczek[i]);
			
			czyJakasBeczkaNaMapie |= czyMoznaStrzelacWBeczke;
			// czy ja w zasiegu jakiejs beczki
			czyJestemWZasieguJakiejkolwiekBeczki |= (myPos-pozycjaBeczki).length() <= 6.f;	

			// czy widac wroga i czy jest on w zasiegu eksplozji
			if(isEnemyVisible && (enemyPos-pozycjaBeczki).length() <= 6.f)
			{
				czyBeczkaWZasieguWroga |= czyMoznaStrzelacWBeczke;	
				// wybieramy najblirzsza beczke
				if((enemyPos-pozycjaBeczki).length() < odlegloscDoNajblizszejBeczki)
				{
					odlegloscDoNajblizszejBeczki = (enemyPos-pozycjaBeczki).length();
					pozycjaNajblizszejBeczki = pozycjaBeczki;
					// czy ja w zasiegu beczki wroga
					czyJestemWZasieguBeczkiWroga = (myPos - pozycjaBeczki).length() <= 6.f;
				}
			}
		}
	}
	
	if(czyBeczkaWZasieguWroga)
	{
		DebugDrawer::getSingleton().drawCircle(pozycjaNajblizszejBeczki + mkVec3::UNIT_Y, 6.f, 30, Ogre::ColourValue::Black, true);
	}
	
	//=================================================================================================================
	
	m_isDDAvaible = m_isDDAvaible && getAI()->isObjectAvailable("cvt_dd");
	mkVec3 ddPosition = getAI()->getObjectPosition("cvt_dd");
	float ddDistance = (ddPosition - myPos).length();

	if(m_isDDAvaible)
	{
		DebugDrawer::getSingleton().drawCircle(ddPosition + mkVec3::UNIT_Y, 1.f, 30, Ogre::ColourValue::Red, true);
	}
	

	//=================================================================================================================

	if(m_resetAngerMode){ 
        m_bb->setStateBool("IsActorAM", false);
        m_resetAngerMode = false;
    }

	// Informacje na temat wroga
	m_bb->setStateBool("IsEnemyVisible",								isEnemyVisible);
	m_bb->setStateBool("isInFireRange",									isEnemyVisible && isInFireRange(target));
	m_bb->setStateBool("isInMeleeRange",								isEnemyVisible && isInMeleeRange(target));
	m_bb->setStateBool("isInMeleeAngle",								angleDiff < getAI()->getMeleeConeSize());
	m_bb->setStateBool("isInMeleeHalfAngle",							angleDiff < getAI()->getMeleeConeSize()/2);
	m_bb->setStateFloat("EnemyDgrDiff",									angleDiff);
	m_bb->setStateVec3("EnemyPos",										enemyPos);

	if(isEnemyVisible) 
		m_bb->setStateBool("isEnemyLastSpoot",							true);
	m_bb->setStateVec3("EnemyLastSpoot",								m_enemyLastSpoot);
    m_bb->setStateFloat("EnemyDistance",								enemyDistance);
	
	// Inforamcje na temat beczki
	m_bb->setStateBool("czyBeczkaWZasieguWroga",						czyBeczkaWZasieguWroga);
	m_bb->setStateBool("czyJakasBeczkaNaMapie",							czyJakasBeczkaNaMapie);
	m_bb->setStateVec3("pozycjaNajblizszejBeczki",						pozycjaNajblizszejBeczki);
	m_bb->setStateBool("czyJestemWZasieguJakiejkolwiekBeczki",			czyJestemWZasieguJakiejkolwiekBeczki);
	m_bb->setStateBool("czyMogeBezpircznieRzucac",						(czyJestemWZasieguJakiejkolwiekBeczki && !czyBeczkaWZasieguWroga) || !czyJestemWZasieguJakiejkolwiekBeczki);


	// informacje na temat DD
	m_bb->setStateBool("isDD",											m_isDDAvaible);
	m_bb->setStateVec3("ddPosition",									ddPosition);
	m_bb->setStateFloat("ddDistance",									ddDistance);


	// Czy nas atakowa³
	if (isEnemyAttack){
		m_bb->setStateBool("IsEnemyAttack",								isEnemyAttack);
		m_bb->setStateBool("M_IsEnemyAttack",							isEnemyAttack);
		m_bb->setStateVec3("AttackDir",									m_attackDir);
		m_bb->setStateVec3("AttackDir_LONG_TIME",						m_attackDir);
	}
	if(isEnemyVisible){
		m_bb->setStateBool("IsEnemyAttack",								false);
	}
	
	m_bb->setStateBool("IsEnemyAttackFor_1s",							czasOdOstatniegoAtaku <= 1000.f);
	m_bb->setStateBool("IsEnemyAttackFor_1.5s",							czasOdOstatniegoAtaku <= 1500.f);
	m_bb->setStateBool("IsEnemyAttackFor_2s",							czasOdOstatniegoAtaku <= 2000.f);

	// Poziom zdrowia
	m_bb->setStateBool("isHealthOnCriticalLevelAndNotInPowerLake",		isHealthOnCriticalLevelAndNotInPowerLake());
	m_bb->setStateBool("isHealthOnCriticalLevel",						isHealthOnCriticalLevel());
	m_bb->setStateBool("isHealthOnAvailableLevel",						isHealthOnAvailableLevel());
	m_bb->setStateFloat("HealthPoints",									getAI()->getHealth());
	m_bb->setStateFloat("ActorHealth",									getAI()->getHealth());
	m_bb->setStateBool("isInPowerLake",									isInPowerLake());
	m_bb->setStateBool("isWaitForAvaibleHealth",						m_isWaitForAvaibleHealth);

	//===========================================================================================================

	m_attackDir = mkVec3::ZERO;											// Reset atakowania
}

/************************************************************************************
*************************************************************************************
************************************************************************************/

bool MK_STIG_ActorController::isInFireRange(Character* target){
	return getAI()->isInShootingRange(target);
}

bool MK_STIG_ActorController::isInMeleeRange(Character* target){
	return target->getSimPos().distance(getAI()->getSimPos()) <= getAI()->getMeleeRange();
}

bool MK_STIG_ActorController::isHealthOnCriticalLevelAndNotInPowerLake(){
	return isHealthOnCriticalLevel() && !isInPowerLake();
}

bool MK_STIG_ActorController::isHealthOnCriticalLevel(){	return getAI()->getHealth() / getAI()->getMaxHealth() < 0.3f; }

bool MK_STIG_ActorController::isInPowerLake(){				return getAI()->isInPowerLake(); }

bool MK_STIG_ActorController::isHealthOnAvailableLevel(){
	return getAI()->getHealth() / getAI()->getMaxHealth() > 0.8f;
}