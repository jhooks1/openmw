/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008-2010  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: http://openmw.sourceforge.net/

  This file (ogre_nif_loader.h) is part of the OpenMW package.

  OpenMW is distributed as free software: you can redistribute it
  and/or modify it under the terms of the GNU General Public License
  version 3, as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  version 3 along with this program. If not, see
  http://www.gnu.org/licenses/ .

 */

#ifndef _OGRE_NIF_LOADER_H_
#define _OGRE_NIF_LOADER_H_

//#include <OgreResource.h>
#include <OgreMesh.h>
#include <assert.h>
#include <string>
#include "../nif/nif_file.hpp"
#include "../nif/node.hpp"
#include "../nif/data.hpp"
#include "../nif/property.hpp"
#include "../nif/controller.hpp"
#include "../nif/extra.hpp"
//#include <meshmagick\MeshMagick.h>
#include "MeshMagick.h"


//#include "../nif/data.hpp"



class BoundsFinder;

namespace Nif
{
    class Node;
    class Transformation;
    class NiTriShape;
    class Vector;
    class Matrix;
}

namespace Mangle
{
    namespace VFS
    {
        class OgreVFS;
    }
}

/** Manual resource loader for NIF meshes. This is the main class
    responsible for translating the internal NIF mesh structure into
    something Ogre can use. Later it will also handle the insertion of
    collision meshes into Bullet / OgreBullet.

    You have to insert meshes manually into Ogre like this:

    NIFLoader::load("somemesh.nif");

    Afterwards, you can use the mesh name "somemesh.nif" normally to
    create entities and so on. The mesh isn't loaded from disk until
    OGRE needs it for rendering. Thus the above load() command is not
    very resource intensive, and can safely be done for a large number
    of meshes at load time.
 */

class NIFLoader : Ogre::ManualResourceLoader
{
    public:
		std::vector<int> numTracks;
        static int numberOfMeshes;
		
        static NIFLoader& getSingleton();
        static NIFLoader* getSingletonPtr();

        virtual void loadResource(Ogre::Resource *resource);

        static Ogre::MeshPtr load(const std::string &name, 
                                    const std::string &group="General");

		static Ogre::MeshPtr loadMirror(const std::string &name,  Ogre::Vector3 vec,
                                    const std::string &group="General");

		    static Ogre::AxisAlignedBox getMeshAabb(Ogre::MeshPtr mesh,
            const Ogre::Matrix4& transform = Ogre::Matrix4::IDENTITY);
		static Ogre::AxisAlignedBox getMeshAabb(Ogre::Mesh* mesh,
			const Ogre::Matrix4& transform = Ogre::Matrix4::IDENTITY);

        static Ogre::AxisAlignedBox getVertexDataAabb(Ogre::VertexData* vd,
            const Ogre::Matrix4& transform = Ogre::Matrix4::IDENTITY);


		void setFlip(bool fl);
		void setVector(Ogre::Vector3 vec);
		void addInMesh(Ogre::Mesh* input);
		std::vector<Nif::NiKeyframeData> getAllanim();
		std::vector<Nif::NiKeyframeData>& getAnim(std::string lowername);
		std::vector<Nif::NiTriShapeCopy>& getShapes(std::string lowername);
        
        Ogre::Vector3 convertVector3(const Nif::Vector& vec);
        Ogre::Quaternion convertRotation(const Nif::Matrix& rot);

    private:
        NIFLoader() : resourceGroup("General"),  mNormaliseNormals(false),
          mUpdateBoundingBox(true),
          mFlipVertexWinding(false),suffix(' '), flip(false)
		{ resourceName = "";}

        NIFLoader(NIFLoader& n) {}

		//MeshMagick stuff
		void processSkeleton(Ogre::Skeleton* skeleton);
		void calculateTransform(int mode);

		void processBone(Ogre::Bone* bone);
		void processIndexData(Ogre::IndexData* indexData);
		void processVertexData(Ogre::VertexData* vertexData);
		void processPositionElement(Ogre::VertexData* vertexData, const Ogre::VertexElement* vertexElem);
		void processDirectionElement(Ogre::VertexData* vertexData, const Ogre::VertexElement* vertexElem);

        void warn(std::string msg);
        void fail(std::string msg);

        void handleNode( Nif::Node *node, int flags,
                        const Nif::Transformation *trafo, BoundsFinder &bounds, Ogre::Bone *parentBone, std::vector<std::string> boneSequence);

        void handleNiTriShape(Nif::NiTriShape *shape, int flags, BoundsFinder &bounds, Nif::Transformation original, std::vector<std::string> boneSequence);

        void createOgreSubMesh(Nif::NiTriShape *shape, const Ogre::String &material, std::list<Ogre::VertexBoneAssignment> &vertexBoneAssignments);

        void createMaterial(const Ogre::String &name,
                            const Nif::Vector &ambient,
                            const Nif::Vector &diffuse,
                            const Nif::Vector &specular,
                            const Nif::Vector &emissive,
                            float glossiness, float alpha,
                            int alphaFlags, float alphaTest,
                            const Ogre::String &texName);

		void insertMeshInsideBase(Ogre::Mesh* mesh);

        void findRealTexture(Ogre::String &texName);

        Ogre::String getUniqueName(const Ogre::String &input);

        //returns the skeleton name of this mesh
        std::string getSkeletonName()
        {
            return resourceName + ".skel";
        }
        
        // This is the interface to the Ogre resource system. It allows us to
        // load NIFs from BSAs, in the file system and in any other place we
        // tell Ogre to look (eg. in zip or rar files.) It's also used to
        // check for the existence of texture files, so we can exchange the
        // extension from .tga to .dds if the texture is missing.
        Mangle::VFS::OgreVFS *vfs;


        std::string resourceName;
        std::string resourceGroup;
      
		short handle;
		char suffix;
		Ogre::Animation* animcore;
		Ogre::Animation* animcore2;
		int assignmentn;
		
		int test;
		std::vector<Nif::NiKeyframeData> allanim;
		std::map<std::string,float> textmappings;
		std::map<std::string,std::map<std::string,float>> textmappingsall;
		std::map<std::string,std::vector<Nif::NiKeyframeData>> allanimmap;
		std::map<std::string,std::vector<Nif::NiTriShapeCopy>> allshapesmap;
		std::vector<Nif::NiTriShapeCopy> shapes;
		bool flip;
		bool center;
		Ogre::Vector3 vector; 
		std::vector<Nif::NiKeyframeData> anim;
		std::vector<Nif::NiTriShapeCopy> s;

		  Ogre::Matrix4 mTransform;
        Ogre::AxisAlignedBox mBoundingBox;
		 bool mNormaliseNormals;
        bool mUpdateBoundingBox;
        bool mFlipVertexWinding;
		bool bNiTri;
		bool baddin;
		std::string triname;
		std::string name;

        // pointer to the ogre mesh which is currently build
        Ogre::Mesh *mesh;
        Ogre::SkeletonPtr mSkel;
		Ogre::SkeletonPtr npcSkel;
		Ogre::SkeletonPtr npcknaSkel;
		std::vector<Ogre::Mesh*> addin;
		
};



#endif
