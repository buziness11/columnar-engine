#include "aggregate.h"
#include "column.h"
#include "types.h"
#include <exception>
#include <memory>

std::shared_ptr<IAggregateState> CountFunc::CreateState() {
    return std::make_shared<CountState>();
}

void CountFunc::Update(std::shared_ptr<IAggregateState> state,
                       const Column& col) {
    StatePtr ptr = std::dynamic_pointer_cast<State>(state);
    if (ptr) {
        ptr->res += col.GetSize();
    } else {
        throw std::exception();
    }
}

Column CountFunc::Finalize(std::shared_ptr<IAggregateState> state, Types) {
    StatePtr ptr = std::dynamic_pointer_cast<State>(state);
    return Column(std::vector<int32_t>(1, ptr->res), Types::kInt32_t);
}