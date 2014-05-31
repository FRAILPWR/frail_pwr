#include "pch.h"
#include "MyBTActorController.h"

MyBTActorController::MyBTActorController( ActorAI* ai ) : IActorController(ai)
{
    bb = new BT::BlackBoard();

    attackDir = mkVec3::ZERO;

    bb->init();

    parser = new BT::Parser();
}

MyBTActorController::~MyBTActorController()
{
    delete bb;
    delete root;
    delete parser;
}

void MyBTActorController::onCreate()
{
	getAI()->lookAt(mkVec3::ZERO);
	bb->setStateFloat("rngFbMax", getAI()->getShootingRange());
    bb->setStateFloat("rngMelee", getAI()->getMeleeRange());
    bb->setStateFloat("dmgConeSize", getAI()->getMeleeConeSize());
	bb->setStateBool("shot", false);

	parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), root);
    parser->parseAliases(bb);
}

void MyBTActorController::onTakeDamage(const SDamageInfo& dmg_info)
{
	attackDir = mkVec3::ZERO-dmg_info.dir;
}

void MyBTActorController::onUpdate( float dt )
{
	updateWorldState(dt);
    BT::Status s = root->tick(bb);

    if(parser->isFileModified(getAI()->getBtTreePath()) && root->isTerminated()){
        parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), root);
        parser->parseAliases(bb);
    }
}

void MyBTActorController::onDebugDraw()
{
	getAI()->drawSensesInfo();
    DebugDrawer::getSingleton().drawCircle(enemySpot, 0.2f, 30, Ogre::ColourValue::Black, true);
}

void MyBTActorController::onDie()
{

}

void MyBTActorController::updateWorldState(float dt)
{
	if (getAI()->getBarrels().size() == 0){
		bb->setStateBool("shot", false);
	}	

	if (attackDir != mkVec3::ZERO){
		bb->setStateVec3("AttackDir", attackDir);
		attackDir = mkVec3::ZERO;
		bb->setStateBool("IsEnemyAttack", true);
	}

	bb->setStateFloat("ActorHealth", getAI()->getHealth());
    
	if(getAI()->getHealth() == getAI()->getMaxHealth()){
		bb->setStateBool("HasActorMaxHealth", true);
	} else{
		bb->setStateBool("HasActorMaxHealth", false);
	}

	if(getAI()->getHealth() < 0.5 * (getAI()->getMaxHealth())){
		bb->setStateBool("HasActorLowHealth", true);
	} else{
		bb->setStateBool("HasActorLowHealth", false);
	}

	if(!getAI()->isInPowerLake()){
		bb->setStateBool("IsActorInPowerLake", false);
	}

	target = getAI()->findClosestEnemyInSight();

	if (target){
		bb->setStateBool("IsEnemyVisible", true);
		float enemyDistance = (float)(target->getSimPos() - getAI()->getSimPos()).length();
		bb->setStateFloat("EnemyDistance", enemyDistance);
		bb->setStateBool("IsEnemySeen", true);
		bb->setStateVec3("LastEnemySpot", target->getSimPos());
		bb->setStateVec3("EnemyPos", target->getSimPos());
		bb->setStateFloat("EnemyDgrDiff", 
			(float)(getAI()->getCharToEnemyAngle(target->getSimPos()).valueDegrees()));
	}
	else{
		bb->setStateBool("IsEnemyVisible", false);
		bb->setStateFloat("EnemyDistance", 10000.f);
		bb->setStateVec3("EnemyPos", mkVec3::ZERO);
		bb->setStateFloat("EnemyDgrDiff", 0.f);
	}
}