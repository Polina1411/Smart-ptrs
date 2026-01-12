#pragma once

#include "sw_fwd.h"  // Forward declaration
#include <utility>
#include <cstddef>  // std::nullptr_t

template <typename T>
class SharedPtr {
private:
    T* ptr_ = nullptr;
    control_block::BasicControlBlock* control_block_ = nullptr;
    SharedPtr(control_block::BasicControlBlock* control_block, T* ptr) {
        if (IsEmpty(control_block)) {
            return;
        }
        control_block_ = control_block;
        ptr_ = ptr;
    }
    using CB = control_block::ControlBlockMakeShared<T>;
    SharedPtr(CB* control_block) : control_block_(control_block) {
        control_block_ = control_block;
        ptr_ = static_cast<T*>(static_cast<void*>(&control_block->value));
        Increment();
    }
    bool IsEmpty(control_block::BasicControlBlock* control_block) {
        return control_block == nullptr || control_block->shared_ref == 0;
    }
    void Clearr() {
        control_block_ = nullptr;
        ptr_ = nullptr;
    }
    void Clear() {
        if (control_block_ == nullptr) {
            return;
        }
        Decrement();
        if (control_block_->shared_ref != 0) {
            return;
        }
        control_block_->DeleteRes();
        if (control_block_->weak_ref == 0) {
            control_block_->DestroyThis();
        }
        Clearr();
    }
    template <typename New>
    void Construct(New* raw_ptr) {
        auto* control_block = new control_block::ControlBlockR<New>(raw_ptr);
        Construct(control_block, raw_ptr);
    }
    template <typename New>
    void Construct(control_block::BasicControlBlock* block, New* ptr) {
        control_block_ = block;
        ptr_ = ptr;
    }

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    template <typename New>
    friend class SharedPtr;
    template <typename New>
    friend class WeakPtr;
    template <typename New, typename... Args>
    friend SharedPtr<New> MakeShared(Args&&... args);
    void Increment() {
        if (control_block_ != nullptr) {
            ++control_block_->shared_ref;
        }
    }
    void Decrement() {
        if (control_block_ != nullptr) {
            --control_block_->shared_ref;
        }
    }
    SharedPtr() = default;
    SharedPtr(std::nullptr_t) {
    }
    template <typename New>
    explicit SharedPtr(New* ptr) : ptr_(ptr) {
        using ContB = control_block::ControlBlockR<New>;
        control_block_ = new ContB(ptr);
        Increment();
    }

    SharedPtr(const SharedPtr& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        Increment();
    }
    SharedPtr(SharedPtr&& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        other.Clearr();
    }
    template <typename New>
    SharedPtr(const SharedPtr<New>& other)
        : control_block_(other.control_block_), ptr_(other.ptr_) {
        Increment();
    }
    template <typename New>
    SharedPtr(SharedPtr<New>&& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        other.Clearr();
    }
    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : control_block_(other.control_block_), ptr_(ptr) {
        Increment();
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (IsEmpty(other.control_block_)) {
            throw BadWeakPtr{};
        }
        control_block_ = other.control_block_;
        ptr_ = other.ptr_;
        Increment();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        Clear();
        control_block_ = other.control_block_;
        ptr_ = other.ptr_;
        Increment();
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        if (this == &other) {
            return *this;
        }
        Clear();
        control_block_ = other.control_block_;
        ptr_ = other.ptr_;
        Increment();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        Clear();
    }
    template <typename New>
    void Reset(New* ptr) {
        Clear();
        Construct(ptr);
        Increment();
    }
    void Swap(SharedPtr& other) {
        std::swap(control_block_, other.control_block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (control_block_ == nullptr) {
            return 0;
        }
        return control_block_->shared_ref;
    }
    explicit operator bool() const {
        return ptr_ != nullptr;
    }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return static_cast<void*>(left.Get()) == static_cast<void*>(right.Get());
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    using ControlBlockN = control_block::ControlBlockMakeShared<T>;
    auto* control_block = new ControlBlockN(std::forward<Args>(args)...);
    return SharedPtr<T>(control_block);
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
