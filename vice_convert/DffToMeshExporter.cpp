#include "DffToMeshExporter.h"
#include "Exception.h"
#include "MaterialDatabase.h"

#include <gtaformats/gtadff.h>


#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreMeshSerializer.h>
#include <OgreMeshManager.h>
#include <OgreHardwareBufferManager.h>

// boost is automatically included by ogre
#include <CrxFile.h>

#include <fstream>
#include <vector>

using std::ifstream;
using std::ios;
using std::vector;

DffToMeshExporter::DffToMeshExporter(Ogre::MeshManager *mmgr,
                                     Ogre::HardwareBufferManager *hwbmgr,
                                     Ogre::MeshSerializer *serializer,
                                     MaterialDatabase* matDB
                                    )
{
    this->m_mmgr = mmgr;
    this->m_hwbmgr = hwbmgr;
    this->m_serializer = serializer;
	this->m_matDB = matDB;
}

DffToMeshExporter::~DffToMeshExporter()
{

}

void DffToMeshExporter::Export(vector<string> matList, string sourceFile, string dffStem, string exportPath, bool bOverwrite)
{
    ifstream sourceStream(sourceFile, ios::in | ios::binary);
	this->Export(matList, &sourceStream, dffStem, exportPath, bOverwrite);
	sourceStream.close();
}

void DffToMeshExporter::Export(vector<string> matList, istream *sourceFile, string dffStem, string exportPath, bool bOverwrite)
{
    DFFLoader loader;
    DFFMesh *dffMesh = loader.loadMesh(sourceFile);

	this->Export(matList, dffMesh, dffStem, exportPath, bOverwrite);
	
	delete dffMesh;
}

void DffToMeshExporter::Export(vector< string > matList, DFFMesh* dffMesh, string dffStem, string exportPath, bool bOverwrite)
{
	assert(dffMesh->getGeometryCount() >= 1);
	
	// for first clump use the normal name;
	// for static objects only use this way
	_exportSingleClump(matList, dffMesh->getGeometry(0), dffStem, dffStem, exportPath, bOverwrite);
	
	for (int i = 1; i < dffMesh->getGeometryCount(); i++) {
		string exportName = dffStem + CLUMP_SUFFIX + std::to_string(i);
		_exportSingleClump(matList, dffMesh->getGeometry(0), dffStem, exportName, exportPath, bOverwrite);
	}
}

// TODO: Add bone export and several other structures
void DffToMeshExporter::_exportSingleClump(vector<string> matList, DFFGeometry *dffGeometry, string dffStem, string exportName, string exportPath, bool bOverwrite)
{
	
    bool bPositions = false;
    bool bNormals = false;
    bool bTexCoords = false;
    bool bVertexColors = false;

    if (dffGeometry->getVertices() != NULL) {
        bPositions = true;
    }

    if (dffGeometry->getNormals() != NULL) {
        bNormals = true;
    }

    if (dffGeometry->getUVCoordSets() != NULL) {
        bTexCoords = true;
    }

    if (dffGeometry->getVertexColors() != NULL) {
        bVertexColors = true;
    }


    Ogre::MeshPtr mesh = this->m_mmgr->createManual(exportName, "General");
    mesh->sharedVertexData = new Ogre::VertexData();

    Ogre::VertexDeclaration *vertexDecl = mesh->sharedVertexData->vertexDeclaration;
    size_t offset = 0;

    if (bPositions) {
        vertexDecl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    }

    if (bNormals) {
        vertexDecl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
    }

    if (bTexCoords) {
        vertexDecl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
    }

    if (bVertexColors) {
        vertexDecl->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE, 0);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    }

    float *vertices = dffGeometry->getVertices();
    float *normals = dffGeometry->getNormals();
    float *uvs = dffGeometry->getUVCoordSets();
    uint8_t *vertexColors = dffGeometry->getVertexColors();
    uint32_t vertexCount = dffGeometry->getVertexCount();

    mesh->sharedVertexData->vertexCount = vertexCount;
    Ogre::HardwareVertexBufferSharedPtr hwvBuf = this->m_hwbmgr->createVertexBuffer(offset, vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    // bind the hardware vertex buffer to current mesh
    mesh->sharedVertexData->vertexBufferBinding->setBinding(0, hwvBuf);

    // lock the buffer to prevent other threads from access this buffer
    float *pVertex = static_cast<float *>(hwvBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

    // variables to calculate bounding box and sphere
    Ogre::Real min_x = 0, min_y = 0, min_z = 0, max_x = 0, max_y = 0, max_z = 0;
    Ogre::Real radius2 = 0;

    for (unsigned long i = 0; i < vertexCount; i++) {
        if (bPositions) {
            float x = vertices[i * 3];
            float y = vertices[i * 3 + 1];
            float z = vertices[i * 3 + 2];

            min_x = std::min(min_x, x);
            min_y = std::min(min_y, y);
            min_z = std::min(min_z, z);
            max_x = std::max(max_x, x);
            max_y = std::max(max_y, y);
            max_z = std::max(max_z, z);
            radius2 = std::max(radius2, x * x + y * y + z * z);

            (*pVertex++) = x;
            (*pVertex++) = y;
            (*pVertex++) = z;
        }

        if (bNormals) {
            (*pVertex++) = normals[i * 3];
            (*pVertex++) = normals[i * 3 + 1];
            (*pVertex++) = normals[i * 3 + 2];
        }

        if (bTexCoords) {
            (*pVertex++) = uvs[i * 2];
            (*pVertex++) = uvs[i * 2 + 1];
        }

        if (bVertexColors) {
            (*pVertex++) = vertexColors[i * 4] / 255.0f;
            (*pVertex++) = vertexColors[i * 4 + 1] / 255.0f;
            (*pVertex++) = vertexColors[i * 4 + 2] / 255.0f;
            (*pVertex++) = vertexColors[i * 4 + 3] / 255.0f;
        }
    }

    hwvBuf->unlock();


    // assumption that material count and geometry count must be the same
    assert(dffGeometry->getMaterialCount() == dffGeometry->getPartCount());
    for (unsigned i = 0; i < dffGeometry->getPartCount(); i++) {
        DFFGeometryPart *dffGeometryPart = dffGeometry->getPart(i);
        // how do we determine whether we need tristrip or quad??

        int indexCount = dffGeometryPart->getIndexCount();
        uint32_t *indices = dffGeometryPart->getIndices();

        // convert to triangleList for faster rendering and Ogre static Geometry only uses triangle List
        // IMPORTANT: gta uses triangle strip rather than triangle fans

        vector<uint16_t> indicesTriList;
        indicesTriList.reserve(indexCount);
        if (dffGeometry->isTriangleStripFormat()) {
            for (int i = 0; i < indexCount - 2; i++) {
                uint16_t a, b, c;
                if (i & 1) {
                    // reverse every odd triangle
                    a = (uint16_t)indices[i];
                    b = (uint16_t)indices[i + 2];
                    c = (uint16_t)indices[i + 1];
                } else {
                    a = (uint16_t)indices[i];
                    b = (uint16_t)indices[i + 1];
                    c = (uint16_t)indices[i + 2];
                }

                // discard degenerate triangles
                if (a == b || b == c || a == c) {
                    continue;
                }

                indicesTriList.push_back(a);
                indicesTriList.push_back(b);
                indicesTriList.push_back(c);
            }
        } else {
            for (int i = 0; i < indexCount; i++) {
                indicesTriList.push_back((uint16_t)indices[i]);
            }
        }

        indexCount = indicesTriList.size();

        Ogre::HardwareIndexBufferSharedPtr hwibuf =
            Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
                Ogre::HardwareIndexBuffer::IT_16BIT,
                indexCount,
                Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        uint16_t *pIndices = static_cast<uint16_t *>(hwibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

        // copy the content directly
        memcpy(pIndices, &indicesTriList[0], indexCount * sizeof(uint16_t));

        hwibuf->unlock();

        Ogre::SubMesh *subMesh = mesh->createSubMesh();
        subMesh->useSharedVertices = true;
        subMesh->indexData->indexBuffer = hwibuf;
        subMesh->indexData->indexCount = indexCount;
        subMesh->indexData->indexStart = 0;
		if (dffGeometry->getBoneCount())
		{}
        //subMesh->operationType = RenderOperation::OT_TRIANGLE_STRIP;

        // assuming one part utilizes only one texture
        //assert((*partIt)->getMaterial()->getTextureCount() == 1);
        DFFMaterial *dffMaterial = dffGeometryPart->getMaterial();

		subMesh->setMaterialName(matList[i]);

    }

    mesh->_setBounds(Ogre::AxisAlignedBox(min_x, min_y, min_z, max_x, max_y, max_z));
    mesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(radius2));
    mesh->load();

	// if bOverwrite is false then check if file exists
	Crx::File filePath(exportPath, false);
	filePath /= exportName + ".mesh";
	
	if (!bOverwrite && filePath.Exists())
	{
		this->m_mmgr->remove(mesh->getName());
		return;
	}
	
	this->m_serializer->exportMesh(&*mesh, filePath.string());
    this->m_mmgr->remove(mesh->getName());
}