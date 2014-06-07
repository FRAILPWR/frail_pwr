#include "pch.h"
#include "Game.h"
#include "Experiment3FSMActorController.h"
#include "contrib/DebugDrawer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
Experiment3FSMActorController::Experiment3FSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
{
    m_gotHit = false;
    m_jumpTime = 0.f;
}

void Experiment3FSMActorController::onCreate()
{
    getAI()->setSpeed(0.f);
    getAI()->lookAt(mkVec3::ZERO);
    scheduleTransitionInNextFrame(new experiment3_sm::IdleState(this));
    updateStateTransition();
}

void Experiment3FSMActorController::onDebugDraw()
{
    __super::onDebugDraw();
}

void Experiment3FSMActorController::onTakeDamage(const SDamageInfo& dmg_info){
    __super::onTakeDamage(dmg_info);
    m_gotHit = true;
}

void Experiment3FSMActorController::onUpdate( float dt )
{
    __super::onUpdate(dt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

experiment3_sm::BaseState::BaseState( Experiment3FSMActorController* controller )
    : sm::State(controller)
{

}

Experiment3FSMActorController* experiment3_sm::BaseState::getController() const
{
    return static_cast<Experiment3FSMActorController*>(sm::State::getController());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
experiment3_sm::IdleState::IdleState(Experiment3FSMActorController* controller)
    : experiment3_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment3_sm::IdleState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);

    mkVec3 newDir = getRandomHorizontalDir();
    newDir.normalise();
    getAI()->startSmoothChangeDir(newDir, 40, 1000);
}

void experiment3_sm::IdleState::onUpdate(float dt)
{
    __super::onUpdate(dt);
    
    Character* target = getAI()->findClosestEnemyInSight();
    float lngt = 0.f;
    if(target)
        lngt = (getAI()->getSimPos()-target->getSimPos()).length();

    if(g_game->getTimeMs() - getController()->getJumpTime() > 5000.f)
    {
        experiment3_sm::JumpState* next_state = new experiment3_sm::JumpState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getGotHit())
    {
        experiment3_sm::BackflipState* next_state = new experiment3_sm::BackflipState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(target && lngt < getAI()->getMeleeRange())
    {
        experiment3_sm::AttackState* next_state = new experiment3_sm::AttackState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 1000.f)
    {
        experiment3_sm::IdleState* next_state = new experiment3_sm::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment3_sm::IdleState::onLeave( State* next_state )
{

}
//////////////////////////////////////////////////////////////////////////
experiment3_sm::JumpState::JumpState(Experiment3FSMActorController* controller)
    : experiment3_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment3_sm::JumpState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
    getAI()->jump();
    getController()->setJumpTime(g_game->getTimeMs());
}

void experiment3_sm::JumpState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 1000.f)
    {
        Character* target = getAI()->findClosestEnemyInSight();
        float lngt = 0.f;
        if(target)
            lngt = (getAI()->getSimPos()-target->getSimPos()).length();

        if(getController()->getGotHit())
        {
            experiment3_sm::BackflipState* next_state = new experiment3_sm::BackflipState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else if(target && lngt < getAI()->getMeleeRange())
        {
            experiment3_sm::AttackState* next_state = new experiment3_sm::AttackState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else
        {
            experiment3_sm::IdleState* next_state = new experiment3_sm::IdleState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
    }
}

void experiment3_sm::JumpState::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////
experiment3_sm::BackflipState::BackflipState(Experiment3FSMActorController* controller)
    : experiment3_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment3_sm::BackflipState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
    getAI()->runAnimation("Backflip",1000);
    getController()->setGotHit(false);
}

void experiment3_sm::BackflipState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 1000.f)
    {
        Character* target = getAI()->findClosestEnemyInSight();
        float lngt = 0.f;
        if(target)
            lngt = (getAI()->getSimPos()-target->getSimPos()).length();

        if(g_game->getTimeMs() - getController()->getJumpTime() > 5000.f)
        {
            experiment3_sm::JumpState* next_state = new experiment3_sm::JumpState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else if(target && lngt < getAI()->getMeleeRange())
        {
            experiment3_sm::AttackState* next_state = new experiment3_sm::AttackState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else
        {
            experiment3_sm::IdleState* next_state = new experiment3_sm::IdleState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
    }
}

void experiment3_sm::BackflipState::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////
experiment3_sm::AttackState::AttackState(Experiment3FSMActorController* controller)
    : experiment3_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment3_sm::AttackState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
    getAI()->runAnimation("Attack3",1500);
    getAI()->hitMelee();
}

void experiment3_sm::AttackState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 1500.f)
    {
        Character* target = getAI()->findClosestEnemyInSight();
        float lngt = 0.f;
        if(target)
            lngt = (getAI()->getSimPos()-target->getSimPos()).length();

        if(g_game->getTimeMs() - getController()->getJumpTime() > 5000.f)
        {
            experiment3_sm::JumpState* next_state = new experiment3_sm::JumpState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else if(getController()->getGotHit())
        {
            experiment3_sm::BackflipState* next_state = new experiment3_sm::BackflipState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else if (!target || lngt >= getAI()->getMeleeRange()){
            experiment3_sm::IdleState* next_state = new experiment3_sm::IdleState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else
        {
            experiment3_sm::AttackState* next_state = new experiment3_sm::AttackState(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
    }
}

void experiment3_sm::AttackState::onLeave( State* next_state )
{
}