#include "pch.h"
#include "Game.h"
#include "BossFSMActorController.h"
#include "contrib/DebugDrawer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
BossFSMActorController::BossFSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
{
    m_gotHit = false;
    m_attackDir = mkVec3::ZERO;
    m_enemyLastPos = mkVec3::ZERO;
    m_enemySeen = false;
    m_angerMode = false;
    m_enemyRunningAway = false;
    m_prevEnemyDist = 1000.f;
    m_prevEnemyDistTime = g_game->getTimeMs();
    m_prevDistSum = 0.f;
}

void BossFSMActorController::onCreate()
{
    getAI()->lookAt(mkVec3::ZERO);
    scheduleTransitionInNextFrame(new boss_sm::PatrolState(this));
    updateStateTransition();
}

void BossFSMActorController::onDebugDraw()
{
    __super::onDebugDraw();
    if(m_enemyLastPos != mkVec3::ZERO)
        DebugDrawer::getSingleton().drawCircle(m_enemyLastPos, 0.2f, 30, Ogre::ColourValue::Black, true);
}

void BossFSMActorController::onTakeDamage(const SDamageInfo& dmg_info){
    __super::onTakeDamage(dmg_info);
    m_gotHit = true;
    m_attackDir = mkVec3::ZERO-dmg_info.dir;
}

void BossFSMActorController::onUpdate( float dt )
{
    updateWorldState();
    __super::onUpdate(dt);
}

void BossFSMActorController::updateWorldState()
{
    m_target = getAI()->findClosestEnemyInSight();
    if(m_target)
    {
        m_enemyLastPos = m_target->getSimPos();
        m_enemySeen = true;
        m_angleDiff = (float)(getAI()->getCharToEnemyAngle(m_target->getSimPos()).valueDegrees());
        m_enemyDistance = (float)(m_target->getSimPos() - getAI()->getSimPos()).length();

        m_prevDistSum += m_enemyDistance - m_prevEnemyDist;
        m_prevEnemyDist = m_enemyDistance;

        if(g_game->getTimeMs()-m_prevEnemyDistTime > 500.f){
            if(m_prevDistSum > 1.f)
                m_enemyRunningAway = true;
            else
                m_enemyRunningAway = false;
            m_prevDistSum = 0.f;
            m_prevEnemyDistTime = g_game->getTimeMs();
        }
    }
}

void BossFSMActorController::rotateToEnemy()
{
    if(m_target)
    {
        size_t steps = 40;
        mkVec3 new_direction = m_target->getSimPos() - getAI()->getSimPos();
        new_direction.normalise();
        getAI()->startSmoothChangeDir(new_direction, steps, 500.f);
        getAI()->setSpeed(0.f);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

boss_sm::BaseState::BaseState( BossFSMActorController* controller )
    : sm::State(controller)
{

}

BossFSMActorController* boss_sm::BaseState::getController() const
{
    return static_cast<BossFSMActorController*>(sm::State::getController());
}
//////////////////////////////////////////////////////////////////////////

boss_sm::PatrolState::PatrolState(BossFSMActorController* controller)
    : boss_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::PatrolState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    //////////////////////////////////////////////////////////////////////////
    mkVec3 new_direction = getRandomHorizontalDir();
    RayCastResult ray_result = getAI()->raycast(new_direction, 1.0f, 5.f);
    while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
        new_direction = getRandomHorizontalDir();
        ray_result = getAI()->raycast(new_direction, 1.0f, 5.f);
    }

    size_t steps = 40;
    getAI()->startSmoothChangeDir(new_direction, steps, 1500);
    getAI()->setSpeed(0.5f);
}

void boss_sm::PatrolState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(getController()->getTarget())
    {
        boss_sm::ReduceDistance* next_state = new boss_sm::ReduceDistance(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getGotHit())
    {
        boss_sm::RevealAttacker* next_state = new boss_sm::RevealAttacker(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getEnemySeen())
    {
        boss_sm::ExploreSpot* next_state = new boss_sm::ExploreSpot(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 3000)
    {
        boss_sm::PatrolState* next_state = new boss_sm::PatrolState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void boss_sm::PatrolState::onLeave( State* next_state )
{
    getAI()->stopSmoothChangeDir();
}

//////////////////////////////////////////////////////////////////////////

boss_sm::RevealAttacker::RevealAttacker(BossFSMActorController* controller)
    : boss_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::RevealAttacker::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    //////////////////////////////////////////////////////////////////////////
    getController()->setGotHit(false);
    getAI()->setSpeed(0.3f);
    size_t steps = 40;
    getAI()->startSmoothChangeDir(getController()->getAttackDir(), steps, 1500);
}

void boss_sm::RevealAttacker::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(getController()->getTarget())
    {
        boss_sm::ReduceDistance* next_state = new boss_sm::ReduceDistance(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 2000)
    {
        boss_sm::PatrolState* next_state = new boss_sm::PatrolState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

//////////////////////////////////////////////////////////////////////////

boss_sm::ExploreSpot::ExploreSpot(BossFSMActorController* controller)
    : boss_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::ExploreSpot::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    //////////////////////////////////////////////////////////////////////////
    if(getController()->getEnemyLastPos() != mkVec3::ZERO){
        getController()->setEnemySeen(false);
        size_t steps = 40;
        mkVec3 new_direction = getController()->getEnemyLastPos() - getAI()->getSimPos();
        new_direction.normalise();
        getAI()->startSmoothChangeDir(new_direction, steps, 250);
        getAI()->setSpeed(1.f);
    }
}

void boss_sm::ExploreSpot::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(getController()->getTarget())
    {
        boss_sm::ReduceDistance* next_state = new boss_sm::ReduceDistance(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getGotHit())
    {
        boss_sm::RevealAttacker* next_state = new boss_sm::RevealAttacker(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getEnemyLastPos() != mkVec3::ZERO){
        if((getAI()->getSimPos() - getController()->getEnemyLastPos()).length() < 3.f){
            boss_sm::PatrolState* next_state = new boss_sm::PatrolState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

boss_sm::ReduceDistance::ReduceDistance(BossFSMActorController* controller)
    : boss_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::ReduceDistance::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    //////////////////////////////////////////////////////////////////////////
    Character* target = getController()->getTarget();
    if(target)
    {
        RayCastResult ray_result = getAI()->raycast(target->getSimPos(), 0.1f, 1.f);
        if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
            getAI()->jump();

        mkVec3 destDir = (target->getSimPos()-getAI()->getSimPos()).normalisedCopy();
        size_t steps = 40;
        getAI()->startSmoothChangeDir(destDir, steps, 300);
        getAI()->setSpeed(1.f);
    }
}

void boss_sm::ReduceDistance::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(!getController()->getTarget()){
        boss_sm::PatrolState* next_state = new boss_sm::PatrolState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getAI()->getHealth() <= getAI()->getMaxHealth()/2 && !getController()->getAngerMode() && getController()->getEnemyDistance() < getAI()->getMeleeRange())
    {
        boss_sm::AttackAngerMode* next_state = new boss_sm::AttackAngerMode(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getEnemyDistance() < getAI()->getMeleeRange())
    {
        boss_sm::AttackMelee* next_state = new boss_sm::AttackMelee(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getEnemyRunningAway() && getController()->getEnemyDistance() >= 10.f && getController()->getEnemyDistance() <= getAI()->getShootingRange())
    {
        boss_sm::AttackFireball* next_state = new boss_sm::AttackFireball(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 300)
    {
        boss_sm::ReduceDistance* next_state = new boss_sm::ReduceDistance(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void boss_sm::ReduceDistance::onLeave( State* next_state )
{
    getAI()->stopSmoothChangeDir();
}

//////////////////////////////////////////////////////////////////////////

boss_sm::AttackFireball::AttackFireball(BossFSMActorController* controller)
    : boss_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::AttackFireball::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    //////////////////////////////////////////////////////////////////////////
    Character* target = getController()->getTarget();
    if(target)
    {
        getController()->rotateToEnemy();
        getAI()->runAnimation("Attack1",600.f);
        getAI()->setSpeed(0.f);
        getController()->setEnemyRunningAway(false);
    }
}

void boss_sm::AttackFireball::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 600.f)
    {
        boss_sm::ReduceDistance* next_state = new boss_sm::ReduceDistance(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void boss_sm::AttackFireball::onLeave(State* next_state)
{
    Character* target = getController()->getTarget();
    if(target)
    {
        getAI()->hitFireball(target->getSimPos());
    }
}

//////////////////////////////////////////////////////////////////////////

boss_sm::AttackAngerMode::AttackAngerMode(BossFSMActorController* controller)
    : boss_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::AttackAngerMode::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    //////////////////////////////////////////////////////////////////////////
    getAI()->setSpeed(0.f);
    Character* target = getController()->getTarget();
    if(target)
    {
        getController()->rotateToEnemy();
        getAI()->runAnimation("HighJump",1000.f);
        getController()->setAngerMode(true);
    }
}

void boss_sm::AttackAngerMode::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 1000.f)
    {
        boss_sm::ReduceDistance* next_state = new boss_sm::ReduceDistance(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void boss_sm::AttackAngerMode::onLeave(State* next_state)
{
    getAI()->hitAngerMode();
}

//////////////////////////////////////////////////////////////////////////

boss_sm::AttackMelee::AttackMelee(BossFSMActorController* controller)
    : boss_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::AttackMelee::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    //////////////////////////////////////////////////////////////////////////
    getAI()->setSpeed(0.f);
    Character* target = getController()->getTarget();
    if(target)
    {
        getController()->rotateToEnemy();
        getAI()->runAnimation("Attack3",1100.f);
        getController()->setAngerMode(true);
    }
}

void boss_sm::AttackMelee::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 1100.f)
    {
        boss_sm::ReduceDistance* next_state = new boss_sm::ReduceDistance(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void boss_sm::AttackMelee::onLeave(State* next_state)
{
    getAI()->hitMelee();
}