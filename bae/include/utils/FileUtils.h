#pragma once
#include <bx/file.h>
#include <bx/filepath.h>
#include <bgfx/bgfx.h>
#include <bimg/decode.h>


namespace bae {
    class FileReader : public bx::FileReader
    {
        typedef bx::FileReader super;

    public:
        virtual bool open(const bx::FilePath& _filePath, bx::Error* _err) override
        {
            return super::open(_filePath.get(), _err);
        }
    };


    void* loadFile(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
    {
        if (bx::open(_reader, _filePath))
        {
            uint32_t size = (uint32_t)bx::getSize(_reader);
            void* data = BX_ALLOC(_allocator, size);
            bx::read(_reader, data, size);
            bx::close(_reader);
            if (NULL != _size)
            {
                *_size = size;
            }
            return data;
        }
        else
        {
            std::string err = "Failed to open: ";
            err.append(_filePath);
            throw std::runtime_error(err);
        }

        if (NULL != _size)
        {
            *_size = 0;
        }

        return NULL;
    }

    // Ripped out of bgfx_utils.cpp
    bgfx::TextureHandle loadTexture(const char* _filePath, uint32_t _flags = UINT32_MAX)
    {
        bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
        auto _reader = bae::FileReader{};
        auto allocator = bx::DefaultAllocator{};

        uint32_t size;
        void* data = loadFile(&_reader, &allocator, _filePath, &size);
        if (nullptr != data)
        {

            bimg::ImageContainer* imageContainer = bimg::imageParse(&allocator, data, size);
            //bool parsed = bimg::imageParse(imageContainer, data, size);

            if (nullptr != &imageContainer)
            {
                const bgfx::Memory* mem = bgfx::copy(
                    imageContainer->m_data,
                    imageContainer->m_size
                );
                BX_FREE(&allocator, data);

                if (imageContainer->m_cubeMap)
                {
                    handle = bgfx::createTextureCube(
                        uint16_t(imageContainer->m_width),
                        1 < imageContainer->m_numMips,
                        imageContainer->m_numLayers,
                        bgfx::TextureFormat::Enum(imageContainer->m_format),
                        _flags,
                        mem
                    );
                }
                else if (1 < imageContainer->m_depth)
                {
                    handle = bgfx::createTexture3D(
                        uint16_t(imageContainer->m_width)
                        , uint16_t(imageContainer->m_height)
                        , uint16_t(imageContainer->m_depth)
                        , 1 < imageContainer->m_numMips
                        , bgfx::TextureFormat::Enum(imageContainer->m_format)
                        , _flags
                        , mem
                    );
                }
                else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), _flags))
                {
                    handle = bgfx::createTexture2D(
                        uint16_t(imageContainer->m_width)
                        , uint16_t(imageContainer->m_height)
                        , 1 < imageContainer->m_numMips
                        , imageContainer->m_numLayers
                        , bgfx::TextureFormat::Enum(imageContainer->m_format)
                        , _flags
                        , mem
                    );
                }

                if (bgfx::isValid(handle))
                {
                    bgfx::setName(handle, _filePath);
                }
            }
        }

        return handle;
    }
}