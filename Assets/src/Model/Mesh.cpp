#include "Assets/Model/Mesh.h"

namespace {

struct VertexProxy {
    size_t index;
    std::byte* data;

    void appendDataTo(std::vector<std::byte>& vertexData, uint32_t count) const {
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

namespace Assets {

void Mesh::deduplicateVertices() {
    uint32_t vertexSize = vertexFormat.byteCount();
    size_t vertexCount  = vertexData.size() / vertexSize;
    Core::Array<std::byte> dedupedVertexData;

    Core::Array<VertexProxy> proxies;
    proxies.reserve(vertexCount);

    for(size_t i = 0; i < vertexCount; i++) {
        proxies.push_back({i, &vertexData[i * vertexSize]});
    }

    std::sort(proxies.begin(), proxies.end(), [=](const VertexProxy& a, const VertexProxy& b) {
        uint32_t firstNotEqualIndex = a.firstNotEqualIndex(b, vertexSize);
        return firstNotEqualIndex != vertexSize && a.data[firstNotEqualIndex] < b.data[firstNotEqualIndex];
    });

    Core::Array<uint32_t> indexRedirects(vertexCount);

    uint32_t currentUniqueVertexIndex = 0;
    const VertexProxy* uniqueVertex   = &proxies[0];
    uniqueVertex->appendDataTo(dedupedVertexData, vertexSize);

    for(const auto& proxy : proxies) {
        if(uniqueVertex->firstNotEqualIndex(proxy, vertexSize) != vertexSize) {
            currentUniqueVertexIndex++;
            uniqueVertex = &proxy;
            uniqueVertex->appendDataTo(dedupedVertexData, vertexSize);
        }
        indexRedirects[proxy.index] = currentUniqueVertexIndex;
    }

    for(uint32_t& index : indexData) {
        index = indexRedirects[index];
    }

    vertexData = std::move(dedupedVertexData);
}
void Mesh::optimizeVertexOrder() {}
}    // namespace Assets