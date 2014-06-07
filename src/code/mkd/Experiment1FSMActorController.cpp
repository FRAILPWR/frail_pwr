#include "pch.h"
#include "Game.h"
#include "Experiment1FSMActorController.h"
#include "contrib/DebugDrawer.h"

#define SPOT_RADIUS 5.f

////////////////////////////////////////////////////////////////////////////////////////////////////
Experiment1FSMActorController::Experiment1FSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
{
    m_shopPosition = mkVec3(33.f, 0.1f, 2.f);
    m_minePosition = mkVec3(-33.f,0.1f,2.f);
    m_pickaxe = false;
    m_npcGold = 0.f;
}

void Experiment1FSMActorController::onCreate()
{
    getAI()->setSpeed(0.f);
    getAI()->lookAt(mkVec3::ZERO);
    scheduleTransitionInNextFrame(new experiment1_sm::GotoMine(this));
    updateStateTransition();
}

void Experiment1FSMActorController::onDebugDraw()
{
    __super::onDebugDraw();
    DebugDrawer::getSingleton().drawCircle(m_shopPosition, SPOT_RADIUS, 30, Ogre::ColourValue(0.f,1.f,0.2f,0.5f), true);
    DebugDrawer::getSingleton().drawCircle(m_minePosition, SPOT_RADIUS, 30, Ogre::ColourValue(1.f,0.9f,0.f,0.5f), true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

experiment1_sm::BaseState::BaseState( Experiment1FSMActorController* controller )
    : sm::State(controller)
{

}

Experiment1FSMActorController* experiment1_sm::BaseState::getController() const
{
    return static_cast<Experiment1FSMActorController*>(sm::State::getController());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
experiment1_sm::IdleState::IdleState(Experiment1FSMActorController* controller)
    : experiment1_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment1_sm::IdleState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
}

void experiment1_sm::IdleState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 100)
    {
        experiment1_sm::IdleState* next_state = new experiment1_sm::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment1_sm::IdleState::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment1_sm::GotoMine::GotoMine(Experiment1FSMActorController* controller)
    : experiment1_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment1_sm::GotoMine::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();

    size_t steps = 40;
    mkVec3 new_direction = getController()->getMinePosition() - getAI()->getSimPos();
    new_direction.normalise();

    getAI()->startSmoothChangeDir(new_direction, steps, 500.f);
    getAI()->setSpeed(0.5f);

    RayCastResult ray_result = getAI()->raycast(new_direction, 0.1f, 0.5f);
    if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
        getAI()->jump();
}

void experiment1_sm::GotoMine::onUpdate(float dt)
{
    __super::onUpdate(dt);
    if((getAI()->getSimPos()-getController()->getMinePosition()).length() <= SPOT_RADIUS){
        experiment1_sm::DigGold* next_state = new experiment1_sm::DigGold(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment1_sm::GotoMine* next_state = new experiment1_sm::GotoMine(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment1_sm::GotoMine::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment1_sm::GotoShop::GotoShop(Experiment1FSMActorController* controller)
    : experiment1_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment1_sm::GotoShop::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();

    size_t steps = 40;
    mkVec3 new_direction = getController()->getShopPosition() - getAI()->getSimPos();
    new_direction.normalise();

    getAI()->startSmoothChangeDir(new_direction, steps, 500.f);
    getAI()->setSpeed(0.5f);

    RayCastResult ray_result = getAI()->raycast(new_direction, 0.1f, 0.5f);
    if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
        getAI()->jump();
}

void experiment1_sm::GotoShop::onUpdate(float dt)
{
    __super::onUpdate(dt);
    if((getAI()->getSimPos()-getController()->getShopPosition()).length() <= SPOT_RADIUS){
        experiment1_sm::BuyPickaxe* next_state = new experiment1_sm::BuyPickaxe(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment1_sm::GotoShop* next_state = new experiment1_sm::GotoShop(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment1_sm::GotoShop::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment1_sm::DigGold::DigGold(Experiment1FSMActorController* controller)
    : experiment1_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment1_sm::DigGold::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);

    float mult = 1.f;
    mult += getController()->hasPickaxe() == true ? 1.f : 0.f;

    float gold = getController()->getNpcGold();
    gold += 10.f * mult;
    getController()->setNpcGold(gold);
    if(!getController()->hasPickaxe()){
        getAI()->runAnimation("Attack1",1000);
    } else {
        getAI()->runAnimation("Attack3",1000);
    }
}

void experiment1_sm::DigGold::onUpdate(float dt)
{
    __super::onUpdate(dt);
    if(getController()->getNpcGold() >= 200 && !getController()->hasPickaxe())
    {
        experiment1_sm::GotoShop* next_state = new experiment1_sm::GotoShop(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getNpcGold() >= 1000)
    {
        experiment1_sm::IdleState* next_state = new experiment1_sm::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 1000)
    {
        experiment1_sm::DigGold* next_state = new experiment1_sm::DigGold(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment1_sm::DigGold::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment1_sm::BuyPickaxe::BuyPickaxe(Experiment1FSMActorController* controller)
    : experiment1_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment1_sm::BuyPickaxe::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();

    getAI()->setSpeed(0.f);
    getController()->setPickaxe(true);
    float gold = getController()->getNpcGold() - 200.f;
    getController()->setNpcGold(gold);
}

void experiment1_sm::BuyPickaxe::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(getController()->hasPickaxe())
    {
        experiment1_sm::GotoMine* next_state = new experiment1_sm::GotoMine(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment1_sm::BuyPickaxe* next_state = new experiment1_sm::BuyPickaxe(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment1_sm::BuyPickaxe::onLeave( State* next_state )
{
}

//////////////////////////////////////////////////////////////////////////