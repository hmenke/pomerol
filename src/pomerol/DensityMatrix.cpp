#include "pomerol/DensityMatrix.hpp"

#include <numeric>

namespace Pomerol {

DensityMatrix::DensityMatrix(const StatesClassification& S, const Hamiltonian& H, RealType beta) :
    Thermal(beta), ComputableObject(), S(S), H(H)
{}

void DensityMatrix::prepare()
{
    if(getStatus() >= Prepared) return;
    BlockNumber NumOfBlocks = S.getNumberOfBlocks();
    parts.reserve(NumOfBlocks);
    RealType GroundEnergy = H.getGroundEnergy();
    // There is one-to-one correspondence between parts of the Hamiltonian
    // and parts of the density matrix itself.
    for(BlockNumber Block = 0; Block < NumOfBlocks; Block++)
        parts.emplace_back(H.getPart(Block), beta, GroundEnergy);
    setStatus(Prepared);
}

void DensityMatrix::compute()
{
    if(getStatus() >= Computed) return;
    RealType Z = 0;
    // A total partition function is a sum over partition functions of
    // all non-normalized parts.
    for(auto & p : parts)
        Z += p.computeUnnormalized();

    // Divide the density matrix by Z.
    for(auto & p : parts)
        p.normalize(Z);

    setStatus(Computed);
}

RealType DensityMatrix::getWeight(QuantumState state) const
{
    if(getStatus() < Computed) { throw StatusMismatch("DensityMatrix is not computed yet."); };
    BlockNumber Block = S.getBlockNumber(state);
    InnerQuantumState InnerState = S.getInnerState(state);

    return parts[Block].getWeight(InnerState);
}

const DensityMatrixPart& DensityMatrix::getPart(BlockNumber in) const
{
    return parts[in];
}

RealType DensityMatrix::getAverageEnergy() const
{
    if(getStatus() < Computed) { throw StatusMismatch("DensityMatrix is not computed yet."); }
    return std::accumulate(parts.begin(),
                           parts.end(),
                           .0,
                           [](double E, DensityMatrixPart const& p) { return E + p.getAverageEnergy(); });
}

void DensityMatrix::truncateBlocks(RealType Tolerance, bool verbose)
{
    for(auto & p : parts)
        p.truncate(Tolerance);

    if(verbose){
        // count retained blocks and states included in those blocks
        QuantumState n_blocks_retained = 0, n_states_retained = 0;
        for(BlockNumber i = 0; i < S.getNumberOfBlocks(); ++i)
            if(isRetained(i)){
                ++n_blocks_retained;
                n_states_retained += S.getBlockSize(i);
            }
        INFO("Number of blocks retained: " << n_blocks_retained);
        INFO("Number of states retained: " << n_states_retained);
    }
}

bool DensityMatrix::isRetained(BlockNumber in) const
{
    return parts[in].isRetained();
}

} // namespace Pomerol
