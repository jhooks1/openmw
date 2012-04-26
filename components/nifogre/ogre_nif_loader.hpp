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

#include <OgreResource.h>
#include <OgreMesh.h>
#include <assert.h>
#include <string>
#include    <boost/algorithm/string.hpp>
#include <Ogre.h>
#include <stdio.h>
#include <iostream>

#include <libs/mangle/vfs/servers/ogre_vfs.hpp>
#include "../nif/nif_file.hpp"
#include "../nif/node.hpp"
#include "../nif/data.hpp"
#include "../nif/property.hpp"
#include "../nif/controller.hpp"
#include "../nif/extra.hpp"
#include <libs/platform/strings.h>
#include "../nif/nif_types.hpp"

#include <vector>
#include <list>
// For warning messages
#include <iostream>
#include <limits>
using namespace boost::algorithm;


class BoundsFinder;

struct ciLessBoost : std::binary_function<std::string, std::string, bool>
{
    bool operator() (const std::string & s1, const std::string & s2) const {
                                               //case insensitive version of is_less
        return lexicographical_compare(s1, s2, is_iless());
    }
};

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

namespace NifOgre
{


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

class SkeletonNIFLoader: public Ogre::ManualResourceLoader{
public:
    static SkeletonNIFLoader& getSingleton();
    static SkeletonNIFLoader* getSingletonPtr();
    SkeletonNIFLoader() : resourceName(""), resourceGroup("General")  {}
    virtual void loadResource(Ogre::Resource *resource);
private:
    Ogre::Skeleton* mSkel;
    Mangle::VFS::OgreVFS *vfs;
    std::string resourceGroup;
    std::string resourceName;
    bool inSkeletonTree;
    void buildBones(Nif::Node *node, Ogre::Bone *parentBone);
    
};
class NIFLoader : Ogre::ManualResourceLoader
{
    public:
        static int numberOfMeshes;
        static NIFLoader& getSingleton();
        static NIFLoader* getSingletonPtr();

        virtual void loadResource(Ogre::Resource *resource);

        static Ogre::MeshPtr load(const std::string &name,
                                    const std::string &group="General");
        //void insertMeshInsideBase(Ogre::Mesh* mesh);
        std::vector<Nif::NiKeyframeData>* getAnim(std::string name);
        std::vector<Nif::NiTriShapeCopy>* getShapes(std::string name);
        std::map<std::string, float>* getTextIndices(std::string name);


        Ogre::Vector3 convertVector3(const Nif::Vector& vec);
        Ogre::Quaternion convertRotation(const Nif::Matrix& rot);
        Ogre::Matrix3 convertRotationToMatrix(const Nif::Matrix& rot);

        void setOutputAnimFiles(bool output);
        void setVerbosePath(std::string path);

    private:

        NIFLoader() : resourceName(""), resourceGroup("General"),  flip(false), mNormaliseNormals(false),
          mFlipVertexWinding(false), mOutputAnimFiles(false), inTheSkeletonTree(false)  {}
        NIFLoader(NIFLoader& n) {}

        void calculateTransform();


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

        std::string verbosePath;
        std::string resourceName;
        std::string resourceGroup;
        Ogre::Matrix4 mTransform;
        Ogre::AxisAlignedBox mBoundingBox;
        bool flip;
        bool mNormaliseNormals;
        bool mFlipVertexWinding;
        bool bNiTri;
        bool mOutputAnimFiles;
        std::multimap<std::string,std::string> MaterialMap;

        // pointer to the ogre mesh which is currently build
        Ogre::Mesh *mesh;
        Ogre::SkeletonPtr mSkel;
        Ogre::Vector3 vector;
        std::vector<Nif::NiTriShapeCopy> shapes;
        std::string name;
        std::string triname;
        std::vector<Nif::NiKeyframeData> allanim;

		std::map<std::string,float> textmappings;
		std::map<std::string,std::map<std::string,float>,ciLessBoost> alltextmappings;
		std::map<std::string,std::vector<Nif::NiKeyframeData>,ciLessBoost> allanimmap;
		std::map<std::string,std::vector<Nif::NiTriShapeCopy>,ciLessBoost> allshapesmap;
        std::vector<Nif::NiKeyframeData> mAnim;
		std::vector<Nif::NiTriShapeCopy> mS;
        std::vector<Ogre::SubMesh*> needBoneAssignments;
        bool inTheSkeletonTree;
        

};

/* Stores four floating point numbers.  Used as a row of a Matrix44. */
struct Float4 {
	float data[4]; /*!< The four floating point numbers stored as an array. */ 

	/*! The bracket operator makes it possible to use this structure like a C++ array.
	 * \param[in] n The index into the data array.  Should be 0, 1, 2, or 3.
	 * \return The value at the given array index by reference so it can be read or set via the bracket operator.
	 */
	float & operator[](int n) {
		return data[n];
	}

	/*! The bracket operator makes it possible to use this structure like a C++ array.
	 * \param[in] n The index into the data array.  Should be 0, 1, 2, or 3.
	 * \return The value at the given array index by reference so it can be read or set via the bracket operator.
	 */
	float operator[](int n) const {
		return data[n];
	}

	/*! Default Constructor.*/
	Float4() {}

	/*! This constructor can be used to set all values in this structure during initialization
	 * \param[in] f1 The value to set the first floating point number to.
	 * \param[in] f2 The value to set the second floating point number to.
	 * \param[in] f3 The value to set the third floating point number to.
	 * \param[in] f4 The value to set the fourth floating point number to.
	 */
	Float4( float f1, float f2, float f3, float f4 ) {
		data[0] = f1;
		data[1] = f2;
		data[2] = f3;
		data[3] = f4;
	}

	

	/*! This function can be used to set all values in the structure at the same time.
	 * \param[in] f1 The value to set the first floating point number to.
	 * \param[in] f2 The value to set the second floating point number to.
	 * \param[in] f3 The value to set the third floating point number to.
	 * \param[in] f4 The value to set the fourth floating point number to.
	 */
	void Set( float f1, float f2, float f3, float f4 ) {
		data[0] = f1;
		data[1] = f2;
		data[2] = f3;
		data[3] = f4;
	}
};
/*! Stores a 4 by 4 matrix used for combined transformations. */
struct Matrix44 {
	/*! The 4x4 identity matrix constant */
	static const Matrix44 IDENTITY;

	Float4 rows[4]; /*!< The three rows of Float3 structures which hold three floating point numbers each. */ 
	
	/*! The bracket operator makes it possible to use this structure like a 4x4 C++ array.
	 * \param[in] n The index into the row array.  Should be 0, 1, 2, or 3.
	 * \return The Float4 structure for the given row index by reference so it can be read or set via the bracket operator.
	 */
	Float4 & operator[](int n) {
		return rows[n];
	}
	Float4 const & operator[](int n) const {
		return rows[n];
	}

	/*! Default constructor. Initializes Matrix to Identity. */
	Matrix44();

	/*! Copy constructor.  Initializes Matrix to another Matrix44.
	 * \param[in] m The matrix to initialize this one to. 
	 */
	Matrix44( const Matrix44 & m ) { memcpy(rows, m.rows, sizeof(Float4) * 4); }

	/*! This constructor can be used to set all values in this matrix during initialization
	 * \param[in] m11 The value to set at row 1, column 1.
	 * \param[in] m12 The value to set at row 1, column 2.
	 * \param[in] m13 The value to set at row 1, column 3.
	 * \param[in] m14 The value to set at row 1, column 4.
	 * \param[in] m21 The value to set at row 2, column 1.
	 * \param[in] m22 The value to set at row 2, column 2.
	 * \param[in] m23 The value to set at row 2, column 3.
	 * \param[in] m24 The value to set at row 2, column 4.
	 * \param[in] m31 The value to set at row 3, column 1.
	 * \param[in] m32 The value to set at row 3, column 2.
	 * \param[in] m33 The value to set at row 3, column 3.
	 * \param[in] m34 The value to set at row 3, column 4.
	 * \param[in] m41 The value to set at row 4, column 1.
	 * \param[in] m42 The value to set at row 4, column 2.
	 * \param[in] m43 The value to set at row 4, column 3.
	 * \param[in] m44 The value to set at row 4, column 4.
	 */
	Matrix44(
		float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44
	) {
		rows[0][0] = m11; rows[0][1] = m12; rows[0][2] = m13; rows[0][3] = m14;
		rows[1][0] = m21; rows[1][1] = m22; rows[1][2] = m23; rows[1][3] = m24;
		rows[2][0] = m31; rows[2][1] = m32; rows[2][2] = m33; rows[2][3] = m34;
		rows[3][0] = m41; rows[3][1] = m42; rows[3][2] = m43; rows[3][3] = m44;
	}

	/*! This constructor allows a 4x4 transform matrix to be initalized from a
	 * translate vector, a 3x3 rotation matrix, and a scale factor.
	 * \param[in] translate The translation vector that specifies the new x, y, and z coordinates.
	 * \param[in] rotation The 3x3 rotation matrix.
	 * \param[in] scale The scale factor.
	 */
	Matrix44( const Ogre::Vector3 & translate, const Ogre::Matrix3& rotation, float scale );

	/*! This constructor allows a 4x4 transform matrix to be initalized from a
	 * a 3x3 rotation matrix.
	 * \param[in] rotation The 3x3 rotation matrix.
	 */
	Matrix44( const Ogre::Matrix3 & rotation );

	/*! This function can be used to set all values in this matrix at the same time.
	 * \param[in] m11 The value to set at row 1, column 1.
	 * \param[in] m12 The value to set at row 1, column 2.
	 * \param[in] m13 The value to set at row 1, column 3.
	 * \param[in] m14 The value to set at row 1, column 4.
	 * \param[in] m21 The value to set at row 2, column 1.
	 * \param[in] m22 The value to set at row 2, column 2.
	 * \param[in] m23 The value to set at row 2, column 3.
	 * \param[in] m24 The value to set at row 2, column 4.
	 * \param[in] m31 The value to set at row 3, column 1.
	 * \param[in] m32 The value to set at row 3, column 2.
	 * \param[in] m33 The value to set at row 3, column 3.
	 * \param[in] m34 The value to set at row 3, column 4.
	 * \param[in] m41 The value to set at row 4, column 1.
	 * \param[in] m42 The value to set at row 4, column 2.
	 * \param[in] m43 The value to set at row 4, column 3.
	 * \param[in] m44 The value to set at row 4, column 4.
	 */
	void Set(
		float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44
	) {
		rows[0][0] = m11; rows[0][1] = m12; rows[0][2] = m13; rows[0][3] = m14;
		rows[1][0] = m21; rows[1][1] = m22; rows[1][2] = m23; rows[1][3] = m24;
		rows[2][0] = m31; rows[2][1] = m32; rows[2][2] = m33; rows[2][3] = m34;
		rows[3][0] = m41; rows[3][1] = m42; rows[3][2] = m43; rows[3][3] = m44;
	}

	/* Multiplies this matrix by another.
	 * \param[in] rh The matrix to multiply this one with.
	 * \return The result of the multiplication.
	 */
	 Matrix44 operator*( const Matrix44 & rh ) const;

	/* Multiplies this matrix by another and sets the result to itself.
	 * \param[in] rh The matrix to multiply this one with.
	 * \return This matrix is returned.
	 */
	Matrix44 & operator*=( const Matrix44 & rh );

	/* Multiplies this matrix by a scalar value.
	 * \param[in] rh The scalar value to multiply each component of this matrix by.
	 * \return The result of the multiplication.
	 */
	Matrix44 operator*( float rh ) const;

	/* Multiplies this matrix by a scalar value and sets the resutl to itself.
	 * \param[in] rh The scalar value to multiply each component of this matrix by.
	 * \return This matrix is returned.
	 */
	Matrix44 & operator*=( float rh );

	/* Multiplies this matrix by a vector with x, y, and z components.
	 * \param[in] rh The vector to multiply this matrix with.
	 * \return The result of the multiplication.
	 */
	Ogre::Vector3 operator*( const Ogre::Vector3 & rh ) const;

	/* Adds this matrix to another.
	 * \param[in] rh The matrix to be added to this one.
	 * \return The result of the addition.
	 */
	Matrix44 operator+( const Matrix44 & rh ) const;

	/* Adds this matrix to another and sets the result to itself.
	 * \param[in] rh The matrix to be added to this one.
	 * \return This matrix is returned.
	 */
	Matrix44 & operator+=( const Matrix44 & rh );

	/* Adds this matrix to another.
	* \param[in] rh The matrix to be added to this one.
	* \return The result of the addition.
	*/
	Matrix44 operator-( const Matrix44 & rh ) const;

	/* Adds this matrix to another and sets the result to itself.
	* \param[in] rh The matrix to be added to this one.
	* \return This matrix is returned.
	*/
	Matrix44 & operator-=( const Matrix44 & rh );

	/* Sets the values of this matrix to those of the given matrix.
	 * \param[in] rh The matrix to copy values from.
	 * \return This matrix is returned.
	 */
	Matrix44 & operator=( const Matrix44 & rh );

	/* Allows the contents of the matrix to be printed to an ostream.
	 * \param[in] lh The ostream to insert the text into.
	 * \param[in] rh The matrix to insert into the stream.
	 * \return The given ostream is returned.
	 */

	/* Compares two 4x4 matricies.  They are considered equal if all components are equal.
	 * \param[in] rh The matrix to compare this one with.
	 * \return true if the matricies are equal, false otherwise.
	 */
	bool operator==( const Matrix44 & rh ) const;

	/* Compares two 4x4 matricies.  They are considered inequal if any corresponding
	 * components are inequal.
	 * \param[in] rh The matrix to compare this one with.
	 * \return true if the matricies are inequal, false otherwise.
	 */
	bool operator!=( const Matrix44 & rh ) const;

	/*! Calculates the transpose of this matrix.
	 * \return The transpose of this matrix.
	 */
	Matrix44 Transpose() const;

	/*! Calculates the determinant of this matrix.
	 * \return The determinant of this matrix.
	 */
	float Determinant() const;

	/*! Calculates the inverse of this matrix.
	 * \return The inverse of this matrix.
	 */ 

	/*! Returns a 3x3 submatrix of this matrix created by skipping the indicated row and column.
	 * \param[in] skip_r The row to skip.  Must be a value between 0 and 3.
	 * \param[in] skip_c The colum to skip.  Must be a value between 0 and 3.
	 * \return The 3x3 submatrix obtained by skipping the indicated row and column.
	 */
	Ogre::Matrix3 Submatrix( int skip_r, int skip_c ) const;

	/*! Calculates the adjunct of this matrix created by skipping the indicated row and column.
	 * \param[in] skip_r The row to skip.  Must be a value between 0 and 3.
	 * \param[in] skip_c The colum to skip.  Must be a value between 0 and 3.
	 * \return The adjunct obtained by skipping the indicated row and column.
	 */
	//float Adjoint( int skip_r, int skip_c ) const;

	Ogre::Matrix3 GetRotation() const;
	float GetScale() const;
	Ogre::Vector3 GetTranslation() const;

	//undocumented, may be removed
	void AsFloatArr( float out[4][4] ) {
		out[0][0] = rows[0][0]; out[0][1] = rows[0][1]; out[0][2] = rows[0][2]; out[0][3] = rows[0][3];
		out[1][0] = rows[1][0]; out[1][1] = rows[1][1]; out[1][2] = rows[1][2]; out[1][3] = rows[1][3];
		out[2][0] = rows[2][0]; out[2][1] = rows[2][1]; out[2][2] = rows[2][2]; out[2][3] = rows[2][3];
		out[3][0] = rows[3][0]; out[3][1] = rows[3][1]; out[3][2] = rows[3][2]; out[3][3] = rows[3][3];
	}

   // undocumented
  void Decompose( Ogre::Vector3 & translate, Ogre::Matrix3& rotation, float & scale ) const;
};

}

#endif
