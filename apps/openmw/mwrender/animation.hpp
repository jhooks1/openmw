#ifndef _GAME_RENDER_ANIMATION_H
#define _GAME_RENDER_ANIMATION_H
#include <components/nif/data.hpp>
#include <openengine/ogre/renderer.hpp>
#include "../mwworld/refdata.hpp"
#include "../mwworld/ptr.hpp"
#include "../mwworld/actiontalk.hpp"
#include "../mwworld/environment.hpp"
#include <components/nif/node.hpp>
#include <map>
#include <openengine/bullet/physic.hpp>

namespace MWRender{

class Animation{
    
   protected:
    
    OEngine::Physic::PhysicEngine* engine;
    OEngine::Render::OgreRenderer &mRend;
    MWWorld::Environment& mEnvironment;
    static std::map<std::string, int> mUniqueIDs;
    Ogre::Quaternion rotate;
    Ogre::Vector3 trans;
    std::vector<std::vector<Nif::NiTriShapeCopy>* > shapeparts;   //All the NiTriShape data that we need for animating an npc

    float time;
	float startTime;
	float stopTime;
	bool loop;
	int animate;
	//Represents a rotation index for each bone
	std::vector<int>rindexI;
    //Represents a translation index for each bone
	std::vector<int>tindexI;
	
	//Only shapes with morphing data will need a shape number
	int shapeNumber;
	std::vector<std::vector<int> > shapeIndexI;

	//Ogre::SkeletonInstance* skel;
     std::vector<Nif::NiTriShapeCopy>* shapes;          //All the NiTriShapeData for this creature
     std::vector<Ogre::Entity*> entityparts;


    std::vector<Nif::NiKeyframeData>* transformations;
    std::map<std::string,float> textmappings;
    Ogre::Entity* base;
    void handleShapes(std::vector<Nif::NiTriShapeCopy>* allshapes, Ogre::Entity* creaturemodel, Ogre::SkeletonInstance *skel);
    void handleAnimationTransforms();
    bool timeIndex( float time, std::vector<float> times, int & i, int & j, float & x );
    std::string getUniqueID(std::string mesh);
	
    public:
     Animation(MWWorld::Environment& _env, OEngine::Render::OgreRenderer& _rend): mRend(_rend), mEnvironment(_env), loop(false), animate(0){};
	 virtual void runAnimation(float timepassed) = 0;
	 void startScript(std::string groupname, int mode, int loops);
     void stopScript();
    
    
     ~Animation();
 
};
}
#endif