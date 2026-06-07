#pragma once
#include "core/column.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>

class IAggregateState {
public:
    virtual ~IAggregateState() = default;
};

class IAggregateFunc {
public:
    virtual ~IAggregateFunc() = default;
    virtual std::shared_ptr<IAggregateState> CreateState() = 0;
    virtual void Update(std::shared_ptr<IAggregateState>, const Column&) = 0;
    virtual Column Finalize(std::shared_ptr<IAggregateState>) = 0;
    virtual Types GetInType() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

class CountState : public IAggregateState {
public:
    CountState() = default;
    ~CountState() override = default;
    size_t res{0};
};

class CountFunc : public IAggregateFunc {
public:
    CountFunc(Types type);
    ~CountFunc() override = default;
    std::shared_ptr<IAggregateState> CreateState() override;
    void Update(std::shared_ptr<IAggregateState>, const Column&) override;
    Column Finalize(std::shared_ptr<IAggregateState>) override;
    Types GetInType() const override;

private:
    Types type_;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

template <typename T>
class SumState : public IAggregateState {
public:
    SumState() = default;
    ~SumState() override = default;
    T res{0};
};

class SumFunc : public IAggregateFunc {
public:
    SumFunc(Types type);
    ~SumFunc() override = default;
    std::shared_ptr<IAggregateState> CreateState() override;
    void Update(std::shared_ptr<IAggregateState>, const Column&) override;
    Column Finalize(std::shared_ptr<IAggregateState>) override;
    Types GetInType() const override;

private:
    Types type_;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

class AvgState : public IAggregateState {
public:
    AvgState() = default;
    ~AvgState() override = default;
    size_t cnt{0};
    long double sum{0};
};

class AvgFunc : public IAggregateFunc {
public:
    AvgFunc(Types type);
    ~AvgFunc() override = default;
    std::shared_ptr<IAggregateState> CreateState() override;
    void Update(std::shared_ptr<IAggregateState>, const Column&) override;
    Column Finalize(std::shared_ptr<IAggregateState>) override;
    Types GetInType() const override;

private:
    Types type_;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

template <typename T>
class CountDistinctState : public IAggregateState {
public:
    CountDistinctState() = default;
    ~CountDistinctState() override = default;
    std::set<T> res{};
};

class CountDistinctFunc : public IAggregateFunc {
public:
    CountDistinctFunc(Types type);
    ~CountDistinctFunc() override = default;
    std::shared_ptr<IAggregateState> CreateState() override;
    void Update(std::shared_ptr<IAggregateState>, const Column&) override;
    Column Finalize(std::shared_ptr<IAggregateState>) override;
    Types GetInType() const override;

private:
    Types type_;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

template <typename T>
class MinState : public IAggregateState {
public:
    MinState(T init_val) : res(init_val) {
    }
    ~MinState() override = default;
    T res{};
};

class MinFunc : public IAggregateFunc {
public:
    MinFunc(Types type);
    ~MinFunc() override = default;
    std::shared_ptr<IAggregateState> CreateState() override;
    void Update(std::shared_ptr<IAggregateState>, const Column&) override;
    Column Finalize(std::shared_ptr<IAggregateState>) override;
    Types GetInType() const override;

private:
    Types type_;
};

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

template <typename T>
class MaxState : public IAggregateState {
public:
    MaxState(T init_val) : res(init_val) {
    }
    ~MaxState() override = default;
    T res{};
};

class MaxFunc : public IAggregateFunc {
public:
    MaxFunc(Types type);
    ~MaxFunc() override = default;
    std::shared_ptr<IAggregateState> CreateState() override;
    void Update(std::shared_ptr<IAggregateState>, const Column&) override;
    Column Finalize(std::shared_ptr<IAggregateState>) override;
    Types GetInType() const override;

private:
    Types type_;
};
