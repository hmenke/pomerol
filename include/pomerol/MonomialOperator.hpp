//
// This file is part of pomerol, an exact diagonalization library aimed at
// solving condensed matter models of interacting fermions.
//
// Copyright (C) 2016-2021 A. Antipov, I. Krivenko and contributors
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/** \file include/pomerol/MonomialOperator.h
** \brief Declaration of field operators : creation and annihilation operators.
**
** \author Igor Krivenko (Igor.S.Krivenko@gmail.com)
** \author Andrey Antipov (Andrey.E.Antipov@gmail.com)
*/
#ifndef POMEROL_INCLUDE_MONOMIALOPERATOR_HPP
#define POMEROL_INCLUDE_MONOMIALOPERATOR_HPP

#include "ComputableObject.hpp"
#include "Hamiltonian.hpp"
#include "HilbertSpace.hpp"
#include "Misc.hpp"
#include "MonomialOperatorPart.hpp"
#include "Operators.hpp"
#include "StatesClassification.hpp"

#include "mpi_dispatcher/misc.hpp"
#include "mpi_dispatcher/mpi_skel.hpp"

#include <boost/bimap.hpp>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Pomerol {

/** \typedef
 * A pair of left and right indices of a part in a Field Operator. Each part is a non-vanishing worldline in an operator
 */
using BlockMapping = std::pair<BlockNumber, BlockNumber>;

/** This class is a parent class for creation/annihilation operators which act
 * on all blocks of quantum states */
class MonomialOperator : public ComputableObject {
public:
    using BlocksBimap = boost::bimaps::bimap<boost::bimaps::set_of<BlockNumber>, boost::bimaps::set_of<BlockNumber>>;
    using BlockMapping = BlocksBimap::value_type;

protected:
    friend class FieldOperatorContainer;

    /** A reference an Operator object (OperatorPresets::C or Cdag). */
    bool MOpComplex;

    std::shared_ptr<void> MOp;

    template <bool Complex> LOperatorTypeRC<Complex> const& getMOp() const {
        assert(MOpComplex == Complex);
        return *std::static_pointer_cast<LOperatorTypeRC<Complex>>(MOp);
    }

    bool Complex;

    /** A reference to a StatesClassification object */
    StatesClassification const& S;
    /** A reference to a Hamiltonian object */
    Hamiltonian const& H;

    libcommute::space_partition const& Partition;
    /** A map between non-vanishing parts (internal numbering) and their R.H.S. BlockNumbers  */
    std::unordered_map<std::size_t, BlockNumber> mapPartsFromRight;
    /** A map between non-vanishing parts (internal numbering) and their L.H.S. BlockNumbers  */
    std::unordered_map<std::size_t, BlockNumber> mapPartsFromLeft;

    BlocksBimap LeftRightBlocks;

    /** A vector of parts */
    std::vector<MonomialOperatorPart> parts;

public:
    /** Constructor
     * \param[in] IndexInfo A reference to an IndexClassification object
     * \param[in] S A reference to a StatesClassification object
     * \param[in] H A reference to a Hamiltonian object
     * \param[in] Index An index of an operator
     */
    template <typename ScalarType, typename... IndexTypes>
    MonomialOperator(libcommute::expression<ScalarType, IndexTypes...> const& MO,
                     HilbertSpace<IndexTypes...> const& HS,
                     StatesClassification const& S,
                     Hamiltonian const& H)
        : MOpComplex(std::is_same<ScalarType, ComplexType>::value),
          MOp(std::make_shared<LOperatorType<ScalarType>>(MO, HS.getFullHilbertSpace())),
          Complex(MOpComplex || H.isComplex()),
          S(S),
          H(H),
          Partition(HS.getSpacePartition()) {
        if(MO.size() > 1)
            throw std::runtime_error("Only monomial expressions are supported");
    }

    bool isComplex() const { return Complex; }

    /** Returns a FieldOperatorPart based on its left BlockNumber */
    MonomialOperatorPart& getPartFromLeftIndex(BlockNumber in);
    MonomialOperatorPart const& getPartFromLeftIndex(BlockNumber in) const;
    /** Returns a FieldOperatorPart based on its right BlockNumber */
    MonomialOperatorPart& getPartFromRightIndex(BlockNumber out);
    MonomialOperatorPart const& getPartFromRightIndex(BlockNumber out) const;

    /** Returns a left BlockNumber for a given right BlockNumber */
    BlockNumber getLeftIndex(BlockNumber RightIndex) const;
    /** Returns a right BlockNumber for a given left BlockNumber */
    BlockNumber getRightIndex(BlockNumber LeftIndex) const;
    /** Returns a reference to BlockMapping */
    BlocksBimap const& getBlockMapping() const;

    /** Virtual method for assigning world-lines */
    template <typename... IndexTypes> void prepare(HilbertSpace<IndexTypes...> const& HS) {
        if(getStatus() >= Prepared)
            return;

        auto const& FullHS = HS.getFullHilbertSpace();
        auto Connections = MOpComplex ? Partition.find_connections(getMOp<true>(), FullHS) :
                                        Partition.find_connections(getMOp<false>(), FullHS);

        parts.reserve(Connections.size());
        for(auto const& Conn : Connections) {
            mapPartsFromRight.emplace(Conn.first, parts.size());
            mapPartsFromLeft.emplace(Conn.second, parts.size());
            LeftRightBlocks.insert(BlockMapping(Conn.second, Conn.first));

            if(MOpComplex)
                parts.emplace_back(getMOp<true>(), S, H.getPart(Conn.first), H.getPart(Conn.second));
            else
                parts.emplace_back(getMOp<false>(), S, H.getPart(Conn.first), H.getPart(Conn.second));
        }

        Status = Prepared;
    }
    /** Computes all world-lines */
    void compute(MPI_Comm const& comm = MPI_COMM_WORLD);

private:
    void checkPrepared() const;
};

class CreationOperator : public MonomialOperator {
    ParticleIndex Index;

public:
    /** Constructor
     * \param[in] IndexInfo A reference to an IndexClassification object
     * \param[in] S A reference to a StatesClassification object
     * \param[in] H A reference to a Hamiltonian object
     * \param[in] Index An index of an operator
     */
    template <typename... IndexTypes>
    CreationOperator(IndexClassification<IndexTypes...> const& IndexInfo,
                     HilbertSpace<IndexTypes...> const& HS,
                     StatesClassification const& S,
                     Hamiltonian const& H,
                     ParticleIndex Index)
        : MonomialOperator(Operators::Detail::apply(Operators::c_dag<double, IndexTypes...>, IndexInfo.getInfo(Index)),
                           HS,
                           S,
                           H),
          Index(Index) {}

    ParticleIndex getIndex() const { return Index; }
};

class AnnihilationOperator : public MonomialOperator {
    ParticleIndex Index;

public:
    /** Constructor
     * \param[in] IndexInfo A reference to an IndexClassification object
     * \param[in] S A reference to a StatesClassification object
     * \param[in] H A reference to a Hamiltonian object
     * \param[in] Index An index of an operator
     */
    template <typename... IndexTypes>
    AnnihilationOperator(IndexClassification<IndexTypes...> const& IndexInfo,
                         HilbertSpace<IndexTypes...> const& HS,
                         StatesClassification const& S,
                         Hamiltonian const& H,
                         ParticleIndex Index)
        : MonomialOperator(Operators::Detail::apply(Operators::c<double, IndexTypes...>, IndexInfo.getInfo(Index)),
                           HS,
                           S,
                           H),
          Index(Index) {}

    ParticleIndex getIndex() const { return Index; }
};

/** A quadratic operator, c_1^+ c_2, in the eigenbasis of a Hamiltonian */
class QuadraticOperator : public MonomialOperator {
    ParticleIndex Index1, Index2;

public:
    /** Constructor
     * \param[in] IndexInfo A reference to an IndexClassification object
     * \param[in] S A reference to a StatesClassification object
     * \param[in] H A reference to a Hamiltonian object
     * \param[in] Index1 An index of a creation operator
     * \param[in] Index2 An index of an annihilation operator
     */
    template <typename... IndexTypes>
    QuadraticOperator(IndexClassification<IndexTypes...> const& IndexInfo,
                      HilbertSpace<IndexTypes...> const& HS,
                      StatesClassification const& S,
                      Hamiltonian const& H,
                      ParticleIndex Index1,
                      ParticleIndex Index2)
        : MonomialOperator(
              Operators::Detail::apply(Operators::c_dag<double, IndexTypes...>, IndexInfo.getInfo(Index1)) *
                  Operators::Detail::apply(Operators::c<double, IndexTypes...>, IndexInfo.getInfo(Index2)),
              HS,
              S,
              H),
          Index1(Index1),
          Index2(Index2) {}

    ParticleIndex getCXXIndex() const { return Index1; }
    ParticleIndex getCIndex() const { return Index2; }
};

} // namespace Pomerol

#endif // #ifndef POMEROL_INCLUDE_MONOMIALOPERATOR_HPP
