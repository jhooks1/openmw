
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
		static int counter = 0;
		static int knacounter = 0;
		
		
        //Ogre::SceneNode *chest;
        ESMS::LiveCellRef<ESM::NPC, MWWorld::RefData> *ref =
            ptr.get<ESM::NPC>();
        assert (ref->base != NULL);
		
		
		const float PI = 3.14159265;
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
		char secondtolast = bodyRaceID.at(bodyRaceID.length() - 2);
		bool female = tolower(secondtolast) == 'f';
		bool beast = bodyRaceID == "b_n_khajiit_m_" || bodyRaceID == "b_n_khajiit_f_" || bodyRaceID == "b_n_argonian_m_" || bodyRaceID == "b_n_argonian_f_";

		
        std::string headModel = "meshes\\" +
            environment.mWorld->getStore().bodyParts.find(headID)->model;

		std::string hairModel = "meshes\\" +
            environment.mWorld->getStore().bodyParts.find(hairID)->model;



        MWRender::Rendering rendering (cellRender, ref->ref);

		
		std::cout << "Race " << bodyRaceID << "\n";
		std::cout << "Name:" << ref->base->name << "\n";
		if(female)
			std::cout << "Is female\n";
		
		std::string smodel = "meshes\\base_anim.nif";
		if(beast)
			smodel = "meshes\\base_animkna.nif";
		
		std::stringstream out;
		
	
		
		
		if(counter > 99 && counter < 1000)
			out << "0";
		else if(counter > 9)
			out << "00";
		else
			out << "000";
		if(beast)
			out << knacounter++;
		else
			out << counter++;
		
		out << ">|";


		smodel += out.str();
		std::cout << "Smodel" << smodel << "\n";
	
		ref->smodel = smodel;

			
			

		//ref->model->getParentSceneNode()->showBoundingBox(true);
	
        //ref->allanim = NIFLoader::getSingletonPtr()->getAllanim();

        const ESM::BodyPart *bodyPart =
            environment.mWorld->getStore().bodyParts.search (bodyRaceID + "chest");

		

		//bodyPart->model->
		Ogre::Vector3 pos = Ogre::Vector3( 20, 20, 20);
		Ogre::Vector3 axis = Ogre::Vector3( 0, 0, 1);
		Ogre::Vector3 skirtpos = Ogre::Vector3(0, 0, 0);
		Ogre::Radian angle = Ogre::Radian(0);
		
		
		int numbers = 0;
		int uppernumbers = 0;
		int neckNumbers = 0;
		
		Ogre::Quaternion q = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(0,0,1)); //1,0,0
		Ogre::Quaternion e = Ogre::Quaternion::IDENTITY;
		Ogre::Vector3 blank = Ogre::Vector3(0,0,0);
        

		const ESM::BodyPart *upperleg = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "upper leg");
		const ESM::BodyPart *groin = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "groin");
		const ESM::BodyPart *arml = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "upper arm");  //We need two
		const ESM::BodyPart *neck = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "neck");
		const ESM::BodyPart *knee = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "knee");
		const ESM::BodyPart *ankle = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "ankle");
		const ESM::BodyPart *foot = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "foot");
		const ESM::BodyPart *feet = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "feet");
		const ESM::BodyPart *tail = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "tail");
		const ESM::BodyPart *wristl = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "wrist");  //We need two
		const ESM::BodyPart *forearml = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "forearm");  //We need two
		const ESM::BodyPart *handl = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "hand");   //We need two
		const ESM::BodyPart *hair = environment.mWorld->getStore().bodyParts.search(hairID);
		const ESM::BodyPart *head = environment.mWorld->getStore().bodyParts.search(headID);
		if(!handl)
			handl = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "hands");
		const ESM::BodyPart* claviclel = environment.mWorld->getStore().bodyParts.search (bodyRaceID + "clavicle");
		const ESM::BodyPart* clavicler = claviclel;
		const ESM::BodyPart* handr = handl;
		const ESM::BodyPart* forearmr = forearml;
		const ESM::BodyPart* wristr = wristl;
		const ESM::BodyPart* armr = arml;
		ref->groin = "";

		int pchest = 1;
		int pupperleg = 1;
		int pgroin = 1;
		int parml = 1;
		int pknee = 1;
		int pankle = 1;
		int pfoot = 1;
		int pwristl = 1;
		int pforearml = 1;
		int phandl = 1;
		int pclaviclel = 1;
		int pclavicler = 1;
		int phandr = 1;
		int pforearmr = 1;
		int pwristr = 1;
		int parmr = 1;
		int phead = 1;
		int phair = 1;
		int pneck = 1;
		int ptail = 1; 

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

		//Priorities
		//Robe 5
		//Skirt 4
		//Armor 3
		//Shoes/Pants/Shirts/Gloves 2
		//Skin 1

			//                                        y was 50
		//not x                y moves parallel to ground
		//                                        -150
		Ogre::Vector3 handPos2 = Ogre::Vector3(-8, 0, 0);
		Ogre::Vector3 handPos = Ogre::Vector3(8, 0, 0);        //40,20, -100   //20
		Ogre::Quaternion handRot = Ogre::Quaternion(Ogre::Radian(PI), Ogre::Vector3(0, 1, 0)) * Ogre::Quaternion(Ogre::Radian(PI), Ogre::Vector3(1, 0, 0));//Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(0, 1, 0)); //1,0,0         //0,1,0
		//handRot * Ogre::Quaternion(Ogre::Radian(3.14/2 ),Ogre::Vector3(1,0,0)) *  Ogre::Quaternion(Ogre::Radian(3.14/2 ),Ogre::Vector3(1,0,0));
		Ogre::Quaternion handRot2 = e; //Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(1,0,0));
		int priority = 1;
		
		while(ilistiter != ilist.end())
		{
			//std::cout << "Item:" <<ilistiter->item.toString();
			const ESM::Clothing *clothes = environment.mWorld->getStore().clothes.search(ilistiter->item.toString());
			const ESM::Armor *armor =  environment.mWorld->getStore().armors.search(ilistiter->item.toString());
			ilistiter++;
			std::vector<ESM::PartReference,std::allocator<ESM::PartReference>> apparelparts;
				std::vector<ESM::PartReference,std::allocator<ESM::PartReference>>::iterator appareliter;
			
			if(clothes){
			    if(!blglove && clothes->data.type == ESM::Clothing::LGlove)
			    {
				    blglove = true;
					priority = 2;
			    }
			    else if(!brglove && clothes->data.type == ESM::Clothing::RGlove)
			    {
				    brglove = true;
					priority = 2;
			    }
			    else if(!bpants && clothes->data.type == ESM::Clothing::Pants)
			    {
				    bpants = true;
					priority = 2;
			    }
			    else if(!brobe && clothes->data.type == ESM::Clothing::Robe)
			    {
				    brobe = true;
					priority = 5;
		pupperleg = pgroin = parml = pknee = pforearml = pclaviclel = pclavicler = pforearmr = parmr = 5;
				groin = upperleg = arml = armr =claviclel =clavicler =forearml = forearmr = knee = 0;
			    }
			    else if(!bshirt && clothes->data.type == ESM::Clothing::Shirt)
			    {
				    bshirt = true;
					priority = 2;
			    }
			    else if(!bshoes && clothes->data.type == ESM::Clothing::Shoes)
			    {
					if(beast)
						continue;
				    bshoes = true;
					priority = 2;
			    }
			    else if(!bskirt && clothes->data.type == ESM::Clothing::Skirt)
			    {
					pupperleg = pknee = 4;
					upperleg = knee = 0;
				    bskirt = true;
					priority = 4;
			    }
			    else
				    continue;
				apparelparts = clothes->parts.parts;
				appareliter = apparelparts.begin();
			}
			else if(armor)
			{
				priority = 3;
				if(armor->data.type == ESM::Armor::Boots && beast)
					continue;
				apparelparts = armor->parts.parts;
				appareliter = apparelparts.begin();
			}
			else 
				continue;
				
				
				while(apparelparts.size() > 0 && appareliter != apparelparts.end())
				{
					//std::cout << "Part: " << appareliter->male << "\n";
					//std::cout << "PartF:" << appareliter->female <<"\n";
					char marker = appareliter->part;
					const ESM::BodyPart *part = environment.mWorld->getStore().bodyParts.search (appareliter->male);
					//if(female && environment.mWorld->getStore().bodyParts.search (appareliter->female))
						//part = environment.mWorld->getStore().bodyParts.search (appareliter->female);

					
					if(true){
					
						//Cuirass represents chest, we should change this, it is confusing
				
						if(marker == ESM::PRT_Cuirass && priority > pchest)
						{
							bodyPart = part;
							pchest = priority;
						}
						else if(marker == ESM::PRT_Groin && priority > pgroin)
						{
							groin = part;
							pgroin = priority;
							
						}
						else if(marker == ESM::PRT_Head && priority > phead)
						{
							head = part;
							phead = priority;
							hair = 0;
						}
						else if(marker == ESM::PRT_Hair && priority > phair)
						{
							hair = part;
							phair = priority;
						}
						else if(marker == ESM::PRT_LAnkle && priority > pankle)
						{
							ankle = part;
							pankle = priority;
						}
						else if(marker == ESM::PRT_LFoot && priority > pfoot)
						{
							foot = part;
							pfoot = priority;
						}
			
						else if(marker == ESM::PRT_LForearm && priority > pforearml)
						{
							forearml = part;
							pforearml = priority;
						}
						else if(marker == ESM::PRT_LWrist && priority > pwristl)
						{
							wristl = part;
							pwristl = priority;
						}
						else if(marker == ESM::PRT_LHand && priority > phandl)
						{
							//handRot = Ogre::Quaternion(Ogre::Radian(PI), Ogre::Vector3(0, 1, 0));
							//handRot = handRot * Ogre::Quaternion(Ogre::Radian(PI ),Ogre::Vector3(1,0,0));
							//handRot = e;


							handl = part;
							phandl = priority;
						}
						else if(marker == ESM::PRT_LKnee && priority > pknee)
						{
							knee = part;
							pknee = priority;
						}
						else if(marker == ESM::PRT_LLeg && priority > pupperleg)
						{
							upperleg = part;
							pupperleg = priority;
						}
						else if(marker == ESM::PRT_LPauldron && priority > pclaviclel)
						{
							claviclel = part;
							pclaviclel = priority;
						}
						else if(marker == ESM::PRT_LUpperarm && priority > parml)
						{
							arml = part;
							parml = priority;
						}
						else if(marker == ESM::PRT_Neck && priority > pneck)
						{
							neck = part;
							pneck = priority;
						}
						else if(marker == ESM::PRT_RAnkle && priority > pankle)
						{
							ankle = part;
							pankle = priority;
						}
						else if(marker == ESM::PRT_RFoot && priority > pfoot)
						{
							foot = part;
							pfoot = priority;
						}
						else if(marker == ESM::PRT_RForearm && priority > pforearmr)
						{
							forearmr = part;
							pforearmr = priority;
						}
						else if(marker == ESM::PRT_RHand && priority > phandr)
						{
							//(51.5, 2, -110)
							//handRot = Ogre::Quaternion(Ogre::Radian(PI), Ogre::Vector3(0, 1, 0));
							//handRot = handRot * Ogre::Quaternion(Ogre::Radian(PI ),Ogre::Vector3(1,0,0));
							
							handr = part;
							phandr = priority;
						}
						else if(marker == ESM::PRT_RKnee && priority > pknee)
						{
							knee = part;
							pknee = priority;
						}
						else if(marker == ESM::PRT_RLeg && priority > pupperleg)
						{
							upperleg = part;
							pupperleg = priority;
						}
						else if(marker == ESM::PRT_RPauldron && priority > pclavicler)
						{
							clavicler= part;
							pclavicler = priority;
						}
						else if(marker == ESM::PRT_RUpperarm && priority > parmr)
						{
							armr = part;
							parmr = priority;
						}
						else if(marker == ESM::PRT_RWrist && priority > pwristr)
						{
							wristr = part;
							pwristr = priority;
						}
						else if(marker == ESM::PRT_Shield)
						{
							;
						}
						else if(marker == ESM::PRT_Skirt && priority > pgroin)
						{
							groin = part;
							pgroin = priority;
							if(pgroin == 4){
								ref->groin = "meshes\\" + groin->model;
							}
							skirtpos = Ogre::Vector3(0, -5, -80);
						}
						else if(marker == ESM::PRT_Tail && priority > ptail)
						{
							tail = part;
							ptail = priority;
						}
						}
						
						

						

							
				
					appareliter++;
				}
			   
		}



		//std::cout << "RACE" << bodyRaceID << "\n";

		Ogre::Vector3 pos2 = Ogre::Vector3(90, 5, 4);     //5
		std::string upperarmpath[2] = {npcName + "chest", npcName + "upper arm"};
		
		Ogre::Quaternion p = Ogre::Quaternion(Ogre::Radian(3.14), Ogre::Vector3(1, 0, 0)); //1,0,0


		if(handl)
		{
			std::string pass;
				pass = handl->model;
				
				cellRender.sendAddinToLoader("meshes\\" + pass + "|>");
				ref->lhand = "meshes\\" + pass + "|>";
				std::cout << ref->lhand << "\n";
			//ref->handl = 
				//cellRender.insertMeshInsideBase("meshes\\" + pass + "|>", "Left Hand", ref->model, handRot, handPos);
				//ref->lhandmodel = cellRender.insertMesh("meshes\\" + pass + "|>", "Test", ref->model, handRot, handPos);
		}
		else
			ref->lhand = "";//ref->handl = 0;
		if(handr){
			//ref->handr = cellRender.insertMesh("meshes\\" + handr->model + "|?", "Right Hand", ref->model, handRot, handPos2);
			cellRender.sendAddinToLoader("meshes\\" + handr->model + "|?");
				ref->rhand = "meshes\\" + handr->model + "|?";
				std::cout << ref->rhand << "\n";
		}
		else
			ref->rhand = "";
		if (bodyPart){
				
			
				ref->chest = "meshes\\" + bodyPart->model + "|\"";
				std::cout << ref->chest << "\n";
				cellRender.sendAddinToLoader(ref->chest);
		}
		else
			ref->chest = "";

		if (tail) {
			//Ogre::Quaternion p2 = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0, 0, 1)); //1,0,0
			//p2 = p2 * Ogre::Quaternion(Ogre::Radian(-3.14 / 2), Ogre::Vector3(0, 1, 0));
			//Ogre::Vector3 tailpos = Ogre::Vector3(0, 75, 0);
			ref->tail = "meshes\\" + tail->model + "|*";
			cellRender.sendAddinToLoader(ref->tail);
			//ref->tail = cellRender.insertMesh("meshes\\" + tail->model + "|*", "Chest", ref->model, q, chestPos);
			//ref->tail = cellRender.insertMesh("meshes\\" + tail->model + "|*", "Chest", ref->model, e, blank);
		}
		else
		{
			ref->tail = "";
			//ref->tail = 0;
			//Ogre::Quaternion p2 = Ogre::Quaternion(Ogre::Radian(3.14 / 2), Ogre::Vector3(0, 0, 1)); //1,0,0
			//p2 = p2 * Ogre::Quaternion(Ogre::Radian(-3.14 / 2), Ogre::Vector3(0, 1, 0));
			//Ogre::Vector3 tailpos = Ogre::Vector3(0, 75, 0);
			//cellRender.insertMesh("meshes\\" + groin->model, "Groin", ref->model, p2, tailpos);
		}

		if(beast && bodyRaceID.compare("b_n_khajiit_m_") == 0)
			feet = foot;

		if(feet){
			//Ogre::Vector3 pos = Ogre::Vector3(-6,5,0);  //y is up
			//Ogre::Vector3 pos2 = Ogre::Vector3(6,5,0);
			ref->lfoot = "meshes\\" + feet->model + "|:";
			ref->rfoot = "meshes\\" + feet->model + "|<";

			cellRender.sendAddinToLoader(ref->lfoot);
			cellRender.sendAddinToLoader(ref->rfoot);
			//cellRender.scaleMesh(Ogre::Vector3(1, -1, 1), addresses, numbers);
		}
		else{
			ref->lfoot = "";
			ref->rfoot = "";
		}
		if(ref->groin != "" && groin){
			std::cout << "Sending to loader";
			cellRender.sendAddinToLoader(ref->groin);
		}
		
		
		ref->model = cellRender.insertAndDeliverMesh(smodel);
		
		
		
		
		//cellRender.insertMesh("meshes\\b\\B_N_Breton_F_Foot.nif", Ogre::Vector3(-1,1,1));        //1, -1, 1
		//cellRender.insertMesh(headModel, "Bip01 Head", ref->model, q * p,Ogre::Vector3(-75, 20, 2));
		Ogre::Vector3 chestPos = Ogre::Vector3(0, 3.5, -98);
		
		
        if (ref->groin == "" && groin){
			cellRender.insertMesh("meshes\\" + groin->model, "Groin", ref->model, handRot, skirtpos);

		}
		
		
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
		
		
		 
		if (armr){
			cellRender.insertMesh("meshes\\" + armr->model, "Right Upper Arm", ref->model, e, blank);
		}
		if(arml){
			cellRender.insertMesh("meshes\\" + arml->model + "*|", "Left Upper Arm", ref->model, e, blank);
		}

		if (forearmr)
		{
				cellRender.insertMesh("meshes\\" + forearmr->model, "Right Forearm", ref->model, e, blank);
		}
		if(forearml)
			cellRender.insertMesh("meshes\\" + forearml->model + "*|", "Left Forearm", ref->model, e, blank);

		if (wristr)
		{
			cellRender.insertMesh("meshes\\" + wristr->model, "Right Wrist", ref->model, e, blank);
		}

		if(wristl)
				cellRender.insertMesh("meshes\\" + wristl->model + "*|", "Left Wrist", ref->model, e, blank);
		

	
		

		if(claviclel)
			cellRender.insertMesh("meshes\\" + claviclel->model + "*|", "Left Clavicle", ref->model, e, blank);
		if(clavicler)
			cellRender.insertMesh("meshes\\" + clavicler->model , "Right Clavicle", ref->model, e, blank);
	
	
		if(neck)
		{
			cellRender.insertMesh("meshes\\" + neck->model, "Neck", ref->model, e, blank);
		}
		if(head)
			cellRender.insertMesh("meshes\\" + head->model, "Head", ref->model, e, blank);
		if(hair)
			cellRender.insertMesh("meshes\\" + hair->model, "Head", ref->model, e, blank);

		
	

			
			std::cout << "F\n";



		
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
