// SPDX-License-Identifier: MS-PL
// ConvertTo(InstanceDescriptor): how a designer writes a value as constructor arguments.
#include <any>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/BoundingSphere.hpp"
#include "Microsoft/Xna/Framework/Design.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ComponentModel/Design/Serialization/InstanceDescriptor.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Design;
using System::ComponentModel::Design::Serialization::InstanceDescriptor;

namespace
{
    void Instance(CnaDemos::DemoContext& ctx)
    {
        // A designer that generates code asks how to construct the value: the converter answers
        // with a constructor and its arguments.
        const BoundingSphereConverter converter;
        const BoundingSphere sphere(Vector3(1.0f, 2.0f, 3.0f), 4.5f);
        ctx.Check(converter.CanConvertTo(System::Type::From<InstanceDescriptor>()),
                  "the converter can describe the value as an InstanceDescriptor");
        const std::any described = converter.ConvertTo(std::any(sphere), System::Type::From<InstanceDescriptor>());
        const auto& descriptor = std::any_cast<const InstanceDescriptor&>(described);
        ctx.Print("BoundingSphere(Center, Radius): {} arguments, complete {}", descriptor.getArgumentsProperty().size(),
                  descriptor.getIsCompleteProperty());
        ctx.Check(descriptor.getIsCompleteProperty() && descriptor.getArgumentsProperty().size() == 2,
                  "two constructor arguments describe it completely");

        // Running the description constructs the same value -- what the generated code would do.
        ctx.Check(std::any_cast<BoundingSphere>(descriptor.Invoke()) == sphere, "Invoke rebuilds the sphere");
        ctx.Check(std::any_cast<BoundingSphere>(converter.ConvertFrom(described)) == sphere,
                  "ConvertFrom accepts the descriptor too");

        const BoundingBox box(Vector3(-1.0f, -2.0f, -3.0f), Vector3(4.0f, 5.0f, 6.0f));
        const BoundingBoxConverter boxes;
        const std::any describedBox =
            boxes.ConvertTo(std::any(box), System::Type::From<InstanceDescriptor>());
        const Ray ray(Vector3(1.0f, 2.0f, 3.0f), Vector3::Forward);
        const RayConverter rays;
        const std::any describedRay =
            rays.ConvertTo(std::any(ray), System::Type::From<InstanceDescriptor>());
        ctx.Check(std::any_cast<BoundingBox>(boxes.ConvertFrom(describedBox)) == box &&
                      std::any_cast<Ray>(rays.ConvertFrom(describedRay)) == ray,
                  "non-parsing BoundingBox and Ray converters accept their constructor descriptors");
    }
}

CNA_DEMOS_CONSOLE(Instance, {
    .id = "design.type_converter.instance_descriptor",
    .title = "Values as constructor calls",
    .description = "Converts a BoundingSphere to an InstanceDescriptor -- its constructor and the arguments Center and "
                   "Radius, as a code-generating designer needs -- and rebuilds the value with Invoke and with "
                   "ConvertFrom.",
    .apis = {"BoundingSphereConverter::BoundingSphereConverter", "BoundingSphereConverter::ConvertTo",
             "BoundingSphereConverter::ConvertFrom", "BoundingBoxConverter::ConvertFrom",
             "RayConverter::ConvertFrom", "MathTypeConverter::CanConvertTo"},
    .tags = {"design", "type-converter", "code-generation", "serialization", "tools"},
})
