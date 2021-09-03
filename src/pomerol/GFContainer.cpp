#include "pomerol/GFContainer.hpp"

namespace Pomerol {

void GFContainer::prepareAll(std::set<IndexCombination2> const& InitialIndices) {
    fill(InitialIndices);
    for(auto& el : ElementsMap)
        el.second->prepare();
}

void GFContainer::computeAll() {
    for(auto& el : ElementsMap)
        el.second->compute();
}

std::shared_ptr<GreensFunction> GFContainer::createElement(IndexCombination2 const& Indices) const {
    return std::make_shared<GreensFunction>(S,
                                            H,
                                            Operators.getAnnihilationOperator(Indices.Index1),
                                            Operators.getCreationOperator(Indices.Index2),
                                            DM);
}

} // namespace Pomerol
