/** \file src/GFContainer.h
** \brief A container for either creation or annihilation operators in eigenvector basis
**
** \author Andrey Antipov (antipov@ct-qmc.org)
*/


#ifndef __INCLUDE_GFCONTAINER_H
#define __INCLUDE_GFCONTAINER_H

#include"Misc.h"
#include"ComputableObject.h"
#include"GreensFunction.h"
#include"FieldOperatorContainer.h"

class GFContainer : public ComputableObject, public Thermal 
{

public:
    struct IndexCombination; 
    GFContainer(StatesClassification &S, Hamiltonian &H, DensityMatrix &DM, IndexClassification& IndexInfo, FieldOperatorContainer& Operators);
    void readInitialIndices(std::vector<IndexCombination*> &in);
    void prepare();
    void compute();
    bool vanishes(ParticleIndex i, ParticleIndex j);
    MatrixType& operator()(long MatsubaraNumber);
    ComplexType operator()(ParticleIndex i, ParticleIndex j, long MatsubaraNumber);
    void dumpToPlainText(long wn);
    std::vector<IndexCombination*> InitialIndices;
private:
    StatesClassification &S;
    Hamiltonian &H;
    DensityMatrix &DM;
    IndexClassification &IndexInfo;
    FieldOperatorContainer &Operators;

    std::map<IndexCombination, GreensFunction*> mapGreensFunctions;
    void defineInitialIndices();
};

struct GFContainer::IndexCombination
{
    ParticleIndex Indices[2];
    friend std::ostream& operator<<(std::ostream& output, const GFContainer::IndexCombination& out);
    bool operator<(const GFContainer::IndexCombination& rhs) const ;
    IndexCombination(ParticleIndex cindex1, ParticleIndex cdagindex2);
};

#endif // endif :: #ifndef __INCLUDE_GFCONTAINER_H
