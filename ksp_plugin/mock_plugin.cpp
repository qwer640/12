#include "ksp_plugin/mock_plugin.hpp"

#include "gmock/gmock.h"

namespace principia {
namespace ksp_plugin {

MockPlugin::MockPlugin()
    : Plugin(Instant(),
      Index(0),
      1 * SIUnit<GravitationalParameter>(),
      Angle()) {}

std::unique_ptr<Transforms<Barycentric, Rendering, Barycentric>>
MockPlugin::NewBodyCentredNonRotatingTransforms(
    Index const reference_body_index) const {
  std::unique_ptr<Transforms<Barycentric, Rendering, Barycentric>> transforms;
  FillBodyCentredNonRotatingTransforms(reference_body_index, &transforms);
  return transforms;
}

std::unique_ptr<Transforms<Barycentric, Rendering, Barycentric>>
MockPlugin::NewBarycentricRotatingTransforms(
    Index const primary_index,
    Index const secondary_index) const {
  std::unique_ptr<Transforms<Barycentric, Rendering, Barycentric>> transforms;
  FillBarycentricRotatingTransforms(primary_index,
                                    secondary_index,
                                    &transforms);
  return transforms;
}

}  // namespace ksp_plugin
}  // namespace principia