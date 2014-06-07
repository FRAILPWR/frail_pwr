#pragma once
#include "StateMachineActorController.h"

class Experiment2FSMActorController : public StateMachineActorController
{
public:
    explicit Experiment2FSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();

    mkVec3 getMinePosition() const { return m_minePosition; }
    mkVec3 getShopPosition() const { return m_shopPosition; }

    bool hasPickaxe() const { return m_pickaxe; }
    void setPickaxe(bool val) { m_pickaxe = val; }

    bool hasHelmet() const { return m_helmet; }
    void setHelmet(bool val) { m_helmet = val; }

    bool hasLantern() const { return m_lantern; }
    void setLantern(bool val) { m_lantern = val; }

    float getNpcGold() const { return m_npcGold; }
    void setNpcGold(float val) { m_npcGold = val; }
private:
    mkVec3 m_shopPosition;
    mkVec3 m_minePosition;
    bool m_pickaxe;
    bool m_helmet;
    bool m_lantern;
    float m_npcGold;
};

namespace experiment2_sm
{
    class BaseState : public sm::State
    {
    public:
        explicit BaseState(Experiment2FSMActorController* controller);

        Experiment2FSMActorController* getController() const;
    };

    class IdleState : public BaseState
    {
    public:
        IdleState(Experiment2FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class GotoMine : public BaseState
    {
    public:
        GotoMine(Experiment2FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class GotoShop : public BaseState
    {
    public:
        GotoShop(Experiment2FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class DigGold : public BaseState
    {
    public:
        DigGold(Experiment2FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class BuyPickaxe : public BaseState
    {
    public:
        BuyPickaxe(Experiment2FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class BuyHelmet : public BaseState
    {
    public:
        BuyHelmet(Experiment2FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class BuyLantern : public BaseState
    {
    public:
        BuyLantern(Experiment2FSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };
}
