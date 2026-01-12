#pragma once

#include "compressed_pair.h"

#include <cstddef>
#include <algorithm>

template <typename T>
struct Slug {
    Slug() = default;
    template <typename New>
    Slug(const Slug<New>&) {
    }
    template <typename New>
    Slug& operator=(const Slug<New>&) {
        return *this;
    }
    void operator()(T* ptr) {
        delete ptr;
    }
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;
    template <typename New>
    Slug(const Slug<New>&) {
    }
    template <typename New>
    Slug& operator=(const Slug<New>&) {
        return *this;
    }
    void operator()(T* ptr) {
        delete[] ptr;
    }
};

template <typename T, typename Deleter = Slug<T>>
class UniquePtrBase {
public:
    void Clear(T* new_ptr = nullptr) {
        T* old_ptr = Get();
        Set(new_ptr);
        if (old_ptr != nullptr) {
            GetDeleter()(old_ptr);
        }
    }
    template <typename New, typename DeleterNew>
    friend class UniquePtrBase;
    UniquePtrBase(const UniquePtrBase&) = delete;
    UniquePtrBase& operator=(const UniquePtrBase&) = delete;
    explicit UniquePtrBase(T* ptr = nullptr) : data_(ptr, Deleter{}) {
    }
    UniquePtrBase(T* ptr, Deleter deleter) : data_(ptr, std::move(deleter)) {
    }
    template <typename New, typename DeleterNew>
    UniquePtrBase(UniquePtrBase<New, DeleterNew>&& other) noexcept
        : data_(static_cast<T*>(other.data_.GetFirst()),
                std::move(static_cast<DeleterNew&>(other.data_.GetSecond()))) {
        using OtherData = typename UniquePtrBase<New, DeleterNew>::Data;
        other.data_ = OtherData(nullptr, DeleterNew{});
    }
    template <typename New, typename DeleterNew>
    UniquePtrBase& operator=(UniquePtrBase<New, DeleterNew>&& other) noexcept {
        if (static_cast<void*>(this) == static_cast<void*>(&other)) {
            return *this;
        }
        UniquePtrBase t = std::move(other);
        Swap(t);
        return *this;
    }

    UniquePtrBase& operator=(std::nullptr_t) {
        Clear();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor
    ~UniquePtrBase() {
        Clear();
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void Set(T* ptr) {
        data_.GetFirst() = ptr;
    }

    T* Release() {
        auto ptr = Get();
        Set(nullptr);
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        Clear(ptr);
    }
    void Swap(UniquePtrBase& other) {
        std::swap(data_.GetFirst(), other.data_.GetFirst());
        std::swap(data_.GetSecond(), other.data_.GetSecond());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return nullptr != Get();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    template <typename U = T>
    U& operator*() const {
        return *static_cast<U*>(Get());
    }
    T* operator->() const {
        return Get();
    }

private:
    using Data = CompressedPair<T*, Deleter>;
    Data data_;
};

// Specialization for arrays
template <typename T, typename Deleter = Slug<T>>
class UniquePtr : public UniquePtrBase<T, Deleter> {
public:
    using Base = UniquePtrBase<T, Deleter>;
    template <typename New, typename DeleterNew>
    friend class UniquePtr;
    using Base::Base;
    using Base::operator=;
    template <typename New, typename DeleterNew>
    UniquePtr(UniquePtr<New, DeleterNew>&& other)
        : Base(std::move(static_cast<UniquePtrBase<New, DeleterNew>&>(other))) {
    }
    template <typename New, typename DeleterNew>
    UniquePtr& operator=(UniquePtr<New, DeleterNew>&& other) {
        static_cast<Base&>(*this) = std::move(static_cast<UniquePtrBase<New, DeleterNew>&>(other));
        return *this;
    }
};
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> : public UniquePtrBase<T, Deleter> {
private:
    using Base = UniquePtrBase<T, Deleter>;

public:
    template <typename New, typename DeleterNew>
    friend class UniquePtr;
    using Base::Base;
    using Base::operator=;
    T& operator[](auto i) {
        return Base ::Get()[i];
    }
    const T& operator[](auto i) const {
        return Base ::Get()[i];
    }
    template <typename New, typename DeleterNew>
    UniquePtr(UniquePtrBase<New[], DeleterNew>&& other)
        : Base(std::move(static_cast<UniquePtrBase<New[], DeleterNew>&>(other))) {
    }
};