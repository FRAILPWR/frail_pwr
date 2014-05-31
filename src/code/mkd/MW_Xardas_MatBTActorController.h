#pragma once
#include "IActorController.h"
#include "BT/Behavior.h"
#include "BT/Parser.h"
#include "BT/BlackBoard.h"
#include "contrib/DebugDrawer.h"


class MW_XardasActorController : public IActorController {
public:
	explicit MW_XardasActorController(ActorAI* ai);
	~MW_XardasActorController();

	virtual void onCreate();
	virtual void onTakeDamage(const SDamageInfo& dmg_info);
	virtual void onUpdate(float dt);
	virtual void onDebugDraw();
	virtual void onDie();

	void updateWorldState(float dt);
private:
	BT::Behavior* m_root;
	BT::BlackBoard* m_bb;
	BT::Parser* m_parser;

	Character* m_target;
	mkVec3 m_attackDir;
	mkVec3 m_enemySpot;
	mkVec3 m_enemySpeed;
	float m_prevEnemyDistTime;
	float m_prevEnemyDist;
	float m_prevDistSum;
	bool m_resetAngerMode;

	float m_probabHit;
	float m_lastHit;
	float m_lastHitDt;

	std::deque<mkVec3> lastSpeeds;
	const static int avgSize = 25;
	mkVec3 speedSum;

	float m_minSpeed;
};