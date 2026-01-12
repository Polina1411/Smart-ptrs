#pragma once
#include <utility>
#include "sw_fwd.h"  // Forward declaration
template <typename T>
class WeakPtr;
// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
private:
    control_block::BasicControlBlock* control_block_ = nullptr;
    T* ptr_ = nullptr;
    void Increment() {
        if (control_block_ != nullptr) {
            ++control_block_->weak_ref;
        }
    }
    void Decrement() {
        if (control_block_ != nullptr) {
            --control_block_->weak_ref;
        }
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
        if (control_block_->shared_ref == 0 && control_block_->weak_ref == 0) {
            control_block_->DestroyThis();
        }
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
    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        Increment();
    }
    WeakPtr(WeakPtr&& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        other.Clearr();
    }
    template <typename New>
    WeakPtr(const WeakPtr<New>& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        Increment();
    }
    template <typename New>
    WeakPtr(WeakPtr<New>&& other) : control_block_(other.control_block_), ptr_(other.ptr_) {
        other.Clearr();
    }
    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    template <typename New>
    WeakPtr(const SharedPtr<New>& other) noexcept
        : control_block_(other.control_block_), ptr_(other.ptr_) {
        Increment();
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this != &other) {
            Clear();
            control_block_ = other.control_block_;
            ptr_ = other.ptr_;
            Increment();
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (this != &other) {
            Clear();
            control_block_ = other.control_block_;
            ptr_ = other.ptr_;
            Increment();
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        WeakPtr ptr;
        Swap(ptr);
    }
    void Swap(WeakPtr& other) {
        std::swap(control_block_, other.control_block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (control_block_ == nullptr) {
            return 0;
        }
        return control_block_->shared_ref;
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }
};
