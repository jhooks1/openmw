/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008-2010  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: http://openmw.sourceforge.net/

  This file (ogre_nif_loader.cpp) is part of the OpenMW package.

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

//loadResource->handleNode->handleNiTriShape->createSubMesh

#include "ogre_nif_loader.hpp"
#include <Ogre.h>
#include <stdio.h>

#include <libs/mangle/vfs/servers/ogre_vfs.hpp>
#include "../nif/nif_file.hpp"
#include "../nif/node.hpp"
#include "../nif/data.hpp"
#include "../nif/property.hpp"
#include "../nif/controller.hpp"
#include "../nif/extra.hpp"
#include <libs/platform/strings.h>

#include <vector>
#include <list>
// For warning messages
#include <iostream>

// float infinity
#include <limits>
#include <float.h>


typedef unsigned char ubyte;

//using namespace std;
using namespace Ogre;
using namespace Nif;
using namespace Mangle::VFS;



NIFLoader& NIFLoader::getSingleton()
{
    static NIFLoader instance;
    return instance;
}

NIFLoader* NIFLoader::getSingletonPtr()
{
    return &getSingleton();
}

 

void NIFLoader::warn(std::string msg)
{
    std::cerr << "NIFLoader: Warn:" << msg << "\n";
}

void NIFLoader::fail(std::string msg)
{
    std::cerr << "NIFLoader: Fail: "<< msg << std::endl;
    assert(1);
}

Vector3 NIFLoader::convertVector3(const Nif::Vector& vec)
{
    return Ogre::Vector3(vec.array);
}

Quaternion NIFLoader::convertRotation(const Nif::Matrix& rot)
{
    Real matrix[3][3];
    
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            matrix[i][j] = rot.v[i].array[j];
        
        return Quaternion(Matrix3(matrix));
}

// Helper class that computes the bounding box and of a mesh
class BoundsFinder
{
    struct MaxMinFinder
    {
        float max, min;

        MaxMinFinder()
        {
            min = std::numeric_limits<float>::infinity();
            max = -min;
        }

        void add(float f)
        {
            if (f > max) max = f;
            if (f < min) min = f;
        }

        // Return Max(max**2, min**2)
        float getMaxSquared()
        {
            float m1 = max*max;
            float m2 = min*min;
            if (m1 >= m2) return m1;
            return m2;
        }
    };

    MaxMinFinder X, Y, Z;

public:
    // Add 'verts' vertices to the calculation. The 'data' pointer is
    // expected to point to 3*verts floats representing x,y,z for each
    // point.
    void add(float *data, int verts)
    {
        for (int i=0;i<verts;i++)
        {
            X.add(*(data++));
            Y.add(*(data++));
            Z.add(*(data++));
        }
    }

    // True if this structure has valid values
    bool isValid()
    {
        return
            minX() <= maxX() &&
            minY() <= maxY() &&
            minZ() <= maxZ();
    }

    // Compute radius
    float getRadius()
    {
        assert(isValid());

        // The radius is computed from the origin, not from the geometric
        // center of the mesh.
        return sqrt(X.getMaxSquared() + Y.getMaxSquared() + Z.getMaxSquared());
    }

    float minX() {
        return X.min;
    }
    float maxX() {
        return X.max;
    }
    float minY() {
        return Y.min;
    }
    float maxY() {
        return Y.max;
    }
    float minZ() {
        return Z.min;
    }
    float maxZ() {
        return Z.max;
    }
};

// Conversion of blend / test mode from NIF -> OGRE.
// Not in use yet, so let's comment it out.
/*
static SceneBlendFactor getBlendFactor(int mode)
{
  switch(mode)
    {
    case 0: return SBF_ONE;
    case 1: return SBF_ZERO;
    case 2: return SBF_SOURCE_COLOUR;
    case 3: return SBF_ONE_MINUS_SOURCE_COLOUR;
    case 4: return SBF_DEST_COLOUR;
    case 5: return SBF_ONE_MINUS_DEST_COLOUR;
    case 6: return SBF_SOURCE_ALPHA;
    case 7: return SBF_ONE_MINUS_SOURCE_ALPHA;
    case 8: return SBF_DEST_ALPHA;
    case 9: return SBF_ONE_MINUS_DEST_ALPHA;
      // [Comment from Chris Robinson:] Can't handle this mode? :/
      // case 10: return SBF_SOURCE_ALPHA_SATURATE;
    default:
      return SBF_SOURCE_ALPHA;
    }
}


// This is also unused
static CompareFunction getTestMode(int mode)
{
  switch(mode)
    {
    case 0: return CMPF_ALWAYS_PASS;
    case 1: return CMPF_LESS;
    case 2: return CMPF_EQUAL;
    case 3: return CMPF_LESS_EQUAL;
    case 4: return CMPF_GREATER;
    case 5: return CMPF_NOT_EQUAL;
    case 6: return CMPF_GREATER_EQUAL;
    case 7: return CMPF_ALWAYS_FAIL;
    default:
      return CMPF_ALWAYS_PASS;
    }
}
*/

void NIFLoader::createMaterial(const String &name,
                           const Vector &ambient,
                           const Vector &diffuse,
                           const Vector &specular,
                           const Vector &emissive,
                           float glossiness, float alpha,
                           int alphaFlags, float alphaTest,
                           const String &texName)
{
    MaterialPtr material = MaterialManager::getSingleton().create(name, resourceGroup);

    // This assigns the texture to this material. If the texture name is
    // a file name, and this file exists (in a resource directory), it
    // will automatically be loaded when needed. If not (such as for
    // internal NIF textures that we might support later), we should
    // already have inserted a manual loader for the texture.
    if (!texName.empty())
    {
        Pass *pass = material->getTechnique(0)->getPass(0);
        /*TextureUnitState *txt =*/
        Ogre::TextureUnitState* state = pass->createTextureUnitState(texName);
	

        // As of yet UNTESTED code from Chris:
        /*pass->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
        pass->setDepthFunction(Ogre::CMPF_LESS_EQUAL);
        pass->setDepthCheckEnabled(true);

        // Add transparency if NiAlphaProperty was present
        if (alphaFlags != -1)
        {
            std::cout << "Alpha flags set!" << endl;
            if ((alphaFlags&1))
            {
                pass->setDepthWriteEnabled(false);
                pass->setSceneBlending(getBlendFactor((alphaFlags>>1)&0xf),
                                       getBlendFactor((alphaFlags>>5)&0xf));
            }
            else
                pass->setDepthWriteEnabled(true);

            if ((alphaFlags>>9)&1)
                pass->setAlphaRejectSettings(getTestMode((alphaFlags>>10)&0x7),
                                             alphaTest);

            pass->setTransparentSortingEnabled(!((alphaFlags>>13)&1));
        }
        else
            pass->setDepthWriteEnabled(true); */
        

        // Add transparency if NiAlphaProperty was present
        if (alphaFlags != -1)
        {
            // The 237 alpha flags are by far the most common. Check
            // NiAlphaProperty in nif/property.h if you need to decode
            // other values. 237 basically means normal transparencly.
            if (alphaFlags == 237)
            {
                // Enable transparency
                pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);

                //pass->setDepthCheckEnabled(false);
                pass->setDepthWriteEnabled(false);
            }
            else
                warn("Unhandled alpha setting for texture " + texName);
        }
    }

    // Add material bells and whistles
    material->setAmbient(ambient.array[0], ambient.array[1], ambient.array[2]);
    material->setDiffuse(diffuse.array[0], diffuse.array[1], diffuse.array[2], alpha);
    material->setSpecular(specular.array[0], specular.array[1], specular.array[2], alpha);
    material->setSelfIllumination(emissive.array[0], emissive.array[1], emissive.array[2]);
    material->setShininess(glossiness);
}

// Takes a name and adds a unique part to it. This is just used to
// make sure that all materials are given unique names.
String NIFLoader::getUniqueName(const String &input)
{
    static int addon = 0;
    static char buf[8];
    snprintf(buf, 8, "_%d", addon++);

    // Don't overflow the buffer
    if (addon > 999999) addon = 0;

    return input + buf;
}

// Check if the given texture name exists in the real world. If it
// does not, change the string IN PLACE to say .dds instead and try
// that. The texture may still not exist, but no information of value
// is lost in that case.
void NIFLoader::findRealTexture(String &texName)
{
    assert(vfs);
    if (vfs->isFile(texName)) return;

    int len = texName.size();
    if (len < 4) return;

    // Change texture extension to .dds
    texName[len-3] = 'd';
    texName[len-2] = 'd';
    texName[len-1] = 's';

}

//Handle node at top

// Convert Nif::NiTriShape to Ogre::SubMesh, attached to the given
// mesh.
void NIFLoader::createOgreSubMesh(NiTriShape *shape, const String &material, std::list<VertexBoneAssignment> &vertexBoneAssignments)
{
	//if(mSkel.isNull() || (mSkel->getNumBones() < 57 || mSkel->getNumBones() > 59)){
    //  cout << "s:" << shape << "\n";
	//if(mSkel.isNull() || (mSkel->getNumBones() < 57 && mSkel->getNumBones() > 59) ){
	
    NiTriShapeData *data = shape->data.getPtr();
	std::string subname = shape->name.toString();
	
    SubMesh *sub = mesh->createSubMesh(subname);
	
	
    int nextBuf = 0;

    // This function is just one long stream of Ogre-barf, but it works
    // great.

    // Add vertices
    int numVerts = data->vertices.length / 3;
    sub->vertexData = new VertexData();
    sub->vertexData->vertexCount = numVerts;
    sub->useSharedVertices = false;
    
	
	//Positions
	//Vertex declarations contain vertex elements
    VertexDeclaration *decl = sub->vertexData->vertexDeclaration;
    decl->addElement(nextBuf, 0, VET_FLOAT3, VES_POSITION);
    
    HardwareVertexBufferSharedPtr vbuf =
        HardwareBufferManager::getSingleton().createVertexBuffer(
            VertexElement::getTypeSize(VET_FLOAT3),
            numVerts, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

	
	if(flip)
	{
		float *datamod = new float[data->vertices.length];
		//std::cout << "Shape" << shape->name.toString() << "\n";
		//std::cout << "MTransform" << mTransform << "\n";
		for(int i = 0; i < numVerts; i++)
		{
			int index = i * 3;
			const float *pos = data->vertices.ptr + index;
		    Ogre::Vector3 original = Ogre::Vector3(*pos  ,*(pos+1), *(pos+2));
			//std::cout << "Original: " << original;
			//rstd::cout << "vectorfirst" << original << "\n";
			original = mTransform * original;
			mBoundingBox.merge(original);
			//std::cout <<" New: " << original << "\n";
			datamod[index] = original.x;
			datamod[index+1] = original.y;
			datamod[index+2] = original.z;
			//std::cout << "vector " << original << "\n";
			//std::cout << "datamod: " << datamod[index+1] << "datamod2: " << *(pos+1) << "\n";
		}
		 vbuf->writeData(0, vbuf->getSizeInBytes(), datamod, false);
	}
	else
	{
		vbuf->writeData(0, vbuf->getSizeInBytes(), data->vertices.ptr, false);
	}
    
    VertexBufferBinding* bind = sub->vertexData->vertexBufferBinding;
    bind->setBinding(nextBuf++, vbuf);

    // Vertex normals
    if (data->normals.length)
    {
        decl->addElement(nextBuf, 0, VET_FLOAT3, VES_NORMAL);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
                   VertexElement::getTypeSize(VET_FLOAT3),
                   numVerts, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);
		
		if(flip)
		{
			Quaternion rotation = mTransform.extractQuaternion();
			rotation.normalise();

			float *datamod = new float[data->normals.length];
			for(int i = 0; i < numVerts; i++)
		    {
			     int index = i * 3;
			     const float *pos = data->normals.ptr + index;
		        Ogre::Vector3 original = Ogre::Vector3(*pos  ,*(pos+1), *(pos+2));
				original = rotation * original;
				 if (mNormaliseNormals)
				{
                original.normalise();
				 }


			    datamod[index] = original.x;
			    datamod[index+1] = original.y;
			    datamod[index+2] = original.z;
		    }
			vbuf->writeData(0, vbuf->getSizeInBytes(), datamod, false);
		}
		else
		{
            vbuf->writeData(0, vbuf->getSizeInBytes(), data->normals.ptr, false);
		}
        bind->setBinding(nextBuf++, vbuf);
    }

    // Vertex colors
    if (data->colors.length)
    {
        const float *colors = data->colors.ptr;
        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        std::vector<RGBA> colorsRGB(numVerts);
        RGBA *pColour = &colorsRGB.front();
        for (int i=0; i<numVerts; i++)
        {
            rs->convertColourValue(ColourValue(colors[0],colors[1],colors[2],
                                               colors[3]),pColour++);
            colors += 4;
        }
        decl->addElement(nextBuf, 0, VET_COLOUR, VES_DIFFUSE);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
                   VertexElement::getTypeSize(VET_COLOUR),
                   numVerts, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        vbuf->writeData(0, vbuf->getSizeInBytes(), &colorsRGB.front(), false);
        bind->setBinding(nextBuf++, vbuf);
    }

    // Texture UV coordinates
    if (data->uvlist.length)
    {



        decl->addElement(nextBuf, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
                   VertexElement::getTypeSize(VET_FLOAT2),
                   numVerts, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		if(flip)
		{
		    float *datamod = new float[data->uvlist.length];
		
		    for(int i = 0; i < data->uvlist.length; i+=2){
			    float x = *(data->uvlist.ptr + i);
			    
			    float y = *(data->uvlist.ptr + i + 1);

			    datamod[i] =x;
				datamod[i + 1] =y;
		    }
			vbuf->writeData(0, vbuf->getSizeInBytes(), datamod, false);
		}
		else
			vbuf->writeData(0, vbuf->getSizeInBytes(), data->uvlist.ptr, false);
        bind->setBinding(nextBuf++, vbuf);
    }


	//Index Data


    // Triangle faces - The total number of triangle points
    int numFaces = data->triangles.length;
	
    if (numFaces)
    {
		
		sub->indexData->indexCount = numFaces;
        sub->indexData->indexStart = 0;
        HardwareIndexBufferSharedPtr ibuf = HardwareBufferManager::getSingleton().
                                            createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
                                                              numFaces,
                                                              HardwareBuffer::HBU_DYNAMIC);

		if(flip && mFlipVertexWinding && sub->indexData->indexCount % 3 == 0){
			sub->indexData->indexBuffer = ibuf;

			uint16 *datamod = new uint16[numFaces];
			int index = 0;
			for (size_t i = 0; i < sub->indexData->indexCount; i+=3)
			{

			     const short *pos = data->triangles.ptr + index;
				uint16 i0 = (uint16) *(pos+0);
				uint16 i1 = (uint16) *(pos+1);
				uint16 i2 = (uint16) *(pos+2);

				//std::cout << "i0: " << i0 << "i1: " << i1 << "i2: " << i2 << "\n";


				datamod[index] = i2;
				datamod[index+1] = i1;
				datamod[index+2] = i0;

				index += 3;
			}
			
			 ibuf->writeData(0, ibuf->getSizeInBytes(), datamod, false);

		}
		else
		     ibuf->writeData(0, ibuf->getSizeInBytes(), data->triangles.ptr, false);
        sub->indexData->indexBuffer = ibuf;
        


    }

    // Set material if one was given
    if (!material.empty()) sub->setMaterialName(material);

    //add vertex bone assignments
	

    for (std::list<VertexBoneAssignment>::iterator it = vertexBoneAssignments.begin();
        it != vertexBoneAssignments.end(); it++)
    {
            sub->addBoneAssignment(*it);
    }

	//mesh->_notifySkeleton(mSkel);
}

// Helper math functions. Reinventing linear algebra for the win!

// Computes B = AxB (matrix*matrix)
static void matrixMul(const Matrix &A, Matrix &B)
{
    for (int i=0;i<3;i++)
    {
        float a = B.v[0].array[i];
        float b = B.v[1].array[i];
        float c = B.v[2].array[i];

        B.v[0].array[i] = a*A.v[0].array[0] + b*A.v[0].array[1] + c*A.v[0].array[2];
        B.v[1].array[i] = a*A.v[1].array[0] + b*A.v[1].array[1] + c*A.v[1].array[2];
        B.v[2].array[i] = a*A.v[2].array[0] + b*A.v[2].array[1] + c*A.v[2].array[2];
    }
}

// Computes C = B + AxC*scale
static void vectorMulAdd(const Matrix &A, const Vector &B, float *C, float scale)
{
    // Keep the original values
    float a = C[0];
    float b = C[1];
    float c = C[2];

    // Perform matrix multiplication, scaling and addition
    for (int i=0;i<3;i++)
        C[i] = B.array[i] + (a*A.v[i].array[0] + b*A.v[i].array[1] + c*A.v[i].array[2])*scale;
}

// Computes B = AxB (matrix*vector)
static void vectorMul(const Matrix &A, float *C)
{
    // Keep the original values
    float a = C[0];
    float b = C[1];
    float c = C[2];

    // Perform matrix multiplication, scaling and addition
    for (int i=0;i<3;i++)
        C[i] = a*A.v[i].array[0] + b*A.v[i].array[1] + c*A.v[i].array[2];
}


void NIFLoader::handleNiTriShape(NiTriShape *shape, int flags, BoundsFinder &bounds, Transformation original, std::vector<std::string> boneSequence)
{
    assert(shape != NULL);


    // Interpret flags
    bool hidden    = (flags & 0x01) != 0; // Not displayed
    bool collide   = (flags & 0x02) != 0; // Use mesh for collision
    bool bbcollide = (flags & 0x04) != 0; // Use bounding box for collision

	//std::cout << "NiTriShape" << shape->name.toString() << "\n";
    // Bounding box collision isn't implemented, always use mesh for now.
    if (bbcollide)
    {
        collide = true;
        bbcollide = false;
    }

    // If the object was marked "NCO" earlier, it shouldn't collide with
    // anything.
    if (flags & 0x800)
    {
        collide = false;
        bbcollide = false;
    }

    if (!collide && !bbcollide && hidden)
        // This mesh apparently isn't being used for anything, so don't
        // bother setting it up.
        return;

    // Material name for this submesh, if any
    String material;
    // Skip the entire material phase for hidden nodes
    if (!hidden)
    {
        // These are set below if present
        NiTexturingProperty *t = NULL;
        NiMaterialProperty *m = NULL;
        NiAlphaProperty *a = NULL;

        // Scan the property list for material information
        PropertyList &list = shape->props;

        int n = list.length();
        for (int i=0; i<n; i++)
        {
            // Entries may be empty
            if (!list.has(i)) continue;

            Property *pr = &list[i];

            if (pr->recType == RC_NiTexturingProperty)
                t = (NiTexturingProperty*)pr;
            else if (pr->recType == RC_NiMaterialProperty)
                m = (NiMaterialProperty*)pr;
            else if (pr->recType == RC_NiAlphaProperty)
                a = (NiAlphaProperty*)pr;
        }

        // Texture
        String texName;
        if (t && t->textures[0].inUse)
        {
            NiSourceTexture *st = t->textures[0].texture.getPtr();
            if (st->external)
            {
                SString tname = st->filename;

                /* findRealTexture checks if the file actually
                   exists. If it doesn't, and the name ends in .tga, it
                   will try replacing the extension with .dds instead
                   and search for that. Bethesda at some at some point
                   converted all their BSA textures from tga to dds for
                   increased load speed, but all texture file name
                   references were kept as .tga.

                   The function replaces the name in place (that's why
                   we cast away the const modifier), but this is no
                   problem since all the nif data is stored in a local
                   throwaway buffer.
                 */
				
                    texName = "textures\\" + tname.toString();
					
                findRealTexture(texName);
					
            }
            else warn("Found internal texture, ignoring.");
        }

        // Alpha modifiers
        int alphaFlags = -1;
        ubyte alphaTest = 0;
        if (a)
        {
            alphaFlags = a->flags;
            alphaTest  = a->data->threshold;
        }

        // Material
        if (m || !texName.empty())
        {
            // If we're here, then this mesh has a material. Thus we
            // need to calculate a snappy material name. It should
            // contain the mesh name (mesh->getName()) but also has to
            // be unique. One mesh may use many materials.
            material = getUniqueName(mesh->getName());

            if (m)
            {
                // Use NiMaterialProperty data to create the data
                const S_MaterialProperty *d = m->data;
                createMaterial(material, d->ambient, d->diffuse, d->specular, d->emissive,
                               d->glossiness, d->alpha, alphaFlags, alphaTest, texName);
            }
            else
            {
                // We only have a texture name. Create a default
                // material for it.
                Vector zero, one;
                for (int i=0; i<3;i++)
                {
                    zero.array[i] = 0.0;
                    one.array[i] = 1.0;
                }

                createMaterial(material, one, one, zero, zero, 0.0, 1.0,
                               alphaFlags, alphaTest, texName);
            }
        }
    } // End of material block, if(!hidden) ...

    /* Do in-place transformation of all the vertices and normals. This
       is pretty messy stuff, but we need it to make the sub-meshes
       appear in the correct place. Neither Ogre nor Bullet support
       nested levels of sub-meshes with transformations applied to each
       level.
    */
	
	  NiTriShapeData *data = shape->data.getPtr();
    int numVerts = data->vertices.length / 3;


    float *ptr = (float*)data->vertices.ptr;
    float *optr = ptr;
	 
	//std::cout << "name" << shape->name.toString() << "/" << *ptr << "\n";
    std::list<VertexBoneAssignment> vertexBoneAssignments;

	
	Nif::NiTriShapeCopy copy = shape->clone();
    //use niskindata for the position of vertices.
    if (!shape->skin.empty())
    {
		
		//std::cout << "Skin is not empty\n";
		//Bone assignments are stored in submeshes, so we don't need to copy them
		//std::string triname
		//std::vector<Ogre::Vector3> vertices;
        //std::vector<Ogre::Vector3> normals;
        //std::vector<Nif::NiSkinData::BoneInfoCopy> boneinfo;
		
		
        // vector that stores if the position if a vertex is absolute
        std::vector<bool> vertexPosAbsolut(numVerts,false);

        float *ptrNormals = (float*)data->normals.ptr;
        //the bone from skin->bones[boneIndex] is linked to skin->data->bones[boneIndex]
        //the first one contains a link to the bone, the second vertex transformation
        //relative to the bone
        int boneIndex = 0;
        Bone *bonePtr;
        Vector3 vecPos;
        Quaternion vecRot;

        std::vector<NiSkinData::BoneInfo> boneList = shape->skin->data->bones;

        /*
        Iterate through the boneList which contains what vertices are linked to
        the bone (it->weights array) and at what position (it->trafo)
        That position is added to every vertex.
        */
        for (std::vector<NiSkinData::BoneInfo>::iterator it = boneList.begin();
                it != boneList.end(); it++)
        {
            if(mSkel.isNull())
            {
                std::cout << "No skeleton for :" << shape->skin->bones[boneIndex].name.toString() << std::endl;
                break;
            }
            //get the bone from bones array of skindata
			if(!mSkel->hasBone(shape->skin->bones[boneIndex].name.toString()))
				std::cout << "We don't have this bone";
            bonePtr = mSkel->getBone(shape->skin->bones[boneIndex].name.toString());

            // final_vector = old_vector + old_rotation*new_vector*old_scale
           

			Nif::NiSkinData::BoneInfoCopy boneinfo;
			boneinfo.trafo.rotation = convertRotation(it->trafo->rotation);
			boneinfo.trafo.trans = convertVector3(it->trafo->trans);
			boneinfo.bonename = shape->skin->bones[boneIndex].name.toString();
            for (unsigned int i=0; i<it->weights.length; i++)
            {
				 vecPos = bonePtr->_getDerivedPosition() +
                bonePtr->_getDerivedOrientation() * convertVector3(it->trafo->trans);

            vecRot = bonePtr->_getDerivedOrientation() * convertRotation(it->trafo->rotation);
                unsigned int verIndex = (it->weights.ptr + i)->vertex;
				boneinfo.weights.push_back(*(it->weights.ptr + i));
                //Check if the vertex is relativ, FIXME: Is there a better solution?
                if (vertexPosAbsolut[verIndex] == false)
                {
                    //apply transformation to the vertices
                    Vector3 absVertPos = vecPos + vecRot * Vector3(ptr + verIndex *3);

					mBoundingBox.merge(absVertPos);
                    //convert it back to float *
                    for (int j=0; j<3; j++)
                        (ptr + verIndex*3)[j] = absVertPos[j];

                    //apply rotation to the normals (not every vertex has a normal)
                    //FIXME: I guessed that vertex[i] = normal[i], is that true?
                    if (verIndex < data->normals.length)
                    {
                        Vector3 absNormalsPos = vecRot * Vector3(ptrNormals + verIndex *3);
                        
                        for (int j=0; j<3; j++)
                            (ptrNormals + verIndex*3)[j] = absNormalsPos[j];
                    }

                    vertexPosAbsolut[verIndex] = true;
                }


                VertexBoneAssignment vba;
                vba.boneIndex = bonePtr->getHandle();
                vba.vertexIndex = verIndex;
                vba.weight = (it->weights.ptr + i)->weight;
	

                vertexBoneAssignments.push_back(vba);
            }
			copy.boneinfo.push_back(boneinfo);

            boneIndex++;
        }
		
    }
    else
    {
		
			copy.boneSequence = boneSequence;
        // Rotate, scale and translate all the vertices,
        const Matrix &rot = shape->trafo->rotation;
        const Vector &pos = shape->trafo->pos;
        float scale = shape->trafo->scale;

		copy.trafo.trans = convertVector3(original.pos);
		copy.trafo.rotation = convertRotation(original.rotation);
		copy.trafo.scale = original.scale;
		//We don't use velocity for anything yet, so it does not need to be saved
	
		// Computes C = B + AxC*scale
        for (int i=0; i<numVerts; i++)
        {
            vectorMulAdd(rot, pos, ptr, scale);
			Ogre::Vector3 absVertPos = Ogre::Vector3(*(ptr + 3 * i), *(ptr + 3 * i + 1), *(ptr + 3 * i + 2));
			mBoundingBox.merge(absVertPos);
            ptr += 3;
        }
		
        // Remember to rotate all the vertex normals as well
        if (data->normals.length)
        {
            ptr = (float*)data->normals.ptr;
            for (int i=0; i<numVerts; i++)
            {
                vectorMul(rot, ptr);
                ptr += 3;
            }
        }
		if(!mSkel.isNull() ){
			int boneIndex;
			Ogre::Bone *parentBone = mSkel->getBone(boneSequence[boneSequence.size() - 1]);
			if(parentBone)
				boneIndex = parentBone->getHandle();
			else
				boneIndex = mSkel->getNumBones() - 1;
			for(int i = 0; i < numVerts; i++){
		 VertexBoneAssignment vba;
                vba.boneIndex = boneIndex;
                vba.vertexIndex = i;
                vba.weight = 1;
				 vertexBoneAssignments.push_back(vba);
			}
		}
    }
	/*
	if(center){
		for(int i = 0; i < numVerts; i++)
		{
			int index = i * 3;
			const float *pos = data->vertices.ptr + index;
		    Ogre::Vector3 original = Ogre::Vector3(*pos  ,*(pos+1), *(pos+2));
			const float *posNormal = data->normals.ptr + index;
			Ogre::Vector3 originalN = Ogre::Vector3(*posNormal  ,*(posNormal+1), *(posNormal+2));
			mBoundingBox.merge(original);
			//std::cout << "Adding Point " << original << "\n";
		}
		return;

	}*/

    if (!hidden)
    {
		
		
	

		shapes.push_back(copy);
        // Add this vertex set to the bounding box
        bounds.add(optr, numVerts);

        // Create the submesh
        createOgreSubMesh(shape, material, vertexBoneAssignments);
    }
}

void NIFLoader::handleNode(Nif::Node *node, int flags,
                           const Transformation *trafo, BoundsFinder &bounds, Ogre::Bone *parentBone, std::vector<std::string> boneSequence)
{
    //if( MWClass::isChest)
    //  cout << "u:" << node << "\n";
    // Accumulate the flags from all the child nodes. This works for all
    // the flags we currently use, at least.
    flags |= node->flags;

    // Check for extra data
    Extra *e = node;
    while (!e->extra.empty())
    {
        // Get the next extra data in the list
        e = e->extra.getPtr();
        assert(e != NULL);

        if (e->recType == RC_NiStringExtraData)
        {
            // String markers may contain important information
            // affecting the entire subtree of this node
            NiStringExtraData *sd = (NiStringExtraData*)e;

            if (sd->string == "NCO")
                // No collision. Use an internal flag setting to mark this.
                flags |= 0x800;
            else if (sd->string == "MRK")
                // Marker objects. These are only visible in the
                // editor. Until and unless we add an editor component to
                // the engine, just skip this entire node.
                return;
        }
    }

    Bone *bone = 0;

    // create skeleton or add bones
    if (node->recType == RC_NiNode)
    {

        if (!mSkel.isNull())     //if there is a skeleton
        {
            std::string name = node->name.toString();
            boneSequence.push_back(name);

            // Quick-n-dirty workaround for the fact that several
            // bones may have the same name.
            if(!mSkel->hasBone(name))
            {
				//std::cout << "Creating a" << name << "\n";
                bone = mSkel->createBone(name);

                if (parentBone)
                  parentBone->addChild(bone);

                bone->setInheritOrientation(true);
                bone->setPosition(convertVector3(node->trafo->pos));
                bone->setOrientation(convertRotation(node->trafo->rotation));
            }
        }
    }
	Transformation original = *(node->trafo);

    // Apply the parent transformation to this node. We overwrite the
    // existing data with the final transformation.
	
    if (trafo)
    {
        // Get a non-const reference to the node's data, since we're
        // overwriting it. TODO: Is this necessary?
        Transformation &final = *((Transformation*)node->trafo);

        // For both position and rotation we have that:
		// Computes C = B + AxC*scale
        // final_vector = old_vector + old_rotation*new_vector*old_scale
        vectorMulAdd(trafo->rotation, trafo->pos, final.pos.array, trafo->scale);
        vectorMulAdd(trafo->rotation, trafo->velocity, final.velocity.array, trafo->scale);

        // Merge the rotations together
        matrixMul(trafo->rotation, final.rotation);

        // Scalar values are so nice to deal with. Why can't everything
        // just be scalar?
        final.scale *= trafo->scale;
    }

    // For NiNodes, loop through children
    if (node->recType == RC_NiNode)
    {
        NodeList &list = ((NiNode*)node)->children;
        int n = list.length();

        for (int i = 0; i<n; i++)
        {
            if (list.has(i))
                handleNode(&list[i], flags, node->trafo, bounds, bone, boneSequence);
		}
			    //repeat later
        


    }
    else if (node->recType == RC_NiTriShape)   //57,58,59
    {

		
            std::string name = node->name.toString();
		
			if (triname == "")
            {
                handleNiTriShape(dynamic_cast<NiTriShape*>(node), flags, bounds, original, boneSequence);
            }
			else if(name.length() >= triname.length())
			{
				std::transform(name.begin(), name.end(), name.begin(), std::tolower);
				if(triname == name.substr(0, triname.length()))
					handleNiTriShape(dynamic_cast<NiTriShape*>(node), flags, bounds, original, boneSequence);
			}
        
    }
}
std::vector<Nif::NiKeyframeData> NIFLoader::getAllanim(){
		return allanim;
}

std::vector<Nif::NiKeyframeData> NIFLoader::getAnim(std::string lowername){
		std::transform(lowername.begin(), lowername.end(), lowername.begin(), std::tolower);
		std::vector<Nif::NiKeyframeData> anim;
		std::map<std::string,std::vector<Nif::NiKeyframeData>>::iterator iter = allanimmap.find(lowername);
		if(iter != allanimmap.end())
			anim = iter->second;
		return anim;
			
}
std::vector<Nif::NiTriShapeCopy> NIFLoader::getShapes(std::string lowername){
		std::transform(lowername.begin(), lowername.end(), lowername.begin(), std::tolower);
		std::vector<Nif::NiTriShapeCopy> s;
		std::map<std::string,std::vector<Nif::NiTriShapeCopy>>::iterator iter = allshapesmap.find(lowername);
		if(iter != allshapesmap.end())
			s = iter->second;
		return s;
}

void NIFLoader::setFlip(bool fl){
	flip = fl;
}
void NIFLoader::setVector(Ogre::Vector3 vec)
{
	vector = vec;
}
void NIFLoader::loadResource(Resource *resource)
{
	center = false;
	mBoundingBox.setNull();
	allanim.clear();
	shapes.clear();
	assignmentn = 0;
    std::string name = resource->getName();
	std::string lowername = name;
	std::transform(lowername.begin(), lowername.end(), lowername.begin(), std::tolower);
	//std::cout << "Name" << name << "\n";
	suffix = name.at(name.length() - 2);
	if( suffix == '*' || suffix == '?' || suffix == '<')
	{
		if(suffix == '*')
			vector = Ogre::Vector3(-1,1,1);
		if(suffix == '?')
			vector = Ogre::Vector3(1,-1,1);
		if(suffix == '<')
			vector = Ogre::Vector3(1,1,-1);
		flip = true;
	}


	
	if(flip)
	{
		//std::cout << "Flipping";
		calculateTransform(0);
	}

	suffix = name.at(name.length() - 1);
	//Part selection on last character of the file string
		//  " Tri Chest
		//  * Tri Tail
		//  : Tri Left Foot
		//  < Tri Right Foot
		//  > Tri Left Hand
		//  ? Tri Right Hand
		//  | Normal

	switch(suffix)
	{
	    case '"':
			triname = "tri chest";
			break;
		case '*':
			triname = "tri tail";
			break;
		case ':':
			triname = "tri left foot";
			break;
		case '<':
			triname = "tri right foot";
			break;
		case '>':
			triname = "tri left hand";
			center = true;
			break;
		case '?':
			triname = "tri right hand";
			center = true;
			break;
		default:
			triname = "";
			break;
	}

   
    resourceName = "";
   
    mesh = 0;
    mSkel.setNull();

    // Set up the VFS if it hasn't been done already
    if (!vfs) vfs = new OgreVFS(resourceGroup);

    // Get the mesh
    mesh = dynamic_cast<Mesh*>(resource);

    assert(mesh);

    // Look it up
    resourceName = mesh->getName();
    //std::cout << resourceName << "\n";

    if (!vfs->isFile(resourceName))
    {
        warn("File not found.");
        return;
    }

    // Helper that computes bounding boxes for us.
    BoundsFinder bounds;

    // Load the NIF. TODO: Wrap this in a try-catch block once we're out
    // of the early stages of development. Right now we WANT to catch
    // every error as early and intrusively as possible, as it's most
    // likely a sign of incomplete code rather than faulty input.
    NIFFile nif(vfs->open(resourceName), resourceName);

    if (nif.numRecords() < 1)
    {
        warn("Found no records in NIF.");
        return;
    }
	
	for(int i = 0; i < nif.numRecords(); i++)

	{
		Nif::Node *n = dynamic_cast<Nif::Node*>(nif.getRecord(i));
	 if (n != NULL && n->recType == RC_NiNode)
    {
        //FIXME: "Bip01" isn't every time the root bone
        if (n->name == "Bip01" || n->name == "Root Bone")  //root node, create a skeleton
        {
            mSkel = SkeletonManager::getSingleton().create(getSkeletonName(), resourceGroup);

            /*if (node->extra->recType == RC_NiTextKeyExtraData )
            {
                //TODO: Get animation names
                std::cout << node->name.toString() << " is root bone and has textkeyextradata!\n";
            }*/
        }

	 }
	}
    // The first record is assumed to be the root node
    Record *r = nif.getRecord(0);
    assert(r != NULL);

    Nif::Node *node = dynamic_cast<Nif::Node*>(r);

    if (node == NULL)
    {
        warn("First record in file was not a node, but a " +
             r->recName.toString() + ". Skipping file.");
        return;
    }

	

    // Handle the node
	std::vector<std::string> boneSequence;
	
	/*
	if(center) 
	{
		handleNode(node, 0, NULL, bounds, 0, boneSequence);
		calculateTransform(1);
		center = false;

		processSkeleton(mSkel.get());
		mBoundingBox.setNull();
		flip = true;
	}*/

    handleNode(node, 0, NULL, bounds, 0, boneSequence);

	


	handle = 0;
	
    // set the bounding value.
    if (bounds.isValid())
    {
		//float widthhalf = (bounds.maxX() - bounds.minX()) / 2;
		//float heighthalf = (bounds.maxY() - bounds.minY()) / 2;
		//float depthhalf = (bounds.maxZ() - bounds.minZ()) / 2;

        mesh->_setBounds(AxisAlignedBox(bounds.minX(), bounds.minY(), bounds.minZ(),
                                        bounds.maxX(), bounds.maxY(), bounds.maxZ()));
        mesh->_setBoundingSphereRadius(bounds.getRadius());
    }

	int acounter = 1;

	
	bool hasAnim = false;
	
	for(int i = 0; i < nif.numRecords(); i++)
	{
		
	Nif::NiKeyframeController *f = dynamic_cast<Nif::NiKeyframeController*>(nif.getRecord(i));
	
	//Nif::Node *o = dynamic_cast<Nif::Node*>(f->target.getPtr());
	//NodeList &list = ((NiNode*)o)->children;



	Nif::Node *n = dynamic_cast<Nif::Node*>(nif.getRecord(i));
	
	

	if(f != NULL)
	{
		hasAnim = true;
		Nif::Node *o = dynamic_cast<Nif::Node*>(f->target.getPtr());
		Nif::NiKeyframeDataPtr data = f->data;
		
		if (f->timeStart == FLT_MAX)
			continue;
		//std::cout <<"Bone" << o->name.toString() << "\n";
		data->setBonename(o->name.toString());
		Nif::NiKeyframeData c;
		c.clone(data.get());
		c.setStartTime(f->timeStart);
		allanim.push_back(c);
		
		if(o->name.toString() == "Bip01"){
			std::cout <<"Creating WholeThing\n";
			animcore = mSkel->createAnimation("WholeThing", f->timeStop);
			animcore2 = mSkel->createAnimation("WholeThing2", f->timeStop);

			Nif::Named *node = dynamic_cast<Nif::Named*> ( f->target.getPtr());
			std::cout << "The target rec: " << node->name.toString() << "\n";
			Ogre::NodeAnimationTrack* mTrack = animcore->createNodeTrack(handle, mSkel->getBone(node->name.toString()));
			Ogre::NodeAnimationTrack* mTrack2 = animcore2->createNodeTrack(handle++, mSkel->getBone(node->name.toString()));

			std::vector<Ogre::Quaternion> quats = data->getQuat();
			std::vector<Ogre::Quaternion>::iterator quatIter = quats.begin();
			std::vector<float> rtime = data->getrTime();
			std::vector<float>::iterator rtimeiter = rtime.begin();

			std::vector<float> ttime = data->gettTime();
			std::vector<float>::iterator ttimeiter = ttime.begin();
			std::vector<Ogre::Vector3> translist1 = data->getTranslist1();
			std::vector<Ogre::Vector3>::iterator transiter = translist1.begin();
			std::vector<Ogre::Vector3> translist2 = data->getTranslist2();
			std::vector<Ogre::Vector3>::iterator transiter2 = translist2.begin();
			std::vector<Ogre::Vector3> translist3 = data->getTranslist3();
			std::vector<Ogre::Vector3>::iterator transiter3 = translist3.begin();


			float tleft = 0;
			float rleft = 0.0;
			float ttotal = 0.0;
			float rtotal = 0;
			Ogre::TransformKeyFrame* mKey;
			Ogre::TransformKeyFrame* mKey2;
			float tused = 0.0;
			float rused = 0.0;
			Ogre::Quaternion lastquat;
			Ogre::Vector3 lasttrans;
			bool rend = false;
			bool tend = false;
			for (int j = 0 ; j < ttime.size(); j++)
			{
			if(data->getTtype() >= 1 && data->getTtype() <= 5)
			{
			Ogre::TransformKeyFrame* mKey = mTrack->createNodeKeyFrame(*ttimeiter);
			Ogre::Vector3 standard = *transiter;
			if(data->getTtype() == 2)
			standard = *transiter * *transiter2 * *transiter3;
			if(j < (ttime.size() - 1))
				mKey->setTranslate(standard);
			transiter++;
			transiter2++;
			transiter3++;
			ttimeiter++;
			}
			}
			float initx = quatIter->x;
			for (int j = 0 ; j < rtime.size(); j++)
			{
			if(data->getRtype() >= 1 && data->getRtype() <= 5)
			{
			Ogre::TransformKeyFrame* mKey2 = mTrack2->createNodeKeyFrame(*rtimeiter);
			Ogre::Quaternion standard = *quatIter;
			if(j < rtime.size() - 1)
			{
				Ogre::Radian angle;
				Ogre::Vector3 axis;
				standard.ToAngleAxis(angle, axis);
				//axis.z = axis.y;
			//Ogre::Quaternion r = Ogre::Quaternion::Slerp(1.0, standard, *(quatIter+1), true);
			    Ogre::Quaternion mod = Ogre::Quaternion(angle, axis);    //y,x,z
				                                                                //z, y, x
				                                                                //x, z, y
				                                                                  //y,y,z
				                                                                 //x,y,y sorta works  y at the end is right
				                                                                 //z,x,y 
				                                                                 //-x,z,y
				                                                                 //-x,-z,y
				        
				mKey2->setRotation(standard);
			}	
			quatIter++;
			rtimeiter++;
			}
			}



		}

		//std::cout << "Controller's Rtype:" <<  data->getRtype() << "Stype: " << data->getStype() << "Ttype:" << data->getTtype() << "\n";
	}
	
	}
	if(hasAnim){
		//std::cout << "Lower" << lowername << "\n";
		allanimmap[lowername] = allanim;
		allshapesmap[lowername] = shapes;
	}
	if(center)
	{
		calculateTransform(1);

		  mBoundingBox.setNull();

        if (mesh->sharedVertexData != NULL)
        {
            processVertexData(mesh->sharedVertexData);
        }

        for(int i = 0;i < mesh->getNumSubMeshes();i++)
        {
            SubMesh* submesh = mesh->getSubMesh(i);
            if (submesh->vertexData != NULL)
            {
                processVertexData(submesh->vertexData);
            }
            if (submesh->indexData != NULL)
            {
            	processIndexData(submesh->indexData);
            }
        }

		processSkeleton(mSkel.get());
		/*
		for(int i = 0; i < mesh->getNumSubMeshes(); i++){
		Ogre::HardwareVertexBufferSharedPtr vbuf = mesh->getSubMesh(i)->vertexData->vertexBufferBinding->getBuffer(0);
		Ogre::Real* pReal = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
			
		for (int i = 0; i < vbuf->getSizeInBytes() / 3 / sizeof(float); i++)
		{
			pReal[i] = 0;
		}
		vbuf->unlock();
		}*/
	}
	if(flip){
	mesh->_setBounds(mBoundingBox, false);
	}
	
    // set skeleton
  if (!mSkel.isNull())
  {
	  //std::cout << "Mskel" << mSkel->getNumBones();
       mesh->_notifySkeleton(mSkel);
  }
  flip = false;
  //std::cout << "7\n";
}

void NIFLoader::processIndexData(IndexData* indexData)
	{
		if (!mFlipVertexWinding)
		{
			// Nothing to do.
			return;
		}

		if (indexData->indexCount % 3 != 0)
		{
            printf("Index number is not a multiple of 3, no vertex winding flipping possible. Skipped.");
            return;
		}

		//print("Flipping index order for vertex winding flipping.", V_HIGH);
		Ogre::HardwareIndexBufferSharedPtr buffer = indexData->indexBuffer;
		unsigned char* data =
               static_cast<unsigned char*>(buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL));

		if(buffer->getType() == Ogre::HardwareIndexBuffer::IT_16BIT)
		{
			// 16 bit
			//print("using 16bit indices", V_HIGH);

			for (size_t i = 0; i < indexData->indexCount; i+=3)
			{
				uint16* i0 = (uint16*)(data+0 * buffer->getIndexSize());
				uint16* i2 = (uint16*)(data+2 * buffer->getIndexSize());

				// flip
				uint16 tmp = *i0;
				*i0 = *i2;
				*i2 = tmp;

				data += 3 * buffer->getIndexSize();
			}
		}
		else
		{
			// 32 bit
			//print("using 32bit indices", V_HIGH);

			for (size_t i = 0; i < indexData->indexCount; i+=3)
			{
				uint32* i0 = (uint32*)(data+0 * buffer->getIndexSize());
				uint32* i2 = (uint32*)(data+2 * buffer->getIndexSize());

				// flip
				uint32 tmp = *i0;
				*i0 = *i2;
				*i2 = tmp;

				data += 3 * buffer->getIndexSize();
			}
		}

		buffer->unlock();
	}
	void NIFLoader::processDirectionElement(VertexData* vertexData,
        const VertexElement* vertexElem)
    {
        // We only want to apply rotation to normal, binormal and tangent, so extract it.
        Quaternion rotation = mTransform.extractQuaternion();
        rotation.normalise();

        Ogre::HardwareVertexBufferSharedPtr buffer =
            vertexData->vertexBufferBinding->getBuffer(vertexElem->getSource());

        unsigned char* data =
            static_cast<unsigned char*>(buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL));
        for (size_t i = 0; i < vertexData->vertexCount; ++i)
        {
            Real* ptr;
            vertexElem->baseVertexPointerToElement(data, &ptr);

            Vector3 vertex(ptr);
            vertex = rotation * vertex;
            if (mNormaliseNormals)
            {
                vertex.normalise();
            }
            ptr[0] = vertex.x;
            ptr[1] = vertex.y;
            ptr[2] = vertex.z;

            data += buffer->getVertexSize();
        }
        buffer->unlock();
    }

    void NIFLoader::processPositionElement(VertexData* vertexData,
        const VertexElement* vertexElem)
    {
        Ogre::HardwareVertexBufferSharedPtr buffer =
            vertexData->vertexBufferBinding->getBuffer(vertexElem->getSource());

        unsigned char* data =
            static_cast<unsigned char*>(buffer->lock(Ogre::HardwareBuffer::HBL_NORMAL));
        for (size_t i = 0; i < vertexData->vertexCount; ++i)
        {
            Real* ptr;
            vertexElem->baseVertexPointerToElement(data, &ptr);

            Vector3 vertex(ptr);
            vertex = mTransform * vertex;
            ptr[0] = vertex.x;
            ptr[1] = vertex.y;
            ptr[2] = vertex.z;
            mBoundingBox.merge(vertex);

            data += buffer->getVertexSize();
        }
        buffer->unlock();
    }


void NIFLoader::processVertexData(VertexData* vertexData)
    {
        const VertexElement* position =
            vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
        if (position != NULL)
        {
            processPositionElement(vertexData, position);
        }

        const VertexElement* normal =
            vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
        if (normal != NULL)
        {
            processDirectionElement(vertexData, normal);
        }

        const VertexElement* binormal =
            vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_BINORMAL);
        if (binormal != NULL)
        {
            processDirectionElement(vertexData, binormal);
        }

        const VertexElement* tangent =
            vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TANGENT);
        if (tangent != NULL)
        {
            processDirectionElement(vertexData, tangent);
        }
    }

void NIFLoader::processSkeleton(Ogre::Skeleton* skeleton)
    {
        Skeleton::BoneIterator it = skeleton->getBoneIterator();
        while (it.hasMoreElements())
        {
            processBone(it.peekNext());
            it.moveNext();
        }
}
void NIFLoader::processBone(Ogre::Bone* bone)
    {
        if (bone->getParent() == NULL)
        {
            // Is root bone, we need to apply full transform
            bone->setPosition(mTransform * bone->getPosition());
            Quaternion rot = mTransform.extractQuaternion();
            rot.normalise();
            bone->setOrientation(rot * bone->getOrientation());
        }
        else
        {
            // Non-root-bone, we apply only scale
            Matrix3 m3x3;
            mTransform.extract3x3Matrix(m3x3);
            Vector3 scale(
                m3x3.GetColumn(0).length(),
                m3x3.GetColumn(1).length(),
                m3x3.GetColumn(2).length());
            bone->setPosition(scale * bone->getPosition());
        }
    }


MeshPtr NIFLoader::load(const std::string &name, 
                         const std::string &group)
{
    MeshManager *m = MeshManager::getSingletonPtr();
    // Check if the resource already exists
	//std::cout << "IN Load" << name << "\n";
    ResourcePtr ptr = m->getByName(name, group);
    MeshPtr resize;

    if (!ptr.isNull()){ 
        
        //if(pieces > 1)
            //cout << "It exists\n";
            resize = MeshPtr(ptr);
        //resize->load();
        //resize->reload();
    }
    else // Nope, create a new one.
    {
		NIFLoader::getSingletonPtr()->setFlip(false);
        resize = MeshManager::getSingleton().createManual(name, group, NIFLoader::getSingletonPtr());
         ResourcePtr ptr = m->getByName(name, group);
         resize = MeshPtr(ptr);
    return resize;

	}

}




  void NIFLoader::calculateTransform(int mode)
    {
        // Calculate transform
        Matrix4 transform = Matrix4::IDENTITY;

		//std::cout << "Calculating transformation...";

		if(mode == 0)
                transform = Matrix4::getScale(vector) * transform;
				//std::cout << "Apply scaling \n";
        if(mode == 1)
		{
			Vector3 translate = Vector3::ZERO;
			if(suffix == '>')                     //left hand
				                                  //We align by the fingertips
				translate = Vector3(-mBoundingBox.getMinimum().x, -mBoundingBox.getCenter().y, -mBoundingBox.getCenter().z);
			else
				translate = Vector3(-mBoundingBox.getMaximum().x, -mBoundingBox.getCenter().y, -mBoundingBox.getCenter().z);
			 
			 transform = Matrix4::getTrans(translate) * transform;
		}

		//ignore, if no mesh given. Without we can't do this op.
            


        // Check whether we have to flip vertex winding.
        // We do have to, if we changed our right hand base.
        // We can test it by using the cross product from X and Y and see, if it is a non-negative
        // projection on Z. Actually it should be exactly Z, as we don't do non-uniform scaling yet,
        // but the test is cheap either way.
        Matrix3 m3;
        transform.extract3x3Matrix(m3);
		
        if (m3.GetColumn(0).crossProduct(m3.GetColumn(1)).dotProduct(m3.GetColumn(2)) < 0)
        {
        	mFlipVertexWinding = true;
        }
		if(mode == 1)
			mFlipVertexWinding = false;

        mTransform = transform;
		//std::cout << "final transform \n";
        //print("final transform " + StringConverter::toString(mTransform), V_HIGH);
    }


	AxisAlignedBox NIFLoader::getMeshAabb(MeshPtr mesh, const Matrix4& transform)
	{
		return getMeshAabb(mesh.get(), transform);
	}
    AxisAlignedBox NIFLoader::getMeshAabb(Mesh* mesh, const Matrix4& transform)
    {
        AxisAlignedBox aabb;
        if (mesh->sharedVertexData != 0)
        {
            aabb.merge(getVertexDataAabb(mesh->sharedVertexData, transform));
        }
        for (unsigned int i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            SubMesh* sm = mesh->getSubMesh(i);
            if (sm->vertexData != 0)
            {
                aabb.merge(getVertexDataAabb(sm->vertexData, transform));
            }
        }

        return aabb;
    }

    AxisAlignedBox NIFLoader::getVertexDataAabb(VertexData* vd, const Matrix4& transform)
    {
        AxisAlignedBox aabb;

        const VertexElement* ve = vd->vertexDeclaration->findElementBySemantic(VES_POSITION);
        HardwareVertexBufferSharedPtr vb = vd->vertexBufferBinding->getBuffer(ve->getSource());

        unsigned char* data = static_cast<unsigned char*>(
            vb->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

        for (size_t i = 0; i < vd->vertexCount; ++i)
        {
            float* v;
            ve->baseVertexPointerToElement(data, &v);
            aabb.merge(transform * Vector3(v[0], v[1], v[2]));

            data += vb->getVertexSize();
        }
        vb->unlock();

        return aabb;
    }



 



MeshPtr NIFLoader::loadMirror(const std::string &name, Ogre::Vector3 vec, 
                         const std::string &group)
{
    MeshManager *m = MeshManager::getSingletonPtr();
    // Check if the resource already exists
	//std::cout << "IN Load" << name << "\n";
    ResourcePtr ptr = m->getByName(name, group);
    MeshPtr resize;
  

    const std::string beast1 ="meshes\\b\\B_N_Khajiit_F_Skins.nif";
    const std::string beast2 ="meshes\\b\\B_N_Khajiit_M_Skins.nif";
    const std::string beast3 ="meshes\\b\\B_N_Argonian_F_Skins.nif";
    const std::string beast4 ="meshes\\b\\B_N_Argonian_M_Skins.nif";

    const std::string beasttail1 ="tail\\b\\B_N_Khajiit_F_Skins.nif";
    const std::string beasttail2 ="tail\\b\\B_N_Khajiit_M_Skins.nif";
    const std::string beasttail3 ="tail\\b\\B_N_Argonian_F_Skins.nif";
    const std::string beasttail4 ="tail\\b\\B_N_Argonian_M_Skins.nif";

    if (!ptr.isNull()){ 
        
        //if(pieces > 1)
            //cout << "It exists\n";
            resize = MeshPtr(ptr);
        //resize->load();
        //resize->reload();
    }
    else // Nope, create a new one.
    {
		NIFLoader::getSingletonPtr()->setVector(vec);
		NIFLoader::getSingletonPtr()->setFlip(true);
        resize = MeshManager::getSingleton().createManual(name, group, NIFLoader::getSingletonPtr());
        //cout <<"EXISTING" << name << "\n";
        
        //if(pieces > 1)
            //cout << "Creating it\n";
   
        
        //resize->load();
        //resize->reload();
        //return 0;

        
        //NIFLoader::getSingletonPtr()->
        /*ResourcePtr ptr = m->getByName(name, group);
    if (!ptr.isNull()){
        if(pieces > 1)
            cout << "It exists\n";
        resize = MeshPtr(ptr);*/
        //return resize;
    }
    return resize;
}
    

/* More code currently not in use, from the old D source. This was
   used in the first attempt at loading NIF meshes, where each submesh
   in the file was given a separate bone in a skeleton. Unfortunately
   the OGRE skeletons can't hold more than 256 bones, and some NIFs go
   way beyond that. The code might be of use if we implement animated
   submeshes like this (the part of the NIF that is animated is
   usually much less than the entire file, but the method might still
   not be water tight.)

// Insert a raw RGBA image into the texture system.
extern "C" void ogre_insertTexture(char* name, uint32_t width, uint32_t height, void *data)
{
  TexturePtr texture = TextureManager::getSingleton().createManual(
      name,         // name
      "General",    // group
      TEX_TYPE_2D,      // type
      width, height,    // width & height
      0,                // number of mipmaps
      PF_BYTE_RGBA,     // pixel format
      TU_DEFAULT);      // usage; should be TU_DYNAMIC_WRITE_ONLY_DISCARDABLE for
                        // textures updated very often (e.g. each frame)

  // Get the pixel buffer
  HardwarePixelBufferSharedPtr pixelBuffer = texture->getBuffer();

  // Lock the pixel buffer and get a pixel box
  pixelBuffer->lock(HardwareBuffer::HBL_NORMAL); // for best performance use HBL_DISCARD!
  const PixelBox& pixelBox = pixelBuffer->getCurrentLock();

  void *dest = pixelBox.data;

  // Copy the data
  memcpy(dest, data, width*height*4);

  // Unlock the pixel buffer
  pixelBuffer->unlock();
}


*/
