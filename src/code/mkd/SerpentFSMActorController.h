#pragma once
#include "StateMachineActorController.h"

class SerpentFSMActorController : public StateMachineActorController
{
public:
    explicit SerpentFSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();
};

namespace serpent
{
    class BaseState : public sm::State
    {
    public:
        explicit BaseState(SerpentFSMActorController* controller);
		const Character* findEnemyInSight() const;

        SerpentFSMActorController* getController() const;
    };

    class IdleState : public BaseState
    {
    public:
        IdleState(SerpentFSMActorController* controller);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

        virtual void onTakeDamage();

        virtual void onDebugDraw();

        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class DistAttackAndPowerLakeState : public BaseState
    {
    public:
        DistAttackAndPowerLakeState(SerpentFSMActorController* controller, const Character* target);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

        virtual void onLeave( State* next_state );

    private:
        const Character* m_target;
        float m_stateStartTime;
    };

	class MeleeState : public BaseState
    {
    public:
        MeleeState(SerpentFSMActorController* controller, const Character* target);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

    private:
        const Character* m_target;
        float m_stateStartTime;
    };

	class HealState : public BaseState
    {
    public:
        HealState(SerpentFSMActorController* controller, const mkVec3& dir);

        void onUpdate(float dt);
        void onEnter(State* prev_state);
		
        virtual void onLeave( State* next_state );

    private:
        mkVec3 m_direction;
        float m_stateStartTime;
    };

	class BuffState : public BaseState
    {
    public:
        BuffState(SerpentFSMActorController* controller, const mkVec3& dir);

        void onUpdate(float dt);
        void onEnter(State* prev_state);
		
        virtual void onLeave( State* next_state );

    private:
        mkVec3 m_direction;
        float m_stateStartTime;
    };
}
