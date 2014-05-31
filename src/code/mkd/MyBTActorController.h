#pragma once
#include "IActorController.h"
#include "BT/Behavior.h"
#include "BT/Parser.h"
#include "BT/BlackBoard.h"
#include "contrib/DebugDrawer.h"

class MyBTActorController : public IActorController {
public:
    explicit MyBTActorController(ActorAI* ai);
    ~MyBTActorController();

    virtual void onCreate();
    virtual void onTakeDamage( const SDamageInfo& dmg_info );
    virtual void onUpdate( float dt );
    virtual void onDebugDraw();
    virtual void onDie();

    void updateWorldState(float dt);
private:
    BT::Behavior* root;
    BT::BlackBoard* bb;
    BT::Parser* parser;

    Character* target;
    mkVec3 attackDir;
    mkVec3 enemySpot;
};