#include "pch.h"
#include "Game.h"
#include "Experiment2FSMActorController.h"
#include "contrib/DebugDrawer.h"

#define SPOT_RADIUS 5.f

////////////////////////////////////////////////////////////////////////////////////////////////////
Experiment2FSMActorController::Experiment2FSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
{
    m_shopPosition = mkVec3(33.f, 0.1f, 2.f);
    m_minePosition = mkVec3(-33.f,0.1f,2.f);
    m_pickaxe = false;
    m_helmet = false;
    m_lantern = false;
    m_npcGold = 0.f;
}

void Experiment2FSMActorController::onCreate()
{
    getAI()->setSpeed(0.f);
    getAI()->lookAt(mkVec3::ZERO);
    scheduleTransitionInNextFrame(new experiment2_sm::GotoMine(this));
    updateStateTransition();
}

void Experiment2FSMActorController::onDebugDraw()
{
    __super::onDebugDraw();
    DebugDrawer::getSingleton().drawCircle(m_shopPosition, SPOT_RADIUS, 30, Ogre::ColourValue(0.f,1.f,0.2f,0.5f), true);
    DebugDrawer::getSingleton().drawCircle(m_minePosition, SPOT_RADIUS, 30, Ogre::ColourValue(1.f,0.9f,0.f,0.5f), true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

experiment2_sm::BaseState::BaseState( Experiment2FSMActorController* controller )
    : sm::State(controller)
{

}

Experiment2FSMActorController* experiment2_sm::BaseState::getController() const
{
    return static_cast<Experiment2FSMActorController*>(sm::State::getController());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
experiment2_sm::IdleState::IdleState(Experiment2FSMActorController* controller)
    : experiment2_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment2_sm::IdleState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
}

void experiment2_sm::IdleState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (g_game->getTimeMs() - m_stateStartTime > 100)
    {
        experiment2_sm::IdleState* next_state = new experiment2_sm::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment2_sm::IdleState::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment2_sm::GotoMine::GotoMine(Experiment2FSMActorController* controller)
    : experiment2_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment2_sm::GotoMine::onEnter(State* prev_state)
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

void experiment2_sm::GotoMine::onUpdate(float dt)
{
    __super::onUpdate(dt);
    if((getAI()->getSimPos()-getController()->getMinePosition()).length() <= SPOT_RADIUS){
        experiment2_sm::DigGold* next_state = new experiment2_sm::DigGold(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment2_sm::GotoMine* next_state = new experiment2_sm::GotoMine(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment2_sm::GotoMine::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment2_sm::GotoShop::GotoShop(Experiment2FSMActorController* controller)
    : experiment2_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment2_sm::GotoShop::onEnter(State* prev_state)
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

void experiment2_sm::GotoShop::onUpdate(float dt)
{
    __super::onUpdate(dt);
    if((getAI()->getSimPos()-getController()->getShopPosition()).length() <= SPOT_RADIUS){
        if(!getController()->hasPickaxe()){
            experiment2_sm::BuyPickaxe* next_state = new experiment2_sm::BuyPickaxe(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else if(!getController()->hasHelmet() && getController()->hasPickaxe()){
            experiment2_sm::BuyHelmet* next_state = new experiment2_sm::BuyHelmet(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
        else if(!getController()->hasLantern() && getController()->hasHelmet() && getController()->hasPickaxe()){
            experiment2_sm::BuyLantern* next_state = new experiment2_sm::BuyLantern(getController());
            getController()->scheduleTransitionInNextFrame(next_state);
        }
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment2_sm::GotoShop* next_state = new experiment2_sm::GotoShop(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment2_sm::GotoShop::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment2_sm::DigGold::DigGold(Experiment2FSMActorController* controller)
    : experiment2_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment2_sm::DigGold::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);

    float mult = 1.f;
    mult += getController()->hasPickaxe() == true ? 1.f : 0.f;
    mult += getController()->hasHelmet() == true ? 1.f : 0.f;
    mult += getController()->hasLantern() == true ? 1.f : 0.f;

    float gold = getController()->getNpcGold();
    gold += 10.f * mult;
    getController()->setNpcGold(gold);

    std::stringstream ss;
    ss << getController()->getNpcGold();
    Ogre::LogManager::getSingleton().logMessage(ss.str());
    if(!getController()->hasPickaxe()){
        getAI()->runAnimation("Attack1",1000);
    } else {
        getAI()->runAnimation("Attack3",1000);
    }
}

void experiment2_sm::DigGold::onUpdate(float dt)
{
    __super::onUpdate(dt);
    if(getController()->getNpcGold() >= 200 && !getController()->hasPickaxe())
    {
        experiment2_sm::GotoShop* next_state = new experiment2_sm::GotoShop(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getNpcGold() >= 400 && !getController()->hasHelmet())
    {
        experiment2_sm::GotoShop* next_state = new experiment2_sm::GotoShop(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getNpcGold() >= 600 && !getController()->hasLantern())
    {
        experiment2_sm::GotoShop* next_state = new experiment2_sm::GotoShop(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if(getController()->getNpcGold() >= 1000)
    {
        experiment2_sm::IdleState* next_state = new experiment2_sm::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 1000)
    {
        experiment2_sm::DigGold* next_state = new experiment2_sm::DigGold(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment2_sm::DigGold::onLeave( State* next_state )
{
}
//////////////////////////////////////////////////////////////////////////

experiment2_sm::BuyPickaxe::BuyPickaxe(Experiment2FSMActorController* controller)
    : experiment2_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment2_sm::BuyPickaxe::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();

    getAI()->setSpeed(0.f);
    getController()->setPickaxe(true);
    float gold = getController()->getNpcGold() - 200.f;
    getController()->setNpcGold(gold);
}

void experiment2_sm::BuyPickaxe::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(getController()->hasPickaxe())
    {
        experiment2_sm::GotoMine* next_state = new experiment2_sm::GotoMine(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment2_sm::BuyPickaxe* next_state = new experiment2_sm::BuyPickaxe(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment2_sm::BuyPickaxe::onLeave( State* next_state )
{
}

//////////////////////////////////////////////////////////////////////////

experiment2_sm::BuyHelmet::BuyHelmet(Experiment2FSMActorController* controller)
    : experiment2_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment2_sm::BuyHelmet::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();

    getAI()->setSpeed(0.f);
    getController()->setHelmet(true);
    float gold = getController()->getNpcGold() - 400.f;
    getController()->setNpcGold(gold);
}

void experiment2_sm::BuyHelmet::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(getController()->hasHelmet())
    {
        experiment2_sm::GotoMine* next_state = new experiment2_sm::GotoMine(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment2_sm::BuyHelmet* next_state = new experiment2_sm::BuyHelmet(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment2_sm::BuyHelmet::onLeave( State* next_state )
{
}

//////////////////////////////////////////////////////////////////////////

experiment2_sm::BuyLantern::BuyLantern(Experiment2FSMActorController* controller)
    : experiment2_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void experiment2_sm::BuyLantern::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();

    getAI()->setSpeed(0.f);
    getController()->setLantern(true);
    float gold = getController()->getNpcGold() - 600.f;
    getController()->setNpcGold(gold);
}

void experiment2_sm::BuyLantern::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if(getController()->hasLantern())
    {
        experiment2_sm::GotoMine* next_state = new experiment2_sm::GotoMine(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else if (g_game->getTimeMs() - m_stateStartTime > 500)
    {
        experiment2_sm::BuyLantern* next_state = new experiment2_sm::BuyLantern(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void experiment2_sm::BuyLantern::onLeave( State* next_state )
{
}