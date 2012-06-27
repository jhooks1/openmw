/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008-2010  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: http://openmw.sourceforge.net/

  This file (node.h) is part of the OpenMW package.

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

#ifndef _NIF_NODE_H_
#define _NIF_NODE_H_

#include "controlled.hpp"
#include "data.hpp"

namespace Nif
{

/** A Node is an object that's part of the main NIF tree. It has
    parent node (unless it's the root), and transformation (location
    and rotation) relative to it's parent.
 */
struct NiNode;
class Node : public Named
{
public:
  NiNode* parent;
  // Node flags. Interpretation depends somewhat on the type of node.
  int flags;
  const Transformation *trafo;
  PropertyList props;

  // Bounding box info
  bool hasBounds;
  const Vector *boundPos;
  const Matrix *boundRot;
  const Vector *boundXYZ; // Box size
  Node(): parent(NULL){};

  void read(NIFFile *nif)
  {
    Named::read(nif);

    flags = nif->getShort();
    trafo = nif->getTrafo();
    props.read(nif);

    hasBounds = !!nif->getInt();
    if(hasBounds)
      {
        nif->getInt(); // always 1
        boundPos = nif->getVector();
        boundRot = nif->getMatrix();
        boundXYZ = nif->getVector();
      }

    boneTrafo = NULL;
    boneIndex = -1;
  }

  // Bone transformation. If set, node is a part of a skeleton.
  const NiSkinData::BoneTrafo *boneTrafo;

  // Bone weight info, from NiSkinData
  const NiSkinData::BoneInfo *boneInfo;

  // Bone index. If -1, this node is either not a bone, or if
  // boneTrafo is set it is the root bone in the skeleton.
  short boneIndex;

  void makeRootBone(const NiSkinData::BoneTrafo *tr)
  {
    boneTrafo = tr;
    boneIndex = -1;
  }

  void makeBone(short ind, const NiSkinData::BoneInfo &bi)
  {
    boneInfo = &bi;
    boneTrafo = bi.trafo;
    boneIndex = ind;
  }
};

struct NiTriShapeCopy
{
 std::string sname;
int shapeNumber;
 std::vector<std::string> boneSequence;
 Nif::NiSkinData::BoneTrafoCopy trafo;
 //Ogre::Quaternion initialBoneRotation;
 //Ogre::Vector3 initialBoneTranslation;
 std::vector<Ogre::Vector3> vertices;
 std::vector<Ogre::Vector3> normals;
 std::vector<Nif::NiSkinData::BoneInfoCopy> boneinfo;
 std::map<int, std::vector<Nif::NiSkinData::IndividualWeight> > vertsToWeights;
 Nif::NiMorphData morph;
};

struct NiNode : Node
{
  NodeList children;
  NodeList effects;

  /* Known NiNode flags:

     0x01 hidden
     0x02 use mesh for collision
     0x04 use bounding box for collision (?)
     0x08 unknown, but common
     0x20, 0x40, 0x80 unknown
   */

  void read(NIFFile *nif)
  {
    Node::read(nif);
    children.read(nif, this);
    effects.read(nif, this);
  }
};

struct NiTriShape : Node
{
  /* Possible flags:
     0x40 - mesh has no vertex normals ?

     Only flags included in 0x47 (ie. 0x01, 0x02, 0x04 and 0x40) have
     been observed so far.
   */

  NiTriShapeDataPtr data;
  NiSkinInstancePtr skin;

  void read(NIFFile *nif)
  {
    Node::read(nif);
    data.read(nif);
    skin.read(nif);
  }

  NiTriShapeCopy clone(){
	  NiTriShapeCopy copy;
      copy.sname = name.toString();
	  float *ptr = (float*)data->vertices.ptr;
	  float *ptrNormals = (float*)data->normals.ptr;
	  int numVerts = data->vertices.length / 3;
	  for(int i = 0; i < numVerts; i++)
	  {
		   float *current = (float*) (ptr + i * 3);
		   copy.vertices.push_back(Ogre::Vector3(*current, *(current + 1), *(current + 2)));

		   if(ptrNormals){
			float *currentNormals = (float*) (ptrNormals + i * 3);
		   copy.normals.push_back(Ogre::Vector3(*currentNormals, *(currentNormals + 1), *(currentNormals + 2)));
	 		  }
      }


  return copy;
  }
};

struct NiCamera : Node
{
  struct Camera
  {
    // Camera frustrum
    float left, right, top, bottom, nearDist, farDist;

    // Viewport
    float vleft, vright, vtop, vbottom;

    // Level of detail modifier
    float LOD;
  };

  const Camera *cam;

  void read(NIFFile *nif)
  {
    Node::read(nif);

    nif->getPtr<Camera>();

    nif->getInt(); // -1
    nif->getInt(); // 0
  }
};

struct NiAutoNormalParticles : Node
{
  NiAutoNormalParticlesDataPtr data;

  void read(NIFFile *nif)
  {
    Node::read(nif);
    data.read(nif);
    nif->getInt(); // -1
  }
};

struct NiRotatingParticles : Node
{
  NiRotatingParticlesDataPtr data;

  void read(NIFFile *nif)
  {
    Node::read(nif);
    data.read(nif);
    nif->getInt(); // -1
  }
};



} // Namespace
#endif
