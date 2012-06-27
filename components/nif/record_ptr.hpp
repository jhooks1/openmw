/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008-2010  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: http://openmw.sourceforge.net/

  This file (record_ptr.h) is part of the OpenMW package.

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

#ifndef _NIF_RECORD_PTR_H_
#define _NIF_RECORD_PTR_H_


#include "nif_file.hpp"
#include <vector>



namespace Nif
{
class Node;
class NiNode;
class Extra;
class Property;
class NiUVData;
class NiPosData;
class NiVisData;
class Controller;
class Controlled;
class NiSkinData;
class NiFloatData;
class NiMorphData;
class NiPixelData;
class NiColorData;
class NiKeyframeData;
class NiTriShapeData;
class NiSkinInstance;
class NiSourceTexture;
class NiRotatingParticlesData;
class NiAutoNormalParticlesData;
/** A reference to another record. It is read as an index from the
    NIF, and later looked up in the index table to get an actual
    pointer.
*/
template <class X>
class RecordPtrT
{
  int index;
  X* ptr;
  NIFFile *nif;

 public:

  RecordPtrT() : index(-2), ptr(NULL) {}

  /// Read the index from the nif
  void read(NIFFile *_nif)
  {
    // Can only read the index once
    assert(index == -2);

    // Store the NIFFile pointer for later
    nif = _nif;

    // And the index, of course
    index = nif->getInt();
  }

  /** Set the pointer explicitly. May be used when you are pointing to
      records in another file, eg. when you have a .nif / .kf pair.
  */
  void set(X *p)
  {
    ptr = p;
    index = -1;
  }

  /// Look up the actual object from the index
  X* getPtr()
  {
    // Have we found the pointer already?
    if(ptr == NULL)
      {
        // Get the record
        assert(index >= 0);
        Record *r = nif->getRecord(index);

        // And cast it
        ptr = dynamic_cast<X*>(r);
        assert(ptr != NULL);
      }
    return ptr;
  }

  /// Syntactic sugar
  X* operator->() { return getPtr(); }
  X& get() { return *getPtr(); }

  /// Pointers are allowed to be empty
  bool empty() { return index == -1 && ptr == NULL; }

  int getIndex() { return index; }
};

/** A list of references to other records. These are read as a list,
    and later converted to pointers as needed. Not an optimized
    implementation.
 */
template <class X>
class RecordListT
{
  typedef RecordPtrT<X> Ptr;
  std::vector<Ptr> list;
  NiNode* parent;
 public:
     RecordListT():parent(NULL) {}

  void read(NIFFile *nif, NiNode* _parent = NULL)
  {
    if(_parent != NULL)
        parent = _parent;
    int len = nif->getInt();
    list.resize(len);

    assert(len >= 0 && len < 1000);
    for(int i=0;i<len;i++)
      list[i].read(nif);
  }

  X& operator[](int index)
    {
      assert(index >= 0 && index < static_cast<int> (list.size()));
      Node* node = dynamic_cast<Node*> (list[index].getPtr());
      if(node != NULL && parent != NULL){
          node->parent = parent;
      }
      return list[index].get();
    }

  bool has(int index)
  {
    assert(index >= 0 && index < static_cast<int> (list.size()));
    return !list[index].empty();
  }

  int getIndex(int index)
    {
      if(has(index)) return list[index].getIndex();
      else return -1;
    }

  int length() { return list.size(); }
};




typedef RecordPtrT<Node> NodePtr;
typedef RecordPtrT<Extra> ExtraPtr;
typedef RecordPtrT<NiUVData> NiUVDataPtr;
typedef RecordPtrT<NiPosData> NiPosDataPtr;
typedef RecordPtrT<NiVisData> NiVisDataPtr;
typedef RecordPtrT<Controller> ControllerPtr;
typedef RecordPtrT<Controlled> ControlledPtr;
typedef RecordPtrT<NiSkinData> NiSkinDataPtr;
typedef RecordPtrT<NiMorphData> NiMorphDataPtr;
typedef RecordPtrT<NiPixelData> NiPixelDataPtr;
typedef RecordPtrT<NiFloatData> NiFloatDataPtr;
typedef RecordPtrT<NiColorData> NiColorDataPtr;
typedef RecordPtrT<NiKeyframeData> NiKeyframeDataPtr;
typedef RecordPtrT<NiTriShapeData> NiTriShapeDataPtr;
typedef RecordPtrT<NiSkinInstance> NiSkinInstancePtr;
typedef RecordPtrT<NiSourceTexture> NiSourceTexturePtr;
typedef RecordPtrT<NiRotatingParticlesData> NiRotatingParticlesDataPtr;
typedef RecordPtrT<NiAutoNormalParticlesData> NiAutoNormalParticlesDataPtr;

typedef RecordListT<Node> NodeList;
typedef RecordListT<Property> PropertyList;

} // Namespace
#endif
