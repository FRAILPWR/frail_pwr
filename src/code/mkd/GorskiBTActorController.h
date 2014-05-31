#pragma once
#include "IActorController.h"
#include "BT/Behavior.h"
#include "BT/Parser.h"
#include "BT/BlackBoard.h"
class GorskiBTActorController : public IActorController
{
public:
	explicit GorskiBTActorController(ActorAI* ai);
	~GorskiBTActorController();

	virtual void onCreate();
	virtual void onTakeDamage(const SDamageInfo& dmgInfo);
	virtual void onUpdate(float dt);
	virtual void onDebugDraw();
	virtual void onDie();

	void updateWorldState(float dt);
	void updateTargetState(Character* target);
private:
	BT::Behavior* m_root;
	BT::BlackBoard* m_bb;
};

