namespace Core::Algorithms {
template <typename T>
void CombineHash(size_t& currentValue, const T& valueToHash) {
    currentValue ^= std::hash<T>{}(valueToHash) + 0x9e3779b9 + (currentValue << 6) + (currentValue >> 2);
}

template <typename... Ts>
size_t CombineHashes(const Ts&... args) {
    size_t hash = 0;
    (CombineHash(hash, args), ...);
    return hash;
}
}    // namespace Core::Algorithms
