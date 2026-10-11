// SPDX-License-Identifier: MS-PL
// MathTypeConverter::GetProperties and CreateInstance: editing a value part by part, as a property grid does.
#include <any>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Design.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Collections/Hashtable.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Design;

namespace
{
    void Properties(CnaDemos::DemoContext& ctx)
    {
        // A grid lists the parts of a value ("expandable"), each with a name and a type.
        const ColorConverter colors;
        ctx.Check(colors.GetPropertiesSupported() && colors.GetCreateInstanceSupported(),
                  "math converters expose parts and can create values from them");
        const auto parts = colors.GetProperties(std::any(Color::Orange));
        std::string names;
        for (int i = 0; i < parts.getCountProperty(); ++i)
        {
            const auto& part = parts.getItem(i);
            names += (i ? ", " : "") + part->getNameProperty() + "=" +
                std::to_string(std::any_cast<SharpRuntime::bytecs>(part->GetValue(std::any(Color::Orange))));
        }
        ctx.Print("Color parts: {}", names);
        ctx.Check(parts.getCountProperty() == 4 && parts.getItem(0)->getNameProperty() == "R",
                  "a Color has the parts R, G, B and A");

        // Editing one part changes the boxed value in place.
        const Vector3Converter vectors;
        std::any boxed = Vector3(1.0f, 2.0f, 3.0f);
        vectors.GetProperties(boxed).getItem("Y")->SetValue(boxed, std::any(20.0f));
        ctx.Check(std::any_cast<Vector3>(boxed) == Vector3(1.0f, 20.0f, 3.0f), "setting Y edits the Vector3");

        // A grid rebuilds a value from all its parts with CreateInstance.
        System::Collections::Hashtable edited;
        edited.setItem("R", std::any(SharpRuntime::bytecs{10}));
        edited.setItem("G", std::any(SharpRuntime::bytecs{200}));
        edited.setItem("B", std::any(SharpRuntime::bytecs{30}));
        edited.setItem("A", std::any(SharpRuntime::bytecs{255}));
        ctx.Check(std::any_cast<Color>(colors.CreateInstance(edited)) == Color(10, 200, 30, 255),
                  "CreateInstance builds the Color from the edited parts");

        // Matrix offers its sixteen elements plus a Translation part for convenience.
        const MatrixConverter matrices;
        std::any transform = Matrix::getIdentityProperty();
        const auto matrixParts = matrices.GetProperties(transform);
        ctx.Print("Matrix parts: {} (first: {})", matrixParts.getCountProperty(), matrixParts.getItem(0)->getNameProperty());
        matrixParts.getItem("Translation")->SetValue(transform, std::any(Vector3(7.0f, 8.0f, 9.0f)));
        ctx.Check(matrixParts.getCountProperty() == 17 &&
                      std::any_cast<Matrix>(transform).getTranslationProperty() == Vector3(7.0f, 8.0f, 9.0f),
                  "Matrix has 17 parts; editing Translation moves the matrix");
    }
}

CNA_DEMOS_CONSOLE(Properties, {
    .id = "design.type_converter.properties",
    .title = "Editing values part by part",
    .description = "Lists a Color's parts with ColorConverter::GetProperties, edits a Vector3's Y through its "
                   "property descriptor, rebuilds a Color with CreateInstance from edited parts, and moves a Matrix "
                   "through its extra Translation part -- what a property grid does.",
    .apis = {"MathTypeConverter::GetProperties", "MathTypeConverter::GetPropertiesSupported",
             "MathTypeConverter::GetCreateInstanceSupported", "ColorConverter::CreateInstance",
             "Vector3Converter::CreateInstance", "MatrixConverter::CreateInstance"},
    .tags = {"design", "type-converter", "property-grid", "editor", "tools"},
})
