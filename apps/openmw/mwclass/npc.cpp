
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
		
		//Part selection on last character of the file string
		//  " Tri Chest
		//  * Tri Tail
		//  : Tri Left Foot
		//  < Tri Right Foot
		//  > Tri Left Hand
		//  ? Tri Right Hand
		//  | Normal

		//Mirroring Parts on second to last character
		//suffix == '*'
		//	vector = Ogre::Vector3(-1,1,1);
		//  suffix == '?'
		//	vector = Ogre::Vector3(1,-1,1);
		//  suffix == '<'
		//	vector = Ogre::Vector3(1,1,-1);




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

		
		std::cout << "Race " << bodyRaceID << "\n";
		std::cout << "Name:" << ref->base->name << "\n";
		

		if(bodyRaceID == "b_n_khajiit_m_" || bodyRaceID == "b_n_khajiit_f_" || bodyRaceID == "b_n_argonian_m_" || bodyRaceID == "b_n_argonian_f_")
			ref->model = cellRender.insertAndDeliverMesh("meshes\\base_animkna.nif");
		else
			ref->model = cellRender.insertAndDeliverMesh("meshes\\base_anim.nif");

		//ref->model->getParentSceneNode()->showBoundingBox(true);
	
        ref->allanim = NIFLoader::getSingletonPtr()->getAllanim();

        const ESM::BodyPart *bodyPart =
            environment.mWorld->getStore().bodyParts.search (bodyRaceID + "chest");

		

		//bodyPart->model->
		Ogre::Vector3 pos = Ogre::Vector3( 20, 20, 20);
		Ogre::Vector3 axis = Ogre::Vector3( 0, 0, 1);
		Ogre::Radian angle = Ogre::Radian(0);
		
		
		int numbers = 0;
		int uppernumbers = 0;
		int neckNumbers = 0;
		
		Ogre::Quaternion q = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(0,0,1)); //1,0,0
		Ogre::Quaternion e = Ogre::Quaternion::IDENTITY;
		Ogre::Vector3 blank = Ogre::Vector3(0,0,0);
        if (bodyPart){
			
			Ogre::Vector3 chestPos = Ogre::Vector3(0, 3.5, -98);

				cellRender.insertMesh("meshes\\" + bodyPart->model + "|\"", "Chest", ref->model, q, chestPos);
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
		const ESM::BodyPart *hand = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "hand");
		const ESM::BodyPart *hands = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "hands");




		//std::cout << "RACE" << bodyRaceID << "\n";

		Ogre::Vector3 pos2 = Ogre::Vector3(90, 5, 4);     //5
		std::string upperarmpath[2] = {npcName + "chest", npcName + "upper arm"};
		
		Ogre::Quaternion p = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(1, 0, 0)); //1,0,0
		q  = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(0, 0, 1)); //1,0,0

		//cellRender.insertMesh("meshes\\b\\B_N_Breton_F_Foot.nif", Ogre::Vector3(-1,1,1));        //1, -1, 1
		//cellRender.insertMesh(headModel, "Bip01 Head", ref->model, q * p,Ogre::Vector3(-75, 20, 2));
        if (groin){
			cellRender.insertMesh("meshes\\" + groin->model, "Groin", ref->model, e, blank);

		}
		if (tail) {
			Ogre::Quaternion p2 = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0, 0, 1)); //1,0,0
			p2 = p2 * Ogre::Quaternion(Ogre::Radian(-3.14 / 2), Ogre::Vector3(0, 1, 0));
			Ogre::Vector3 tailpos = Ogre::Vector3(0, 75, 0);
			cellRender.insertMesh("meshes\\" + tail->model + "|*", "Bip01 Tail", ref->model, p2, tailpos);
		}
		else
		{
			
			Ogre::Quaternion p2 = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0, 0, 1)); //1,0,0
			p2 = p2 * Ogre::Quaternion(Ogre::Radian(-3.14 / 2), Ogre::Vector3(0, 1, 0));
			Ogre::Vector3 tailpos = Ogre::Vector3(0, 75, 0);
			//cellRender.insertMesh("meshes\\" + groin->model, "Groin", ref->model, p2, tailpos);
		}
		
		q = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0, 1, 0)); //1,0,0
		p = Ogre::Quaternion(Ogre::Radian(3.14 ), Ogre::Vector3(0, 0, 1));

		if(upperleg){
			cellRender.insertMesh("meshes\\" + upperleg->model + "*|", "Left Upper Leg", ref->model, e, blank);
			cellRender.insertMesh("meshes\\" + upperleg->model, "Right Upper Leg", ref->model, e, blank);
			
		}
		if(knee)
		{
			cellRender.insertMesh("meshes\\" + knee->model + "*|", "Left Knee", ref->model, e, blank);  //e
			cellRender.insertMesh("meshes\\" + knee->model, "Right Knee", ref->model, e,blank);   //e
			
		}
		if(ankle){
			
			cellRender.insertMesh("meshes\\" + ankle->model + "*|", "Left Ankle", ref->model, e, blank); //Ogre::Quaternion(Ogre::Radian(3.14 / 4), Ogre::Vector3(1, 0, 0)),blank); //1,0,0, blank);
			cellRender.insertMesh("meshes\\" + ankle->model, "Right Ankle", ref->model, e,blank);
		}
		if(foot){
			if(bodyRaceID.compare("b_n_khajiit_m_") == 0)
			{
				feet = foot;
			}
			else
			{
				cellRender.insertMesh("meshes\\" + foot->model, "Right Foot", ref->model, e, blank);
				cellRender.insertMesh("meshes\\" + foot->model + "*|", "Left Foot", ref->model, e, blank);
			}
		}
		if(feet){
			Ogre::Vector3 pos = Ogre::Vector3(-6,5,0);  //y is up
			Ogre::Vector3 pos2 = Ogre::Vector3(6,5,0);

			cellRender.insertMesh("meshes\\" + feet->model + "|:", "Left Foot", ref->model, p, pos);
			cellRender.insertMesh("meshes\\" + feet->model + "|<", "Right Foot", ref->model, p, pos2);
			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), addresses, numbers);
		}
		
		 
		if (arm){
			cellRender.insertMesh("meshes\\" + arm->model, "Right Upper Arm", ref->model, e, blank);
			cellRender.insertMesh("meshes\\" + arm->model + "*|", "Left Upper Arm", ref->model, e, blank);
		}

		if (forearm)
		{
				cellRender.insertMesh("meshes\\" + forearm->model, "Right Forearm", ref->model, e, blank);
				cellRender.insertMesh("meshes\\" + forearm->model + "*|", "Left Forearm", ref->model, e, blank);
		}

		if (wrist)
		{

			
				cellRender.insertMesh("meshes\\" + wrist->model, "Right Wrist", ref->model, e, blank);
				cellRender.insertMesh("meshes\\" + wrist->model + "*|", "Left Wrist", ref->model, e, blank);
			

		}
		

		//                                        y was 50
		//not x                y moves parallel to ground
		//                                        -150
		Ogre::Vector3 handPos2 = Ogre::Vector3(51, 2, -110);
		Ogre::Vector3 handPos = Ogre::Vector3(-51, 2, -110);        //40,20, -100   //20
		Ogre::Quaternion handRot = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(0, 1, 0)); //1,0,0         //0,1,0
		handRot = handRot * Ogre::Quaternion(Ogre::Radian(3.14/2 ),Ogre::Vector3(1,0,0)) *  Ogre::Quaternion(Ogre::Radian(3.14/2 ),Ogre::Vector3(1,0,0));
		Ogre::Quaternion handRot2 = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(1,0,0));
		if(hand)
		{
			std::string pass;
				pass = hand->model;

			cellRender.insertMesh("meshes\\" + pass + "|>", "Left Hand", ref->model, handRot, handPos);
			cellRender.insertMesh("meshes\\" + pass + "|?", "Right Hand", ref->model, handRot, handPos2);
		}
		if(hands)
		{
			std::string pass;
				pass =hands->model;	
			
			cellRender.insertMesh("meshes\\" + pass + "|>", "Left Hand", ref->model, handRot, handPos);
			cellRender.insertMesh("meshes\\" + pass + "|?", "Right Hand", ref->model, handRot, handPos2);
		}



			cellRender.insertMesh(headModel, "Head", ref->model, e, blank);
			cellRender.insertMesh(hairModel, "Head", ref->model, e, blank);
	
		if(neck)
		{
			cellRender.insertMesh("meshes\\" + neck->model, "Neck", ref->model, e, blank);
		}


		std::vector<ESM::ContItem, std::allocator<ESM::ContItem>> ilist = ref->base->inventory.list;
		std::vector<ESM::ContItem, std::allocator<ESM::ContItem>>::iterator ilistiter = ilist.begin();

		bool blglove = false;
		bool brglove = false;
		bool bpants = false;
		bool brobe = false;
		bool bshirt = false;
		bool bshoes = false;
		bool bskirt = false;
	
		//Belts, Rings, and Amulets exist as meshes that you can drop in the world,
		//but they don't actually wrap around the npc.  They do not need to be rendered.

		
		while(ilistiter != ilist.end())
		{
			//std::cout << "Item:" <<ilistiter->item.toString();
			const ESM::Clothing *clothes = environment.mWorld->getStore().clothes.search(ilistiter->item.toString());
			ilistiter++;
			
			if(clothes){
			    if(!blglove && clothes->data.type == ESM::Clothing::LGlove)
			    {
				    blglove = true;
			    }
			    else if(!brglove && clothes->data.type == ESM::Clothing::RGlove)
			    {
				    brglove = true;
			    }
			    else if(!bpants && clothes->data.type == ESM::Clothing::Pants)
			    {
				    bpants = true;
			    }
			    else if(!brobe && clothes->data.type == ESM::Clothing::Robe)
			    {
				    brobe = true;
			    }
			    else if(!bshirt && clothes->data.type == ESM::Clothing::Shirt)
			    {
				    bshirt = true;
			    }
			    else if(!bshoes && clothes->data.type == ESM::Clothing::Shoes)
			    {
				    bshoes = true;
			    }
			    else if(!bskirt && clothes->data.type == ESM::Clothing::Skirt)
			    {
				    bskirt = true;
			    }
			    else
				    continue;

				
				std::vector<ESM::PartReference,std::allocator<ESM::PartReference>> clothingparts = clothes->parts.parts;
				std::vector<ESM::PartReference,std::allocator<ESM::PartReference>>::iterator clothingpartsiter = clothingparts.begin();
				
				while(clothingpartsiter != clothingparts.end())
				{
					std::cout << "Part: " << clothingpartsiter->male << "\n";
					char marker = clothingpartsiter->part;
					const ESM::BodyPart *part = environment.mWorld->getStore().bodyParts.search (clothingpartsiter->male);
					
					
					if(part)
					{
						std::string bonename = "";
						switch (marker)
						{

							
							
							//Cuirass represents chest, we should change this, it is confusing
							case ESM::PRT_Cuirass:
								bonename = "Chest";
								break;
							case ESM::PRT_Groin:
								bonename = "Groin";
								break;
							case ESM::PRT_Head :
								bonename = "Head";
								break;
							case ESM::PRT_LAnkle:
								bonename = "Left Ankle";
								break;
							case ESM::PRT_LFoot :
								bonename = "Left Foot";
								break;
							case ESM::PRT_LForearm :
								bonename = "Left Forearm";
								break;
							case ESM::PRT_LHand :
								bonename = "Left Hand";
								break;
							case ESM::PRT_LKnee :
								bonename = "Left Knee";
								break;
							case ESM::PRT_LLeg :
								bonename = "Left Upper Leg";
								break;
							case ESM::PRT_LPauldron :
								bonename = "Left Clavicle";
								break;
							case ESM::PRT_LUpperarm :
								bonename = "Left Upper Arm";
								break;
							case ESM::PRT_LWrist :
								bonename = "Left Wrist";
								break;
							case ESM::PRT_Neck:
								bonename = "Neck";
								break;
							case ESM::PRT_RAnkle:
								bonename = "Right Ankle";
								break;
							case ESM::PRT_RFoot:
								bonename = "Right Foot";
								break;
							case ESM::PRT_RForearm:
								bonename = "Right Forearm";
								break;
							case ESM::PRT_RHand:
								bonename = "Right Hand";
								break;
							case ESM::PRT_RKnee:
								bonename = "Right Knee";
								break;
							case ESM::PRT_RLeg:
								bonename = "Right Upper Leg";
								break;
							case ESM::PRT_RPauldron:
								bonename = "Right Clavicle";
								break;
							case ESM::PRT_RWrist:
								bonename = "Right Wrist";
								break;
							case ESM::PRT_Shield:
								bonename = "Shield";
								break;
							case ESM::PRT_Skirt:
								bonename = "Groin";
								break;
							case ESM::PRT_Tail:
								bonename = "Tail";
								break;
						}
							
					}
					clothingpartsiter++;
				}
			    //Insert the clothes on top of body parts
			}
	

			
			
		}


		
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
