#include "pch.h"
#include "Actions.h"
#include "MatUtils.h"

namespace BT {
    Action::Action(ActorAI* ai) 
        : m_AI(ai), m_actionStarted(0.f), m_duration(0.f)
    {

    }

    Action::~Action()
    {

    }

    Status Action::update()
    {
        return BH_FAILURE;
    }

    void Action::onInitialize(BlackBoard* bb)
    {
        m_bb = bb;
        m_actionStarted = g_game->getTimeMs();
    }

    void Action::onTerminate( Status status )
    {

    }

    void Action::addCondition(Condition* condition)
    {
        m_conditions.push_back(condition);
    }

    void Action::addInterruption(Condition* condition)
    {
        m_interruptions.push_back(condition);
    }

    bool Action::validateConditions(BlackBoard* bb)
    {
        for(size_t i=0; i<m_conditions.size(); ++i){
            if(!m_conditions[i].validateCondition(bb))
                return false;
        }
        return true;
    }

    bool Action::validateInterruptions(BlackBoard* bb)
    {
        for(size_t i=0; i<m_interruptions.size(); ++i){
            if(m_interruptions[i].validateCondition(bb))
                return true;
        }
        return false;
    }

    inline Status Action::runUntil()
    {
        if(g_game->getTimeMs() - getActionStarted() >= getDuration())
            return BH_SUCCESS;

        if(getStatus() == BH_RUNNING)
            return BH_RUNNING;

        return BH_INVALID;
    }

    inline bool Action::isValid()
    {
        if(isInterruptible()){
            if(!validateConditions(m_bb) || validateInterruptions(m_bb)){
                m_AI->stopSmoothChangeDir();
                m_AI->stopAnimation();
                return false;
            }
        } else {
            if(!(validateConditions(m_bb) || getStatus() == BH_RUNNING))
                return false;
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////////////

    namespace BOSS {
        Patrol::Patrol(ActorAI* ai) 
            : Action(ai)
        {

        }

        Patrol::~Patrol()
        {

        }

        Status Patrol::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("Patrol!");
            mkVec3 new_direction = getRandomHorizontalDir();
            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                new_direction = getRandomHorizontalDir();
                ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            }

            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/2);
            m_AI->setSpeed(0.5f);

            return BH_RUNNING;
        }

        void Patrol::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Patrol::onTerminate( Status status )
        {
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        RevealAttacker::RevealAttacker(ActorAI* ai) 
            : Action(ai)
        {

        }

        RevealAttacker::~RevealAttacker()
        {

        }

        Status RevealAttacker::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 attackDir = m_bb->getStateVec3("AttackDir",isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Reveal!");
            m_AI->setSpeed(0.3f);
            size_t steps = 40;
            m_AI->startSmoothChangeDir(attackDir, steps, getDuration());

            return BH_RUNNING;
        }

        void RevealAttacker::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void RevealAttacker::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemyAttack",false);
            m_bb->setStateVec3("AttackDir",mkVec3::ZERO);
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        ReduceDistance::ReduceDistance(ActorAI* ai) 
            : Action(ai)
        {

        }

        ReduceDistance::~ReduceDistance()
        {

        }

        Status ReduceDistance::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            RayCastResult ray_result = m_AI->raycast(enemyPos, 0.1f, 1.f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            //Ogre::LogManager::getSingleton().logMessage("Reduce!");
            mkVec3 destDir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, getDuration());
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void ReduceDistance::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void ReduceDistance::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        AttackMelee::AttackMelee(ActorAI* ai) 
            : Action(ai)
        {

        }

        AttackMelee::~AttackMelee()
        {

        }

        Status AttackMelee::update()
        {
            if(isInterruptible()){
                if(!validateConditions(m_bb))
                    return BH_FAILURE;
            } else {
                if(!(validateConditions(m_bb) || getStatus() == BH_RUNNING))
                    return BH_FAILURE;
            }

            if(g_game->getTimeMs() - getActionStarted() >= getDuration())
                return BH_SUCCESS;

            if(getStatus() == BH_RUNNING)
                return BH_RUNNING;

            //Ogre::LogManager::getSingleton().logMessage("AttackMelee!");
            m_AI->hitMelee();
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AttackMelee::onTerminate( Status status )
        {   

        }

        //////////////////////////////////////////////////////////////////////////

        AttackFireball::AttackFireball(ActorAI* ai) 
            : Action(ai)
        {

        }

        AttackFireball::~AttackFireball()
        {

        }

        Status AttackFireball::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("AttackFireball!");
            m_AI->hitFireball(enemyPos);
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AttackFireball::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AttackFireball::onTerminate( Status status )
        {   

        }

        //////////////////////////////////////////////////////////////////////////

        AngerMode::AngerMode(ActorAI* ai) 
            : Action(ai)
        {

        }

        AngerMode::~AngerMode()
        {

        }

        Status AngerMode::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("AngerMode!");
            m_AI->hitAngerMode();
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AngerMode::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AngerMode::onTerminate( Status status )
        {   
            if(status == BH_SUCCESS)
                m_bb->setStateBool("IsActorAM",true);
        }

        //////////////////////////////////////////////////////////////////////////

        ExploreSpot::ExploreSpot(ActorAI* ai) 
            : Action(ai)
        {

        }

        ExploreSpot::~ExploreSpot()
        {

        }

        Status ExploreSpot::update()
        {
            if(!isValid())
                return BH_FAILURE;

            bool isValid = true;
            mkVec3 lastSeenSpot = m_bb->getStateVec3("LastEnemySpot", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((lastSeenSpot - m_AI->getSimPos()).length() < 3.f)
                return BH_SUCCESS;

            if(getStatus() != BH_RUNNING){
                //Ogre::LogManager::getSingleton().logMessage("Explore spot!");
                size_t steps = 40;
                mkVec3 new_direction = lastSeenSpot - m_AI->getSimPos();
                new_direction.normalise();
                m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
                m_AI->setSpeed(1.f);
            }

            return BH_RUNNING;
        }

        void ExploreSpot::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void ExploreSpot::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemySeen",false);
        }

        //////////////////////////////////////////////////////////////////////////

        Rotate::Rotate(ActorAI* ai) 
            : Action(ai)
        {

        }

        Rotate::~Rotate()
        {

        }

        Status Rotate::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Rotate!");
            size_t steps = 40;
            mkVec3 new_direction = enemyPos - m_AI->getSimPos();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Rotate::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Rotate::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        KeepDistance::KeepDistance(ActorAI* ai) 
            : Action(ai)
        {

        }

        KeepDistance::~KeepDistance()
        {

        }

        Status KeepDistance::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("KeepDistance!");
            size_t steps = 40;
            mkVec3 new_direction = m_AI->getSimPos() - enemyPos;
            new_direction.normalise();

            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 10.f);
            if(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                return BH_FAILURE;
            } else {
                m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/3);
                m_AI->setSpeed(1.0f);
                return BH_RUNNING;
            }

            return BH_RUNNING;
        }

        void KeepDistance::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void KeepDistance::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        AnimAttackMelee::AnimAttackMelee(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAttackMelee::~AnimAttackMelee()
        {

        }

        Status AnimAttackMelee::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAttackMelee::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        AnimAttackPunch::AnimAttackPunch(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAttackPunch::~AnimAttackPunch()
        {

        }

        Status AnimAttackPunch::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack1",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAttackPunch::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAttackPunch::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        AnimAngerMode::AnimAngerMode(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAngerMode::~AnimAngerMode()
        {

        }

        Status AnimAngerMode::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("HighJump",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAngerMode::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAngerMode::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        AnimJump::AnimJump(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimJump::~AnimJump()
        {

        }

        Status AnimJump::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Jump",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimJump::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimJump::onTerminate( Status status )
        {
            if(status == BH_SUCCESS)
                m_bb->setStateFloat("LastJumpTime",g_game->getTimeMs());
        }

        //////////////////////////////////////////////////////////////////////////

        AnimBackflip::AnimBackflip(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimBackflip::~AnimBackflip()
        {

        }

        Status AnimBackflip::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Backflip",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimBackflip::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimBackflip::onTerminate( Status status )
        {
            m_bb->setStateBool("GotHit",false);
        }

    }

    namespace EXPERIMENTS {

        GotoShop::GotoShop(ActorAI* ai) 
            : Action(ai)
        {

        }

        GotoShop::~GotoShop()
        {

        }

        Status GotoShop::update()
        {
            if(!isValid())
                return BH_FAILURE;

            bool isValid = true;
            mkVec3 shopSpot = m_bb->getStateVec3("ShopSpot", isValid);
            float spotRadius = m_bb->getStateFloat("SpotRadius", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((shopSpot - m_AI->getSimPos()).length() < spotRadius)
                return BH_SUCCESS;

            mkVec3 new_direction = shopSpot - m_AI->getSimPos();
            new_direction.normalise();

            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, 500.f);
            m_AI->setSpeed(0.7f);


            RayCastResult ray_result = m_AI->raycast(new_direction, 0.1f, 0.5f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            return BH_RUNNING;
        }

        void GotoShop::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void GotoShop::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        GotoMine::GotoMine(ActorAI* ai) 
            : Action(ai)
        {

        }

        GotoMine::~GotoMine()
        {

        }

        Status GotoMine::update()
        {
            if(!isValid())
                return BH_FAILURE;

            bool isValid = true;
            mkVec3 mineSpot = m_bb->getStateVec3("MineSpot", isValid);
            float spotRadius = m_bb->getStateFloat("SpotRadius", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((mineSpot - m_AI->getSimPos()).length() < spotRadius)
                return BH_SUCCESS;

            mkVec3 new_direction = mineSpot - m_AI->getSimPos();
            new_direction.normalise();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, 500.f);
            m_AI->setSpeed(0.7f);

            RayCastResult ray_result = m_AI->raycast(new_direction, 0.1f, 0.5f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            return BH_RUNNING;
        }

        void GotoMine::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void GotoMine::onTerminate( Status status )
        {
        }

        //////////////////////////////////////////////////////////////////////////

        DigGold::DigGold(ActorAI* ai) 
            : Action(ai)
        {

        }

        DigGold::~DigGold()
        {

        }

        Status DigGold::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);
            bool pickaxe = m_bb->getStateBool("IsPickaxeBought",isValid);

            if(!isValid)
                return BH_FAILURE;

            if(!pickaxe){
                m_bb->setStateFloat("NPCGold",gold+10.f);
                m_AI->runAnimation("Attack1",getDuration());
            } else {
                m_bb->setStateFloat("NPCGold",gold+25.f);
                m_AI->runAnimation("Attack3",getDuration());
            }

            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void DigGold::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void DigGold::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        BuyPickaxe::BuyPickaxe(ActorAI* ai) 
            : Action(ai)
        {

        }

        BuyPickaxe::~BuyPickaxe()
        {

        }

        Status BuyPickaxe::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);

            if(!isValid)
                return BH_FAILURE;

            m_bb->setStateFloat("NPCGold",gold-200.f);
            m_bb->setStateBool("IsPickaxeBought",true);
            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void BuyPickaxe::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void BuyPickaxe::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        BuyHelmet::BuyHelmet(ActorAI* ai) 
            : Action(ai)
        {

        }

        BuyHelmet::~BuyHelmet()
        {

        }

        Status BuyHelmet::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);

            if(!isValid)
                return BH_FAILURE;

            m_bb->setStateFloat("NPCGold",gold-400.f);
            m_bb->setStateBool("IsHelmetBought",true);
            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void BuyHelmet::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void BuyHelmet::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        BuyLantern::BuyLantern(ActorAI* ai) 
            : Action(ai)
        {

        }

        BuyLantern::~BuyLantern()
        {

        }

        Status BuyLantern::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);

            if(!isValid)
                return BH_FAILURE;

            m_bb->setStateFloat("NPCGold",gold-600.f);
            m_bb->setStateBool("IsLanternBought",true);
            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void BuyLantern::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void BuyLantern::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        Idle::Idle(ActorAI* ai) 
            : Action(ai)
        {

        }

        Idle::~Idle()
        {

        }

        Status Idle::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("Idle!");
            size_t steps = 40;
            mkVec3 new_direction = getRandomHorizontalDir();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Idle::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Idle::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

    }

    namespace AK_BEST {

        //AK_BEST_Idle

        AK_BEST_Idle::AK_BEST_Idle(ActorAI* ai) 
            : Action(ai)
        {
        }

        AK_BEST_Idle::~AK_BEST_Idle()
        {
        }

        Status AK_BEST_Idle::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            size_t stepCount = 50;
            mkVec3 dir = getRandomHorizontalDir();
            dir.normalise();
            m_AI->startSmoothChangeDir(dir, stepCount, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AK_BEST_Idle::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_Idle::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //AK_BEST_FollowEnemysLastPos

        AK_BEST_FollowEnemysLastPos::AK_BEST_FollowEnemysLastPos(ActorAI* ai) 
            : Action(ai)
        {
        }

        AK_BEST_FollowEnemysLastPos::~AK_BEST_FollowEnemysLastPos()
        {
        }

        Status AK_BEST_FollowEnemysLastPos::update()
        {
            if(!isValid())
                return BH_FAILURE;

            bool isValid = true;
            mkVec3 enemysLastPos = m_bb->getStateVec3("LastEnemySpot", isValid);
            bool IsActorInPowerLake = m_bb->getStateBool("IsActorInPowerLake", isValid);
            bool HasActorMaxHealth = m_bb->getStateBool("HasActorMaxHealth", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((enemysLastPos - m_AI->getSimPos()).length() < 4.f)
                return BH_SUCCESS;

            if(getStatus() != BH_RUNNING){
                size_t stepCount = 50;
                mkVec3 dir = enemysLastPos - m_AI->getSimPos();
                dir.normalise();
                m_AI->startSmoothChangeDir(dir, stepCount, getDuration());

                if (IsActorInPowerLake && !HasActorMaxHealth)
                    m_AI->setSpeed(0.f);
                else
                    m_AI->setSpeed(1.f);	
            }

            return BH_RUNNING;
        }

        void AK_BEST_FollowEnemysLastPos::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_FollowEnemysLastPos::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemySeen", false);
        }

        //AK_BEST_InEnemysAttackDir

        AK_BEST_InEnemysAttackDir::AK_BEST_InEnemysAttackDir(ActorAI* ai) 
            : Action(ai)
        {
        }

        AK_BEST_InEnemysAttackDir::~AK_BEST_InEnemysAttackDir()
        {
        }

        Status AK_BEST_InEnemysAttackDir::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 attackDir = m_bb->getStateVec3("AttackDir", isValid);

            if(!isValid)
                return BH_FAILURE;

            size_t stepCount = 50;
            m_AI->startSmoothChangeDir(attackDir, stepCount, getDuration());

            return BH_RUNNING;
        }

        void AK_BEST_InEnemysAttackDir::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_InEnemysAttackDir::onTerminate( Status status )
        {   
            m_bb->setStateVec3("AttackDir", mkVec3::ZERO);
            m_bb->setStateBool("IsEnemyAttack", false);
            m_AI->stopSmoothChangeDir();
        }

        //AK_BEST_MeleeAttack

        AK_BEST_MeleeAttack::AK_BEST_MeleeAttack(ActorAI* ai) 
            : Action(ai)
        {
        }

        AK_BEST_MeleeAttack::~AK_BEST_MeleeAttack()
        {
        }

        Status AK_BEST_MeleeAttack::update()
        {
            if(g_game->getTimeMs() - getActionStarted() >= getDuration())
                return BH_SUCCESS;

            if(getStatus() == BH_RUNNING)
                return BH_RUNNING;

            m_AI->hitMelee();
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AK_BEST_MeleeAttack::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_MeleeAttack::onTerminate( Status status )
        {   
        }

        //AK_BEST_Turn

        AK_BEST_Turn::AK_BEST_Turn(ActorAI* ai) 
            : Action(ai)
        {
        }

        AK_BEST_Turn::~AK_BEST_Turn()
        {
        }

        Status AK_BEST_Turn::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            size_t stepCount = 50;
            mkVec3 dir = enemyPos - m_AI->getSimPos();
            dir.normalise();
            m_AI->startSmoothChangeDir(dir, stepCount, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AK_BEST_Turn::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_Turn::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //AK_BEST_FireballAttack

        AK_BEST_FireballAttack::AK_BEST_FireballAttack(ActorAI* ai) 
            : Action(ai)
        {
        }

        AK_BEST_FireballAttack::~AK_BEST_FireballAttack()
        {
        }

        Status AK_BEST_FireballAttack::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);
            bool shot = m_bb->getStateBool("shot", isValid);

            if(!isValid)
                return BH_FAILURE;

            float distFromEnemyPosToZero = (float)(enemyPos - mkVec3::ZERO).length();
            float distFromActorPosToZero = (float)(m_AI->getSimPos() - mkVec3::ZERO).length();

            std::vector<ModelObject*> resVect = m_AI->getBarrels();

            bool b = false;
            size_t steps = 40;

            if(distFromEnemyPosToZero < 10.f && distFromActorPosToZero > 12.f && !shot)
            {
                for(int i = 0; i < resVect.size(); i++)
                {
                    if (NULL != resVect[i]){
                        mkVec3 barrelPos = resVect[i]->getWorldPosition();
                        if (m_AI->isPosInShootingRange(barrelPos)){
                            m_bb->setStateBool("shot", true);
                            b = true;

                            mkVec3 destDir = (barrelPos-m_AI->getSimPos()).normalisedCopy();
                            m_AI->startSmoothChangeDir(destDir, steps, getDuration());
                            m_AI->hitFireball(barrelPos);
                        }
                        if (b) 
                            break;
                    }
                    if (b) 
                        break;
                }
                if(!b){
                    mkVec3 destDir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
                    m_AI->startSmoothChangeDir(destDir, steps, getDuration());
                    m_AI->hitFireball(enemyPos);
                }
            }
            else{
                mkVec3 destDir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
                m_AI->startSmoothChangeDir(destDir, steps, getDuration());
                m_AI->hitFireball(enemyPos);
            }

            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AK_BEST_FireballAttack::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_FireballAttack::onTerminate( Status status )
        {
        }

        //AK_BEST_FollowEnemy

        AK_BEST_FollowEnemy::AK_BEST_FollowEnemy(ActorAI* ai) 
            : Action(ai)
        {
        }

        AK_BEST_FollowEnemy::~AK_BEST_FollowEnemy()
        {
        }

        Status AK_BEST_FollowEnemy::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            mkVec3 dir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(dir, steps, getDuration());

            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void AK_BEST_FollowEnemy::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_FollowEnemy::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //AK_BEST_GoInPowerLakeDir

        AK_BEST_GoInPowerLakeDir::AK_BEST_GoInPowerLakeDir(ActorAI* ai) 
            : Action(ai)
        {

        }

        AK_BEST_GoInPowerLakeDir::~AK_BEST_GoInPowerLakeDir()
        {

        }

        Status AK_BEST_GoInPowerLakeDir::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((m_AI->getPowerLakePosition() - m_AI->getSimPos()).length() < 6.f)
                return BH_SUCCESS;

            if(getStatus() != BH_RUNNING){
                size_t steps = 40;
                mkVec3 dir = m_AI->getPowerLakePosition() - m_AI->getSimPos();
                dir.normalise();
                m_AI->startSmoothChangeDir(dir, steps, getDuration());
                m_AI->setSpeed(1.f);
            }

            return BH_RUNNING;
        }

        void AK_BEST_GoInPowerLakeDir::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AK_BEST_GoInPowerLakeDir::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsActorInPowerLake", true);
        }

    }

    namespace BLUE {
        Patrol::Patrol(ActorAI* ai) 
            : Action(ai)
        {

        }

        Patrol::~Patrol()
        {

        }

        Status Patrol::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            mkVec3 new_direction = getRandomHorizontalDir();
            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                new_direction = getRandomHorizontalDir();
                ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            }

            m_AI->startSmoothChangeDir(new_direction,40,getDuration()/2);
            m_AI->setSpeed(0.5f);

            return BH_RUNNING;
        }

        void Patrol::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Patrol::onTerminate(Status status)
        {
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        RevealAttacker::RevealAttacker(ActorAI* ai) 
            : Action(ai)
        {

        }

        RevealAttacker::~RevealAttacker()
        {

        }

        Status RevealAttacker::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 attackDir = m_bb->getStateVec3("AttackDir",isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Reveal!");
            m_AI->setSpeed(0.3f);
            size_t steps = 40;
            m_AI->startSmoothChangeDir(attackDir, steps, getDuration());

            return BH_RUNNING;
        }

        void RevealAttacker::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void RevealAttacker::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemyAttack",false);
            m_bb->setStateVec3("AttackDir",mkVec3::ZERO);
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        Medkit::Medkit(ActorAI* ai) 
            : Action(ai)
        {

        }

        Medkit::~Medkit()
        {

        }

        Status Medkit::update()
        {
            if(isInterruptible()){
                if(!validateConditions(m_bb))
                    return BH_FAILURE;
            } else {
                if(!(validateConditions(m_bb) || getStatus() == BH_RUNNING))
                    return BH_FAILURE;
            }

            if(g_game->getTimeMs() - getActionStarted() >= getDuration())
                return BH_SUCCESS;

            if(getStatus() == BH_RUNNING)
                return BH_RUNNING;

            m_bb->setStateBool("IsHealing", true);

            mkVec3 medkitPos = m_AI->getMedkitPosition();

            if(m_AI->isPositionVisible(medkitPos)) {
                mkVec3 new_direction = medkitPos - m_AI->getSimPos();
                new_direction.normalise();

                int r = rand() % 100;

                if(r > 50) {
                    m_AI->setDirection(new_direction - M_PI/6);
                } else {
                    m_AI->setDirection(new_direction + M_PI/6);
                }
            } else {
                m_AI->lookAt(medkitPos);
                mkVec3 vec = rotate_horz_vec(m_AI->getSimDir(), -M_PI/6);
                m_AI->startSmoothChangeDir(vec,5,getDuration());
            }

            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void Medkit::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Medkit::onTerminate(Status status)
        {

        }

        //////////////////////////////////////////////////////////////////////////

        Lake::Lake(ActorAI* ai) 
            : Action(ai)
        {

        }

        Lake::~Lake()
        {

        }

        Status Lake::update()
        {
            if(isInterruptible()){
                if(!validateConditions(m_bb))
                    return BH_FAILURE;
            } else {
                if(!(validateConditions(m_bb) || getStatus() == BH_RUNNING))
                    return BH_FAILURE;
            }

            if(g_game->getTimeMs() - getActionStarted() >= getDuration())
                return BH_SUCCESS;

            if(getStatus() == BH_RUNNING)
                return BH_RUNNING;

            m_bb->setStateBool("IsHealing", true);

            mkVec3 lakePos = m_AI->getPowerLakePosition();

            if(m_AI->isPositionVisible(lakePos)) {
                mkVec3 new_direction = lakePos - m_AI->getSimPos();
                new_direction.normalise();

                int r = rand() % 100;

                if(r > 50) {
                    m_AI->setDirection(new_direction - M_PI/6);
                } else {
                    m_AI->setDirection(new_direction + M_PI/6);
                }
            } else {
                m_AI->lookAt(lakePos);
                mkVec3 vec = rotate_horz_vec(m_AI->getSimDir(), -M_PI/6);
                m_AI->startSmoothChangeDir(vec,5,getDuration());
            }

            if((m_AI->getSimPos() - m_AI->getPowerLakePosition()).length() <= 10.0f) {
                m_AI->setSpeed(0.f);
            } else {
                m_AI->setSpeed(1.f);
            }

            return BH_RUNNING;
        }

        void Lake::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Lake::onTerminate(Status status)
        {

        }

        //////////////////////////////////////////////////////////////////////////

        Rotate::Rotate(ActorAI* ai) 
            : Action(ai)
        {

        }

        Rotate::~Rotate()
        {

        }

        Status Rotate::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            size_t steps = 40;
            mkVec3 new_direction = enemyPos - m_AI->getSimPos();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Rotate::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Rotate::onTerminate(Status status)
        {
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        Chase::Chase(ActorAI* ai) 
            : Action(ai)
        {

        }

        Chase::~Chase()
        {

        }

        Status Chase::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            m_AI->lookAt(enemyPos);
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void Chase::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Chase::onTerminate(Status status)
        {

        }

        //////////////////////////////////////////////////////////////////////////

        Shoot::Shoot(ActorAI* ai) 
            : Action(ai)
        {

        }

        Shoot::~Shoot()
        {

        }

        Status Shoot::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValidPos = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValidPos);

            std::vector<ModelObject*> barrels = m_AI->getBarrels();
            if(barrels.size() > 0) {
                ModelObject* barrel = NULL;
                int i = 0;

                while(i < barrels.size() && barrel == NULL) {
                    barrel = barrels[i];

                    //odleglosc przeciwnika od beczki
                    if((barrel->getWorldPosition() - enemyPos).length() > 8) {
                        barrel = NULL;
                    } else if((barrel->getWorldPosition() - m_AI->getSimPos()).length() < 15) {
                        barrel = NULL;
                    }

                    i++;
                }

                if(barrel != NULL) {
                    mkVec3 new_direction = barrel->getWorldPosition() - m_AI->getSimPos();
                    new_direction.normalise();
                    m_AI->setDirection(new_direction);
                    m_AI->hitFireball(barrel->getWorldPosition());
                } else {
                    mkVec3 new_direction = enemyPos - m_AI->getSimPos();
                    new_direction.normalise();
                    m_AI->setDirection(new_direction);
                    m_AI->hitFireball(enemyPos);
                }
            } else {
                mkVec3 new_direction = enemyPos - m_AI->getSimPos();
                new_direction.normalise();
                m_AI->setDirection(new_direction);
                m_AI->hitFireball(enemyPos);
            }

            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Shoot::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Shoot::onTerminate(Status status)
        {
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        Strafe::Strafe(ActorAI* ai) 
            : Action(ai)
        {

        }

        Strafe::~Strafe()
        {

        }

        Status Strafe::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValidPos = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValidPos);

            mkVec3 new_direction = enemyPos - m_AI->getSimPos();
            new_direction.normalise();

            int r = rand() % 100;

            if(r > 50) {
                m_AI->setDirection(new_direction - M_PI/6);
            } else {
                m_AI->setDirection(new_direction + M_PI/6);
            }

            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void Strafe::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Strafe::onTerminate(Status status)
        {

        }

        //////////////////////////////////////////////////////////////////////////

        Melee::Melee(ActorAI* ai) 
            : Action(ai)
        {

        }

        Melee::~Melee()
        {

        }

        Status Melee::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos",isValid);

            m_AI->lookAt(enemyPos);

            m_AI->hitMelee();
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Melee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Melee::onTerminate(Status status)
        {

        }

        //////////////////////////////////////////////////////////////////////////

        AnimAttackMelee::AnimAttackMelee(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAttackMelee::~AnimAttackMelee()
        {

        }

        Status AnimAttackMelee::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAttackMelee::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        AnimAttackPunch::AnimAttackPunch(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAttackPunch::~AnimAttackPunch()
        {

        }

        Status AnimAttackPunch::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack1",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAttackPunch::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAttackPunch::onTerminate( Status status )
        {   
        }
    }

    namespace GG_GorskiBT
    {

        // Destroy barrels

        DestroyBarrels::DestroyBarrels(ActorAI* ai) : Action(ai)
        {
        }

        DestroyBarrels::~DestroyBarrels()
        {
        }

        Status DestroyBarrels::update()
        {
            if (!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if (s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 barrelPos = m_bb->getStateVec3("BarrelPos", isValid);

            if (!isValid)
                return BH_FAILURE;

            float timer = m_bb->getStateFloat("BarrelTimer", isValid);
            if (!isValid || timer > 0.0f)
                return BH_FAILURE;

            m_AI->hitFireball(barrelPos);
            m_bb->setStateFloat("BarrelTimer", 10000.0f);
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void DestroyBarrels::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void DestroyBarrels::onTerminate(Status status)
        {
        }

        // Pick up the buff

        PickupBuff::PickupBuff(ActorAI* ai) : Action(ai)
        {
        }

        PickupBuff::~PickupBuff()
        {
        }

        Status PickupBuff::update()
        {
            if (!isValid())
                return BH_FAILURE;
            Status s = runUntil();
            if (s != BH_INVALID)
                return s;

            // locate the buffs
            mkVec3 buffPos = m_AI->getBuffPosition();
            mkVec3 direction = buffPos - m_AI->getSimPos();
            m_AI->setDirection(direction);
            m_AI->setSpeed(1.0f);

            return BH_RUNNING;
        }

        void PickupBuff::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void PickupBuff::onTerminate(Status status)
        {
            m_AI->setSpeed(0.0f);
        }

        // Heal

        Heal::Heal(ActorAI* ai) : Action(ai)
        {
        }

        Heal::~Heal()
        {
        }

        Status Heal::update()
        {
            if (!isValid())
                return BH_FAILURE;
            Status s = runUntil();
            if (s != BH_INVALID)
                return s;

            // locate the power lake
            mkVec3 powerLakePos = m_AI->getPowerLakePosition();
            mkVec3 direction = powerLakePos - m_AI->getSimPos();
            m_AI->setDirection(direction);
            m_AI->setSpeed(1.0f);

            return BH_RUNNING;
        }

        void Heal::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Heal::onTerminate(Status status)
        {
            m_AI->setSpeed(0.0f);
        }
    }

    namespace MC_SIRO{
        MC_SIRO_GoToHealingField::MC_SIRO_GoToHealingField(ActorAI* ai)
            :Action(ai)
        {

        }

        MC_SIRO_GoToHealingField::~MC_SIRO_GoToHealingField()
        {

        }

        Status MC_SIRO_GoToHealingField::update()
        {
            if(!isValid())
            {
                m_AI->setSpeed(0.f);
                return BH_FAILURE;
            }

            bool isValid = true;
            mkVec3 destination = m_AI->getPowerLakePosition();
            float spotRadius = 7.f;

            if(!isValid)
            {
                m_AI->setSpeed(0.f);
                return BH_FAILURE;
            }

            if((destination - m_AI->getSimPos()).length() < spotRadius){
                return BH_SUCCESS;
            }

            mkVec3 new_direction = destination - m_AI->getSimPos();
            new_direction.normalise();

            size_t steps = 40; 

            RayCastResult ray_result =m_AI->raycast(new_direction, 1.0f, 10.f);
            if(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                // Ogre::LogManager::getSingleton().logMessage("Raycast");
                new_direction = getRandomHorizontalDir();
                RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
                while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                    //	Ogre::LogManager::getSingleton().logMessage("while");
                    new_direction = getRandomHorizontalDir();
                    ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
                }
            }
            m_AI->startSmoothChangeDir(new_direction, steps, 500.f);
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void MC_SIRO_GoToHealingField::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_GoToHealingField::onTerminate( Status status )
        {   
        }
        ///////////////////////////////////////
        MC_SIRO_Rotate::MC_SIRO_Rotate(ActorAI* ai) 
            : Action(ai)
        {

        }

        MC_SIRO_Rotate::~MC_SIRO_Rotate()
        {

        }

        Status MC_SIRO_Rotate::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Rotate!");
            size_t steps = 40;
            mkVec3 new_direction = enemyPos - m_AI->getSimPos();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void MC_SIRO_Rotate::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_Rotate::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////
        MC_SIRO_LookForEnemy::MC_SIRO_LookForEnemy(ActorAI* ai) 
            : Action(ai)
        {

        }

        MC_SIRO_LookForEnemy::~MC_SIRO_LookForEnemy()
        {

        }

        Status MC_SIRO_LookForEnemy::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Rotate!");
            size_t steps = 40;
            mkVec3 new_direction = mkVec3(mkVec3::UNIT_Y);
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void MC_SIRO_LookForEnemy::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_LookForEnemy::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////
        MC_SIRO_AttackMelee::MC_SIRO_AttackMelee(ActorAI* ai) 
            : Action(ai)
        {

        }

        MC_SIRO_AttackMelee::~MC_SIRO_AttackMelee()
        {

        }

        Status MC_SIRO_AttackMelee::update()
        {
            if(isInterruptible()){
                if(!validateConditions(m_bb))
                    return BH_FAILURE;
            } else {
                if(!(validateConditions(m_bb) || getStatus() == BH_RUNNING))
                    return BH_FAILURE;
            }

            if(g_game->getTimeMs() - getActionStarted() >= getDuration())
                return BH_SUCCESS;

            if(getStatus() == BH_RUNNING)
                return BH_RUNNING;

            //Ogre::LogManager::getSingleton().logMessage("AttackMelee!");
            m_AI->hitMelee();
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void MC_SIRO_AttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_AttackMelee::onTerminate( Status status )
        {   

        }

        //////////////////////////////////////////////////////////////////////////

        MC_SIRO_AttackFireball::MC_SIRO_AttackFireball(ActorAI* ai) 
            : Action(ai)
        {

        }

        MC_SIRO_AttackFireball::~MC_SIRO_AttackFireball()
        {

        }

        Status MC_SIRO_AttackFireball::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);
            float enemySpeed = m_bb->getStateFloat("EnemySpeed", isValid);
            float enemyDis = m_bb->getStateFloat("EnemyDistance", isValid);

            if(!isValid)
                return BH_FAILURE;

            //std::string str = boost::lexical_cast<std::string>(enemySpeed);
            //Ogre::LogManager::getSingleton().logMessage("Prêdkoœæ gracza: " + str);

            //str = boost::lexical_cast<std::string>(enemyDis);
            //Ogre::LogManager::getSingleton().logMessage("Odleglosc gracza: " + str);

            //str = boost::lexical_cast<std::string>(m_AI->getShootingRange());			
            //Ogre::LogManager::getSingleton().logMessage("AttackFireball!: " + str);

            if(enemySpeed < 0.035f){
                if(enemyDis > m_AI->getShootingRange()/2  && enemyDis<m_AI->getShootingRange() -5){
                    m_AI->hitFireball(enemyPos+25*enemySpeed*m_bb->getStateVec3("EnemyDirection", isValid));
                    m_AI->setSpeed(0.f);
                }else{
                    if(enemyDis >= m_AI->getShootingRange() - 10){
                        m_AI->hitFireball(enemyPos+70*enemySpeed*m_bb->getStateVec3("EnemyDirection", isValid));
                        m_AI->setSpeed(0.f);
                    }else{
                        m_AI->hitFireball(enemyPos+20*enemySpeed*m_bb->getStateVec3("EnemyDirection", isValid));
                        m_AI->setSpeed(0.f);
                    }
                }
            }else{
                if(enemyDis > m_AI->getShootingRange()/2 && enemyDis < m_AI->getShootingRange() -10){
                    m_AI->hitFireball(enemyPos+40*enemySpeed*m_bb->getStateVec3("EnemyDirection", isValid));
                    m_AI->setSpeed(0.f);
                }else{
                    if(enemyDis >= m_AI->getShootingRange() - 5){
                        m_AI->hitFireball(enemyPos+60*enemySpeed*m_bb->getStateVec3("EnemyDirection", isValid));
                        m_AI->setSpeed(0.f);
                    }else{
                        m_AI->hitFireball(enemyPos+20*enemySpeed*m_bb->getStateVec3("EnemyDirection", isValid));
                        m_AI->setSpeed(0.f);
                    }
                }
            }
            return BH_RUNNING;
        }

        void MC_SIRO_AttackFireball::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_AttackFireball::onTerminate( Status status )
        {   

        }

        //////////////////////////////////////////////////////////////////////////

        MC_SIRO_AnimAttackMelee::MC_SIRO_AnimAttackMelee(ActorAI* ai) 
            : Action(ai)
        {

        }

        MC_SIRO_AnimAttackMelee::~MC_SIRO_AnimAttackMelee()
        {

        }

        Status MC_SIRO_AnimAttackMelee::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void MC_SIRO_AnimAttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_AnimAttackMelee::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        MC_SIRO_AnimAttackRange::MC_SIRO_AnimAttackRange(ActorAI* ai) 
            : Action(ai)
        {

        }

        MC_SIRO_AnimAttackRange::~MC_SIRO_AnimAttackRange()
        {

        }

        Status MC_SIRO_AnimAttackRange::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack1",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void MC_SIRO_AnimAttackRange::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_AnimAttackRange::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        MC_SIRO_ShootBarrel::MC_SIRO_ShootBarrel(ActorAI* ai) 
            : Action(ai)
        {

        }

        MC_SIRO_ShootBarrel::~MC_SIRO_ShootBarrel()
        {

        }

        Status MC_SIRO_ShootBarrel::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("AttackFireball!");
            /*	mkVec3 barrel_pos = m_AI->getBarrels()[0]->getWorldPosition();
            if((float)(barrel_pos - m_AI->getSimPos()).length() < m_AI->getShootingRange())
            {
            m_AI->hitFireball(barrel_pos);
            m_AI->setSpeed(0.f);
            }else{
            barrel_pos = m_AI->getBarrels()[1]->getWorldPosition();
            if((float)(barrel_pos - m_AI->getSimPos()).length() < m_AI->getShootingRange())
            {
            m_AI->hitFireball(barrel_pos);
            m_AI->setSpeed(0.f);
            }
            }*/

            if(m_AI->getBarrels().size() > 0){
                bool found = false;
                for(int i = 0; i <m_AI->getBarrels().size(); i++){
                    mkVec3 barrel_pos = m_AI->getBarrels()[i]->getWorldPosition();

                    if((float)(barrel_pos - m_AI->getSimPos()).length() < m_AI->getShootingRange() && !found){
                        m_AI->setSpeed(0.f);
                        m_AI->hitFireball(barrel_pos);
                        found = true;
                    }
                }

            }else{
                return BH_FAILURE;
            }

            return BH_RUNNING;
        }

        void MC_SIRO_ShootBarrel::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_ShootBarrel::onTerminate( Status status )
        {   

        }

        //////////////////////////////////////////////////////////////////////////

        MC_SIRO_FindEnemy::MC_SIRO_FindEnemy(ActorAI* ai)
            :Action(ai)
        {

        }

        MC_SIRO_FindEnemy::~MC_SIRO_FindEnemy()
        {

        }

        Status MC_SIRO_FindEnemy::update()
        {
            if(!isValid())
            {
                m_AI->setSpeed(0.f);
                return BH_FAILURE;
            }

            bool isValid = true;
            mkVec3 destination = m_bb->getStateVec3("EnemyLastSeenPos", isValid);
            float spotRadius = 7.f;

            if(!isValid)
            {
                m_AI->setSpeed(0.f);
                return BH_FAILURE;
            }

            if((destination - m_AI->getSimPos()).length() < spotRadius){
                return BH_SUCCESS;
            }

            mkVec3 new_direction = destination - m_AI->getSimPos();
            new_direction.normalise();

            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, 500.f);
            m_AI->setSpeed(1.f);


            RayCastResult ray_result = m_AI->raycast(new_direction, 0.1f, 0.5f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            return BH_RUNNING;
        }

        void MC_SIRO_FindEnemy::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_FindEnemy::onTerminate( Status status )
        {   
        }
        ///////////////////////////////////////

        MC_SIRO_WaitForEnemy::MC_SIRO_WaitForEnemy(ActorAI* ai)
            :Action(ai)
        {

        }

        MC_SIRO_WaitForEnemy::~MC_SIRO_WaitForEnemy()
        {

        }

        Status MC_SIRO_WaitForEnemy::update()
        {
            if(!isValid())
            {
                m_AI->setSpeed(0.f);
                return BH_FAILURE;
            }

            bool isValid = true;			

            if(!isValid)
            {
                m_AI->setSpeed(0.f);
                return BH_FAILURE;
            }

            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void MC_SIRO_WaitForEnemy::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MC_SIRO_WaitForEnemy::onTerminate( Status status )
        {   
        }
        ///////////////////////////////////////
    }

    namespace MK_STIG{


        MK_STIG_Action::MK_STIG_Action(ActorAI* ai) : Action(ai)
            , Atack_1_anim_duration(550)
            , Atack_3_anim_duration(1150)
        {}

        void MK_STIG_Action::onInitialize(BlackBoard* bb)
        {__super::onInitialize(bb);}

        void MK_STIG_Action::onTerminate(Status status)
        {}

        //==================================================
        //==================================================
        //==================================================

        AttackMelee::AttackMelee(ActorAI* ai) 
            : MK_STIG_Action(ai)
            , m_hited(false)
        {}

        AttackMelee::~AttackMelee()
        {}

        Status AttackMelee::update()
        {

            if(!isValid())
                return BH_FAILURE;

            if(g_game->getTimeMs() - getActionStarted() >= getDuration() * 3/4 && !m_hited){
                m_AI->hitMelee();
                m_hited = true;
            }

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack3", Atack_3_anim_duration);
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
            m_hited = false;
        }

        //============================================================================
        ReduceDistanceAndRangeAtack::ReduceDistanceAndRangeAtack(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {}

        ReduceDistanceAndRangeAtack::~ReduceDistanceAndRangeAtack()
        {}

        Status ReduceDistanceAndRangeAtack::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            RayCastResult ray_result = m_AI->raycast(enemyPos, 0.1f, 1.f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            mkVec3 destDir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, 200);
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void ReduceDistanceAndRangeAtack::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);
            if(isValid)
                m_AI->hitFireball(enemyPos);
        }

        void ReduceDistanceAndRangeAtack::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }
        //======================================================================

        GoToHeal::GoToHeal(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {}

        GoToHeal::~GoToHeal()
        {}

        Status GoToHeal::update()
        {

            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;


            mkVec3 destDir = (m_AI->getPowerLakePosition() - m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, getDuration());
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void GoToHeal::onInitialize(BlackBoard* bb)
        { __super::onInitialize(bb);}

        void GoToHeal::onTerminate( Status status )
        { m_AI->stopSmoothChangeDir();}

        //======================================================================

        Forward::Forward(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {}

        Forward::~Forward()
        {}

        Status Forward::update()
        {

            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void Forward::onInitialize(BlackBoard* bb)
        {__super::onInitialize(bb);}

        void Forward::onTerminate( Status status )
        {
            m_AI->stopSmoothChangeDir();
            m_AI->setSpeed(0.f);
        }

        //======================================================================

        AttackFireball::AttackFireball(ActorAI* ai) : MK_STIG_Action(ai)
            , m_hited(false)
            , m_animated(false)
        {}

        AttackFireball::~AttackFireball()
        {}

        Status AttackFireball::update()
        {
            if(!isValid())
                return BH_FAILURE;

            float timOfAction = g_game->getTimeMs() - getActionStarted();
            float pozostalyCzas = getDuration() - timOfAction;


            if(pozostalyCzas <= Atack_1_anim_duration ){

                if(pozostalyCzas <= Atack_1_anim_duration * 1/4 && !m_hited){

                    bool isValid = true;
                    mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);
                    if(!isValid) return BH_FAILURE;

                    m_AI->hitFireball(enemyPos);
                    m_hited = true;
                }

                if(!m_animated) {
                    m_AI->runAnimation("Attack1", Atack_1_anim_duration);
                    m_animated = true;
                }
            }
            //===============================================================
            //				   Obracanie w kierunku wroga
            //===============================================================
            bool isValid = true;
            mkVec3 position = m_bb->getStateVec3("EnemyPos", isValid);
            if(!isValid) return BH_FAILURE;

            mkVec3 destDir = (position - m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, 100);
            //===============================================================

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            return BH_RUNNING;
        }

        void AttackFireball::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
            m_hited = false;
            m_animated = false;

        }

        void AttackFireball::onTerminate( Status status )
        {m_AI->stopSmoothChangeDir();}

        //======================================================================

        Rotate::Rotate(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {

        }

        Rotate::~Rotate()
        {

        }

        Status Rotate::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Rotate!");
            size_t steps = 40;
            mkVec3 new_direction = enemyPos - m_AI->getSimPos();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Rotate::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Rotate::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //======================================================================


        RevealAttacker::RevealAttacker(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {

        }

        RevealAttacker::~RevealAttacker()
        {

        }

        Status RevealAttacker::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 attackDir = m_bb->getStateVec3("AttackDir",isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Reveal!");
            m_AI->setSpeed(0.3f);
            size_t steps = 40;
            m_AI->startSmoothChangeDir(attackDir, steps, getDuration());

            return BH_RUNNING;
        }

        void RevealAttacker::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void RevealAttacker::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemyAttack",false);
            m_bb->setStateVec3("AttackDir",mkVec3::ZERO);
            m_AI->stopSmoothChangeDir();
        }

        //======================================================================


        Idle::Idle(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {

        }

        Idle::~Idle()
        {

        }

        Status Idle::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("Idle!");
            size_t steps = 40;
            mkVec3 new_direction = getRandomHorizontalDir();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Idle::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Idle::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }




        //======================================================================

        Patrol::Patrol(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {}

        Patrol::~Patrol()
        {}

        Status Patrol::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("Patrol!");
            mkVec3 new_direction = getRandomHorizontalDir();
            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                new_direction = getRandomHorizontalDir();
                ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            }

            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/2);
            m_AI->setSpeed(0.5f);

            return BH_RUNNING;
        }

        void Patrol::onInitialize(BlackBoard* bb)
        {__super::onInitialize(bb);}

        void Patrol::onTerminate( Status status )
        {m_AI->stopSmoothChangeDir();}

        //======================================================================

        ReduceDistance::ReduceDistance(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {}

        ReduceDistance::~ReduceDistance()
        {}

        Status ReduceDistance::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //RayCastResult ray_result = m_AI->raycast(enemyPos, 0.1f, 1.f);

            mkVec3 destDir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, getDuration());
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void ReduceDistance::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void ReduceDistance::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //======================================================================

        GoToLastEnemySpot::GoToLastEnemySpot(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {}

        GoToLastEnemySpot::~GoToLastEnemySpot()
        {}

        Status GoToLastEnemySpot::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyLastSpoot", isValid);

            if(!isValid)
                return BH_FAILURE;

            RayCastResult ray_result = m_AI->raycast(enemyPos, 0.1f, 1.f);

            mkVec3 destDir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, getDuration());
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void GoToLastEnemySpot::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void GoToLastEnemySpot::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();

            bool isValid = true;

            mkVec3 lastEnemyPos = m_bb->getStateVec3("EnemyLastSpoot", isValid);
            if ((lastEnemyPos - m_AI->getSimPos()).length() < 3.f){
                m_bb->setStateVec3("EnemyLastSpoot", mkVec3::ZERO);
                m_bb->setStateBool("isEnemyLastSpoot", false);
            }
        }



        //============================================================================
        HitBarrel::HitBarrel(ActorAI* ai) 
            : MK_STIG_Action(ai)
            , m_hited(false)
            , m_animated(false)
        {}

        HitBarrel::~HitBarrel()
        {}

        Status HitBarrel::update()
        {

            if(!isValid())
                return BH_FAILURE;

            float timOfAction = g_game->getTimeMs() - getActionStarted();
            float pozostalyCzas = getDuration() - timOfAction;


            if(pozostalyCzas <= Atack_1_anim_duration ){

                if(pozostalyCzas <= Atack_1_anim_duration * 1/4 && !m_hited){

                    bool isValid = true;
                    mkVec3 pozycjaNajblizszejBeczki = m_bb->getStateVec3("pozycjaNajblizszejBeczki", isValid);
                    if(!isValid) return BH_FAILURE;

                    m_AI->hitFireball(pozycjaNajblizszejBeczki);
                    m_hited = true;
                    m_bb->setStateFloat("kiedyStrzelilesWBeczke", g_game->getTimeMs());

                }

                if(!m_animated) {
                    m_AI->runAnimation("Attack1", Atack_1_anim_duration);
                    m_animated = true;
                }
            }


            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            return BH_RUNNING;

            //RayCastResult ray_result = m_AI->raycast(enemyPos, 0.1f, 1.f);
            //if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
        }

        void HitBarrel::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
            m_hited = false;
            m_animated = false;
        }

        void HitBarrel::onTerminate( Status status )
        {}

        //============================================================================
        GoForDD::GoForDD(ActorAI* ai) 
            : MK_STIG_Action(ai)
        {}

        GoForDD::~GoForDD()
        {}

        Status GoForDD::update()
        {

            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 ddPosition = m_bb->getStateVec3("ddPosition", isValid);
            if(!isValid) return BH_FAILURE;

            mkVec3 destDir = (ddPosition-m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, getDuration());

            float angleDiff = (float)(m_AI->getCharToEnemyAngle(ddPosition).valueDegrees());

            if (angleDiff > 10.f)
                m_AI->setSpeed(0.1f);
            else 
                m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void GoForDD::onInitialize(BlackBoard* bb)
        {__super::onInitialize(bb);}

        void GoForDD::onTerminate( Status status )
        {
            m_AI->stopSmoothChangeDir();
        }

        //============================================================================

    }

    namespace MW_XARDAS
    {
        MakeHell::MakeHell(ActorAI* ai)
            : Action(ai)
        {

        }

        MakeHell::~MakeHell()
        {

        }

        BT::Status MakeHell::update()
        {
            if (!isValid())
                return BT::BH_FAILURE;

            BT::Status s = runUntil();
            if (s != BT::BH_INVALID)
                return s;


            bool isValid = true;
            bool hellPossible = m_bb->getStateBool("EnemyCloseToBarrel", isValid);
            if (!isValid)
                return BT::BH_FAILURE;

            mkVec3 barrel = m_bb->getStateVec3("BarrelOfDeath", isValid);
            if (!isValid)
                return BT::BH_FAILURE;

            m_AI->hitFireball(barrel);

            return BT::BH_RUNNING;
        }

        void MakeHell::onInitialize(BT::BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void MakeHell::onTerminate(BT::Status status)
        {

        }


        SmartShot::SmartShot(ActorAI* ai)
            : Action(ai)
        {

        }

        SmartShot::~SmartShot()
        {

        }

        BT::Status SmartShot::update()
        {
            if (!isValid())
                return BT::BH_FAILURE;

            BT::Status s = runUntil();
            if (s != BT::BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyFeet", isValid);

            if (!isValid)
                return BT::BH_FAILURE;

            mkVec3 enemySpeed = m_bb->getStateVec3("EnemySpeed", isValid);


            if (!isValid)
                m_AI->hitFireball(enemyPos);
            else
                m_AI->hitFireball(PredictFireball(m_AI->getSimPos(), enemyPos, enemySpeed, 30));

            m_AI->setSpeed(0.f);

            return BT::BH_RUNNING;
        }

        void SmartShot::onInitialize(BT::BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void SmartShot::onTerminate(BT::Status status)
        {

        }


        SmartAvgShot::SmartAvgShot(ActorAI* ai)
            : Action(ai)
        {

        }

        SmartAvgShot::~SmartAvgShot()
        {

        }

        BT::Status SmartAvgShot::update()
        {
            if (!isValid())
                return BT::BH_FAILURE;

            BT::Status s = runUntil();
            if (s != BT::BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyFeet", isValid);

            if (!isValid)
                return BT::BH_FAILURE;

            mkVec3 enemySpeed = m_bb->getStateVec3("EnemyAvgSpeed", isValid);


            if (!isValid)
                m_AI->hitFireball(enemyPos);
            else
                m_AI->hitFireball(PredictFireball(m_AI->getSimPos(), enemyPos, enemySpeed, 30));

            m_AI->setSpeed(0.f);

            return BT::BH_RUNNING;
        }

        void SmartAvgShot::onInitialize(BT::BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void SmartAvgShot::onTerminate(BT::Status status)
        {

        }

        Dodge::Dodge(ActorAI* ai)
            : Action(ai)
        {
        }

        Dodge::~Dodge()
        {

        }

        BT::Status Dodge::update()
        {
            if (!isValid())
                return BT::BH_FAILURE;

            BT::Status s = runUntil();
            if (s != BT::BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if (!isValid)
                return BT::BH_FAILURE;

            mkVec3 dir = enemyPos - m_AI->getSimPos();
            dir.normalise();
            mkVec3 z = m_AI->getPosForHeight(1) - m_AI->getPosForHeight(0);
            mkVec3 movDir = z.crossProduct(dir) * (d ? 1.f : -1.f);

            m_AI->startSmoothChangeDir(dir, 40, 200);
            m_AI->setMovementDir(movDir);
            m_AI->setSpeed(1.f);
            m_AI->jump();

            return BT::BH_RUNNING;
        }

        void Dodge::onInitialize(BT::BlackBoard* bb)
        {
            __super::onInitialize(bb);

            d = rand() & 1;
        }

        void Dodge::onTerminate(BT::Status status)
        {
            m_AI->setMovementDir(mkVec3::ZERO);
        }


        StepBack::StepBack(ActorAI* ai)
            : Action(ai)
        {
        }

        StepBack::~StepBack()
        {

        }

        BT::Status StepBack::update()
        {
            if (!isValid())
                return BT::BH_FAILURE;

            BT::Status s = runUntil();
            if (s != BT::BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if (!isValid)
                return BT::BH_FAILURE;

            mkVec3 dir = enemyPos - m_AI->getSimPos();
            dir.normalise();

            m_AI->setMovementDir(-dir);
            m_AI->startSmoothChangeDir(dir, 40, 500);
            m_AI->setSpeed(1.f);

            return BT::BH_RUNNING;
        }

        void StepBack::onInitialize(BT::BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void StepBack::onTerminate(BT::Status status)
        {
            m_AI->setMovementDir(mkVec3::ZERO);
        }



        AnimFireballDodge::AnimFireballDodge(ActorAI* ai)
            : Action(ai)
        {

        }

        AnimFireballDodge::~AnimFireballDodge()
        {

        }

        BT::Status AnimFireballDodge::update()
        {
            if (!isValid())
                return BT::BH_FAILURE;

            BT::Status s = runUntil();
            if (s != BT::BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if (!isValid)
                return BT::BH_FAILURE;

            mkVec3 dir = enemyPos - m_AI->getSimPos();
            dir.normalise();
            mkVec3 z = m_AI->getPosForHeight(1) - m_AI->getPosForHeight(0);
            mkVec3 movDir = z.crossProduct(dir) * (d ? 1.f : -1.f);

            m_AI->runAnimation("Attack1", getDuration());
            m_AI->setMovementDir(movDir);
            m_AI->setSpeed(1.f);

            m_AI->jump();

            return BT::BH_RUNNING;
        }

        void AnimFireballDodge::onInitialize(BT::BlackBoard* bb)
        {
            __super::onInitialize(bb);
            d = rand() & 1;
        }

        void AnimFireballDodge::onTerminate(BT::Status status)
        {
        }

        AnimFireballStepBack::AnimFireballStepBack(ActorAI* ai)
            : Action(ai)
        {

        }

        AnimFireballStepBack::~AnimFireballStepBack()
        {

        }

        BT::Status AnimFireballStepBack::update()
        {
            if (!isValid())
                return BT::BH_FAILURE;

            BT::Status s = runUntil();
            if (s != BT::BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if (!isValid)
                return BT::BH_FAILURE;

            mkVec3 dir = enemyPos - m_AI->getSimPos();
            dir.normalise();

            m_AI->runAnimation("Attack1", getDuration());
            m_AI->setMovementDir(-dir);
            m_AI->setSpeed(1.f);

            m_AI->jump();

            return BT::BH_RUNNING;
        }

        void AnimFireballStepBack::onInitialize(BT::BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimFireballStepBack::onTerminate(BT::Status status)
        {
        }
    }

    //////////////////////////////////////////////////////////////////////////

    namespace SM {

        SM_SzatanskaMiotla_RecoveryState::SM_SzatanskaMiotla_RecoveryState(ActorAI* ai) 
            : Action(ai)
        {
        }

        SM_SzatanskaMiotla_RecoveryState::~SM_SzatanskaMiotla_RecoveryState()
        {}

        Status SM_SzatanskaMiotla_RecoveryState::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 powerLakePosition = m_bb->getStateVec3("PowerLakePosition", isValid);
            mkVec3 medkitPosition = m_bb->getStateVec3("MedkitPosition", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((powerLakePosition - m_AI->getSimPos()).length() == 0.f || (medkitPosition - m_AI->getSimPos()).length() == 0.f)
                return BH_SUCCESS;

            if(getStatus() != BH_RUNNING){
                size_t steps = 40;
                mkVec3 new_direction = powerLakePosition - m_AI->getSimPos();
                if(m_AI->isMedkitAvailable() && (medkitPosition - m_AI->getSimPos()).length()*2 < new_direction.length()){
                    new_direction = medkitPosition - m_AI->getSimPos();
                }
                new_direction.normalise();
                RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 10.f);
                if(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                    return BH_FAILURE;
                } else {
                    m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/3);
                    m_AI->setSpeed(1.0f);
                    return BH_RUNNING;
                }
            }
            return BH_RUNNING;
        }

        void SM_SzatanskaMiotla_RecoveryState::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void SM_SzatanskaMiotla_RecoveryState::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemySeen",false);
        }

        SM_SzatanskaMiotla_Patrol::SM_SzatanskaMiotla_Patrol(ActorAI* ai) 
            : Action(ai)
        {

        }

        SM_SzatanskaMiotla_Patrol::~SM_SzatanskaMiotla_Patrol()
        {

        }

        Status SM_SzatanskaMiotla_Patrol::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 powerPosition = m_bb->getStateVec3("PowerPosition", isValid);
            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Patrol!");
            mkVec3 new_direction = getRandomHorizontalDir();
            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                new_direction = getRandomHorizontalDir();
                ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            }
            if(m_AI->isBuffAvailable()){
                new_direction = powerPosition - m_AI->getSimPos();
                new_direction.normalise();
                size_t steps = 40;
                m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/2);
                m_AI->setSpeed(1.f);
            } else {
                size_t steps = 40;
                m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/2);
                m_AI->setSpeed(1.f);
            }
            return BH_RUNNING;
        }

        void SM_SzatanskaMiotla_Patrol::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void SM_SzatanskaMiotla_Patrol::onTerminate( Status status )
        {
            m_AI->stopSmoothChangeDir();
        }

    }

    //////////////////////////////////////////////////////////////////////////

    namespace HAN {
        //////////////////////////////////////////////////////////////////////////

        GoToPowerPoll::GoToPowerPoll(ActorAI* ai) 
            : Action(ai)
        {

        }

        GoToPowerPoll::~GoToPowerPoll()
        {

        }

        Status GoToPowerPoll::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 pollSpot = m_AI->getPowerLakePosition();

            if(!isValid)
                return BH_FAILURE;

            RayCastResult ray_result = m_AI->raycast(pollSpot, 0.1f, 1.f);

            //if(getStatus() != BH_RUNNING){
            //Ogre::LogManager::getSingleton().logMessage("Explore spot!");
            size_t steps = 40;
            mkVec3 new_direction = (pollSpot - m_AI->getSimPos()).normalisedCopy();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/3);
            m_AI->setSpeed(1.f);
            // }

            return BH_RUNNING;
        }

        void GoToPowerPoll::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void GoToPowerPoll::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        TakeBuff::TakeBuff(ActorAI* ai) 
            : Action(ai)
        {

        }

        TakeBuff::~TakeBuff()
        {

        }

        Status TakeBuff::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 buffSpot = m_AI->getBuffPosition();

            if(!isValid)
                return BH_FAILURE;

            RayCastResult ray_result = m_AI->raycast(buffSpot, 0.1f, 1.f);

            //if(getStatus() != BH_RUNNING){
            //Ogre::LogManager::getSingleton().logMessage("Explore spot!");
            size_t steps = 40;
            mkVec3 new_direction = (buffSpot - m_AI->getSimPos()).normalisedCopy();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(1.f);
            // }

            return BH_RUNNING;
        }

        void TakeBuff::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void TakeBuff::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        KeepDistanceWithFireball::KeepDistanceWithFireball(ActorAI* ai) 
            : Action(ai)
        {

        }

        KeepDistanceWithFireball::~KeepDistanceWithFireball()
        {

        }

        Status KeepDistanceWithFireball::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;
            Character* m_target = m_AI->findClosestEnemyInSight();

            //Ogre::LogManager::getSingleton().logMessage("KeepDistance!");
            size_t steps = 40;
            mkVec3 new_direction = enemyPos - m_AI->getSimPos();
            new_direction.normalise();

            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 10.f);
            if(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                return BH_FAILURE;
            } else {

                m_AI->runAnimation("Attack1",getDuration()/3);
                m_AI->setSpeed(0.f);

                m_AI->hitFireball(enemyPos);
                m_AI->setSpeed(0.f);

                mkVec3 mod = mkVec3(30, 30, 0);
                m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/3);
                m_AI->setMovementDir(new_direction + mod);
                m_AI->setSpeed(1.0f);
                return BH_RUNNING;
            }

            return BH_RUNNING;
        }

        void KeepDistanceWithFireball::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void KeepDistanceWithFireball::onTerminate( Status status )
        {   
            m_AI->setMovementDir(mkVec3::ZERO);

        }


        //////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////

        TakeMedicine::TakeMedicine(ActorAI* ai) 
            : Action(ai)
        {

        }

        TakeMedicine::~TakeMedicine()
        {

        }

        Status TakeMedicine::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 medicineSpot = m_AI->getMedkitPosition();

            if(!isValid)
                return BH_FAILURE;

            RayCastResult ray_result = m_AI->raycast(medicineSpot, 0.1f, 1.f);

            //if(getStatus() != BH_RUNNING){
            //Ogre::LogManager::getSingleton().logMessage("Explore spot!");
            size_t steps = 40;
            mkVec3 new_direction = (medicineSpot - m_AI->getSimPos()).normalisedCopy();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/3);
            m_AI->setSpeed(1.f);
            // }

            return BH_RUNNING;
        }

        void TakeMedicine::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void TakeMedicine::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }


        //////////////////////////////////////////////////////////////////////////
    }

}