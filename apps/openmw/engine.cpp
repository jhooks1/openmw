#include "engine.hpp"
#include "components/esm/loadcell.hpp"

#include <cassert>

#include <iostream>
#include <utility>

#include <OgreVector3.h>
#include <Ogre.h>

#include "components/esm/records.hpp"
#include <components/nifogre/ogre_nif_loader.hpp>
#include <components/esm_store/cell_store.hpp>
#include <components/misc/fileops.hpp>
#include <components/bsa/bsa_archive.hpp>
#include <components/esm/loadregn.hpp>
#include <components/esm/esm_reader.hpp>
#include <openengine/gui/manager.hpp>
#include "mwgui/window_manager.hpp"

#include "mwinput/inputmanager.hpp"

#include "mwscript/scriptmanager.hpp"
#include "mwscript/compilercontext.hpp"
#include "mwscript/interpretercontext.hpp"
#include "mwscript/extensions.hpp"
#include "mwscript/globalscripts.hpp"

#include "mwsound/soundmanager.hpp"

#include "mwworld/world.hpp"
#include "mwworld/ptr.hpp"
#include "mwworld/environment.hpp"
#include "mwworld/class.hpp"
#include "mwworld/player.hpp"
#include <components/nif/nif_file.hpp>
#include <components/nif/node.hpp>
#include <components/nif/data.hpp>
#include <components/nif/property.hpp>
#include <components/nif/controller.hpp>
#include <components/nif/extra.hpp>

#include "mwclass/classes.hpp"

#include "mwdialogue/dialoguemanager.hpp"

#include "mwmechanics/mechanicsmanager.hpp"

#include <OgreRoot.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <OSX/macUtils.h>
#endif

#include <MyGUI_WidgetManager.h>
#include "mwgui/class.hpp"
#include "path.hpp"

#include "components/nifbullet/bullet_nif_loader.hpp"

//using namespace ESM;

//each creature/npc is assigned one aindex



void OMW::Engine::executeLocalScripts()
{
    for (MWWorld::World::ScriptList::const_iterator iter (
        mEnvironment.mWorld->getLocalScripts().begin());
        iter!=mEnvironment.mWorld->getLocalScripts().end(); ++iter)
    {
        if (mIgnoreLocalPtr.isEmpty() || mIgnoreLocalPtr!=iter->second)
        {
            MWScript::InterpreterContext interpreterContext (mEnvironment,
                &iter->second.getRefData().getLocals(), MWWorld::Ptr (iter->second));
            mScriptManager->run (iter->first, interpreterContext);

            if (mEnvironment.mWorld->hasCellChanged())
                break;
        }
    }

    mIgnoreLocalPtr = MWWorld::Ptr();
}

void OMW::Engine::clearIndices(){
	creaturea.clear();
	npca.clear();
}

void OMW::Engine::handleAnimationTransform(Nif::NiKeyframeData& data, aindex &a, int slot) {
	
	if(a.time < data.getStartTime())
	{
		return;
	}
    Ogre::SkeletonInstance *skel = a.skel;
    if(skel->hasBone(data.getBonename())){
        Ogre::Bone* bone = skel->getBone(data.getBonename());
	
        float x;
	    std::vector<Ogre::Quaternion> quats = data.getQuat();

        std::vector<float> ttime = data.gettTime();
        std::vector<float>::iterator ttimeiter = ttime.begin();
        std::vector<Ogre::Vector3> translist1 = data.getTranslist1();
        //std::vector<Ogre::Vector3>::iterator transiter = translist1.begin();
        std::vector<Ogre::Vector3> translist2 = data.getTranslist2();
        //std::vector<Ogre::Vector3>::iterator transiter2 = translist2.begin();
        std::vector<Ogre::Vector3> translist3 = data.getTranslist3();
    
        std::vector<float> rtime = data.getrTime();
        int rindexJ = 0;
	    timeIndex(a.time, rtime, a.rindexI[slot], rindexJ, x);
	    int tindexJ = 0;

        timeIndex(a.time, ttime, a.tindexI[slot], tindexJ, x);
	
	    if(translist1.size() > 0){
            Ogre::Vector3 v1 = translist1[a.tindexI[slot]];
            Ogre::Vector3 v2 = translist1[tindexJ];
            Ogre::Vector3 t = v1 + (v2 - v1) * x;
	        bone->setPosition(t); 
	    }
	
	    if(quats.size() > 0){
		    Ogre::Quaternion r = Ogre::Quaternion::Slerp(x, quats[a.rindexI[slot]], quats[rindexJ], true);
		    bone->setOrientation(r);
	    }


        skel->getManualBonesDirty();
        skel->_updateTransforms();
	    skel->_notifyManualBonesDirty();
	
	    Ogre::Entity* ent = a.base;

        ent->getAllAnimationStates()->_notifyDirty();
        ent->_updateAnimation();
	    ent->_notifyMoved();
	}  
}

bool OMW::Engine::timeIndex( float time, std::vector<float> times, int & i, int & j, float & x ){
	int count;
	if (  (count = times.size()) > 0 )
	{
		if ( time <= times[0] )
		{
			i = j = 0;
			x = 0.0;
			return true;
		}
		if ( time >= times[count - 1] )
		{
			i = j = count - 1;
			x = 0.0;
			return true;
		}
		
		if ( i < 0 || i >= count )
			i = 0;
		
		float tI = times[i];
		if ( time > tI )
		{
			j = i + 1;
			float tJ;
			while ( time >= ( tJ = times[j]) )
			{
				i = j++;
				tI = tJ;
			}
			x = ( time - tI ) / ( tJ - tI );
			return true;
		}
		else if ( time < tI )
		{
			j = i - 1;
			float tJ;
			while ( time <= ( tJ = times[j] ) )
			{
				i = j--;
				tI = tJ;
			}
			x = ( time - tI ) / ( tJ - tI );
			return true;
		}
		else
		{
			j = i;
			x = 0.0;
			return true;
		}
	}
	else
		return false;

}
void OMW::Engine::handleShapes(std::vector<Nif::NiTriShapeCopy> allshapes, Ogre::Entity* creaturemodel, Ogre::SkeletonInstance *skel)
{
	std::vector<Nif::NiTriShapeCopy>::iterator allshapesiter;
	for(allshapesiter = allshapes.begin(); allshapesiter != allshapes.end(); allshapesiter++)
		{
			Nif::NiTriShapeCopy copy = *allshapesiter;
			std::map<unsigned int, bool> vertices;
			std::map<unsigned int, bool> normals;
			std::vector<Nif::NiSkinData::BoneInfoCopy> boneinfovector =  copy.boneinfo;
	
			//std::cout << "Name " << copy.sname << "\n";

			    if(boneinfovector.size() > 0){

				Ogre::HardwareVertexBufferSharedPtr vbuf = creaturemodel->getMesh()->getSubMesh(copy.sname)->vertexData->vertexBufferBinding->getBuffer(0);
		            Ogre::Real* pReal = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
			       Ogre::HardwareVertexBufferSharedPtr vbufNormal = creaturemodel->getMesh()->getSubMesh(copy.sname)->vertexData->vertexBufferBinding->getBuffer(1);
		            Ogre::Real* pRealNormal = static_cast<Ogre::Real*>(vbufNormal->lock(Ogre::HardwareBuffer::HBL_NORMAL));
				for (int i = 0; i < boneinfovector.size(); i++)
				{
					Nif::NiSkinData::BoneInfoCopy boneinfo = boneinfovector[i];
					if(skel->hasBone(boneinfo.bonename)){
					Ogre::Bone *bonePtr = skel->getBone(boneinfo.bonename);
					Ogre::Vector3 vecPos = bonePtr->_getDerivedPosition() + bonePtr->_getDerivedOrientation() * boneinfo.trafo.trans;
					Ogre::Quaternion vecRot = bonePtr->_getDerivedOrientation() * boneinfo.trafo.rotation;
					//std::cout << "Bone" << bonePtr->getName() << "\n";
					 for (unsigned int j=0; j < boneinfo.weights.size(); j++)
					 {
						  unsigned int verIndex = boneinfo.weights[j].vertex;
						  if(vertices.find(verIndex) == vertices.end())
						  {
							  Ogre::Vector3 absVertPos = vecPos + vecRot * copy.vertices[verIndex];
							  absVertPos = absVertPos * boneinfo.weights[j].weight;
							  vertices[verIndex] = true;
							   Ogre::Real* addr = (pReal + 3 * verIndex);
							  *addr = absVertPos.x;
							  *(addr+1) = absVertPos.y;
				              *(addr+2) = absVertPos.z;

								//std::cout << "Vertex" << vertices[verIndex] << "\n";
						  }
						  else 
						  {
							
							   Ogre::Vector3 absVertPos = vecPos + vecRot * copy.vertices[verIndex];
							   absVertPos = absVertPos * boneinfo.weights[j].weight;
							   Ogre::Vector3 old = Ogre::Vector3(pReal + 3 * verIndex);
							   absVertPos = absVertPos + old;
							   Ogre::Real* addr = (pReal + 3 * verIndex);
							  *addr = absVertPos.x;
							  *(addr+1) = absVertPos.y;
				              *(addr+2) = absVertPos.z;
							  //std::cout << "Vertex" << verIndex << "Weight: " << boneinfo.weights[i].weight << "was seen twice\n";

						  }
						  
						  if(normals.find(verIndex) == normals.end())
						  {
							  Ogre::Vector3 absNormalsPos = vecRot * copy.normals[verIndex];
							  absNormalsPos = absNormalsPos * boneinfo.weights[j].weight;
							  normals[verIndex] = true;
							  Ogre::Real* addr = (pRealNormal + 3 * verIndex);
							  *addr = absNormalsPos.x;
				              *(addr+1) = absNormalsPos.y;
				              *(addr+2) = absNormalsPos.z;
						  }
						  else
						  {
							   Ogre::Vector3 absNormalsPos = vecRot * copy.normals[verIndex];
							  absNormalsPos = absNormalsPos * boneinfo.weights[j].weight;
							 Ogre::Vector3 old = Ogre::Vector3(pRealNormal + 3 * verIndex);
							 absNormalsPos = absNormalsPos + old;

							  Ogre::Real* addr = (pRealNormal + 3 * verIndex);
							  *addr = absNormalsPos.x;
				              *(addr+1) = absNormalsPos.y;
				              *(addr+2) = absNormalsPos.z;

						  }

					 }
				}
				
					
				}
				   
				   
				}
				else
				{
					//Ogre::Bone *bonePtr = creaturemodel->getSkeleton()->getBone(copy.bonename);
					Ogre::Quaternion shaperot = copy.trafo.rotation;
					Ogre::Vector3 shapetrans = copy.trafo.trans;
					float shapescale = copy.trafo.scale;
					std::vector<std::string> boneSequence = copy.boneSequence;
					std::vector<std::string>::iterator boneSequenceIter = boneSequence.begin();
					Ogre::Vector3 transmult;
						Ogre::Quaternion rotmult;
						float scale;
					if(creaturemodel->getSkeleton()->hasBone(*boneSequenceIter)){
					Ogre::Bone *bonePtr = creaturemodel->getSkeleton()->getBone(*boneSequenceIter);
					
						
					

						transmult = bonePtr->getPosition();
						rotmult = bonePtr->getOrientation();
						scale = bonePtr->getScale().x;
						boneSequenceIter++;
					    for(; boneSequenceIter != boneSequence.end(); boneSequenceIter++)
					    {
							if(creaturemodel->getSkeleton()->hasBone(*boneSequenceIter)){
							Ogre::Bone *bonePtr = creaturemodel->getSkeleton()->getBone(*boneSequenceIter);
								// Computes C = B + AxC*scale
								transmult = transmult + rotmult * bonePtr->getPosition();
								rotmult = rotmult * bonePtr->getOrientation();
								scale = scale * bonePtr->getScale().x;
							}
						    //std::cout << "Bone:" << *boneSequenceIter << "   ";
					    }
						transmult = transmult + rotmult * shapetrans;
						rotmult = rotmult * shaperot;
						scale = shapescale * scale;

						//std::cout << "Position: " << transmult << "Rotation: " << rotmult << "\n";
					}
					else
					{
						transmult = shapetrans;
						rotmult = shaperot;
						scale = shapescale;
					}
		
					Ogre::HardwareVertexBufferSharedPtr vbuf = creaturemodel->getMesh()->getSubMesh(copy.sname)->vertexData->vertexBufferBinding->getBuffer(0);
		            Ogre::Real* pReal = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
			        Ogre::HardwareVertexBufferSharedPtr vbufNormal = creaturemodel->getMesh()->getSubMesh(copy.sname)->vertexData->vertexBufferBinding->getBuffer(1);
		            Ogre::Real* pRealNormal = static_cast<Ogre::Real*>(vbufNormal->lock(Ogre::HardwareBuffer::HBL_DISCARD));
					std::vector<Ogre::Vector3> allvertices = copy.vertices;
					std::vector<Ogre::Vector3> allnormals = copy.normals;

					// Computes C = B + AxC*scale
					 // final_vector = old_vector + old_rotation*new_vector*old_scale/
					
					for(int i = 0; i < allvertices.size(); i++){
						Ogre::Vector3 current = transmult + rotmult * allvertices[i];
						Ogre::Real* addr = pReal + i * 3;
					    *addr = current.x;
						*(addr+1) = current.y;
						*(addr + 2) = current.z;

					}
					for(int i = 0; i < allnormals.size(); i++){
						Ogre::Vector3 current =rotmult * allnormals[i];
						Ogre::Real* addr = pRealNormal + i * 3;
					    *addr = current.x;
						*(addr+1) = current.y;
						*(addr + 2) = current.z;

					}

				}
		}

		for(allshapesiter = allshapes.begin(); allshapesiter != allshapes.end(); allshapesiter++)
		{
			Nif::NiTriShapeCopy copy = *allshapesiter;
			Ogre::HardwareVertexBufferSharedPtr vbuf = creaturemodel->getMesh()->getSubMesh(copy.sname)->vertexData->vertexBufferBinding->getBuffer(0);

			        Ogre::HardwareVertexBufferSharedPtr vbufNormal = creaturemodel->getMesh()->getSubMesh(copy.sname)->vertexData->vertexBufferBinding->getBuffer(1);
					vbuf->unlock();
					vbufNormal->unlock();
		}
}

bool OMW::Engine::frameStarted(const Ogre::FrameEvent& evt)
{
    if(mShowFPS)
    {
        mEnvironment.mWindowManager->wmSetFPS(mOgre.getFPS());
    }

    if(mUseSound && !(mEnvironment.mSoundManager->isMusicPlaying()))
    {
        // Play some good 'ol tunes
        mEnvironment.mSoundManager->startRandomTitle();
    }
	
	

    std::string effect;

    MWWorld::Ptr::CellStore *current = mEnvironment.mWorld->getPlayer().getPlayer().getCell();
	
	
	
	ESMS::CellRefList<ESM::Creature,MWWorld::RefData>::List creatureData = (current->creatures).list;
	ESMS::CellRefList<ESM::Creature,MWWorld::RefData>::List::iterator creaturedataiter = creatureData.begin();

		ESMS::CellRefList<ESM::NPC,MWWorld::RefData>::List npcdata = (current->npcs).list;
	ESMS::CellRefList<ESM::NPC,MWWorld::RefData>::List::iterator npcdataiter = npcdata.begin();

	first2 = true;

	
	if(creatureData.size() > 0 && creaturea.size() > 0)
	{
		if(creaturedataiter->model != creaturea[0].base)
			clearIndices();
	}
	else if(npcdata.size() > 0 && npca.size() > 0)
	{
		if(npcdataiter->model != npca[0].base)
			clearIndices();
	}
	



	for(int i = 0; i < creatureData.size(); i++)
	{
		
		ESMS::LiveCellRef<ESM::Creature,MWWorld::RefData> item = *creaturedataiter;

		std::vector<Nif::NiKeyframeData> allanim = NIFLoader::getSingletonPtr()->getAnim(item.smodel);
		std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.smodel);
		prev = item.smodel;
		std::vector<Nif::NiKeyframeData>::iterator allanimiter;
		std::vector<Nif::NiTriShapeCopy>::iterator allshapesiter;
		
		if(creaturea.size() == i)
		{
			

			aindex a;
			
			a.time = 0.0 ;
			a.first = true;
			a.base = item.model;
			a.skel = a.base->getSkeleton();
			//a.absoluterot = creaturemodel->getParentSceneNode()->getOrientation();
			//a.absolutepos = creaturemodel->getParentSceneNode()->getPosition();
			//a.initialrot = creaturemodel->getSkeleton()->getBone("Bip01")->getOrientation();
			
			for(int init = 0; init < allanim.size(); init++){
				a.rindexI.push_back(0);
				//a.rindexJ.push_back(0);
				a.tindexI.push_back(0);
				//a.tindexJ.push_back(0);
			}
			creaturea.push_back(a);
			
		}

		aindex& r = creaturea[i];
		r.time += evt.timeSinceLastFrame;
		
		if(!mOgre.getCamera()->isVisible(item.model->getWorldBoundingBox())){
		  creaturedataiter++;
		   continue;
	   }
		
		
	   Ogre::Bone* b =r.skel->getRootBone();
	   b->setOrientation(.3,.3,.3,.3);   //This is a trick
	   r.skel->getManualBonesDirty();
    r.skel->_updateTransforms();
	r.skel->_notifyManualBonesDirty();
	
	Ogre::Entity* ent = r.base;

    ent->getAllAnimationStates()->_notifyDirty();
    ent->_updateAnimation();
	ent->_notifyMoved();
	
		int o = 0;
		for (allanimiter = allanim.begin(); allanimiter != allanim.end(); allanimiter++)
		{
			handleAnimationTransform(*allanimiter, r, o);
			


			o++;
		}
		handleShapes(allshapes, r.base, r.skel);
		
		creaturedataiter++;
	}

	
	
	for(int i = 0; i < npcdata.size(); i++)
	{
		ESMS::LiveCellRef<ESM::NPC,MWWorld::RefData> item = *npcdataiter;
		Ogre::Entity* npcmodel = item.model;
		//std::cout << "Timej" << NIFLoader::getSingletonPtr()->getTime(item.smodel, "Idle3: Start") << "\n";
		
        if(prev != item.smodel)
        {
            prev = item.smodel;
            allanim = NIFLoader::getSingletonPtr()->getAnim(prev);

            if(allanim.size() == 0){
                 break;
            }

       }

       if(npca.size() == i)
       {
            aindex a;
            a.time = 0.0;
			a.sinceUpdate = 0.0;
			a.base = item.model;
			a.skel = a.base->getSkeleton();
			
            //a.absoluterot = npcmodel->getParentSceneNode()->getOrientation();
            //a.absolutepos = npcmodel->getParentSceneNode()->getPosition();
            //a.initialrot = npcmodel->getSkeleton()->getBone("Bip01")->getOrientation();
            //a.first = true;
            for(int init = 0; init < allanim.size(); init++){
                 a.rindexI.push_back(0);
                 //a.rindexJ.push_back(0);
                 a.tindexI.push_back(0);
                 //a.tindexJ.push_back(0);
            }
            npca.push_back(a);
       }
       std::vector<Nif::NiKeyframeData>::iterator allanimiter;


       aindex& r = npca[i];

	   

       r.time += evt.timeSinceLastFrame;
	  
	   
	   if(!mOgre.getCamera()->isVisible(npcmodel->getWorldBoundingBox())){
		  npcdataiter++;
		   continue;
	   }
	  
       int o = 0;
	 
	    Ogre::Bone* b =r.skel->getRootBone();
	   b->setOrientation(.3,.3,.3,.3);   //This is a trick, ogre discards the first transformation
	   r.skel->getManualBonesDirty();
    r.skel->_updateTransforms();
	r.skel->_notifyManualBonesDirty();
	
	Ogre::Entity* ent = r.base;

    ent->getAllAnimationStates()->_notifyDirty();
    ent->_updateAnimation();
	ent->_notifyMoved();
       for (allanimiter = allanim.begin(); allanimiter != allanim.end(); allanimiter++)
       {
             handleAnimationTransform(*allanimiter, r, o);

            o++;
       }
	   
	
	   if(item.lhand != ""){
	   std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.lhand);
       handleShapes(allshapes, npcmodel, npcmodel->getSkeleton());
	   }
	   if(item.rhand != ""){
	   std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.rhand);
       handleShapes(allshapes, npcmodel, npcmodel->getSkeleton());
	   }
	   if(item.chest != ""){
	   std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.chest);
       handleShapes(allshapes, npcmodel, npcmodel->getSkeleton());
	   }
	   if(item.tail != ""){
	   std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.tail);
       handleShapes(allshapes, npcmodel, npcmodel->getSkeleton());
	   }
	   if(item.rfoot != ""){
	   std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.rfoot);
       handleShapes(allshapes, npcmodel, npcmodel->getSkeleton());
	   }
	   if(item.lfoot != ""){
	   std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.lfoot);
       handleShapes(allshapes, npcmodel, npcmodel->getSkeleton());
	   }
	   if(item.groin != ""){
	   std::vector<Nif::NiTriShapeCopy> allshapes = NIFLoader::getSingletonPtr()->getShapes(item.groin);
       handleShapes(allshapes, npcmodel, npcmodel->getSkeleton());
	   }
       npcdataiter++;
}



	



    //If the region has changed
    if(!(current->cell->data.flags & current->cell->Interior) && timer.elapsed() >= 10){
        timer.restart();
        if (test.name != current->cell->region)
        {
            total = 0;
            test = (ESM::Region) *(mEnvironment.mWorld->getStore().regions.find(current->cell->region));
        }

        if(test.soundList.size() > 0)
        {
            std::vector<ESM::Region::SoundRef>::iterator soundIter = test.soundList.begin();
            //mEnvironment.mSoundManager
            if(total == 0){
                while (!(soundIter == test.soundList.end()))
                {
                    ESM::NAME32 go = soundIter->sound;
                    int chance = (int) soundIter->chance;
                    //std::cout << "Sound: " << go.name <<" Chance:" <<  chance << "\n";
                    soundIter++;
                    total += chance;
                }
            }

            srand ( time(NULL) );
            int r = rand() % total;        //old random code
            int pos = 0;
            soundIter = test.soundList.begin();
            while (!(soundIter == test.soundList.end()))
            {
                const ESM::NAME32 go = soundIter->sound;
                int chance = (int) soundIter->chance;
                //std::cout << "Sound: " << go.name <<" Chance:" <<  chance << "\n";
                soundIter++;
                if( r - pos < chance)
                {
                    effect = go.name;
                    //play sound
                    std::cout << "Sound: " << go.name <<" Chance:" <<  chance << "\n";
                    mEnvironment.mSoundManager->playSound(effect, 20.0, 1.0);

                    break;

                }
                pos += chance;
            }
        }

        //mEnvironment.mSoundManager->playSound(effect, 1.0, 1.0);
        //printf("REGION: %s\n", test.name);

    }
    else if(current->cell->data.flags & current->cell->Interior)
    {
        test.name = "";
    }

    try
    {
        mEnvironment.mFrameDuration = evt.timeSinceLastFrame;

        // global scripts
        mEnvironment.mGlobalScripts->run (mEnvironment);

        bool changed = mEnvironment.mWorld->hasCellChanged();

        // local scripts
        executeLocalScripts(); // This does not handle the case where a global script causes a cell
                               // change, followed by a cell change in a local script during the same
                               // frame.

        // passing of time
        if (mEnvironment.mWindowManager->getMode()==MWGui::GM_Game)
            mEnvironment.mWorld->advanceTime (
                mEnvironment.mFrameDuration*mEnvironment.mWorld->getTimeScaleFactor()/3600);

        if (changed) // keep change flag for another frame, if cell changed happend in local script
            mEnvironment.mWorld->markCellAsUnchanged();

        // update actors
        std::vector<std::pair<std::string, Ogre::Vector3> > movement;
        mEnvironment.mMechanicsManager->update (movement);

        if (focusFrameCounter++ == focusUpdateFrame)
        {
            std::string handle = mEnvironment.mWorld->getFacedHandle();

            std::string name;

            if (!handle.empty())
            {
                MWWorld::Ptr ptr = mEnvironment.mWorld->getPtrViaHandle (handle);

                if (!ptr.isEmpty())
                    name = MWWorld::Class::get (ptr).getName (ptr);
            }

            if (!name.empty())
                std::cout << "Object: " << name << std::endl;

            focusFrameCounter = 0;
        }

        if (mEnvironment.mWindowManager->getMode()==MWGui::GM_Game)
            mEnvironment.mWorld->doPhysics (movement, mEnvironment.mFrameDuration);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error in framelistener: " << e.what() << std::endl;
    }
    //std::cout << "TESTING2";

    return true;
}

OMW::Engine::Engine()
  : mPhysicEngine (0)
  , mShowFPS (false)
  , mDebug (false)
  , mVerboseScripts (false)
  , mNewGame (false)
  , mUseSound (true)
  , mCompileAll (false)
  , mScriptManager (0)
  , mScriptContext (0)
  , mGuiManager (0)
{
	lastTime = 0;
    MWClass::registerClasses();
}

OMW::Engine::~Engine()
{
    delete mGuiManager;
    delete mEnvironment.mWorld;
    delete mEnvironment.mSoundManager;
    delete mEnvironment.mGlobalScripts;
    delete mEnvironment.mMechanicsManager;
    delete mEnvironment.mDialogueManager;
    delete mScriptManager;
    delete mScriptContext;
    delete mPhysicEngine;
}

// Load all BSA files in data directory.

void OMW::Engine::loadBSA()
{
    boost::filesystem::directory_iterator end;

    for (boost::filesystem::directory_iterator iter (mDataDir); iter!=end; ++iter)
    {
        if (boost::filesystem::extension (iter->path())==".bsa")
        {
            std::cout << "Adding " << iter->path().string() << std::endl;
            addBSA(iter->path().string());
        }
    }
}

// add resources directory
// \note This function works recursively.

void OMW::Engine::addResourcesDirectory (const boost::filesystem::path& path)
{
    mOgre.getRoot()->addResourceLocation (path.string(), "FileSystem",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);
}

// Set data dir

void OMW::Engine::setDataDir (const boost::filesystem::path& dataDir)
{
    mDataDir = boost::filesystem::system_complete (dataDir);
}

// Set resource dir
void OMW::Engine::setResourceDir (const boost::filesystem::path& parResDir)
{
    mResDir = boost::filesystem::system_complete(parResDir);
}

// Set start cell name (only interiors for now)

void OMW::Engine::setCell (const std::string& cellName)
{
	std::cout << "Change cell\n";
    mCellName = cellName;
}

// Set master file (esm)
// - If the given name does not have an extension, ".esm" is added automatically
// - Currently OpenMW only supports one master at the same time.

void OMW::Engine::addMaster (const std::string& master)
{
    assert (mMaster.empty());
    mMaster = master;

    // Append .esm if not already there
    std::string::size_type sep = mMaster.find_last_of (".");
    if (sep == std::string::npos)
    {
        mMaster += ".esm";
    }
}

void OMW::Engine::enableDebugMode()
{
    mDebug = true;
}

void OMW::Engine::enableVerboseScripts()
{
    mVerboseScripts = true;
}

void OMW::Engine::setNewGame()
{
    mNewGame = true;
}

// Initialise and enter main loop.

void OMW::Engine::go()
{
	first= true;
    assert (!mEnvironment.mWorld);
    assert (!mDataDir.empty());
    assert (!mCellName.empty());
    assert (!mMaster.empty());

    test.name = "";
    total = 0;



    std::cout << "Data directory: " << mDataDir << "\n";

    std::string cfgDir = OMW::Path::getPath(OMW::Path::GLOBAL_CFG_PATH, "openmw", "");
    std::string cfgUserDir = OMW::Path::getPath(OMW::Path::USER_CFG_PATH, "openmw", "");
    std::string plugCfg = "plugins.cfg";
    std::string ogreCfg = "ogre.cfg";
    ogreCfg.insert(0, cfgUserDir);

    //A local plugins.cfg will be used if it exist, otherwise look in the default path
    if(!isFile(plugCfg.c_str()))
    {
        plugCfg.insert(0, cfgDir);
    }

    mOgre.configure(!isFile(ogreCfg.c_str()), cfgUserDir, plugCfg, false);

    addResourcesDirectory (mDataDir / "Meshes");
    addResourcesDirectory (mDataDir / "Textures");

    // This has to be added BEFORE MyGUI is initialized, as it needs
    // to find core.xml here.
    addResourcesDirectory(mResDir / "mygui");

    // Create the window
    mOgre.createWindow("OpenMW");

    loadBSA();

    // Create physics. shapeLoader is deleted by the physic engine
    ManualBulletShapeLoader* shapeLoader = new ManualBulletShapeLoader();
    mPhysicEngine = new OEngine::Physic::PhysicEngine(shapeLoader);

    // Create the world
    mEnvironment.mWorld = new MWWorld::World (mOgre, mPhysicEngine, mDataDir, mMaster, mResDir, mNewGame, mEnvironment);


    // Set up the GUI system
    mGuiManager = new OEngine::GUI::MyGUIManager(mOgre.getWindow(), mOgre.getScene(), false, cfgDir);
    MyGUI::FactoryManager::getInstance().registerFactory<MWGui::Widgets::MWSkill>("Widget");
    MyGUI::FactoryManager::getInstance().registerFactory<MWGui::Widgets::MWAttribute>("Widget");
    MyGUI::FactoryManager::getInstance().registerFactory<MWGui::Widgets::MWSpell>("Widget");
    MyGUI::FactoryManager::getInstance().registerFactory<MWGui::Widgets::MWSpellEffect>("Widget");
    MyGUI::FactoryManager::getInstance().registerFactory<MWGui::Widgets::MWDynamicStat>("Widget");

    // Create window manager - this manages all the MW-specific GUI windows
    MWScript::registerExtensions (mExtensions);

    mEnvironment.mWindowManager = new MWGui::WindowManager(mGuiManager->getGui(), mEnvironment,
        mExtensions, mShowFPS, mNewGame);

    // Create sound system
    mEnvironment.mSoundManager = new MWSound::SoundManager(mOgre.getRoot(),
                                                           mOgre.getCamera(),
                                                           mEnvironment.mWorld->getStore(),
                                                           (mDataDir),
                                                           mUseSound);

    // Create script system
    mScriptContext = new MWScript::CompilerContext (MWScript::CompilerContext::Type_Full,
        mEnvironment);
    mScriptContext->setExtensions (&mExtensions);

    mScriptManager = new MWScript::ScriptManager (mEnvironment.mWorld->getStore(), mVerboseScripts,
        *mScriptContext);

    mEnvironment.mGlobalScripts = new MWScript::GlobalScripts (mEnvironment.mWorld->getStore(),
        *mScriptManager);

    // Create game mechanics system
    mEnvironment.mMechanicsManager = new MWMechanics::MechanicsManager (mEnvironment);

    // Create dialog system
    mEnvironment.mDialogueManager = new MWDialogue::DialogueManager (mEnvironment);

    // load cell
    ESM::Position pos;
    pos.rot[0] = pos.rot[1] = pos.rot[2] = 0;
    pos.pos[2] = 0;

    if (const ESM::Cell *exterior = mEnvironment.mWorld->getExterior (mCellName))
    {
        mEnvironment.mWorld->indexToPosition (exterior->data.gridX, exterior->data.gridY,
            pos.pos[0], pos.pos[1], true);
        mEnvironment.mWorld->changeToExteriorCell (pos);
    }
    else
    {
        pos.pos[0] = pos.pos[1] = 0;
        mEnvironment.mWorld->changeToInteriorCell (mCellName, pos);
    }

    // Sets up the input system
    MWInput::MWInputManager input(mOgre, mEnvironment.mWorld->getPlayer(),
                                  *mEnvironment.mWindowManager, mDebug, *this);
    mEnvironment.mInputManager = &input;

    focusFrameCounter = 0;

    std::cout << "\nPress Q/ESC or close window to exit.\n";

    mOgre.getRoot()->addFrameListener (this);

    // Play some good 'ol tunes
      mEnvironment.mSoundManager->startRandomTitle();

    // scripts
    if (mCompileAll)
    {
        typedef ESMS::ScriptListT<ESM::Script>::MapType Container;

        Container scripts = mEnvironment.mWorld->getStore().scripts.list;

        int count = 0;
        int success = 0;

        for (Container::const_iterator iter (scripts.begin()); iter!=scripts.end(); ++iter, ++count)
            if (mScriptManager->compile (iter->first))
                ++success;

        if (count)
            std::cout
                << "compiled " << success << " of " << count << " scripts ("
                << 100*static_cast<double> (success)/count
                << "%)"
                << std::endl;

    }

    // Start the main rendering loop
    mOgre.start();

    std::cout << "Quitting peacefully.\n";
}

void OMW::Engine::activate()
{
    // TODO: This is only a workaround. The input dispatcher should catch any exceptions thrown inside
    // the input handling functions. Looks like this will require an OpenEngine modification.
    try
    {
        std::string handle = mEnvironment.mWorld->getFacedHandle();

        if (handle.empty())
            return;

        MWWorld::Ptr ptr = mEnvironment.mWorld->getPtrViaHandle (handle);

        if (ptr.isEmpty())
            return;

        MWScript::InterpreterContext interpreterContext (mEnvironment,
            &ptr.getRefData().getLocals(), ptr);

        boost::shared_ptr<MWWorld::Action> action =
            MWWorld::Class::get (ptr).activate (ptr, mEnvironment.mWorld->getPlayer().getPlayer(),
            mEnvironment);

        interpreterContext.activate (ptr, action);

        std::string script = MWWorld::Class::get (ptr).getScript (ptr);

        if (!script.empty())
        {
            mIgnoreLocalPtr = ptr;
            mScriptManager->run (script, interpreterContext);
        }

        if (!interpreterContext.hasActivationBeenHandled())
        {
            interpreterContext.executeActivation();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Activation failed: " << e.what() << std::endl;
    }
}

void OMW::Engine::setCompileAll (bool all)
{
    mCompileAll = all;
}
