#include "TextureToDDSExporter.h"
#include "MaterialDatabase.h"
#include <gtaformats/gtatxd.h>
#include <nv_dds.h>
#include <CrxFile.h>

using std::ios;

TextureToDDSExporter::TextureToDDSExporter(MaterialDatabase* matDB)
{
    this->m_matDB = matDB;
    //ilInit();
	
	
}

TextureToDDSExporter::~TextureToDDSExporter()
{

}


void TextureToDDSExporter::Export(string sourceFile, string txdStem, string exportPath, bool bOverwrite)
{
    ifstream sourceStream(sourceFile, ios::in | ios::binary);

    this->Export(&sourceStream, txdStem, exportPath, bOverwrite);
    sourceStream.close();
}

void TextureToDDSExporter::Export(TXDArchive* txdArchive, string txdName, string exportPath, bool bOverwrite)
{
	for (int i = 0; i < txdArchive->getTextureCount(); i++)
	{
		TXDTextureHeader* texHeader = txdArchive->getHeader(i);
		uint8_t* image = txdArchive->getTextureData(texHeader);
		
		_exportSingleTexture(*texHeader, image, exportPath, bOverwrite);
		delete [] image;
	}
}

void TextureToDDSExporter::Export(istream* sourceFile, string txdStem, string exportPath, bool bOverwrite)
{
    TXDArchive txdArchive(sourceFile);
	this->Export(&txdArchive, txdStem, exportPath, bOverwrite);
}

void TextureToDDSExporter::_exportSingleTexture(const TXDTextureHeader& texHeader, uint8_t* data, string exportPath, bool bOverwrite)
{
	Crx::File filePath(exportPath, false);
	filePath /= string(texHeader.getDiffuseName() + ".dds");
	
	if (!bOverwrite && filePath.Exists())
	{
		return;
	}

    // image to be exported as dds
    nv_dds::CDDSImage nv_image;
    TXDCompression compression = texHeader.getCompression();

    if (compression == DXT1 || compression == DXT3)
    {
        int nv_texFormat = 0;
        int nv_components = 0;

        switch (compression)
        {
        case DXT1:
            nv_texFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            nv_components = 3;
            break;
        case DXT3:
            nv_texFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            nv_components = 4;
            break;
        }

        unsigned int h = texHeader.getHeight();
        unsigned int w = texHeader.getWidth();
        unsigned int bpp = texHeader.getBytesPerPixel();
        unsigned int size = texHeader.computeMipmapDataSize(0);

        nv_dds::CTexture nv_tex(w, h, 1, size, data);

        // Add mipmaps
        data += size;
        // 0 is the base image
        for (int i = 1; i < texHeader.getMipmapCount(); i++)
        {
            w /= 2;
            h /= 2;

            if (w < 4  ||  h < 4)
                break;

            int mipsize = texHeader.computeMipmapDataSize(i);
            nv_dds::CSurface nv_surface(w, h, 1, mipsize, data);
            nv_tex.add_mipmap(nv_surface);

            data += mipsize;
        }

        nv_image.create_textureFlat(nv_texFormat, nv_components, nv_tex);
    }
    else
    {
        unsigned int h = texHeader.getHeight();
        unsigned int w = texHeader.getWidth();

        TXDTextureHeader to = texHeader;
        //toMip.setRasterFormat(RasterFormatR8G8B8A8);
        to.setRasterFormat(RasterFormatB8G8R8A8); // For some fucking reasons RGBA is inverted

        uint8_t* pixels = new uint8_t[h*w*4];
        TXDConverter conv;
        conv.convert(texHeader, to, data, pixels, 0, 0);

        nv_dds::CTexture nv_tex(w, h, 1, w * h * 4, pixels);
        delete [] pixels;

        data += texHeader.computeMipmapDataSize(0);
        // start from 1 as 0 is the base image
        for (int i = 1 ; i < to.getMipmapCount() ; i++) {

            h /= 2;
            w /= 2;

            if (w < 4  ||  h < 4)
                break;

            uint8_t* pixels = new uint8_t[w*h*4];

            TXDTextureHeader toMip = texHeader;
            //toMip.setRasterFormat(RasterFormatR8G8B8A8);
            toMip.setRasterFormat(RasterFormatB8G8R8A8); // For some fucking reasons RGBA is inverted

            TXDConverter mipConv;
            mipConv.convert(texHeader, toMip, data, pixels, i, i);

            nv_dds::CSurface nv_surface(w, h, 1, w*h*4, pixels);
            nv_tex.add_mipmap(nv_surface);

            data += texHeader.computeMipmapDataSize(i);
            delete[] pixels;
        }

        nv_image.create_textureFlat(nv_dds::DDSF_RGBA, 4, nv_tex);
    }

    nv_image.save(filePath.string(), false);
}
