#pragma once
#include "StateMachineActorController.h"

class BossFSMActorController : public StateMachineActorController
{
public:
    explicit BossFSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate(float dt);

    void rotateToEnemy();

    bool getGotHit() const { return m_gotHit; }
    void setGotHit(bool val) { m_gotHit = val; }
    mkVec3 getAttackDir() const { return m_attackDir; }
    void setAttackDir(mkVec3 val) { m_attackDir = val; }
    mkVec3 getEnemyLastPos() const { return m_enemyLastPos; }
    void setEnemyLastPos(mkVec3 val) { m_enemyLastPos = val; }
    Character* getTarget() const { return m_target; }
    void setTarget(Character* val) { m_target = val; }
    bool getEnemySeen() const { return m_enemySeen; }
    void setEnemySeen(bool val) { m_enemySeen = val; }
    float getAngleDiff() const { return m_angleDiff; }
    void setAngleDiff(float val) { m_angleDiff = val; }
    bool getAngerMode() const { return m_angerMode; }
    void setAngerMode(bool val) { m_angerMode = val; }
    float getEnemyDistance() const { return m_enemyDistance; }
    void setEnemyDistance(float val) { m_enemyDistance = val; }
    bool getEnemyRunningAway() const { return m_enemyRunningAway; }
    void setEnemyRunningAway(bool val) { m_enemyRunningAway = val; }
private:
    void updateWorldState();
    bool m_gotHit;
    mkVec3 m_attackDir;
    mkVec3 m_enemyLastPos;
    bool m_enemySeen;
    Character* m_target;
    float m_angleDiff;
    bool m_angerMode;
    float m_enemyDistance;
    bool m_enemyRunningAway;
    float m_prevEnemyDist;
    float m_prevDistSum;
    float m_prevEnemyDistTime;
};

namespace boss_sm
{
    class BaseState : public sm::State
    {
    public:
        explicit BaseState(BossFSMActorController* controller);

        BossFSMActorController* getController() const;
    };

    class PatrolState : public BaseState
    {
    public:
        PatrolState(BossFSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave(State* next_state);

    private:
        float m_stateStartTime;
    };

    class RevealAttacker : public BaseState
    {
    public:
        RevealAttacker(BossFSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);

    private:
        float m_stateStartTime;
    };

    class ExploreSpot : public BaseState
    {
    public:
        ExploreSpot(BossFSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);

    private:
        float m_stateStartTime;
    };

    class ReduceDistance : public BaseState
    {
    public:
        ReduceDistance(BossFSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave(State* next_state);

    private:
        float m_stateStartTime;
    };

    class AttackFireball : public BaseState
    {
    public:
        AttackFireball(BossFSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave(State* next_state);

    private:
        float m_stateStartTime;
    };

    class AttackAngerMode : public BaseState
    {
    public:
        AttackAngerMode(BossFSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave(State* next_state);

    private:
        float m_stateStartTime;
    };

    class AttackMelee : public BaseState
    {
    public:
        AttackMelee(BossFSMActorController* controller);

        virtual void onUpdate(float dt);
        virtual void onEnter(State* prev_state);
        virtual void onLeave(State* next_state);

    private:
        float m_stateStartTime;
    };
}
