#include "Assets/Importers/OBJImporter.h"

#include <Core/Containers/Span.h>

#include <Core/Algorithms/Strings.h>
#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/Logging/Logger.h>

#include "AssetsCore.h"

#include <optional>

namespace {
template <typename T>
const T& OBJIndexFind(Core::Array<T>& values, int64_t index) {
    if(index < 0) {
        return values[values.count() + index];
    } else {
        // indicies start at 1
        return values[index - 1];
    }
}
struct Vector3 {
    float x, y, z;
    Vector3(const Core::Array<std::string_view>& elements)
      : x(Core::Algorithms::String::ParseFloat(elements[1])),
        y(Core::Algorithms::String::ParseFloat(elements[2])),
        z(Core::Algorithms::String::ParseFloat(elements[3])) {}

    void appendTo(Core::Array<std::byte>& data) const {
        data.insertAll(Core::ToBytes(x));
        data.insertAll(Core::ToBytes(y));
        data.insertAll(Core::ToBytes(z));
    }
};

struct Vector2 {
    float x, y;
    Vector2(const Core::Array<std::string_view>& elements)
      : x(Core::Algorithms::String::ParseFloat(elements[1])), y(Core::Algorithms::String::ParseFloat(elements[2])) {}

    void appendTo(Core::Array<std::byte>& data) const {
        data.insertAll(Core::ToBytes(x));
        data.insertAll(Core::ToBytes(y));
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

    ASSIGN_OR_ASSERT(std::string data, Core::IO::ReadTextFile(filename));

    Core::Array<Vector3> positions;
    Core::Array<Vector3> normals;
    Core::Array<Vector2> textureCoordinates;

    uint64_t currentMeshPartStartIndex = 0;
    std::string currentMeshPartName    = "default";

    Core::Array<std::string_view> elements;
    Core::Array<std::string_view> face;

    const uint32_t PositionElements = 3;
    const uint32_t NormalElements   = 3;
    const uint32_t TextureElements  = 2;

    Core::Array<std::string_view> lines = Core::Algorithms::String::SplitLines(data);
    for(auto& line : lines) {
        // There may be more than one space between elements, this will break in such a case :(
        Core::Algorithms::String::SplitIntoBuffer(line, " \t", elements, Core::Algorithms::String::Filter::Empty);
        if(elements.isEmpty()) {
            continue;
        }

        if(elements[0] == "v") {
            positions.emplace(elements);
        } else if(elements[0] == "vt") {
            textureCoordinates.emplace(elements);
        } else if(elements[0] == "vn") {
            normals.emplace(elements);
        } else if(elements[0] == "usemtl") {
            mesh.meshParts.emplace(
                  MeshPart{currentMeshPartName,
                           {currentMeshPartStartIndex, mesh.indexData.count() - currentMeshPartStartIndex}});
            currentMeshPartStartIndex = mesh.indexData.count();
            currentMeshPartName       = elements[1];
        } else if(elements[0] == "f") {
            // We can divide by format.totalSize here, even before it is set below because we initialize it to 1.
            // See definition of VertexFormat.
            uint32_t vertexCount = 1;
            if(!mesh.vertexData.isEmpty()) {
                vertexCount = static_cast<uint32_t>(mesh.vertexData.count() / mesh.vertexFormat.byteCount());
            }


            int faceVertices = static_cast<int>(elements.count()) - 1;

            for(int i = 0; i < faceVertices; ++i) {
                Core::Algorithms::String::SplitIntoBuffer(elements[i + 1], '/', face);

                if(i > 1) {
                    mesh.indexData.insert(vertexCount + 0);
                    mesh.indexData.insert(vertexCount + i - 1);
                    mesh.indexData.insert(vertexCount + i);
                }

                int64_t vertexIndex = Core::Algorithms::String::ParseInt64(face[0]);
                std::optional<int64_t> textureCoordinateIndex;
                std::optional<int64_t> normalIndex;

                if(face.count() > 1 && face[1] != "") {
                    textureCoordinateIndex = Core::Algorithms::String::ParseInt64(face[1]);
                }

                if(face.count() > 2 && face[2] != "") {
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
                    ASSERT(vertexProperty.elementCount == 0 || vertexProperty.byteOffset == currentOffset);

                    vertexProperty.usage        = Assets::VertexUsage::NORMAL;
                    vertexProperty.byteOffset   = currentOffset;
                    vertexProperty.elementCount = 3;

                    OBJIndexFind(normals, *normalIndex).appendTo(mesh.vertexData);
                    currentOffset += NormalElements * sizeof(float);
                }
                if(textureCoordinateIndex) {
                    auto& vertexProperty = mesh.vertexFormat.properties[Assets::VertexUsage::TEXTURE];
                    ASSERT(vertexProperty.elementCount == 0 || vertexProperty.byteOffset == currentOffset);

                    vertexProperty.usage        = Assets::VertexUsage::TEXTURE;
                    vertexProperty.byteOffset   = currentOffset;
                    vertexProperty.elementCount = 2;

                    OBJIndexFind(textureCoordinates, *textureCoordinateIndex).appendTo(mesh.vertexData);
                    currentOffset += TextureElements * sizeof(float);
                }
            }
        }
    }

    mesh.meshParts.emplace(MeshPart{currentMeshPartName,
                                    {currentMeshPartStartIndex, mesh.indexData.count() - currentMeshPartStartIndex}});

    return mesh;
}
}    // namespace Assets::OBJ
