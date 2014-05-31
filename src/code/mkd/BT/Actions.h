#pragma once
#include "BT/Behavior.h"
#include "ActorAI.h"
#include "Character.h"
#include "Game.h"

namespace BT {
    class Action : public Behavior {
    public:
        explicit Action(ActorAI* ai);
        virtual ~Action();

        virtual Status update();
        virtual void onInitialize(BlackBoard* bb);
        virtual void onTerminate(Status status);

        bool validateConditions(BlackBoard* bb);
        bool validateInterruptions(BlackBoard* bb);
        void addCondition(Condition* condition);
        void addInterruption(Condition* condition);
        size_t conditionSize() { return m_conditions.size(); }

        inline Status runUntil();
        inline bool isValid();

        std::string getName() const { return m_name; }
        void setName(std::string val) { m_name = val; }
        float getDuration() const { return m_duration; }
        void setDuration(float val) { m_duration = val; }
        bool isInterruptible() const { return m_interruptible; }
        void setInterruptible(bool val) { m_interruptible = val; }
        float getActionStarted() const { return m_actionStarted; }
        void setActionStarted(float val) { m_actionStarted = val; }
    protected:
        ActorAI* m_AI;
        boost::ptr_vector<Condition> m_conditions;
        boost::ptr_vector<Condition> m_interruptions;
        std::string m_name;
        float m_duration;
        bool m_interruptible;
        float m_actionStarted;
    };

    namespace BOSS {

        class Patrol : public Action {
        public:
            explicit Patrol(ActorAI* ai);
            virtual ~Patrol();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class RevealAttacker : public Action {
        public:
            explicit RevealAttacker(ActorAI* ai);
            virtual ~RevealAttacker();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class ReduceDistance : public Action {
        public:
            explicit ReduceDistance(ActorAI* ai);
            virtual ~ReduceDistance();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AttackMelee : public Action {
        public:
            explicit AttackMelee(ActorAI* ai);
            virtual ~AttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AttackFireball : public Action {
        public:
            explicit AttackFireball(ActorAI* ai);
            virtual ~AttackFireball();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AngerMode : public Action {
        public:
            explicit AngerMode(ActorAI* ai);
            virtual ~AngerMode();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class ExploreSpot : public Action {
        public:
            explicit ExploreSpot(ActorAI* ai);
            virtual ~ExploreSpot();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Rotate : public Action {
        public:
            explicit Rotate(ActorAI* ai);
            virtual ~Rotate();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class KeepDistance : public Action {
        public:
            explicit KeepDistance(ActorAI* ai);
            virtual ~KeepDistance();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAttackMelee : public Action {
        public:
            explicit AnimAttackMelee(ActorAI* ai);
            virtual ~AnimAttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAttackPunch : public Action {
        public:
            explicit AnimAttackPunch(ActorAI* ai);
            virtual ~AnimAttackPunch();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAngerMode : public Action {
        public:
            explicit AnimAngerMode(ActorAI* ai);
            virtual ~AnimAngerMode();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimJump : public Action {
        public:
            explicit AnimJump(ActorAI* ai);
            virtual ~AnimJump();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimBackflip : public Action {
        public:
            explicit AnimBackflip(ActorAI* ai);
            virtual ~AnimBackflip();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

    }

    namespace EXPERIMENTS {
        class GotoShop : public Action {
        public:
            explicit GotoShop(ActorAI* ai);
            virtual ~GotoShop();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class GotoMine : public Action {
        public:
            explicit GotoMine(ActorAI* ai);
            virtual ~GotoMine();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class DigGold : public Action {
        public:
            explicit DigGold(ActorAI* ai);
            virtual ~DigGold();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class BuyPickaxe : public Action {
        public:
            explicit BuyPickaxe(ActorAI* ai);
            virtual ~BuyPickaxe();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class BuyHelmet : public Action {
        public:
            explicit BuyHelmet(ActorAI* ai);
            virtual ~BuyHelmet();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class BuyLantern : public Action {
        public:
            explicit BuyLantern(ActorAI* ai);
            virtual ~BuyLantern();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Idle : public Action {
        public:
            explicit Idle(ActorAI* ai);
            virtual ~Idle();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
    }

    namespace AK_BEST {

        class AK_BEST_Idle : public Action {
        public:
            explicit AK_BEST_Idle(ActorAI* ai);
            virtual ~AK_BEST_Idle();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AK_BEST_FollowEnemysLastPos : public Action {
        public:
            explicit AK_BEST_FollowEnemysLastPos(ActorAI* ai);
            virtual ~AK_BEST_FollowEnemysLastPos();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AK_BEST_InEnemysAttackDir : public Action {
        public:
            explicit AK_BEST_InEnemysAttackDir(ActorAI* ai);
            virtual ~AK_BEST_InEnemysAttackDir();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AK_BEST_MeleeAttack : public Action {
        public:
            explicit AK_BEST_MeleeAttack(ActorAI* ai);
            virtual ~AK_BEST_MeleeAttack();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AK_BEST_Turn : public Action {
        public:
            explicit AK_BEST_Turn(ActorAI* ai);
            virtual ~AK_BEST_Turn();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AK_BEST_FireballAttack : public Action {
        public:
            explicit AK_BEST_FireballAttack(ActorAI* ai);
            virtual ~AK_BEST_FireballAttack();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AK_BEST_FollowEnemy : public Action {
        public:
            explicit AK_BEST_FollowEnemy(ActorAI* ai);
            virtual ~AK_BEST_FollowEnemy();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AK_BEST_GoInPowerLakeDir : public Action {
        public:
            explicit AK_BEST_GoInPowerLakeDir(ActorAI* ai);
            virtual ~AK_BEST_GoInPowerLakeDir();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

    }

    namespace BLUE {
        class Patrol : public Action {
        public:
            explicit Patrol(ActorAI* ai);
            virtual ~Patrol();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class RevealAttacker : public Action {
        public:
            explicit RevealAttacker(ActorAI* ai);
            virtual ~RevealAttacker();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Medkit : public Action {
        public:
            explicit Medkit(ActorAI* ai);
            virtual ~Medkit();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Lake : public Action {
        public:
            explicit Lake(ActorAI* ai);
            virtual ~Lake();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Rotate : public Action {
        public:
            explicit Rotate(ActorAI* ai);
            virtual ~Rotate();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Chase : public Action {
        public:
            explicit Chase(ActorAI* ai);
            virtual ~Chase();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Shoot : public Action {
        public:
            explicit Shoot(ActorAI* ai);
            virtual ~Shoot();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Strafe : public Action {
        public:
            explicit Strafe(ActorAI* ai);
            virtual ~Strafe();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Melee : public Action {
        public:
            explicit Melee(ActorAI* ai);
            virtual ~Melee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAttackMelee : public Action {
        public:
            explicit AnimAttackMelee(ActorAI* ai);
            virtual ~AnimAttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAttackPunch : public Action {
        public:
            explicit AnimAttackPunch(ActorAI* ai);
            virtual ~AnimAttackPunch();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
    }

    namespace GG_GorskiBT
    {
        class DestroyBarrels : public Action
        {
        public:
            explicit DestroyBarrels(ActorAI* ai);
            virtual ~DestroyBarrels();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class PickupBuff : public Action
        {
        public:
            explicit PickupBuff(ActorAI* ai);
            virtual ~PickupBuff();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Heal : public Action
        {
        public:
            explicit Heal(ActorAI* ai);
            virtual ~Heal();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
    }

    namespace MC_SIRO{

        class MC_SIRO_GoToHealingField : public Action{
        public:
            explicit MC_SIRO_GoToHealingField(ActorAI* ai);
            virtual ~MC_SIRO_GoToHealingField();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_Rotate : public Action {
        public:
            explicit MC_SIRO_Rotate(ActorAI* ai);
            virtual ~MC_SIRO_Rotate();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_LookForEnemy : public Action {
        public:
            explicit MC_SIRO_LookForEnemy (ActorAI* ai);
            virtual ~MC_SIRO_LookForEnemy ();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_AttackMelee : public Action {
        public:
            explicit MC_SIRO_AttackMelee(ActorAI* ai);
            virtual ~MC_SIRO_AttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_AttackFireball : public Action {
        public:
            explicit MC_SIRO_AttackFireball(ActorAI* ai);
            virtual ~MC_SIRO_AttackFireball();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_AnimAttackMelee : public Action {
        public:
            explicit MC_SIRO_AnimAttackMelee(ActorAI* ai);
            virtual ~MC_SIRO_AnimAttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_AnimAttackRange : public Action {
        public:
            explicit MC_SIRO_AnimAttackRange(ActorAI* ai);
            virtual ~MC_SIRO_AnimAttackRange();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_ShootBarrel : public Action {
        public:
            explicit MC_SIRO_ShootBarrel(ActorAI* ai);
            virtual ~MC_SIRO_ShootBarrel();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_FindEnemy : public Action {
        public:
            explicit MC_SIRO_FindEnemy(ActorAI* ai);
            virtual ~MC_SIRO_FindEnemy();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class MC_SIRO_WaitForEnemy : public Action {
        public:
            explicit MC_SIRO_WaitForEnemy(ActorAI* ai);
            virtual ~MC_SIRO_WaitForEnemy();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
    }

    namespace MK_STIG{

        class MK_STIG_Action : public Action {
        public:
            explicit MK_STIG_Action(ActorAI* ai);
            //virtual ~MK_STIG_Action();

            //virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);

            float Atack_1_anim_duration;
            float Atack_3_anim_duration;
        };

        //==================================================
        //==================================================
        //==================================================

        class AttackMelee : public MK_STIG_Action {
        public:
            explicit AttackMelee(ActorAI* ai);
            virtual ~AttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
        private:
            bool m_hited;
        };

        //==================================================

        class ReduceDistanceAndRangeAtack : public MK_STIG_Action {
        public:
            explicit ReduceDistanceAndRangeAtack(ActorAI* ai);
            virtual ~ReduceDistanceAndRangeAtack();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class GoToHeal : public MK_STIG_Action {
        public:
            explicit GoToHeal(ActorAI* ai);
            virtual ~GoToHeal();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class Forward : public MK_STIG_Action {
        public:
            explicit Forward(ActorAI* ai);
            virtual ~Forward();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class AttackFireball : public MK_STIG_Action {
        public:
            explicit AttackFireball(ActorAI* ai);
            virtual ~AttackFireball();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        private:
            bool m_hited;
            bool m_animated;
        };

        //==================================================

        class Rotate : public MK_STIG_Action {
        public:
            explicit Rotate(ActorAI* ai);
            virtual ~Rotate();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class RevealAttacker : public MK_STIG_Action {
        public:
            explicit RevealAttacker(ActorAI* ai);
            virtual ~RevealAttacker();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class Idle : public MK_STIG_Action {
        public:
            explicit Idle(ActorAI* ai);
            virtual ~Idle();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class Patrol : public MK_STIG_Action {
        public:
            explicit Patrol(ActorAI* ai);
            virtual ~Patrol();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class ReduceDistance : public MK_STIG_Action {
        public:
            explicit ReduceDistance(ActorAI* ai);
            virtual ~ReduceDistance();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class GoToLastEnemySpot : public MK_STIG_Action {
        public:
            explicit GoToLastEnemySpot(ActorAI* ai);
            virtual ~GoToLastEnemySpot();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        //==================================================

        class HitBarrel : public MK_STIG_Action {
        public:
            explicit HitBarrel(ActorAI* ai);
            virtual ~HitBarrel();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        private:
            bool m_hited;
            bool m_animated;
        };

        //==================================================

        class GoForDD : public MK_STIG_Action {
        public:
            explicit GoForDD(ActorAI* ai);
            virtual ~GoForDD();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        private:

        };

    }

    namespace MW_XARDAS
    {


        class MakeHell : public BT::Action
        {
        public:
            explicit MakeHell(ActorAI* ai);
            virtual ~MakeHell();

            virtual BT::Status update();
            virtual void onInitialize(BT::BlackBoard* bb);
            virtual void onTerminate(BT::Status status);
        };

        class SmartShot : public BT::Action
        {
        public:
            explicit SmartShot(ActorAI* ai);
            virtual ~SmartShot();

            virtual BT::Status update();
            virtual void onInitialize(BT::BlackBoard* bb);
            virtual void onTerminate(BT::Status status);
        };

        class SmartAvgShot : public BT::Action
        {
        public:
            explicit SmartAvgShot(ActorAI* ai);
            virtual ~SmartAvgShot();

            virtual BT::Status update();
            virtual void onInitialize(BT::BlackBoard* bb);
            virtual void onTerminate(BT::Status status);
        };

        class Dodge : public BT::Action
        {
        public:
            explicit Dodge(ActorAI* ai);
            virtual ~Dodge();

            virtual BT::Status update();
            virtual void onInitialize(BT::BlackBoard* bb);
            virtual void onTerminate(BT::Status status);

        private:
            bool d;
        };

        class StepBack : public BT::Action
        {
        public:
            explicit StepBack(ActorAI* ai);
            virtual ~StepBack();

            virtual BT::Status update();
            virtual void onInitialize(BT::BlackBoard* bb);
            virtual void onTerminate(BT::Status status);

        private:
        };


        class AnimFireballDodge : public BT::Action
        {
        public:
            explicit AnimFireballDodge(ActorAI* ai);
            virtual ~AnimFireballDodge();

            virtual BT::Status update();
            virtual void onInitialize(BT::BlackBoard* bb);
            virtual void onTerminate(BT::Status status);
        private:
            bool d;
        };

        class AnimFireballStepBack : public BT::Action
        {
        public:
            explicit AnimFireballStepBack(ActorAI* ai);
            virtual ~AnimFireballStepBack();

            virtual BT::Status update();
            virtual void onInitialize(BT::BlackBoard* bb);
            virtual void onTerminate(BT::Status status);
        private:
            bool d;
        };

    }

    namespace SM {
        class SM_SzatanskaMiotla_RecoveryState : public Action {
        public:
            explicit SM_SzatanskaMiotla_RecoveryState(ActorAI* ai);
            virtual ~SM_SzatanskaMiotla_RecoveryState();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
        class SM_SzatanskaMiotla_Patrol : public Action {
        public:
            explicit SM_SzatanskaMiotla_Patrol(ActorAI* ai);
            virtual ~SM_SzatanskaMiotla_Patrol();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
    }

    namespace HAN {
        class TakeMedicine : public Action {
        public:
            explicit TakeMedicine(ActorAI* ai);
            virtual ~TakeMedicine();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class GoToPowerPoll : public Action {
        public:
            explicit GoToPowerPoll(ActorAI* ai);
            virtual ~GoToPowerPoll();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class TakeBuff : public Action {
        public:
            explicit TakeBuff(ActorAI* ai);
            virtual ~TakeBuff();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class KeepDistanceWithFireball : public Action {
        public:
            explicit KeepDistanceWithFireball(ActorAI* ai);
            virtual ~KeepDistanceWithFireball();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
    }
}