#include "Core/IO/WindowedBufferView.h"

namespace Core::IO {

WindowedBufferView::WindowedBufferView(std::byte* buffer, uint64_t bufferSize, uint64_t window)
  : data(buffer), size(bufferSize), windowSize(window) {}
WindowedBufferView::WindowedBufferView(Core::Array<std::byte>& buffer, uint64_t window)
  : data(buffer.data()), size(buffer.size()), windowSize(window) {}
const std::byte* WindowedBufferView::operator*() const {
    return data;
}

WindowedBufferView::operator bool() const {
    return size > 0;
}

WindowedBufferView& operator++(WindowedBufferView& view) {
    assert(view.size >= view.windowSize);

    view.data += view.windowSize;
    view.size -= view.windowSize;

    return view;
}

bool operator==(const WindowedBufferView& a, const WindowedBufferView& b) {
    return a.data == b.data && a.size == b.size && a.windowSize == b.windowSize;
}

bool operator!=(const WindowedBufferView& a, const WindowedBufferView& b) {
    return !(a == b);
}
}    // namespace Core::IO