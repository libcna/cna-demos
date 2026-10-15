// SPDX-License-Identifier: MS-PL
// Native Dictionary<string, object> loading: typed std::any values and lookup errors.
#include <any>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CNA/Content/ObjectDictionaryEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ReflectiveTypeReader.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/InvalidCastException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Content;

namespace
{
    struct DictionaryEntry
    {
        std::int32_t magic = 0;
        float weight = 0.0f;
    };

    class ObjectDictionaryDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();

            // The fixture contains one game-defined C# class. RegisterShared is required because
            // Dictionary<string, object> dispatches reference-type values through a reader index.
            ReflectiveTypeReaderBuilder<DictionaryEntry>("CNA.Demos.DictionaryEntry")
                .Field(&DictionaryEntry::magic)
                .Field(&DictionaryEntry::weight)
                .RegisterShared();

            auto values = getContentProperty().Load<std::map<std::string, std::any>>(
                "dictionaries/member_audit");
            CNA::Content::ObjectDictionaryEXT dictionary(std::move(values));

            ctx.Check(
                dictionary.GetTypeName() ==
                    "System.Collections.Generic.Dictionary`2[System.String,System.Object]",
                "the default logical type name preserves Dictionary<string, object>");
            ctx.Check(dictionary.getValuesProperty().size() == 16 &&
                          dictionary.ContainsKey("Sphere") &&
                          !dictionary.ContainsKey("Missing"),
                      "the native dictionary exposes all sixteen entries and exact membership");

            ctx.Check(dictionary.Get<std::string>("Name") == "triangles" &&
                          dictionary.Get<std::int32_t>("Integer") == -7 &&
                          dictionary.Get<float>("Single") == 1.25f &&
                          dictionary.Get<double>("Double") == 2.5,
                      "typed Get preserves string, integer and floating-point values");
            ctx.Check(dictionary.Get<Color>("Color") == Color(10, 20, 30, 40) &&
                          dictionary.Get<Vector3>("Vector3") == Vector3(4.0f, 5.0f, 6.0f),
                      "typed Get preserves authored XNA value types");

            const auto& array = dictionary.Get<std::vector<Vector3>>("Array");
            ctx.Check(array == std::vector<Vector3>{Vector3::Zero, Vector3::UnitX, Vector3::UnitY},
                      "a reference-shaped Vector3 array keeps its element order and values");

            const auto& custom =
                dictionary.Get<std::shared_ptr<DictionaryEntry>>("Custom");
            ctx.Check(custom && custom->magic == 1234 && custom->weight == 0.5f,
                      "RegisterShared reconstructs the game-defined dictionary value");

            const std::any& raw = dictionary["UnknownUInt32"];
            ctx.Check(raw.type() == typeid(std::uint32_t) &&
                          std::any_cast<std::uint32_t>(raw) == 77U,
                      "operator[] exposes an untyped value without losing its exact C++ type");

            bool wrongTypeRejected = false;
            try
            {
                (void)dictionary.Get<float>("Integer");
            }
            catch (const System::InvalidCastException&)
            {
                wrongTypeRejected = true;
            }
            ctx.Check(wrongTypeRejected,
                      "Get rejects a present entry requested as the wrong C++ type");

            bool missingKeyRejected = false;
            try
            {
                (void)dictionary["Missing"];
            }
            catch (const System::Collections::Generic::KeyNotFoundException&)
            {
                missingKeyRejected = true;
            }
            ctx.Check(missingKeyRejected, "operator[] rejects an absent key");

            std::map<std::string, std::any> typedValues;
            typedValues.emplace("Count", std::int32_t{3});
            CNA::Content::ObjectDictionaryEXT typed(
                std::move(typedValues),
                "System.Collections.Generic.Dictionary`2[System.String,System.Int32]");
            ctx.Check(typed.GetTypeName() ==
                              "System.Collections.Generic.Dictionary`2[System.String,System.Int32]" &&
                          typed.Get<std::int32_t>("Count") == 3,
                      "the explicit constructor retains a closed dictionary's logical type");
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(24, 30, 42));
            DemoGame::Draw(gameTime);
        }
    };
}

CNA_DEMOS_GAME(ObjectDictionaryDemo, {
    .id = "content.xnb.object_dictionary",
    .title = "Native XNB object dictionary",
    .description = "Loads a real Dictionary<string, object> XNB into native std::any values, boxes it as "
                   "ObjectDictionaryEXT, and demonstrates typed and untyped lookup, logical type names, "
                   "custom reflective values and lookup failures.",
    .apis = {"CNA::Content::ObjectDictionaryEXT", "CNA::Content::ObjectDictionaryEXT::ObjectDictionaryEXT",
             "CNA::Content::ObjectDictionaryEXT::GetTypeName", "CNA::Content::ObjectDictionaryEXT::ContainsKey",
             "CNA::Content::ObjectDictionaryEXT::Get", "CNA::Content::ObjectDictionaryEXT::operator[]",
             "CNA::Content::ObjectDictionaryEXT::getValuesProperty", "ReflectiveTypeReaderBuilder::Field",
             "ReflectiveTypeReaderBuilder::RegisterShared", "ContentManager::Load"},
    .tags = {"content", "xnb", "dictionary", "any", "reflective", "custom"},
    .frames = 3,
    .deterministic = true,
    .assets = {"dictionaries/member_audit.xnb"},
})
