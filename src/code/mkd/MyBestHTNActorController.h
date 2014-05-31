#pragma once
#include "IActorController.h"
#include "Player.h"
#include "utils.h"
#include "HTN\Planner.h"

class MyBestHTNActorController : public IActorController
{
public:
    typedef bool (MyBestHTNActorController::*ctrlrAction)(float);

	explicit MyBestHTNActorController(ActorAI* ai);
	~MyBestHTNActorController();

	virtual void onCreate();
	virtual void onTakeDamage(const SDamageInfo& dmg_info);
	virtual void onUpdate(float dt);
	virtual void onDebugDraw();
    virtual void onDie();
private:
    std::map<std::string, ctrlrAction> m_actions;
    HTN::Planner *m_planner;
    Character *m_target;
    mkVec3 m_enemyLastPos;
    mkVec3 m_attackDir;
    bool m_enemyRunningAway;
    float m_prevEnemyDist;
    float m_prevDistSum;
    float m_prevEnemyDistTime;
	bool m_isPowerLakeReached;
	bool m_isStayingInPowerLake;
	bool m_isBuffAvailable;
	bool m_isAIBuffed;
	bool m_isGoingToPowerLake;
	bool m_canAttackEnemy;
	bool m_isEnemyAttacking;
	float m_prevHealth;
	bool m_isLookingForEnemy;
	float m_lastAttackTime;

    void updateWorldState(float dt);
    void executeTask(HTN::pOperator op);

	//----------actions----------
	bool actionPatrol(float duration);
    bool actionRotateToEnemy(float duration);
	bool actionAttackMelee(float duration);
	bool actionAttackFireball(float duration);
	bool actionAttackFireballMoving(float duration);
	bool actionAttackFireballRotating(float duration);
	bool actionReduceDistance(float duration);
	bool actionGoToPowerLake(float duration);
	bool actionStayInPowerLake(float duration);
	bool actionTakeBuff(float duration);
	bool actionDestroyBarrels(float duration);
	bool actionFindAttacker(float duration);
	bool actionGoToEnemyLastSpot(float duration);
	bool actionTakeMedkit(float duration);
	bool actionRest(float duration);
    //////////////////////////////////////////////////////////////////////////
    bool animAttackMelee(float duration);
    bool animAttackFireball(float duration);
};