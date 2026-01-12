#pragma once
#include <utility>
#include <type_traits>
template <typename T, bool is_empty = std::is_empty_v<T> && !std::is_final_v<T>>
struct Value;

template <typename T>
struct Value<T, true> : T {
    template <typename... Args>
    Value(Args&&... args) : T(std::forward<Args>(args)...) {
    }
    T& Get() {
        return *static_cast<T*>(this);
    }
    const T& Get() const {
        return *static_cast<const T*>(this);
    }
};

template <typename T>
struct Value<T, false> {
    T val;
    template <typename... Args>
    Value(Args&&... args) : val(std::forward<Args>(args)...) {
    }
    T& Get() {
        return val;
    }
    const T& Get() const {
        return val;
    }
};

template <typename F>
struct First : Value<F> {
    template <typename... Args>
    First(Args&&... args) : Value<F>(std::forward<Args>(args)...) {
    }
};

template <typename S>
struct Second : Value<S> {
    template <typename... Args>
    Second(Args&&... args) : Value<S>(std::forward<Args>(args)...) {
    }
};

template <typename F, typename S>
class CompressedPair : First<F>, Second<S> {
public:
    CompressedPair() = default;
    template <typename FirstT, typename SecondT>
    CompressedPair(FirstT&& first, SecondT&& second)
        : First<F>(std::forward<FirstT>(first)), Second<S>(std::forward<SecondT>(second)) {
    }

    F& GetFirst() {
        return First<F>::Get();
    }
    const F& GetFirst() const {
        return First<F>::Get();
    }
    S& GetSecond() {
        return Second<S>::Get();
    };
    const S& GetSecond() const {
        return Second<S>::Get();
    };
};