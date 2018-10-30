#include "Importers/OBJImporter.h"

#include <Core/Algorithms/Strings.h>
#include <Core/File.h>
#include <Core/Logging/Logger.h>

#include "AssetsCore.h"

namespace {
template <typename T>
const T& OBJIndexFind(std::vector<T>& values, int64_t index) {
    if(index < 0) {
        return values[values.size() + index];
    } else {
        // indicies start at 1
        return values[index - 1];
    }
}
struct Vector3 {
    float x, y, z;
    Vector3(const std::vector<std::string_view>& elements)
      : x(Core::Algorithms::String::ParseFloat(elements[1])),
        y(Core::Algorithms::String::ParseFloat(elements[2])),
        z(Core::Algorithms::String::ParseFloat(elements[3])) {}

    void appendTo(std::vector<float>& data) const {
        data.push_back(x);
        data.push_back(y);
        data.push_back(z);
    }
};

struct Vector2 {
    float x, y;
    Vector2(const std::vector<std::string_view>& elements)
      : x(Core::Algorithms::String::ParseFloat(elements[1])), y(Core::Algorithms::String::ParseFloat(elements[2])) {}

    void appendTo(std::vector<float>& data) const {
        data.push_back(x);
        data.push_back(y);
    }
};

struct VertexProxy {
    size_t index;
    float* data;

    void appendDataTo(std::vector<float>& vertexData, uint32_t count) const {
        vertexData.insert(vertexData.end(), data, data + count);
    }

    uint32_t firstNotEqualIndex(const VertexProxy& other, uint32_t maxElementsToCheck) const {
        for(uint32_t i = 0; i < maxElementsToCheck; i++) {
            if(data[i] != other.data[i]) {
                return i;
            }
        }
        return maxElementsToCheck;
    }
};
}    // namespace

namespace Assets::OBJ {

Core::LogCategory OBJImporter("OBJImporter", &Assets);

OBJModel Import(const std::filesystem::path& filename) {
    OBJModel model;

    std::optional<std::string> data = Core::File::ReadTextFile(filename);
    if(!data) {
        Core::Log::Error(OBJImporter, "Could not load file '{}'", filename.string());
        return model;
    }

    std::unordered_map<std::string, size_t> meshPartIndices;

    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> textureCoordinates;

    Mesh defaultMesh;
    std::string currentMaterialName = "unknown";

    Mesh* currentMesh = &defaultMesh;

    std::vector<std::string_view> elements;
    std::vector<std::string_view> face;

    const uint32_t PositionElements = 3;
    const uint32_t NormalElements   = 3;
    const uint32_t TextureElements  = 2;

    std::vector<std::string_view> lines = Core::Algorithms::String::SplitLines(*data);
    for(auto& line : lines) {
        // There may be more than one space between elements, this will break in such a case :(
        Core::Algorithms::String::SplitIntoBuffer(line, " \t", elements, Core::Algorithms::String::Filter::Empty);
        if(elements.size() == 0) {
            continue;
        }

        if(elements[0] == "v") {
            positions.emplace_back(elements);
        } else if(elements[0] == "vt") {
            textureCoordinates.emplace_back(elements);
        } else if(elements[0] == "vn") {
            normals.emplace_back(elements);
        } else if(elements[0] == "usemtl") {
            currentMaterialName = elements[1];
            auto meshPartIndex  = meshPartIndices.find(currentMaterialName);
            if(meshPartIndex == meshPartIndices.end()) {
                meshPartIndices[currentMaterialName] = model.meshes.size();
                currentMesh                          = &model.meshes.emplace_back();
            } else {
                currentMesh = &model.meshes[meshPartIndex->second];
            }
        } else if(elements[0] == "f") {
            // We can divide by format.totalSize here, even before it is set below because we initialize it to 1.
            // See definition of VertexFormat.
            uint32_t vertexCount =
                  static_cast<uint32_t>(currentMesh->vertexData.size() / currentMesh->vertexFormat.elementCount);

            int faceVertices = static_cast<int>(elements.size()) - 1;

            for(int i = 0; i < faceVertices; ++i) {
                Core::Algorithms::String::SplitIntoBuffer(elements[i + 1], '/', face);

                if(i > 1) {
                    currentMesh->indexData.push_back(vertexCount + 0);
                    currentMesh->indexData.push_back(vertexCount + i - 1);
                    currentMesh->indexData.push_back(vertexCount + i);
                }

                int64_t vertexIndex = Core::Algorithms::String::ParseInt64(face[0]);
                std::optional<int64_t> textureCoordinateIndex;
                std::optional<int64_t> normalIndex;

                if(face.size() > 1 && face[1] != "") {
                    textureCoordinateIndex = Core::Algorithms::String::ParseInt64(face[1]);
                }

                if(face.size() > 2 && face[2] != "") {
                    normalIndex = Core::Algorithms::String::ParseInt64(face[2]);
                }

                OBJIndexFind(positions, vertexIndex).appendTo(currentMesh->vertexData);
                uint32_t currentOffset = PositionElements;

                if(normalIndex) {
                    assert(!currentMesh->vertexFormat.normalOffset ||
                           currentMesh->vertexFormat.normalOffset.value() == currentOffset);
                    currentMesh->vertexFormat.normalOffset = currentOffset;
                    OBJIndexFind(normals, *normalIndex).appendTo(currentMesh->vertexData);
                    currentOffset += NormalElements;
                }
                if(textureCoordinateIndex) {
                    assert(!currentMesh->vertexFormat.textureCoordinateOffset ||
                           currentMesh->vertexFormat.textureCoordinateOffset.value() == currentOffset);
                    currentMesh->vertexFormat.textureCoordinateOffset = currentOffset;
                    OBJIndexFind(textureCoordinates, *textureCoordinateIndex).appendTo(currentMesh->vertexData);
                    currentOffset += TextureElements;
                }

                currentMesh->vertexFormat.elementCount = currentOffset;
            }
        }
    }

    if(!defaultMesh.vertexData.empty()) {
        model.meshes.emplace_back(defaultMesh);
    }

    return model;
}
}    // namespace Assets::OBJ
