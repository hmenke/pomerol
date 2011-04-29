#ifndef __INCLUDE_TWOPARTICLEGFCONTAINER_H
#define __INCLUDE_TWOPARTICLEGFCONTAINER_H

#include"Misc.h"
#include"ComputableObject.h"
#include"FourIndexObject.h"
#include"TwoParticleGF.h"
#include"FieldOperatorContainer.h"

class TwoParticleGFContainer : public ComputableObject, public FourIndexContainerObject, public Thermal
{
public:
    struct Element;
private:
    StatesClassification &S;
    Hamiltonian &H;
    DensityMatrix &DM; 
    IndexClassification &IndexInfo;
    FieldOperatorContainer &Operators;
    std::vector<IndexCombination*> InitialCombinations;
    std::vector<IndexCombination*> NonTrivialCombinations;
    std::map<IndexCombination,Element*>  mapNonTrivialCombinations;
    long NumberOfMatsubaras;
    void defineInitialIndices();
public:
    TwoParticleGFContainer(StatesClassification &S, Hamiltonian &H, DensityMatrix &DM, IndexClassification& IndexInfo, FieldOperatorContainer& Operators);

    void readInitialIndices(std::vector<IndexCombination*>&);
    void prepare();
    void compute(long NumberOfMatsubaras);
    bool vanishes(const IndexCombination&);
    long getNumberOfMatsubaras() const;

    ComplexType operator()(const IndexCombination&, long MatsubaraNumber1, long MatsubaraNumber2, long MatsubaraNumber3); 

    void dump();
    const std::vector<IndexCombination*>& getNonTrivialCombinations();
    const std::vector<IndexCombination*>& getTrivialCombinations();
private:
    void addInnerPermutationsOfIndexCombination(const IndexCombination *in_nontrivial);
};

struct TwoParticleGFContainer::Element
{
    TwoParticleGF* Computable2PGF;
    Permutation4 FrequenciesPermutation;
    bool isComputed;
    Element(TwoParticleGF* Computable2PGF, Permutation4 FrequenciesPermutation, bool isComputed);
    IndexCombination getLinkedIndices();
};
#endif // endif :: #ifndef __INCLUDE_TWOPARTICLEGFCONTAINER_H
