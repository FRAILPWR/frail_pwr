#pragma once
#include "StateMachineActorController.h"

class Experiment1FSMActorController : public StateMachineActorController
{
public:
    explicit Experiment1FSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();

    mkVec3 getMinePosition() const { return m_minePosition; }
    mkVec3 getShopPosition() const { return m_shopPosition; }

    bool hasPickaxe() const { return m_pickaxe; }
    void setPickaxe(bool val) { m_pickaxe = val; }

    float getNpcGold() const { return m_npcGold; }
    void setNpcGold(float val) { m_npcGold = val; }
private:
    mkVec3 m_shopPosition;
    mkVec3 m_minePosition;
    bool m_pickaxe;
    float m_npcGold;
};

namespace experiment1_sm
{
    class BaseState : public sm::State
    {
    public:
        explicit BaseState(Experiment1FSMActorController* controller);

        Experiment1FSMActorController* getController() const;
    };

    class IdleState : public BaseState
    {
    public:
        IdleState(Experiment1FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class GotoMine : public BaseState
    {
    public:
        GotoMine(Experiment1FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class GotoShop : public BaseState
    {
    public:
        GotoShop(Experiment1FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class DigGold : public BaseState
    {
    public:
        DigGold(Experiment1FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class BuyPickaxe : public BaseState
    {
    public:
        BuyPickaxe(Experiment1FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };
}
