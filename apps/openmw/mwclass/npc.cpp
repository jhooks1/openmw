
#include "npc.hpp"

#include <components/esm/loadnpc.hpp>
#include <components/nifogre/ogre_nif_loader.hpp>

#include "../mwmechanics/creaturestats.hpp"
#include "../mwmechanics/npcstats.hpp"

#include "../mwworld/ptr.hpp"
#include "../mwworld/actiontalk.hpp"
#include "../mwworld/environment.hpp"
#include "../mwworld/world.hpp"

#include "../mwrender/cellimp.hpp"

#include "../mwmechanics/mechanicsmanager.hpp"
#include <OgreSceneNode.h>


namespace
{
    const Ogre::Radian kOgrePi (Ogre::Math::PI);
    const Ogre::Radian kOgrePiOverTwo (Ogre::Math::PI / Ogre::Real(2.0));
}

namespace MWClass
{
    std::string Npc::getId (const MWWorld::Ptr& ptr) const
    {
        ESMS::LiveCellRef<ESM::NPC, MWWorld::RefData> *ref =
            ptr.get<ESM::NPC>();

        return ref->base->mId;
    }

    void Npc::insertObj (const MWWorld::Ptr& ptr, MWRender::CellRenderImp& cellRender,
        MWWorld::Environment& environment) const
    {
        //Ogre::SceneNode *chest;
        ESMS::LiveCellRef<ESM::NPC, MWWorld::RefData> *ref =
            ptr.get<ESM::NPC>();
        assert (ref->base != NULL);
		
		
		//mTool->



		std::string hairID = ref->base->hair;
        std::string headID = ref->base->head;
		std::string npcName = ref->base->name;
		//std::cout << "NPC: " << npcName << "\n";

        //get the part of the bodypart id which describes the race and the gender
        std::string bodyRaceID = headID.substr(0, headID.find_last_of("head_") - 4);
        std::string headModel = "meshes\\" +
            environment.mWorld->getStore().bodyParts.find(headID)->model;

		std::string hairModel = "meshes\\" +
            environment.mWorld->getStore().bodyParts.find(hairID)->model;

        MWRender::Rendering rendering (cellRender, ref->ref);
		ref->model = cellRender.insertAndDeliverMesh("meshes\\base_anim.nif");
        ref->allanim = NIFLoader::getSingletonPtr()->getAllanim();

        const ESM::BodyPart *bodyPart =
            environment.mWorld->getStore().bodyParts.search (bodyRaceID + "chest");

		

		//bodyPart->model->
		Ogre::Vector3 pos = Ogre::Vector3( 20, 20, 20);
		Ogre::Vector3 axis = Ogre::Vector3( 0, 0, 1);
		Ogre::Radian angle = Ogre::Radian(0);
		
		std::string addresses[6] = {"", "", "", "","", ""};
		std::string addresses2[6] = {"", "", "", "", "", ""};
		std::string upperleft[5] = {"", "", "", "", ""};
		std::string upperright[5] = {"", "", "", "", ""};
		std::string neckandup[5] = {"", "", "","",""};
		std::string empty[6] = {"", "", "", "","", ""};
		int numbers = 0;
		int uppernumbers = 0;
		int neckNumbers = 0;
		
		Ogre::Quaternion q = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0,1,0)); //1,0,0
		Ogre::Quaternion e = Ogre::Quaternion::IDENTITY;
		Ogre::Vector3 blank = Ogre::Vector3(0,0,0);
        if (bodyPart){
			//Ogre::Bone *chest = skeleton->getBone("Chest");
		    //if(chest){
			//Ogre::Entity* baseChest = cellRender.insertBase("meshes\\" + bodyPart->model, false);
			    //cellRender.insertMesh("meshes\\" + bodyPart->model, "Chest", base);
			

			//Using all the same body parts now because offsets are not implemented
				//cellRender.insertMesh("meshes\\" + bodyPart->model, "Bip01 Spine1", ref->model, q);
				//cellRender.insertMesh("meshes\\" + bodyPart->model, "Chest", ref->model,e);
				//cellRender.insertMesh(baseChest, "Chest", base, q);
				//cellRender.insertMesh("meshes\\" + bodyPart->model, "Bip01 Pelvis", base, q);
				//cellRender.insertMesh("meshes\\" + bodyPart->model, "Chest", base, q);
			//}
           /*cellRender.insertMesh("meshes\\" + bodyPart->model, pos, axis, angle, npcName + "chest", addresses, numbers, true);   //2 0
		   addresses2[numbers] = npcName + "chest";
		   addresses[numbers++] = npcName + "chest";
		   upperleft[uppernumbers] = npcName + "chest";
		   upperright[uppernumbers++] = npcName + "chest";
		   neckandup[neckNumbers++] = npcName + "chest";*/
		}
		   //std::cout << "GETTING NPC PART";
		//Orgre::SceneNode test = cellRender.getNpcPart();

		const ESM::BodyPart *upperleg = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "upper leg");
		const ESM::BodyPart *groin = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "groin");
		const ESM::BodyPart *arm = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "upper arm");
		const ESM::BodyPart *neck = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "neck");
		const ESM::BodyPart *head = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "head");
		const ESM::BodyPart *knee = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "knee");
		const ESM::BodyPart *ankle = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "ankle");
		const ESM::BodyPart *foot = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "foot");
		const ESM::BodyPart *feet = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "feet");
		const ESM::BodyPart *tail = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "tail");
		const ESM::BodyPart *wrist = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "wrist");
		const ESM::BodyPart *forearm = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "forearm");
		const ESM::BodyPart *hand = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "hand.1st");
		const ESM::BodyPart *hands = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "hands.1st");


		//std::cout << "RACE" << bodyRaceID << "\n";

		Ogre::Vector3 pos2 = Ogre::Vector3(90, 5, 4);     //5
		std::string upperarmpath[2] = {npcName + "chest", npcName + "upper arm"};
		
		Ogre::Quaternion p = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(1, 0, 0)); //1,0,0
		q  = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0, 1, 0)); //1,0,0

		cellRender.insertMesh("meshes\\b\\b_n_breton_f_foot.nif", Ogre::Vector3(3,3,3));
		//cellRender.insertMesh(headModel, "Bip01 Head", ref->model, q * p,Ogre::Vector3(-75, 20, 2));
        if (groin){
			//cellRender.insertMesh("meshes\\" + groin->model, "Groin", ref->model, e, blank);
			//cellRender.insertMesh("meshes\\" + groin->model, "Bip01 Pelvis", ref->model, p * q, pos2);
            //cellRender.insertMesh("meshes\\" + groin->model, pos2, axis, Ogre::Radian(3.14), npcName + "groin", addresses, numbers);
			
			addresses2[numbers] = npcName + "groin";
			addresses[numbers++] = npcName + "groin";
			cellRender.insertMesh("meshes\\" + groin->model, "Groin", ref->model, e, blank);
			//cellRender.insertMesh("meshes\\" + groin->model, "Right Upper Leg", ref->model, e, blank);

		}
		if (tail) {
			//cellRender.insertMesh("tail\\" + tail->model, Ogre::Vector3(0 , 0, -76), axis, Ogre::Radian(3.14), npcName + "tail", addresses, numbers, "tail");
			//std::cout << "TAIL\n";
		}
		
		q = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0, 1, 0)); //1,0,0
		p = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(1, 0, 0));
		//addresses[1] = npcName + "groin";
		if(upperleg){
			cellRender.insertMesh("meshes\\" + upperleg->model, "Left Upper Leg", ref->model, e, blank);
			cellRender.insertMesh("meshes\\" + upperleg->model, "Right Upper Leg", ref->model, e, blank);
			
			
			
			
			
			
			
		//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), addresses, numbers);
		}
		if(knee)
		{
			cellRender.insertMesh("meshes\\" + knee->model, "Left Knee", ref->model, e, blank);  //e
			cellRender.insertMesh("meshes\\" + knee->model, "Right Knee", ref->model, e,blank);   //e
			
		}
		if(ankle){
			
			cellRender.insertMesh("meshes\\" + ankle->model, "Left Ankle", ref->model, e, blank); //Ogre::Quaternion(Ogre::Radian(3.14 / 4), Ogre::Vector3(1, 0, 0)),blank); //1,0,0, blank);
			cellRender.insertMesh("meshes\\" + ankle->model, "Right Ankle", ref->model, e,blank);
		}
		if(foot){
			if(bodyRaceID.compare("b_n_khajiit_m_") == 0)
			{
				feet = foot;
			}
			else
			{
				cellRender.insertMesh("meshes\\" + foot->model, "Left Foot", ref->model, e, blank);
			cellRender.insertMesh("meshes\\" + foot->model, "Right Foot", ref->model, e, blank);
				//cellRender.insertMesh ("meshes\\" + foot->model, Ogre::Vector3( 0, -4, -15), axis, Ogre::Radian(0), npcName + "foot", addresses, numbers);

				//cellRender.insertMesh ("meshes\\" + foot->model, Ogre::Vector3( 0, -4, -15), axis, Ogre::Radian(0), npcName + "foot2", addresses2, numbers);
				addresses2[numbers] = npcName + "foot2";
				addresses[numbers++] = npcName + "foot";
			}
			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), addresses, numbers);
		}
		if(feet){
			
			cellRender.insertMesh("meshes\\" + feet->model, "Left Foot", ref->model, e, blank);
			cellRender.insertMesh("meshes\\" + feet->model, "Right Foot", ref->model, e, blank);
			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), addresses, numbers);
		}
		
		 
		if (arm){
			cellRender.insertMesh("meshes\\" + arm->model, "Right Upper Arm", ref->model, e, blank);
				cellRender.insertMesh("meshes\\" + arm->model, "Left Upper Arm", ref->model, e, blank);
			
		    //cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), upperleft, uppernumbers);        //1 -1 1
		}

		if (forearm)
		{
				cellRender.insertMesh("meshes\\" + forearm->model, "Right Forearm", ref->model, e, blank);
					cellRender.insertMesh("meshes\\" + forearm->model, "Left Forearm", ref->model, e, blank);
		}
		//else
		//	std::cout << npcName << "has no forearm";
		if (wrist)
		{
			//cellRender.insertMesh("meshes\\" + wrist->model, "Right Wrist", ref->model, e, blank);
			//		cellRender.insertMesh("meshes\\" + wrist->model, "Left Wrist", ref->model, e, blank);
			if(upperleft[uppernumbers - 1].compare(npcName + "upper arm") == 0)
			{
				
				//cellRender.insertMesh("meshes\\b\\B_N_Argonian_M_Forearm.nif", Ogre::Vector3(-12.5, 0, 0), Ogre::Vector3(0, 0, 0), Ogre::Radian(3.14), npcName + "forearm", upperleft, uppernumbers);
				//cellRender.insertMesh("meshes\\b\\B_N_Argonian_M_Forearm.nif", Ogre::Vector3(-12.5, 0, 0), Ogre::Vector3(0, 0, 0), Ogre::Radian(3.14), npcName + "forearm2", upperright, uppernumbers);
				upperleft[uppernumbers] = npcName + "forearm";
				upperright[uppernumbers++] = npcName + "forearm2";

			}
			//cellRender.insertMesh("meshes\\" + wrist->model, Ogre::Vector3(-9.5, 0, 0), Ogre::Vector3(0, 0, 0), Ogre::Radian(3.14), npcName + "wrist", upperleft, uppernumbers);
			//cellRender.insertMesh("meshes\\" + wrist->model, Ogre::Vector3(-9.5, 0, 0), Ogre::Vector3(0, 0, 0), Ogre::Radian(3.14), npcName + "wrist2", upperright, uppernumbers);
			upperleft[uppernumbers] = npcName + "wrist";
			upperright[uppernumbers++] = npcName + "wrist2";
		}
		

		if(hand)
		{
			std::string pass;
			if(hand->model.compare("b\\B_N_Dark Elf_F_Hands.1st.NIF")==0 && bodyRaceID.compare("b_n_dark elf_m_") == 0)
				pass = "b\\B_N_Dark Elf_M_Hands.1st.NIF";	
			else
				pass = hand->model;
			//cellRender.insertMesh("meshes\\" + pass, Ogre::Vector3(42, 1, -110), Ogre::Vector3(0, 0, 0), Ogre::Radian(3.14), npcName + "hand", upperleft, uppernumbers,false);   //0, 100, -100    0,0,120
			//cellRender.insertMesh("meshes\\" + pass, Ogre::Vector3(42, 1, -110), Ogre::Vector3(0, 0,0), Ogre::Radian(3.14), npcName + "hand2", upperright, uppernumbers, false);   //0, 100, -100    0,0,120
			upperleft[uppernumbers] = npcName + "hand";
			upperright[uppernumbers++] = npcName + "hand2";
			//cellRender.rotateMesh(Ogre::Vector3(0, 0,0),  Ogre::Radian(3.14), upperleft, uppernumbers);



			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), upperleft, uppernumbers);
			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), upperright, uppernumbers);
		}
		if(hands)
		{
			std::string pass;
			if(hands->model.compare("b\\B_N_Redguard_F_Hands.1st.nif")==0 && bodyRaceID.compare("b_n_redguard_m_") == 0)
				pass = "b\\B_N_Redguard_M_Hands.1st.nif";
			else if(hands->model.compare("b\\B_N_Imperial_M_Hands.1st.nif") == 0 && bodyRaceID.compare("b_n_nord_m_") == 0)
				pass = "b\\B_N_Nord_M_Hands.1st.nif";
			else
				pass =hands->model;								//-50, 0, -120
			/*cellRender.insertMesh("meshes\\" + pass, Ogre::Vector3(42, 1,-110), Ogre::Vector3(0, 0, 0), Ogre::Radian(3.14), npcName + "hand", upperleft, uppernumbers, false);   //0, 100, -100    42, 0, -110
			//cellRender.insertMesh("meshes\\" + hands->model, Ogre::Vector3(42, 0,110), Ogre::Vector3(1, 0, 0), Ogre::Radian(3.14), npcName + "hand", upperleft, uppernumbers, false);   //0, 100, -100    42, 0, -110
			cellRender.insertMesh("meshes\\" + pass, Ogre::Vector3(42, 1, -110), Ogre::Vector3(0, 0, 0), Ogre::Radian(3.14), npcName + "hand2", upperright, uppernumbers, false);   //0, 100, -100    0,0,120*/
			upperleft[uppernumbers] = npcName + "hand";
			upperright[uppernumbers++] = npcName + "hand2";
			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), upperleft, uppernumbers);
			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), upperright, uppernumbers);
		}



			cellRender.insertMesh(headModel, "Head", ref->model, e, blank);
			//cellRender.insertMesh ("meshes\\" + neck->model, Ogre::Vector3( 0, 0, 120), axis, Ogre::Radian(3.14), npcName + "neck", neckandup, neckNumbers);
	
		if(neck)
		{
			cellRender.insertMesh("meshes\\" + neck->model, "Neck", ref->model, e, blank);
		}
		//cellRender.insertMesh (headModel, Ogre::Vector3( 0, 0, 5), axis, Ogre::Radian(0), npcName + "head", neckandup, neckNumbers);
		neckandup[neckNumbers++] = npcName + "head";
		//cellRender.insertMesh (hairModel, Ogre::Vector3( 0, -1, 0), axis, Ogre::Radian(0), npcName + "hair", neckandup, neckNumbers);
		
		ref->mData.setHandle (rendering.end (ref->mData.isEnabled()));
		
    }

    void Npc::enable (const MWWorld::Ptr& ptr, MWWorld::Environment& environment) const
    {
        environment.mMechanicsManager->addActor (ptr);
    }

    void Npc::disable (const MWWorld::Ptr& ptr, MWWorld::Environment& environment) const
    {
        environment.mMechanicsManager->removeActor (ptr);
    }

    std::string Npc::getName (const MWWorld::Ptr& ptr) const
    {
        ESMS::LiveCellRef<ESM::NPC, MWWorld::RefData> *ref =
            ptr.get<ESM::NPC>();

        return ref->base->name;
    }

    MWMechanics::CreatureStats& Npc::getCreatureStats (const MWWorld::Ptr& ptr) const
    {
        if (!ptr.getRefData().getCreatureStats().get())
        {
            boost::shared_ptr<MWMechanics::CreatureStats> stats (
                new MWMechanics::CreatureStats);

            ESMS::LiveCellRef<ESM::NPC, MWWorld::RefData> *ref = ptr.get<ESM::NPC>();

            stats->mAttributes[0].set (ref->base->npdt52.strength);
            stats->mAttributes[1].set (ref->base->npdt52.intelligence);
            stats->mAttributes[2].set (ref->base->npdt52.willpower);
            stats->mAttributes[3].set (ref->base->npdt52.agility);
            stats->mAttributes[4].set (ref->base->npdt52.speed);
            stats->mAttributes[5].set (ref->base->npdt52.endurance);
            stats->mAttributes[6].set (ref->base->npdt52.personality);
            stats->mAttributes[7].set (ref->base->npdt52.luck);
            stats->mDynamic[0].set (ref->base->npdt52.health);
            stats->mDynamic[1].set (ref->base->npdt52.mana);
            stats->mDynamic[2].set (ref->base->npdt52.fatigue);

            stats->mLevel = ref->base->npdt52.level;

            ptr.getRefData().getCreatureStats() = stats;
        }

        return *ptr.getRefData().getCreatureStats();
    }

    MWMechanics::NpcStats& Npc::getNpcStats (const MWWorld::Ptr& ptr) const
    {
        if (!ptr.getRefData().getNpcStats().get())
        {
            boost::shared_ptr<MWMechanics::NpcStats> stats (
                new MWMechanics::NpcStats);

            ESMS::LiveCellRef<ESM::NPC, MWWorld::RefData> *ref = ptr.get<ESM::NPC>();

            if (!ref->base->faction.empty())
            {
                // TODO research how initial rank is stored. The information in loadnpc.hpp are at
                // best very unclear.
                stats->mFactionRank[ref->base->faction] = 0;
            }

            for (int i=0; i<27; ++i)
                stats->mSkill[i].setBase (ref->base->npdt52.skills[i]);

            ptr.getRefData().getNpcStats() = stats;
        }

        return *ptr.getRefData().getNpcStats();
    }

    boost::shared_ptr<MWWorld::Action> Npc::activate (const MWWorld::Ptr& ptr,
        const MWWorld::Ptr& actor, const MWWorld::Environment& environment) const
    {
        return boost::shared_ptr<MWWorld::Action> (new MWWorld::ActionTalk (ptr));
    }

    MWWorld::ContainerStore<MWWorld::RefData>& Npc::getContainerStore (const MWWorld::Ptr& ptr)
        const
    {
        if (!ptr.getRefData().getContainerStore().get())
        {
            boost::shared_ptr<MWWorld::ContainerStore<MWWorld::RefData> > store (
                new MWWorld::ContainerStore<MWWorld::RefData>);

            // TODO add initial content

            ptr.getRefData().getContainerStore() = store;
        }

        return *ptr.getRefData().getContainerStore();
    }

    std::string Npc::getScript (const MWWorld::Ptr& ptr) const
    {
        ESMS::LiveCellRef<ESM::NPC, MWWorld::RefData> *ref =
            ptr.get<ESM::NPC>();

        return ref->base->script;
    }

    void Npc::setForceStance (const MWWorld::Ptr& ptr, Stance stance, bool force) const
    {
        MWMechanics::NpcStats& stats = getNpcStats (ptr);

        switch (stance)
        {
            case Run:

                stats.mForceRun = force;
                break;

            case Sneak:

                stats.mForceSneak = force;
                break;

            case Combat:

                throw std::runtime_error ("combat stance not enforcable for NPCs");
        }
    }

    void Npc::setStance (const MWWorld::Ptr& ptr, Stance stance, bool set) const
    {
        MWMechanics::NpcStats& stats = getNpcStats (ptr);

        switch (stance)
        {
            case Run:

                stats.mRun = set;
                break;

            case Sneak:

                stats.mSneak = set;
                break;

            case Combat:

                stats.mCombat = set;
                break;
        }
    }

    bool Npc::getStance (const MWWorld::Ptr& ptr, Stance stance, bool ignoreForce) const
    {
        MWMechanics::NpcStats& stats = getNpcStats (ptr);

        switch (stance)
        {
            case Run:

                if (!ignoreForce && stats.mForceRun)
                    return true;

                return stats.mRun;

            case Sneak:

                if (!ignoreForce && stats.mForceSneak)
                    return true;

                return stats.mSneak;

            case Combat:

                return stats.mCombat;
        }

        return false;
    }

    float Npc::getSpeed (const MWWorld::Ptr& ptr) const
    {
        return getStance (ptr, Run) ? 600 : 300; // TODO calculate these values from stats
    }

    MWMechanics::Movement& Npc::getMovementSettings (const MWWorld::Ptr& ptr) const
    {
        if (!ptr.getRefData().getMovement().get())
        {
            boost::shared_ptr<MWMechanics::Movement> movement (
                new MWMechanics::Movement);

            ptr.getRefData().getMovement() = movement;
        }

        return *ptr.getRefData().getMovement();
    }

    Ogre::Vector3 Npc::getMovementVector (const MWWorld::Ptr& ptr) const
    {
        Ogre::Vector3 vector (0, 0, 0);

        if (ptr.getRefData().getMovement().get())
        {
            vector.x = - ptr.getRefData().getMovement()->mLeftRight * 200;
            vector.y = ptr.getRefData().getMovement()->mForwardBackward * 200;

            if (getStance (ptr, Run, false))
                vector *= 2;
        }

        return vector;
    }

    void Npc::registerSelf()
    {
        boost::shared_ptr<Class> instance (new Npc);

        registerClass (typeid (ESM::NPC).name(), instance);
    }
}
