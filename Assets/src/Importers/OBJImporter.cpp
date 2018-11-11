#include "Assets/Importers/OBJImporter.h"

#include <Core/Algorithms/Strings.h>
#include <Core/IO/FileSystem/FileSystem.h>
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

    void appendTo(std::vector<std::byte>& data) const {
        data.insert(data.end(),
                    reinterpret_cast<const std::byte*>(&x),
                    reinterpret_cast<const std::byte*>(&x) + sizeof(float));
        data.insert(data.end(),
                    reinterpret_cast<const std::byte*>(&y),
                    reinterpret_cast<const std::byte*>(&y) + sizeof(float));
        data.insert(data.end(),
                    reinterpret_cast<const std::byte*>(&z),
                    reinterpret_cast<const std::byte*>(&z) + sizeof(float));
    }
};

struct Vector2 {
    float x, y;
    Vector2(const std::vector<std::string_view>& elements)
      : x(Core::Algorithms::String::ParseFloat(elements[1])), y(Core::Algorithms::String::ParseFloat(elements[2])) {}

    void appendTo(std::vector<std::byte>& data) const {
        data.insert(data.end(),
                    reinterpret_cast<const std::byte*>(&x),
                    reinterpret_cast<const std::byte*>(&x) + sizeof(float));
        data.insert(data.end(),
                    reinterpret_cast<const std::byte*>(&y),
                    reinterpret_cast<const std::byte*>(&y) + sizeof(float));
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

Mesh Import(const std::filesystem::path& filename) {
    Mesh mesh;

    std::optional<std::string> data = Core::IO::ReadTextFile(filename);
    if(!data) {
        Core::Log::Error(OBJImporter, "Could not load file '{}'", filename.string());
        return mesh;
    }

    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> textureCoordinates;

    uint64_t currentMeshPartStartIndex = 0;
    std::string currentMeshPartName = "default";

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
            mesh.meshParts.emplace_back(MeshPart{currentMeshPartName,
                                        {currentMeshPartStartIndex, mesh.indexData.size() - currentMeshPartStartIndex}});
            currentMeshPartStartIndex = mesh.indexData.size();
            currentMeshPartName       = elements[1];
        } else if(elements[0] == "f") {
            // We can divide by format.totalSize here, even before it is set below because we initialize it to 1.
            // See definition of VertexFormat.
            uint32_t vertexCount = 1;
            if(!mesh.vertexData.empty()) {
                vertexCount = static_cast<uint32_t>(mesh.vertexData.size() / mesh.vertexFormat.byteCount());
            }


            int faceVertices = static_cast<int>(elements.size()) - 1;

            for(int i = 0; i < faceVertices; ++i) {
                Core::Algorithms::String::SplitIntoBuffer(elements[i + 1], '/', face);

                if(i > 1) {
                    mesh.indexData.push_back(vertexCount + 0);
                    mesh.indexData.push_back(vertexCount + i - 1);
                    mesh.indexData.push_back(vertexCount + i);
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

                OBJIndexFind(positions, vertexIndex).appendTo(mesh.vertexData);
                auto& vertexProperty        = mesh.vertexFormat.properties[Assets::VertexUsage::POSITION];
                vertexProperty.usage        = Assets::VertexUsage::POSITION;
                vertexProperty.byteOffset   = 0;
                vertexProperty.elementCount = 3;

                uint32_t currentOffset = PositionElements * sizeof(float);

                if(normalIndex) {
                    auto& vertexProperty = mesh.vertexFormat.properties[Assets::VertexUsage::NORMAL];
                    assert(vertexProperty.elementCount == 0 || vertexProperty.byteOffset == currentOffset);

                    vertexProperty.usage        = Assets::VertexUsage::NORMAL;
                    vertexProperty.byteOffset   = currentOffset;
                    vertexProperty.elementCount = 3;

                    OBJIndexFind(normals, *normalIndex).appendTo(mesh.vertexData);
                    currentOffset += NormalElements * sizeof(float);
                }
                if(textureCoordinateIndex) {
                    auto& vertexProperty = mesh.vertexFormat.properties[Assets::VertexUsage::TEXTURE];
                    assert(vertexProperty.elementCount == 0 || vertexProperty.byteOffset == currentOffset);

                    vertexProperty.usage        = Assets::VertexUsage::TEXTURE;
                    vertexProperty.byteOffset   = currentOffset;
                    vertexProperty.elementCount = 2;

                    OBJIndexFind(textureCoordinates, *textureCoordinateIndex).appendTo(mesh.vertexData);
                    currentOffset += TextureElements * sizeof(float);
                }
            }
        }
    }

    mesh.meshParts.emplace_back(MeshPart{currentMeshPartName,
                                {currentMeshPartStartIndex, mesh.indexData.size() - currentMeshPartStartIndex}});

    return mesh;
}
}    // namespace Assets::OBJ
