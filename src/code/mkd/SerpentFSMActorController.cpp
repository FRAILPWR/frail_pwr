#include "pch.h"
#include "Game.h"
#include "SerpentFSMActorController.h"
#include "contrib/DebugDrawer.h"
#include "Level.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
SerpentFSMActorController::SerpentFSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
{

}

void SerpentFSMActorController::onCreate()
{
    getAI()->lookAt(mkVec3::ZERO);
    scheduleTransitionInNextFrame(new serpent::IdleState(this));
    updateStateTransition();
}

void SerpentFSMActorController::onDebugDraw()
{
    __super::onDebugDraw();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

serpent::BaseState::BaseState( SerpentFSMActorController* controller )
    : sm::State(controller)
{

}

SerpentFSMActorController* serpent::BaseState::getController() const
{
    return static_cast<SerpentFSMActorController*>(sm::State::getController());
}

const Character* serpent::BaseState::findEnemyInSight() const
{
	const CharacterVec& spotted_actors = getAI()->getSpottedActors();
	for (size_t i = 0; i < spotted_actors.size(); ++i)
	{
		const Character* other_character = spotted_actors[i];
		if (getAI()->isEnemy(other_character) && !other_character->isDead())
			return other_character;
	}

	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
serpent::IdleState::IdleState(SerpentFSMActorController* controller)
    : serpent::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void serpent::IdleState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    //const Character* target = findEnemyInSight();
	Character* target = getAI()->findClosestEnemyInSight();

	if(target){
		if((target->getSimPos() - getAI()->getSimPos()).length() <= getAI()->getMeleeRange()) {
			getController()->scheduleTransitionInNextFrame(new serpent::MeleeState(getController(),target));
		} else {
			getController()->scheduleTransitionInNextFrame(new serpent::DistAttackAndPowerLakeState(getController(),target));
		}
	} else {
		if(getAI()->isBuffAvailable()) {
			const mkVec3 myPositon = getAI()->getSimPos();
			const mkVec3 buffPosition = getAI()->getBuffPosition();
		
			mkVec3 destinationVector = buffPosition - myPositon;
			destinationVector.y = 0;
			destinationVector.normalise();

			getController()->scheduleTransitionInNextFrame(new serpent::BuffState(getController(),destinationVector));
		} else {
			if(getAI()->getHealth() < 100 && getAI()->isMedkitAvailable()) {
				const mkVec3 myPositon = getAI()->getSimPos();
				const mkVec3 medkitPosition = getAI()->getMedkitPosition();
		
				mkVec3 destinationVector = medkitPosition - myPositon;
				destinationVector.y = 0;
				destinationVector.normalise();

				getController()->scheduleTransitionInNextFrame(new serpent::HealState(getController(),destinationVector));
			}
		}
	}

    const float time_to_change_state = 2000;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        serpent::IdleState* next_state = new serpent::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void serpent::IdleState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
}

void serpent::IdleState::onTakeDamage()
{
    getAI()->jump();
}

void serpent::IdleState::onDebugDraw()
{

}

void serpent::IdleState::onLeave( State* next_state )
{
    getAI()->runAnimation("Backflip",800);
}

//////////////////////////////////////////////////////////////////////////

serpent::DistAttackAndPowerLakeState::DistAttackAndPowerLakeState(SerpentFSMActorController* controller, const Character* target)
    : serpent::BaseState(controller)
    , m_target(target)
    , m_stateStartTime(-1.f)
{

}

void serpent::DistAttackAndPowerLakeState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    const float time_to_change_state = 600;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        getAI()->runAnimation("Attack1",650);
		//const Character* target = findEnemyInSight();
		Character* target = getAI()->findClosestEnemyInSight();

		if(target && !target->isInPowerLake()) {
			serpent::DistAttackAndPowerLakeState* next_state = new serpent::DistAttackAndPowerLakeState(getController(),target);
			getController()->scheduleTransitionInNextFrame(next_state);
		} else {
			getController()->scheduleTransitionInNextFrame(new serpent::IdleState(getController()));
		}
    }
}

void serpent::DistAttackAndPowerLakeState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);

    m_stateStartTime = g_game->getTimeMs();
	
	getAI()->hitFireball(m_target->getSimPos());
	
	const mkVec3 myPositon = getAI()->getSimPos();
	const mkVec3 powerLakePosition = getAI()->getPowerLakePosition();
	const mkVec3 buffPosition = getAI()->getBuffPosition();

	if(getAI()->isPositionVisible(powerLakePosition)) {
		mkVec3 destinationVector = powerLakePosition - myPositon;
		destinationVector.y = 0;
		destinationVector.normalise();
		getAI()->setDirection(destinationVector);
	} else {
		mkVec3 destinationVector = buffPosition - myPositon;
		destinationVector.y = 0;
		destinationVector.normalise();
		getAI()->setDirection(destinationVector);
	}

    getAI()->setSpeed(1.f);
}

void serpent::DistAttackAndPowerLakeState::onLeave( State* next_state )
{
	getAI()->resetMovementDir();
    getAI()->setSpeed(0.f);
}
///////////////////////////////////////////////////////////////////////////////////////////////
serpent::MeleeState::MeleeState(SerpentFSMActorController* controller, const Character* target)
    : serpent::BaseState(controller)
    , m_target(target)
    , m_stateStartTime(-1.f)
{

}
void serpent::MeleeState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    const float time_to_change_state = 1200;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        getAI()->runAnimation("Attack3",1200);
        serpent::IdleState* next_state = new serpent::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void serpent::MeleeState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);

    m_stateStartTime = g_game->getTimeMs();

	getAI()->resetDirection();
	getAI()->setSpeed(0.f);
	getAI()->lookAt(m_target->getSimPos());
	getAI()->hitMelee();

}
//////////////////////////////////////////////////////////////////////////////////////////////////
serpent::HealState::HealState(SerpentFSMActorController* controller, const mkVec3& dir)
    : serpent::BaseState(controller)
    , m_direction(dir)
    , m_stateStartTime(-1.f)
{

}

void serpent::HealState::onUpdate(float dt)
{
    __super::onUpdate(dt);

	if(!getAI()->isMedkitAvailable()) {
        serpent::IdleState* next_state = new serpent::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
	}

    const float time_to_change_state = 300;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        serpent::IdleState* next_state = new serpent::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void serpent::HealState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);

    m_stateStartTime = g_game->getTimeMs();

	if(getAI()->isPositionVisible(getAI()->getPowerLakePosition())) {
		getAI()->setDirection(m_direction);
	} else {
		getAI()->setDirection(getRandomHorizontalDir());
	}
    getAI()->setSpeed(1.f);
}

void serpent::HealState::onLeave( State* next_state )
{
	getAI()->resetMovementDir();
    getAI()->setSpeed(1.5f);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
serpent::BuffState::BuffState(SerpentFSMActorController* controller, const mkVec3& dir)
    : serpent::BaseState(controller)
    , m_direction(dir)
    , m_stateStartTime(-1.f)
{

}

void serpent::BuffState::onUpdate(float dt)
{
    __super::onUpdate(dt);

	if(!getAI()->isBuffAvailable()) {
        serpent::IdleState* next_state = new serpent::IdleState(getController());
	}

	Character* target = getAI()->findClosestEnemyInSight();
	if(target) {
        serpent::IdleState* next_state = new serpent::IdleState(getController());
	}

    const float time_to_change_state = 500;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        serpent::IdleState* next_state = new serpent::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void serpent::BuffState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);

    m_stateStartTime = g_game->getTimeMs();

	if(getAI()->isPositionVisible(m_direction)) {
		getAI()->setDirection(m_direction);
	} else {
		getAI()->setDirection(getRandomHorizontalDir());
	}
    getAI()->setSpeed(1.f);
}

void serpent::BuffState::onLeave( State* next_state )
{
	getAI()->resetMovementDir();
    getAI()->setSpeed(0.f);
}