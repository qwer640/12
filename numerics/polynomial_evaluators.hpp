#pragma once

#include "base/macros.hpp"
#include "numerics/polynomial.hpp"
#include "quantities/quantities.hpp"

namespace principia {
namespace numerics {
namespace internal_polynomial_evaluators {

using quantities::Derivative;
using quantities::Square;

// The structs below are only needed because the language won't let us have (1)
// a partial specialization of a function (2) an explicit specialization of a
// member function template in an unspecialized class template.  Sigh.
// We use FORCE_INLINE because we have to write this recursively, but we really
// want linear code.

template<typename Value, typename Argument, int degree>
struct EstrinEvaluator {
  using Coefficients =
      typename PolynomialInMonomialBasis<Value, Argument, degree>::Coefficients;

  static FORCE_INLINE Value Evaluate(Coefficients const& coefficients,
                                     Argument const& argument);
};

template<typename Value, typename Argument, int degree>
struct HornerEvaluator {
  using Coefficients =
      typename PolynomialInMonomialBasis<Value, Argument, degree>::Coefficients;

  static FORCE_INLINE Value Evaluate(Coefficients const& coefficients,
                                     Argument const& argument);
  static FORCE_INLINE Derivative<Value, Argument>
  EvaluateDerivative(Coefficients const& coefficients,
                     Argument const& argument);
};

}  // namespace internal_polynomial_evaluators

using internal_polynomial_evaluators::EstrinEvaluator;
using internal_polynomial_evaluators::HornerEvaluator;

}  // namespace numerics
}  // namespace principia

#include "numerics/polynomial_evaluators_body.hpp"
