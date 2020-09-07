#include "Core/IO/BufferView.h"

namespace Core::IO {

BufferViewWindow::BufferViewWindow(BufferView bufferView, uint64_t windowSize)
  : view(bufferView), window(windowSize), windowStart(0) {}
BufferViewWindow::BufferViewWindow(Core::Array<std::byte>& buffer, uint64_t windowSize)
  : view(BufferView{buffer.rawData(), buffer.count()}), window(windowSize), windowStart(0) {}

const std::byte* BufferViewWindow::operator*() const {
    return view.data + windowStart;
}

BufferViewWindow::operator bool() const {
    return windowStart < view.size;
}

BufferViewWindow& operator++(BufferViewWindow& view) {
    ASSERT(view.view.size >= view.windowStart);
    view.windowStart += view.window;

    return view;
}

bool operator==(const BufferViewWindow& a, const BufferViewWindow& b) {
    return a.view.data == b.view.data && a.view.size == b.view.size && a.window == b.window &&
           a.windowStart == b.windowStart;
}

bool operator!=(const BufferViewWindow& a, const BufferViewWindow& b) {
    return !(a == b);
}
}    // namespace Core::IO