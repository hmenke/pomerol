//
// This file is part of pomerol, an exact diagonalization library aimed at
// solving condensed matter models of interacting fermions.
//
// Copyright (C) 2016-2021 A. Antipov, I. Krivenko and contributors
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// \file include/pomerol/TermList.hpp
/// \brief List of terms forming Lehmann representation of a correlation function.
/// \author Igor Krivenko (Igor.S.Krivenko@gmail.com)

#ifndef POMEROL_INCLUDE_TERMLIST_HPP
#define POMEROL_INCLUDE_TERMLIST_HPP

#include "Misc.hpp"

#include "mpi_dispatcher/misc.hpp"

#include <cstddef>
#include <set>
#include <utility>
#include <vector>

namespace Pomerol {

/// \addtogroup Misc
///@{

/// \brief A list of terms contributing to the Lehmann representation of a correlation function.
///
/// A set-like container storing a list of terms contributing to the Lehmann representation of a correlation function.
///
/// The terms must allow sorting using a comparison function \p TermType::Compare.
/// Similar terms (i.e. those equivalent w.r.t. \p TermType::Compare) are automatically collected
/// and reduced to one term using \p operator+=().
/// A term T is considered negligible and is automatically removed from the
/// container if \p TermType::IsNegligible(T, current_number_of_terms + 1) evaluates to true.
/// \tparam TermType Type of a single term.
template <typename TermType> class TermList {

    /// Type of the term comparison predicate.
    using Compare = typename TermType::Compare;
    /// Type of the term 'is negligible' predicate.
    using IsNegligible = typename TermType::IsNegligible;

    /// The ordered set of terms.
    std::set<TermType, Compare> data;
    /// The 'is negligible' predicate.
    IsNegligible is_negligible;

public:
    /// Constructor.
    /// \param[in] compare Compare predicate for the underlying \p std::set object.
    /// \param[in] is_negligible Predicate that determines whether a term can be neglected.
    TermList(Compare const& compare, IsNegligible const& is_negligible) : data(compare), is_negligible(is_negligible) {}

    /// Add a new term to the container
    /// \param[in] term Term to be added
    void add_term(TermType const& term) {
        auto it = data.find(term);
        if(it == data.end()) { // new term
            data.emplace(term);
        } else { // similar term
            TermType sum = *it;
            sum += term;
            data.erase(*it);
            if(!is_negligible(sum, data.size() + 1))
                data.emplace(sum);
        }
    }

    /// Number of terms in the container.
    std::size_t size() const { return data.size(); }

    /// Remove all terms from the container.
    void clear() { data.clear(); }

    /// Access the underlying set of terms.
    std::set<TermType, Compare> const& as_set() const { return data; }

    /// Access the 'is negligible' predicate.
    IsNegligible const& get_is_negligible() const { return is_negligible; }

    /// Forward arguments to \p TermType:::operator() of each term in the container
    /// and return a sum of their return values.
    /// \tparam Args Types of the arguments.
    /// \param[in] args Arguments to be passes to the terms.
    template <typename... Args> ComplexType operator()(Args&&... args) const {
        ComplexType res = 0;
        for(auto const& t : data)
            res += t(std::forward<Args>(args)...);
        return res;
    }

    /// Broadcast terms from a root MPI rank to all other ranks in a communicator.
    /// \param[in] comm The MPI communicator for the broadcast operation.
    /// \param[in] root Rank of the root MPI process.
    void broadcast(MPI_Comm const& comm, int root) {
        auto comp = data.key_comp();
        comp.broadcast(comm, root);

        long n_terms;
        if(pMPI::rank(comm) == root) { // Broadcast the terms from this process
            n_terms = data.size();
            std::vector<TermType> v(data.begin(), data.end());
            MPI_Bcast(&n_terms, 1, MPI_LONG, root, comm);
            MPI_Bcast(v.data(), v.size(), TermType::mpi_datatype(), root, comm);
        } else { // Receive terms
            MPI_Bcast(&n_terms, 1, MPI_LONG, root, comm);
            std::vector<TermType> v(n_terms);
            MPI_Bcast(v.data(), v.size(), TermType::mpi_datatype(), root, comm);
            data = std::set<TermType, Compare>(v.begin(), v.end(), comp);
        }

        is_negligible.broadcast(comm, root);
    }

    /// Check if all terms in the container are properly ordered and are not negligible.
    bool check_terms() const {
        if(size() == 0)
            return true;
        auto prev_it = data.begin();
        if(is_negligible(*prev_it, data.size() + 1))
            return false;
        if(size() == 1)
            return true;

        Compare const& compare = data.key_comp();

        auto it = prev_it;
        for(++it; it != data.end(); ++it, ++prev_it) {
            if(is_negligible(*it, data.size() + 1) || !compare(*prev_it, *it))
                return false;
        }
        return true;
    }
};

///@}

} // namespace Pomerol

#endif // #ifndef POMEROL_INCLUDE_TERMLIST_HPP
