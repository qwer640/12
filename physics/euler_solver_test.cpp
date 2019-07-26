﻿
#include "physics/euler_solver.hpp"

#include <algorithm>
#include <array>
#include <random>

#include "geometry/named_quantities.hpp"
#include "geometry/r3_element.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "quantities/elementary_functions.hpp"
#include "quantities/quantities.hpp"
#include "quantities/si.hpp"
#include "testing_utilities/almost_equals.hpp"

namespace principia {
namespace physics {

using geometry::Instant;
using geometry::R3Element;
using quantities::AngularFrequency;
using quantities::AngularMomentum;
using quantities::MomentOfInertia;
using quantities::Cos;
using quantities::Sin;
using quantities::SIUnit;
using quantities::Sqrt;
using quantities::Time;
using quantities::si::Radian;
using quantities::si::Second;
using testing_utilities::AlmostEquals;

class EulerSolverTest : public ::testing::Test {
};

// Check that we are able to retrieve the initial state for random choices of
// the moments of inertia and the angular momentum.
TEST_F(EulerSolverTest, InitialStateRandom) {
  std::mt19937_64 random(42);
  std::uniform_real_distribution<> moment_of_inertia_distribution(0.0, 10.0);
  std::uniform_real_distribution<> angular_momentum_distribution(-10.0, 10.0);
  for (int i = 0; i < 1000; ++i) {
    // Make sure that the moments of inertia are properly ordered.
    std::array<double, 3> randoms{moment_of_inertia_distribution(random),
                                  moment_of_inertia_distribution(random),
                                  moment_of_inertia_distribution(random)};
    std::sort(randoms.begin(), randoms.end());
    R3Element<MomentOfInertia> const moments_of_inertia{
        randoms[0] * SIUnit<MomentOfInertia>(),
        randoms[1] * SIUnit<MomentOfInertia>(),
        randoms[2] * SIUnit<MomentOfInertia>()};

    EulerSolver::AngularMomentumBivector initial_angular_momentum(
        {angular_momentum_distribution(random) * SIUnit<AngularMomentum>(),
         angular_momentum_distribution(random) * SIUnit<AngularMomentum>(),
         angular_momentum_distribution(random) * SIUnit<AngularMomentum>()});

    EulerSolver const solver(
        moments_of_inertia, initial_angular_momentum, Instant());
    auto const computed_initial_angular_momentum =
        solver.AngularMomentumAt(Instant());

    EXPECT_THAT(computed_initial_angular_momentum,
                AlmostEquals(initial_angular_momentum, 0, 209))
        << moments_of_inertia << " " << initial_angular_momentum;
  }
}

// Same as above, but exercises the symmetrical cases where at least two moments
// of inertia are equal.
TEST_F(EulerSolverTest, InitialStateSymmetrical) {
  std::mt19937_64 random(42);
  std::uniform_real_distribution<> angular_momentum_distribution(-10.0, 10.0);

  R3Element<MomentOfInertia> const moments_of_inertia1{
      2 * SIUnit<MomentOfInertia>(),
      2 * SIUnit<MomentOfInertia>(),
      3 * SIUnit<MomentOfInertia>()};
  R3Element<MomentOfInertia> const moments_of_inertia2{
      2 * SIUnit<MomentOfInertia>(),
      3 * SIUnit<MomentOfInertia>(),
      3 * SIUnit<MomentOfInertia>()};
  R3Element<MomentOfInertia> const moments_of_inertia3{
      3 * SIUnit<MomentOfInertia>(),
      3 * SIUnit<MomentOfInertia>(),
      3 * SIUnit<MomentOfInertia>()};

  for (int i = 0; i < 100; ++i) {
    EulerSolver::AngularMomentumBivector initial_angular_momentum(
        {angular_momentum_distribution(random) * SIUnit<AngularMomentum>(),
         angular_momentum_distribution(random) * SIUnit<AngularMomentum>(),
         angular_momentum_distribution(random) * SIUnit<AngularMomentum>()});
    {
      EulerSolver const solver1(
          moments_of_inertia1, initial_angular_momentum, Instant());
      auto const computed_initial_angular_momentum1 =
          solver1.AngularMomentumAt(Instant());

      EXPECT_THAT(computed_initial_angular_momentum1,
                  AlmostEquals(initial_angular_momentum, 0, 87))
          << moments_of_inertia1 << " " << initial_angular_momentum;
    }
    {
      EulerSolver const solver2(
          moments_of_inertia2, initial_angular_momentum, Instant());
      auto const computed_initial_angular_momentum2 =
          solver2.AngularMomentumAt(Instant());

      EXPECT_THAT(computed_initial_angular_momentum2,
                  AlmostEquals(initial_angular_momentum, 0, 50))
          << moments_of_inertia2 << " " << initial_angular_momentum;
    }
    {
      EulerSolver const solver3(
          moments_of_inertia3, initial_angular_momentum, Instant());
      auto const computed_initial_angular_momentum3 =
          solver3.AngularMomentumAt(Instant());

      EXPECT_THAT(computed_initial_angular_momentum3,
                  AlmostEquals(initial_angular_momentum, 0, 0))
          << moments_of_inertia3 << " " << initial_angular_momentum;
    }
  }
}

// Same as above, but exercises all the formulæ.  We compute an angular
// momentum by fixing its first coordinate and picking the third coordinate so
// that it falls in the right interval.  (The second coordinate turns out to be
// irrelevant.)
TEST_F(EulerSolverTest, InitialStateFormulæ) {
  std::mt19937_64 random(42);
  std::uniform_real_distribution<> moment_of_inertia_distribution(0.0, 10.0);
  std::uniform_real_distribution<> angular_momentum_distribution(-10.0, 10.0);
  for (int i = 0; i < 1000; ++i) {
    // Make sure that the moments of inertia are properly ordered.
    std::array<double, 3> randoms{moment_of_inertia_distribution(random),
                                  moment_of_inertia_distribution(random),
                                  moment_of_inertia_distribution(random)};
    std::sort(randoms.begin(), randoms.end());
    auto const I₁ = randoms[0] * SIUnit<MomentOfInertia>();
    auto const I₂ = randoms[1] * SIUnit<MomentOfInertia>();
    auto const I₃ = randoms[2] * SIUnit<MomentOfInertia>();
    R3Element<MomentOfInertia> const moments_of_inertia{I₁, I₂, I₃};

    // G² = T * (I₁ + I₂)
    {
      auto const mx =
          angular_momentum_distribution(random) * SIUnit<AngularMomentum>();
      auto mz = mx * Sqrt(((I₂ - I₁) * I₃) / ((2.0 * I₃ - I₂ - I₁) * I₁));
      if (i % 2 == 0) {
        mz = -mz;
      }
      EulerSolver::AngularMomentumBivector initial_angular_momentum(
          {mx, SIUnit<AngularMomentum>(), mz});
      EulerSolver const solver(
          moments_of_inertia, initial_angular_momentum, Instant());

      auto const computed_initial_angular_momentum =
          solver.AngularMomentumAt(Instant());
      EXPECT_THAT(computed_initial_angular_momentum,
                  AlmostEquals(initial_angular_momentum, 0, 356))
          << moments_of_inertia << " " << initial_angular_momentum;
    }

    // G² = 2 * T * I₂
    {
      auto const mx =
          angular_momentum_distribution(random) * SIUnit<AngularMomentum>();
      auto mz = mx * Sqrt(((I₂ - I₁) * I₃) / ((I₃ - I₂) * I₁));
      if (i % 2 == 0) {
        mz = -mz;
      }
      EulerSolver::AngularMomentumBivector initial_angular_momentum(
          {mx, SIUnit<AngularMomentum>(), mz});
      EulerSolver const solver(
          moments_of_inertia, initial_angular_momentum, Instant());

      auto const computed_initial_angular_momentum =
          solver.AngularMomentumAt(Instant());
      // NOTE(phl): The largest error happens to actually go through
      // Formula::ii and is on the z component (x and y are fine).  That's
      // probably related to the fact that Δ₂ is very small.
      EXPECT_THAT(computed_initial_angular_momentum,
                  AlmostEquals(initial_angular_momentum, 0, 11126))
          << moments_of_inertia << " " << initial_angular_momentum;
    }

    // G² = T * (I₂ + I₃)
    {
      auto const mx =
          angular_momentum_distribution(random) * SIUnit<AngularMomentum>();
      auto mz = mx * Sqrt(((I₂ + I₃ - 2.0 * I₁) * I₃) / ((I₃ - I₂) * I₁));
      if (i % 2 == 0) {
        mz = -mz;
      }
      EulerSolver::AngularMomentumBivector initial_angular_momentum(
          {mx, SIUnit<AngularMomentum>(), mz});
      EulerSolver const solver(
          moments_of_inertia, initial_angular_momentum, Instant());

      auto const computed_initial_angular_momentum =
          solver.AngularMomentumAt(Instant());
      EXPECT_THAT(computed_initial_angular_momentum,
                  AlmostEquals(initial_angular_momentum, 0, 2711))
          << moments_of_inertia << " " << initial_angular_momentum;
    }
  }
}

// This test and the next come from
// http://n.ethz.ch/~stiegerc/HS09/Mechanik/Unterlagen/Lecture19.pdf.
TEST_F(EulerSolverTest, ShortFatSymmetricTopPrecession) {
  R3Element<MomentOfInertia> const moments_of_inertia{
      3.0 * SIUnit<MomentOfInertia>(),
      3.0 * SIUnit<MomentOfInertia>(),
      9.0 * SIUnit<MomentOfInertia>()};

  EulerSolver::AngularMomentumBivector const initial_angular_momentum(
      {0.0 * SIUnit<AngularMomentum>(),
       5.0 * SIUnit<AngularMomentum>(),
       7.0 * SIUnit<AngularMomentum>()});

  // Correspondence with the referential of lecture 19: x = e1, y = e2, z = e3.
  AngularFrequency Ω = initial_angular_momentum.coordinates().z *
                       (moments_of_inertia[0] - moments_of_inertia[2]) /
                       (moments_of_inertia[0] * moments_of_inertia[2]);

  EulerSolver const solver(
      moments_of_inertia, initial_angular_momentum, Instant());
  for (Time t = 0 * Second; t < 5.0 * Second; t += 0.1 * Second) {
    auto const angular_momentum_at_t = solver.AngularMomentumAt(Instant() + t);
    EXPECT_THAT(angular_momentum_at_t,
                AlmostEquals(EulerSolver::AngularMomentumBivector(
                                 {5.0 * Sin(Ω * t) * SIUnit<AngularMomentum>(),
                                  5.0 * Cos(Ω * t) * SIUnit<AngularMomentum>(),
                                  7.0 * SIUnit<AngularMomentum>()}),
                             0,
                             102))
        << t;
  }
}

TEST_F(EulerSolverTest, TallSkinnySymmetricTopPrecession) {
  R3Element<MomentOfInertia> const moments_of_inertia{
      3.0 * SIUnit<MomentOfInertia>(),
      9.0 * SIUnit<MomentOfInertia>(),
      9.0 * SIUnit<MomentOfInertia>()};

  EulerSolver::AngularMomentumBivector const initial_angular_momentum(
      {7.0 * SIUnit<AngularMomentum>(),
       0.0 * SIUnit<AngularMomentum>(),
       5.0 * SIUnit<AngularMomentum>()});

  // Correspondence with the referential of lecture 19:  x = e3, y = e1, z = e2.
  AngularFrequency Ω = initial_angular_momentum.coordinates().x *
                       (moments_of_inertia[1] - moments_of_inertia[0]) /
                       (moments_of_inertia[1] * moments_of_inertia[0]);

  EulerSolver const solver(
      moments_of_inertia, initial_angular_momentum, Instant());
  for (Time t = 0 * Second; t < 5.0 * Second; t += 0.1 * Second) {
    auto const angular_momentum_at_t = solver.AngularMomentumAt(Instant() + t);
    EXPECT_THAT(angular_momentum_at_t,
                AlmostEquals(EulerSolver::AngularMomentumBivector(
                                 {7.0 * SIUnit<AngularMomentum>(),
                                  5.0 * Sin(Ω * t) * SIUnit<AngularMomentum>(),
                                  5.0 * Cos(Ω * t) * SIUnit<AngularMomentum>(),
                                  }),
                             0,
                             34))
        << t;
  }
}

}  // namespace physics
}  // namespace principia