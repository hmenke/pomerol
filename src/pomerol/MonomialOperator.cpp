#include "pomerol/MonomialOperator.hpp"

#include <cstdlib>

namespace Pomerol {

MonomialOperator::BlocksBimap const& MonomialOperator::getBlockMapping() const
{
    if(getStatus() < Prepared) { ERROR("MonomialOperator is not prepared yet."); throw exStatusMismatch(); }
    return LeftRightBlocks;
}

void MonomialOperator::compute(const MPI_Comm& comm)
{
    if(getStatus() < Prepared) throw exStatusMismatch();
    if(getStatus() >= Computed) return;

    std::size_t Size = parts.size();
    for(std::size_t BlockIn = 0; BlockIn < Size; BlockIn++){
        INFO_NONEWLINE( (int) ((1.0*BlockIn/Size) * 100 ) << "  " << std::flush);
        parts[BlockIn].compute();
    };

    setStatus(Computed);
}

MonomialOperatorPart& MonomialOperator::getPartFromRightIndex(BlockNumber out)
{
    if(getStatus() < Prepared) { ERROR("MonomialOperator is not prepared yet."); throw exStatusMismatch(); }
    return parts[mapPartsFromRight.find(out)->second];
}

const MonomialOperatorPart& MonomialOperator::getPartFromRightIndex(BlockNumber out) const
{
    if(getStatus() < Prepared) { ERROR("MonomialOperator is not prepared yet."); throw exStatusMismatch(); }
    return parts[mapPartsFromRight.find(out)->second];
}

MonomialOperatorPart& MonomialOperator::getPartFromLeftIndex(BlockNumber in)
{
    if(getStatus() < Prepared) { ERROR("MonomialOperator is not prepared yet."); throw exStatusMismatch(); }
    return parts[mapPartsFromLeft.find(in)->second];
}

const MonomialOperatorPart& MonomialOperator::getPartFromLeftIndex(BlockNumber in) const
{
    if(getStatus() < Prepared) { ERROR("MonomialOperator is not prepared yet."); throw exStatusMismatch(); }
    return parts[mapPartsFromLeft.find(in)->second];
}

BlockNumber MonomialOperator::getRightIndex(BlockNumber LeftIndex) const
{
    if(getStatus() < Prepared) { ERROR("MonomialOperator is not prepared yet."); throw exStatusMismatch(); }

    BlocksBimap::left_const_iterator it =  LeftRightBlocks.left.find(LeftIndex);
    return it != LeftRightBlocks.left.end() ? it->second : INVALID_BLOCK_NUMBER;
}

BlockNumber MonomialOperator::getLeftIndex(BlockNumber RightIndex) const
{
    if(getStatus() < Prepared) { ERROR("MonomialOperator is not prepared yet."); throw exStatusMismatch(); }

    BlocksBimap::right_const_iterator it =  LeftRightBlocks.right.find(RightIndex);
    return (it != LeftRightBlocks.right.end()) ? it->second : INVALID_BLOCK_NUMBER;
}

} // namespace Pomerol
