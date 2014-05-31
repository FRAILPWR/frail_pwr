#pragma once
#include "IActorController.h"
#include "BT/Behavior.h"
#include "BT/Parser.h"
#include "BT/BlackBoard.h"
#include "contrib/DebugDrawer.h"

class MK_STIG_ActorController : public IActorController {
public:
    explicit MK_STIG_ActorController(ActorAI* ai);
    ~MK_STIG_ActorController();

    virtual void onCreate();
    virtual void onTakeDamage( const SDamageInfo& dmg_info );
    virtual void onUpdate( float dt );
    virtual void onDebugDraw();
    virtual void onDie();

    void updateWorldState(float dt);
private:
    BT::Behavior* m_root;
    BT::BlackBoard* m_bb;
    BT::Parser* m_parser;

	//========================================================
	mkVec3 m_attackDir;
	mkVec3 m_enemyLastSpoot;
	float m_lastAtackTIme;
	bool m_resetAngerMode;
	bool m_isWaitForAvaibleHealth;
	bool m_isDDAvaible;


	bool isInMeleeRange(Character* target);
	bool isInFireRange(Character* target);
	
	bool isHealthOnCriticalLevelAndNotInPowerLake();
	bool isHealthOnCriticalLevel();
	bool isInPowerLake();
	bool isHealthOnAvailableLevel();



};