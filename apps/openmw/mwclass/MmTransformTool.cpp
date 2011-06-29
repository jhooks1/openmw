/*
This file is part of MeshMagick - An Ogre mesh file manipulation tool.
Copyright (C) 2007-2010 Daniel Wickert

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "MmTransformTool.h"

#include <OgreAnimation.h>
//#include <OgreStringConverter.h>

;
using namespace Ogre;
	

    TransformTool::TransformTool()
        : mTransform(Matrix4::IDENTITY),
          mNormaliseNormals(false),
          mUpdateBoundingBox(true),
          mFlipVertexWinding(false)
    {
		;
    }


	

    void TransformTool::processSkeletonFile(SkeletonPtr skeleton, bool calcTransform)
    {

        if (calcTransform)
        {
            calculateTransform();
        }
        processSkeleton(skeleton);
        //skeletonSerializer->saveSkeleton(outFile, true);
        //print("Skeleton saved as " + outFile + ".");
    }

    MeshPtr TransformTool::processMeshFile(MeshPtr mesh, Ogre::Vector3 scale)
    {
        //print("Processing mesh...");
		std::cout << "Processing mesh";
        calculateTransform(mesh, scale);
        processMesh(mesh);
		return mesh;
		/*
        if (mFollowSkeletonLink && mesh->hasSkeleton())
        {
            // In this case keep file name and also keep already determined transform
            String skeletonFileName = ToolUtils::getSkeletonFileName(mesh, inFile);
            processSkeletonFile(skeletonFileName, skeletonFileName, false);
        }*/
    }

	void TransformTool::processSkeleton(Ogre::SkeletonPtr skeleton)
	{
		processSkeleton(skeleton.get());
	}

    void TransformTool::processSkeleton(Ogre::Skeleton* skeleton)
    {
        Ogre::Skeleton::BoneIterator it = skeleton->getBoneIterator();
        while (it.hasMoreElements())
        {
            processBone(it.peekNext());
            it.moveNext();
        }

        for (unsigned short aniIdx = 0; aniIdx < skeleton->getNumAnimations(); ++aniIdx)
        {
            Ogre::Animation* ani = skeleton->getAnimation(aniIdx);
//            processAnimation(ani);
        }
    }

	/*
    void TransformTool::processAnimation(Ogre::Animation* ani)
    {
        print("Processing animation " + ani->getName() + "...", V_HIGH);

        // We only need to apply scaling and rotation, no translation.
        Matrix3 m3x3;
        mTransform.extract3x3Matrix(m3x3);
        Vector3 scale(
            m3x3.GetColumn(0).length(),
            m3x3.GetColumn(1).length(),
            m3x3.GetColumn(2).length());
        Animation::NodeTrackIterator trackIt = ani->getNodeTrackIterator();
        while (trackIt.hasMoreElements())
        {
            NodeAnimationTrack* track = trackIt.getNext();
            // We need to apply full transform to root bone translations and only scale to the others.
            if (track->getAssociatedNode()->getParent() == NULL)
            {
                // An animation track for a skeleton is only supposed to have
                // TransformKeyFrames, so just use these here.
                for (unsigned short frameIdx = 0; frameIdx < track->getNumKeyFrames(); ++frameIdx)
                {
                    TransformKeyFrame* keyframe = track->getNodeKeyFrame(frameIdx);
                    keyframe->setTranslate(m3x3 * keyframe->getTranslate());
                }
            }
            else
            {
                for (unsigned short frameIdx = 0; frameIdx < track->getNumKeyFrames(); ++frameIdx)
                {
                    TransformKeyFrame* keyframe = track->getNodeKeyFrame(frameIdx);
                    keyframe->setTranslate(scale * keyframe->getTranslate());
                }
            }
        }
    }*/

    void TransformTool::processBone(Ogre::Bone* bone)
    {
        //print("Processing bone " + bone->getName() + "...", V_HIGH);
		std::cout << "Processing bone" << bone->getName();
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

	void TransformTool::transform(Ogre::MeshPtr mesh, Ogre::Matrix4 transformation, bool followSkeleton)
	{
		mTransform = transformation;
		processMesh(mesh);

		if (followSkeleton && mesh->hasSkeleton())
		{
			processSkeleton(mesh->getSkeleton());
		}
	}

	void TransformTool::transform(Ogre::SkeletonPtr skeleton, Ogre::Matrix4 transformation)
	{
		mTransform = transformation;
		processSkeleton(skeleton);
	}

	void TransformTool::processMesh(Ogre::MeshPtr mesh)
	{
		processMesh(mesh.get());
	}

    void TransformTool::processMesh(Ogre::Mesh* mesh)
    {
        mBoundingBox.setNull();

        if (mesh->sharedVertexData != NULL)
        {
            processVertexData(mesh->sharedVertexData);
        }

        for(int i = 0;i < mesh->getNumSubMeshes();i++)
        {
         Ogre::SubMesh* submesh = mesh->getSubMesh(i);
		 //submesh->
		   //sub->
            if (submesh->vertexData != NULL)
            {
                processVertexData(submesh->vertexData);
            }
            if (submesh->indexData != NULL)
            {
            	processIndexData(submesh->indexData);
            }
        }

        // Process poses, if there are any
        for (unsigned short i = 0; i < mesh->getPoseCount(); ++i)
        {
            processPose(mesh->getPose(i));
        }

        // If there are vertex animations, process these too.
        if (mesh->hasVertexAnimation())
        {
            // Then process morph targets
            unsigned short count = mesh->getNumAnimations();
            for (unsigned short i = 0; i < count; ++i)
            {
                Animation* anim = mesh->getAnimation(i);
                Animation::VertexTrackIterator it = anim->getVertexTrackIterator();
                while (it.hasMoreElements())
                {
                    VertexAnimationTrack* track = it.getNext();
                    if (track->getAnimationType() == VAT_MORPH)
                    {
                        for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
                        {
                            processVertexMorphKeyFrame(track->getVertexMorphKeyFrame(i),
                                track->getAssociatedVertexData()->vertexCount);
                        }
                    }
                }
            }
        }

        if (mUpdateBoundingBox)
        {
            mesh->_setBounds(mBoundingBox, false);
        }
    }

	void TransformTool::processIndexData(IndexData* indexData)
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
		std::cout << "Flipping index order for vertex winding flipping." << V_HIGH;
		//print("Flipping index order for vertex winding flipping.", V_HIGH);
		Ogre::HardwareIndexBufferSharedPtr buffer = indexData->indexBuffer;
		unsigned char* data =
               static_cast<unsigned char*>(buffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

		if(buffer->getType() == Ogre::HardwareIndexBuffer::IT_16BIT)
		{
			// 16 bit
			std::cout << "Using 16bit indices" << V_HIGH;
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
//			print("using 32bit indices", V_HIGH);
			std::cout << "Using 32bit indices" << V_HIGH;

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

    void TransformTool::processVertexData(VertexData* vertexData)
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

    void TransformTool::processPositionElement(VertexData* vertexData,
        const VertexElement* vertexElem)
    {
        Ogre::HardwareVertexBufferSharedPtr buffer =
            vertexData->vertexBufferBinding->getBuffer(vertexElem->getSource());

        unsigned char* data =
            static_cast<unsigned char*>(buffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
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

    void TransformTool::processDirectionElement(VertexData* vertexData,
        const VertexElement* vertexElem)
    {
        // We only want to apply rotation to normal, binormal and tangent, so extract it.
        Quaternion rotation = mTransform.extractQuaternion();
        rotation.normalise();

        Ogre::HardwareVertexBufferSharedPtr buffer =
            vertexData->vertexBufferBinding->getBuffer(vertexElem->getSource());

        unsigned char* data =
            static_cast<unsigned char*>(buffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
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

    void TransformTool::processPose(Pose* pose)
    {
        Matrix3 m3x3;
        mTransform.extract3x3Matrix(m3x3);

        Pose::VertexOffsetIterator it = pose->getVertexOffsetIterator();
        while (it.hasMoreElements())
        {
            Vector3 offset = it.peekNextValue();
            Vector3 newOffset = m3x3 * offset;
            *it.peekNextValuePtr() = newOffset;
            it.moveNext();
        }
    }

    void TransformTool::processVertexMorphKeyFrame(VertexMorphKeyFrame* keyframe,
        size_t vertexCount)
    {
        Vector3* positions = static_cast<Vector3*>(
            keyframe->getVertexBuffer()->lock(HardwareBuffer::HBL_READ_ONLY));
        for (size_t i = 0; i < vertexCount; ++i)
        {
            positions[i] = mTransform * positions[i];
        }
        keyframe->getVertexBuffer()->unlock();
    }

	/*
    void TransformTool::setOptions(const OptionList& options)
    {
        mOptions = options;

        mNormaliseNormals = !OptionsUtil::isOptionSet(options, "no-normalise-normals");
        if (!mNormaliseNormals)
        {
            print("Don't normalise normals", V_HIGH);
        }
        mUpdateBoundingBox = !OptionsUtil::isOptionSet(options, "no-update-boundingbox");
        if (!mUpdateBoundingBox)
        {
            print("Don't update bounding box", V_HIGH);
        }
        mFlipVertexWinding = OptionsUtil::isOptionSet(options, "flip-vertex-winding");
        if (mFlipVertexWinding)
        {
            print("Flip vertex winding", V_HIGH);
        }
    }*/

    void TransformTool::calculateTransform(Ogre::MeshPtr mesh, Ogre::Vector3 scale)
    {
        // Calculate transform
        Ogre::Matrix4 transform = Ogre::Matrix4::IDENTITY;

                transform = Ogre::Matrix4::getScale(scale) * transform;
				std::cout << "Applying scaling";
                //print("Apply scaling " + StringConverter::toString(scale), V_HIGH);
           
            

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

        mTransform = transform;
        //print("final transform " + StringConverter::toString(mTransform), V_HIGH);
    }