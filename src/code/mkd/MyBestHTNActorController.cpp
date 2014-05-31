#include "pch.h"
#include "MyBestHTNActorController.h"
#include "contrib/DebugDrawer.h"
#include "cstdarg"

MyBestHTNActorController::MyBestHTNActorController( ActorAI* ai ) : IActorController(ai)
{
    m_planner = new HTN::Planner();
    m_planner->init(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath());

    //actions////////////////////////////////////////////////////////////////////////
    m_actions["opPatrol"] = &MyBestHTNActorController::actionPatrol;
    m_actions["opAttackMelee"] = &MyBestHTNActorController::actionAttackMelee;
    m_actions["opAttackFireball"] = &MyBestHTNActorController::actionAttackFireball;
	m_actions["opGoToPowerLake"] = &MyBestHTNActorController::actionGoToPowerLake;
	m_actions["opStayInPowerLake"] = &MyBestHTNActorController::actionStayInPowerLake;
	m_actions["opReduceDistance"] = &MyBestHTNActorController::actionReduceDistance;
	m_actions["opReduceDistanceLong"] = &MyBestHTNActorController::actionReduceDistance;
	m_actions["opRotateToEnemy"] = &MyBestHTNActorController::actionRotateToEnemy;
	m_actions["opTakeBuff"] = &MyBestHTNActorController::actionTakeBuff;
	m_actions["opDestroyBarrels"] = &MyBestHTNActorController::actionDestroyBarrels;
	m_actions["opFindAttacker"] = &MyBestHTNActorController::actionFindAttacker;
	m_actions["opGoToEnemyLastSpot"] = &MyBestHTNActorController::actionGoToEnemyLastSpot;
	m_actions["opTakeMedkit"] = &MyBestHTNActorController::actionTakeMedkit;
	m_actions["opRest"] = &MyBestHTNActorController::actionRest;
    //////////////////////////////////////////////////////////////////////////
    m_actions["animAttackFireball"] = &MyBestHTNActorController::animAttackFireball;
    m_actions["animAttackMelee"] = &MyBestHTNActorController::animAttackMelee;

    m_enemyRunningAway = false;
    m_prevEnemyDist = 1000.f;
    m_prevEnemyDistTime = 0.f;
    m_prevDistSum = 0.f;
    m_enemyLastPos = mkVec3::ZERO;
    m_target = NULL;
	m_isPowerLakeReached = false;
	m_isStayingInPowerLake = false;
	m_isBuffAvailable = true;
	m_isAIBuffed = false;
	m_isGoingToPowerLake = false;
	m_canAttackEnemy = false;
	m_prevHealth = getAI()->getHealth();
	m_isEnemyAttacking = false;
	m_isLookingForEnemy = false;
}

MyBestHTNActorController::~MyBestHTNActorController(){
    delete m_planner;
}

//////////////////////////////////////////////////////////////////////////

void MyBestHTNActorController::onCreate(){
    getAI()->lookAt(mkVec3::ZERO);
    //ai_specific////////////////////////////////////////////////////////////////////////
    m_planner->setStateFloat("rngMelee",getAI()->getMeleeRange());
	m_planner->setStateFloat("nearMeleeRange", (getAI()->getMeleeRange()+5.f));
	m_planner->setStateFloat("rngFbMax", getAI()->getShootingRange());
    m_planner->setStateFloat("nearRngFbMax",getAI()->getShootingRange()*0.9);
    m_planner->setStateFloat("dmgConeSize",getAI()->getMeleeConeSize());
    m_planner->setStateFloat("HealthAMLimit",(getAI()->getMaxHealth())/2);
	m_planner->setStateFloat("ActorMaxHealth",(getAI()->getMaxHealth()));
    //conditions////////////////////////////////////////////////////////////////////////
    m_planner->setStateBool("IsEnemyVisible",false);
    m_planner->setStateBool("IsEnemyDead",false);
    m_planner->setStateBool("IsEnemyAttack",false);
    m_planner->setStateBool("IsEnemyRunningAway",false);
    m_planner->setStateBool("IsEnemySeen",false);
    m_planner->setStateFloat("EnemyDistance",FLT_MAX);
    m_planner->setStateFloat("ActorHealth",0.f);
    m_planner->setStateFloat("EnemyDgrDiff",0.f);
	m_planner->setStateBool("IsBuffAvailable", true);
	m_planner->setStateBool("IsAIBuffed", false);
	m_planner->setStateBool("IsGoingToPowerLake", false);
	m_planner->setStateBool("IsPowerLakeReached", false);
	m_planner->setStateBool("IsStayingInPowerLake", false);
	m_planner->setStateFloat("EnemyDistanceFromBarrels", FLT_MAX);
	m_planner->setStateFloat("MaxEnemyDistanceFromBarrels", FLT_MAX);
	m_planner->setStateFloat("AIDistanceFromBarrels", FLT_MIN);
	m_planner->setStateFloat("AIDistanceFromBuff", FLT_MAX);
	m_planner->setStateBool("EnemyCanBeAttackedByMelee", false);
	m_planner->setStateBool("IsEnemyAttacking", false);
	m_planner->setStateBool("IsMedkitAvailable", true);
	m_planner->setStateFloat("MedkitDistance", FLT_MAX);
	m_planner->setStateFloat("TimeSinceLastAttack", FLT_MAX);
	m_planner->setStateBool("IsEnemyInPowerLake", false);
	m_planner->setStateBool("IsEnemyGoingToBuff", true);
	m_planner->setStateFloat("EnemyDistanceFromBuff", FLT_MAX);
	m_planner->setStateFloat("AIDistanceFromPowerLake", FLT_MAX);

	m_lastAttackTime = FLT_MIN;

    Ogre::LogManager::getSingleton().logMessage("HTN controller created!");
}

void MyBestHTNActorController::onTakeDamage(const SDamageInfo& dmg_info){
    m_attackDir = mkVec3::ZERO-dmg_info.dir;
	m_isEnemyAttacking = true;
	m_lastAttackTime = getAI()->getTimeMs();
}

void MyBestHTNActorController::onUpdate(float dt){
    updateWorldState(dt);

    std::vector<HTN::pTask> plan = m_planner->getPlan(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath());
    HTN::pOperator newTask;
    HTN::PlanResult result = m_planner->resolvePlan(plan, dt, newTask);

    switch (result)
    {
    case HTN::PLAN_EMPTYPLAN:
        getAI()->setSpeed(0.f);
        break;
    case HTN::PLAN_INTERRUPTED:
        getAI()->stopSmoothChangeDir();
        getAI()->stopAnimation();
    case HTN::PLAN_NEW:
		executeTask(newTask);
        break;
    case HTN::PLAN_RUNNING:
        break;
    }
}

void MyBestHTNActorController::onDebugDraw(){
    getAI()->drawSensesInfo();
    if(m_enemyLastPos != mkVec3::ZERO)
        DebugDrawer::getSingleton().drawCircle(m_enemyLastPos, 0.2f, 30, Ogre::ColourValue::Black, true);
}

void MyBestHTNActorController::onDie()
{
    
}

//////////////////////////////////////////////////////////////////////////

void MyBestHTNActorController::updateWorldState(float dt){
	m_planner->setStateFloat("AIDistanceFromPowerLake", (getAI()->getPowerLakePosition()-getAI()->getSimPos()).length() );
	float timeSinceLastAttack = getAI()->getTimeMs()-m_lastAttackTime;
	if(m_lastAttackTime == FLT_MIN) {
		timeSinceLastAttack = FLT_MAX;
	}
	m_planner->setStateFloat("TimeSinceLastAttack", timeSinceLastAttack);
	m_planner->setStateBool("IsMedkitAvailable", getAI()->isMedkitAvailable());
	if(getAI()->isMedkitAvailable()) {
		m_planner->setStateFloat("MedkitDistance", (getAI()->getSimPos() - getAI()->getMedkitPosition()).length());
	}
    
    m_planner->setStateFloat("ActorHealth",getAI()->getHealth());

    m_target = getAI()->findClosestEnemyInSight();
    m_planner->setStateBool("IsEnemyVisible",m_target);

	if(getAI()->isBuffAvailable() && m_target) {
		float prevEnemyDistFromBuff = (getAI()->getBuffPosition() - m_enemyLastPos).length();
		float currentEnemyDistFromBuff = (getAI()->getBuffPosition() - m_target->getSimPos()).length();
		float enemyAngleToBuff = m_target->getCharToEnemyAngle(getAI()->getBuffPosition()).valueDegrees();
		if(prevEnemyDistFromBuff > currentEnemyDistFromBuff && enemyAngleToBuff < 30) {
			m_planner->setStateFloat("EnemyDistanceFromBuff", currentEnemyDistFromBuff);
			m_planner->setStateBool("IsEnemyGoingToBuff", true);
		}
	}
	else {
		m_planner->setStateFloat("EnemyDistanceFromBuff", FLT_MAX);
		m_planner->setStateBool("IsEnemyGoingToBuff", false);
	}

    if(m_target){
		m_isLookingForEnemy = false;
        float angleDiff = (float)(getAI()->getCharToEnemyAngle(m_target->getSimPos()).valueDegrees());
        m_planner->setStateFloat("EnemyDgrDiff",angleDiff);
        ActorAI *targetAI = dynamic_cast<ActorAI*>(m_target);
        Player *targetPlayer = dynamic_cast<Player*>(m_target);
        if(targetAI)
            m_planner->setStateBool("IsEnemyDead",targetAI->getHealth() > 0.f ? false : true);
        else if(targetPlayer)
            m_planner->setStateBool("IsEnemyDead",targetPlayer->getHealth() > 0.f ? false : true);

        float enemyDistance = (float)(m_target->getSimPos() - getAI()->getSimPos()).length();
        m_planner->setStateFloat("EnemyDistance",enemyDistance);

        m_prevDistSum += enemyDistance - m_prevEnemyDist;
        m_prevEnemyDist = enemyDistance;

        m_prevEnemyDistTime = m_prevEnemyDistTime - dt;
        if(m_prevEnemyDistTime <= 0.f){
            if(m_prevDistSum > 1.f)
                m_enemyRunningAway = true;
            else
                m_enemyRunningAway = false;
            m_prevDistSum = 0.f;
            m_prevEnemyDistTime = 0.5f;
        }

        m_planner->setStateBool("IsEnemyRunningAway",m_enemyRunningAway);

        m_enemyLastPos = m_target->getSimPos();
        m_planner->setStateBool("IsEnemySeen",true);
    } else {
		m_planner->setStateBool("IsEnemyInPowerLake", false);
        m_planner->setStateBool("IsEnemyVisible",false);
        m_planner->setStateBool("IsEnemyDead",false);
        m_planner->setStateBool("IsEnemyRunningAway",false);
        m_planner->setStateFloat("EnemyDistance",1000.f);
        m_planner->setStateFloat("EnemyDistanceDiff",0.f);
        m_planner->setStateFloat("EnemyDgrDiff",0.f);
    }

	if( getAI()->isInPowerLake() ) {
		m_isPowerLakeReached = true;
		m_planner->setStateBool("IsPowerLakeReached", true);
	}
	else {
		m_isPowerLakeReached = false;
		m_planner->setStateBool("IsPowerLakeReached", false);
	}

	if(getAI()->isBuffAvailable()) {
		m_isBuffAvailable = true;
		m_planner->setStateBool("IsBuffAvailable", true);
		m_planner->setStateFloat("AIDistanceFromBuff", (getAI()->getBuffPosition() - getAI()->getSimPos()).length());
	}
	else {
		m_isBuffAvailable = false;
		m_planner->setStateBool("IsBuffAvailable", false);
		m_planner->setStateFloat("AIDistanceFromBuff", FLT_MAX);
	}

	m_isAIBuffed = getAI()->hasBuff();
	m_planner->setStateBool("IsAIBuffed", m_isAIBuffed);

	m_planner->setStateBool("IsGoingToPowerLake", m_isGoingToPowerLake);

	m_planner->setStateBool("IsStayingInPowerLake", m_isStayingInPowerLake);

	float enemyDistanceFromBarrels = FLT_MAX;
	float maxEnemyDistanceFromBarrels = FLT_MIN;
	if(m_target) {
		for(int i=0; i<getAI()->getBarrels().size(); i++) {
			float distance = (getAI()->getBarrels()[i]->getWorldPosition() - m_target->getSimPos()).squaredLength();
			if(distance < enemyDistanceFromBarrels) {
				enemyDistanceFromBarrels = distance;
			}
			if(distance > maxEnemyDistanceFromBarrels) {
				maxEnemyDistanceFromBarrels = distance;
			}
		}
	}
	m_planner->setStateFloat("EnemyDistanceFromBarrels", enemyDistanceFromBarrels);
	m_planner->setStateFloat("MaxEnemyDistanceFromBarrels", maxEnemyDistanceFromBarrels);

	float AIDistanceFromBarrels = FLT_MAX;
	if(m_target) {
		for(int i=0; i<getAI()->getBarrels().size(); i++) {
			float distance = (getAI()->getBarrels()[i]->getWorldPosition() - getAI()->getSimPos()).squaredLength();
			if(distance < AIDistanceFromBarrels) {
				AIDistanceFromBarrels = distance;
			}
		}
	}
	m_planner->setStateFloat("AIDistanceFromBarrels", enemyDistanceFromBarrels);

	if(m_target) {
		float angleDiff = (float)(getAI()->getCharToEnemyAngle(m_target->getSimPos()).valueDegrees());
		m_canAttackEnemy = m_target && (m_target->getSimPos() - getAI()->getSimPos()).length()<=getAI()->getMeleeRange() 
			&& angleDiff<=getAI()->getMeleeConeSize();
		m_planner->setStateBool("EnemyCanBeAttackedByMelee", m_canAttackEnemy);
		m_planner->setStateBool("IsEnemyInPowerLake", m_target->isInPowerLake());
	}

	m_isEnemyAttacking = getAI()->getHealth() < m_prevHealth;
	m_planner->setStateBool("IsEnemyAttacking", m_isEnemyAttacking);
	m_prevHealth = getAI()->getHealth();
}

void MyBestHTNActorController::executeTask(HTN::pOperator nextTask){
    ctrlrAction action = m_actions[nextTask->getName()];
    (this->*action)(nextTask->getDuration());
}

//----------actions----------
bool MyBestHTNActorController::actionPatrol(float duration) {
	if(m_isGoingToPowerLake || (getAI()->isBuffAvailable() && !getAI()->hasBuff())) {
		return false;
	}
    mkVec3 new_direction = getRandomHorizontalDir();
    RayCastResult ray_result = getAI()->raycast(new_direction, 1.0f, 5.f);
    while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
        new_direction = getRandomHorizontalDir();
        ray_result = getAI()->raycast(new_direction, 1.0f, 5.f);
    }

    size_t steps = 20;
    getAI()->startSmoothChangeDir(new_direction, steps, duration/2);
    getAI()->setSpeed(1.f);

    return true;
}

bool MyBestHTNActorController::actionRotateToEnemy(float duration){
    if(!m_target || (getAI()->isBuffAvailable() && !getAI()->hasBuff()))
        return false;

    size_t steps = 20;
    mkVec3 new_direction = m_target->getSimPos() - getAI()->getSimPos();
    new_direction.normalise();
    getAI()->startSmoothChangeDir(new_direction, steps, duration);
    getAI()->setSpeed(0.f);

    return true;
}

bool MyBestHTNActorController::actionAttackMelee(float duration){
	if(!m_target || !m_canAttackEnemy)
        return false;

    getAI()->hitMelee();
    getAI()->setSpeed(0.f);

    return true;
}

bool MyBestHTNActorController::actionAttackFireball(float duration){
	if(!m_target && m_enemyLastPos==mkVec3::ZERO)
        return false;
	if(m_target) {
		const float animAttackFireballDuration = 1150;
		const float actionAttackFireballDuration = 50;		
		mkVec3 target_direction = m_target->getSimDir()*m_target->getRealSpeed()
			*(animAttackFireballDuration/actionAttackFireballDuration) + m_target->getSimPos();
		getAI()->hitFireball(target_direction);
	}
	else {
		getAI()->hitFireball(m_enemyLastPos);
	}
    getAI()->setSpeed(0.f);
    return true;
}

bool MyBestHTNActorController::actionReduceDistance(float duration){
	if(!m_target) {
        return false;
	}

    mkVec3 destDir = (m_target->getSimPos()-getAI()->getSimPos()).normalisedCopy();
    size_t steps = 20;
    getAI()->startSmoothChangeDir(destDir, steps, duration);
    getAI()->setSpeed(1.f);

    return true;
}

bool MyBestHTNActorController::actionGoToPowerLake(float duration)
{
	m_isGoingToPowerLake = true;

	mkVec3 new_direction = getAI()->getPowerLakePosition();
    RayCastResult ray_result = getAI()->raycast(new_direction, 1.0f, 10.f);
    while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
		new_direction = getRandomHorizontalDir();
		ray_result = getAI()->raycast(new_direction, 1.0f, 10.f);
    }
	
	float angleBetweenPowerLakeAndAI = getAI()->getCharToEnemyAngle(getAI()->getPowerLakePosition()).valueDegrees();
	float distanceFromPowerLake = (getAI()->getPowerLakePosition() - getAI()->getSimPos()).length();

	size_t steps = 20;
	getAI()->startSmoothChangeDir(new_direction, steps, duration);
	getAI()->setSpeed(1.0f);

	if(angleBetweenPowerLakeAndAI<10 || (distanceFromPowerLake<15 && angleBetweenPowerLakeAndAI<40)) {
		getAI()->stopSmoothChangeDir();	
	}

    return true;
}

bool MyBestHTNActorController::actionStayInPowerLake(float duration)
{
	m_isStayingInPowerLake = true;
	m_planner->setStateBool("IsStayingInPowerLake", true);
	m_isGoingToPowerLake = false;

    getAI()->setSpeed(0.f);

	mkVec3 direction = (m_enemyLastPos-getAI()->getSimPos()).normalisedCopy();
	float angle = getAI()->getCharToEnemyAngle(direction).valueDegrees();
	if(angle > 30) {
		size_t steps = 20;
		getAI()->startSmoothChangeDir(direction, steps, duration);
	}
	
    return true;
}

bool MyBestHTNActorController::actionTakeBuff(float duration)
{
	if(m_isGoingToPowerLake)
		return false;
	if(m_isPowerLakeReached && getAI()->getHealth()<getAI()->getMaxHealth()/2)
		return false;

	if(!m_isBuffAvailable) {
		return false;
	}
	
	mkVec3 new_direction = (getAI()->getBuffPosition() - getAI()->getSimPos()).normalisedCopy();
    RayCastResult ray_result = getAI()->raycast(new_direction, 1.0f, 10.f);
    while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
		new_direction = getRandomHorizontalDir();
		ray_result = getAI()->raycast(new_direction, 1.0f, 10.f);
    } 

	float angleBetweenBuffAndAI = getAI()->getCharToEnemyAngle(getAI()->getBuffPosition()).valueDegrees();
	float distanceFromBuff = (getAI()->getBuffPosition() - getAI()->getSimPos()).length();

	size_t steps = 20;
	getAI()->startSmoothChangeDir(new_direction, steps, duration);
	if(angleBetweenBuffAndAI < 5.f || (distanceFromBuff < 5 && angleBetweenBuffAndAI < 20)) {
		getAI()->stopSmoothChangeDir();
		getAI()->setSpeed(1.0f);
	}
	else {
		getAI()->setSpeed(0.f);
	}
    return true;
}

bool MyBestHTNActorController::actionDestroyBarrels(float duration) {
	if(getAI()->getBarrels().size()>0) {
		animAttackFireball(duration);
		for(int i=0; i<getAI()->getBarrels().size(); i++) {
			if( (getAI()->getBarrels()[i]->getWorldPosition()-getAI()->getSimPos()).length() <= getAI()->getShootingRange() ) {
				getAI()->hitFireball(getAI()->getBarrels()[i]->getWorldPosition());
				return true;
			}
		}
	}
	return false;
}

bool MyBestHTNActorController::actionFindAttacker( float duration )
{
	if(m_isGoingToPowerLake || m_target) {
		return false;
	}
	m_isLookingForEnemy = true;
	
    size_t steps = 20;
	mkVec3 new_direction = -getAI()->getSimDir();
	new_direction.normalise();
	getAI()->startSmoothChangeDir(new_direction, steps, duration);
	getAI()->setSpeed(0.0f);

    return true;
}

bool MyBestHTNActorController::actionGoToEnemyLastSpot( float duration )
{
	if(m_enemyLastPos==mkVec3::ZERO)
		return false;

	if( (getAI()->getSimPos() - m_enemyLastPos).length()<3 ) {
		m_isLookingForEnemy = false;
		m_planner->setStateBool("IsEnemySeen",false);
		return false;
	}

	if(getAI()->getHealth() <= getAI()->getMaxHealth()/2) {
		m_isLookingForEnemy = false;
		m_isGoingToPowerLake = true;
	}

	float angleBetweenAIAndEnemyLastSpot = getAI()->getCharToEnemyAngle(m_enemyLastPos).valueDegrees();

	m_isLookingForEnemy = true;
	if(angleBetweenAIAndEnemyLastSpot < 5.f) {
		getAI()->setSpeed(1.f);
	}
	else {
		size_t steps = 20;
		mkVec3 new_direction = (m_enemyLastPos - getAI()->getSimPos()).normalisedCopy();

		RayCastResult ray_result = getAI()->raycast(new_direction, 1.0f, 5.f);
		if(ray_result.hit && ray_result.collision_type == RayCastResult::Environment) {
			m_isLookingForEnemy = false;
			m_planner->setStateBool("IsEnemySeen",false);
			return false;
		}

		getAI()->startSmoothChangeDir(new_direction, steps, duration);
		getAI()->setSpeed(0.f);
	}

    return true;
}

bool MyBestHTNActorController::actionTakeMedkit( float duration ) {
	if(!getAI()->isMedkitAvailable() || getAI()->getHealth()==getAI()->getMaxHealth())
		return false;

	size_t steps = 20;
	mkVec3 new_direction = (getAI()->getMedkitPosition() - getAI()->getSimPos()).normalisedCopy();
	getAI()->startSmoothChangeDir(new_direction, steps, duration);
	getAI()->setSpeed(1.f);

    return true;
}

bool MyBestHTNActorController::animAttackMelee( float duration )
{
    if(!m_target || !m_canAttackEnemy)
        return false;

    getAI()->runAnimation("Attack3",duration);
    getAI()->setSpeed(0.f);

    return true;
}

bool MyBestHTNActorController::animAttackFireball( float duration )
{
    if(!m_target)
        return false;
    getAI()->runAnimation("Attack1",duration);
    getAI()->setSpeed(0.f);

    return true;
}

bool MyBestHTNActorController::actionRest( float duration ) {
	if(m_isStayingInPowerLake && getAI()->getHealth()==getAI()->getMaxHealth()) {
		m_isStayingInPowerLake = false;
	}
	return true; 
}