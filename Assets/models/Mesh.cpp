#include "assets/models/Mesh.h"

#include "core/containers/Array.h"

#include <algorithm>

namespace {

struct VertexProxy {
    size_t index;
    Core::Span<std::byte> data;

    void appendDataTo(Core::Array<std::byte>& vertexData) const {
        vertexData.insertAll(data);
    }

    uint32_t firstNotEqualIndex(const VertexProxy& other) const {
        for(uint32_t i = 0; i < data.count(); i++) {
            if(data[i] != other.data[i]) {
                return i;
            }
        }
        return data.count();
    }
};
}    // namespace

namespace Assets {

void Mesh::deduplicateVertices() {
    uint32_t vertexSize = vertexFormat.byteCount();
    size_t vertexCount  = vertexData.count() / vertexSize;
    Core::Array<std::byte> dedupedVertexData;

    Core::Array<VertexProxy> proxies;

    for(size_t i = 0; i < vertexCount; i++) {
        proxies.insert(VertexProxy{
              .index = i,
              .data  = Core::Span(&vertexData[i * vertexSize], vertexSize),
        });
    }

    std::sort(proxies.begin(), proxies.end(), [=](const VertexProxy& a, const VertexProxy& b) {
        uint32_t firstNotEqualIndex = a.firstNotEqualIndex(b);
        return firstNotEqualIndex != vertexSize && a.data[firstNotEqualIndex] < b.data[firstNotEqualIndex];
    });

    Core::Array<uint32_t> indexRedirects(0, vertexCount);

    uint32_t currentUniqueVertexIndex = 0;
    const VertexProxy* uniqueVertex   = &proxies[0];
    uniqueVertex->appendDataTo(dedupedVertexData);

    for(const auto& proxy : proxies) {
        if(uniqueVertex->firstNotEqualIndex(proxy) != vertexSize) {
            currentUniqueVertexIndex++;
            uniqueVertex = &proxy;
            uniqueVertex->appendDataTo(dedupedVertexData);
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