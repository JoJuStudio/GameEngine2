// source/asset/TinyGltf.cpp

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include "tiny_gltf.h"
#include <string>

namespace tinygltf {

// Dummy function: WriteImageData, because we disabled real saving.
bool WriteImageData(const std::string*,
    const std::string*,
    const Image*,
    bool,
    const FsCallbacks*,
    const URICallbacks*,
    std::string*,
    void*)
{
    return false;
}

} // namespace tinygltf
