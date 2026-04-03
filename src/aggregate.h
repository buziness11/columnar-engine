#include "column.h"
#include <cstddef>
#include <cstdint>
#include <memory>

class IAggregateState {
    friend class IAggregateFunc;

public:
    virtual ~IAggregateState() = default;
    // virtual void Merge(std::shared_ptr<IAggregateState>) = 0;
};

class CountState : public IAggregateState {
public:
    CountState() = default;
    ~CountState() override = default;
    size_t res{0};
};

class IAggregateFunc {
public:
    virtual ~IAggregateFunc() = default;
    virtual std::shared_ptr<IAggregateState> CreateState() = 0;
    virtual void Update(std::shared_ptr<IAggregateState>, const Column&) = 0;
    // virtual void UpdateBatch(std::vector<std::shared_ptr<IAggregateState>>,
    //                          Column) = 0;
    virtual Column Finalize(std::shared_ptr<IAggregateState>, Types) = 0;
};

class CountFunc : public IAggregateFunc {
public:
    ~CountFunc() override = default;
    std::shared_ptr<IAggregateState> CreateState() override;
    void Update(std::shared_ptr<IAggregateState>, const Column&) override;
    // void UpdateBatch(std::vector<std::shared_ptr<IAggregateState>>,
    //                  Column) override = 0;
    Column Finalize(std::shared_ptr<IAggregateState>, Types) override;

private:
    using State = CountState;
    using StatePtr = std::shared_ptr<State>;
};