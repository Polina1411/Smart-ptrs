#pragma once

#include <exception>
namespace control_block {
struct BasicControlBlock {
public:
    int shared_ref = 0;
    int weak_ref = 0;
    void DeleteRes() {
        Deleter();
    }
    BasicControlBlock() = default;
    void DestroyThis() {
        delete this;
    }
    virtual ~BasicControlBlock() = default;

protected:
    virtual void Deleter() = 0;
};
template <typename T>
struct ControlBlockR : BasicControlBlock {
    T* ptr_;
    ControlBlockR() = default;
    ControlBlockR(T* pointer) : ptr_(pointer) {
    }
    virtual void Deleter() override {
        delete ptr_;
    }
};
template <typename T>
struct ControlBlockMakeShared : public BasicControlBlock {
    alignas(T) char value[sizeof(T)];
    template <typename... Args>
    ControlBlockMakeShared(Args&&... args) {
        new (static_cast<T*>(static_cast<void*>(value))) T(std::forward<Args>(args)...);
    }
    virtual void Deleter() override {
        static_cast<T*>(static_cast<void*>(value))->~T();
    }
};
}  // namespace control_block

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename New>
class EnableSharedFromThis;