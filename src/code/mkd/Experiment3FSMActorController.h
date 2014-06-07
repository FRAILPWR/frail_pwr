#pragma once
#include "StateMachineActorController.h"

class Experiment3FSMActorController : public StateMachineActorController
{
public:
    explicit Experiment3FSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate(float dt);

    bool getGotHit() const { return m_gotHit; }
    void setGotHit(bool val) { m_gotHit = val; }
    float getJumpTime() const { return m_jumpTime; }
    void setJumpTime(float val) { m_jumpTime = val; }
private:
    bool m_gotHit;
    float m_jumpTime;
};

namespace experiment3_sm
{
    class BaseState : public sm::State
    {
    public:
        explicit BaseState(Experiment3FSMActorController* controller);

        Experiment3FSMActorController* getController() const;
    };

    class IdleState : public BaseState
    {
    public:
        IdleState(Experiment3FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class JumpState : public BaseState
    {
    public:
        JumpState(Experiment3FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class BackflipState : public BaseState
    {
    public:
        BackflipState(Experiment3FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class AttackState : public BaseState
    {
    public:
        AttackState(Experiment3FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };
}
