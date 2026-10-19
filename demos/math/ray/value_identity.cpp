// SPDX-License-Identifier: MS-PL
// Ray construction, public fields, value identity, hashing and text.
#include <any>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Ray.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void RayValueIdentity(CnaDemos::DemoContext& ctx)
    {
        const Ray authored(Vector3(-1.0f, 2.0f, -3.0f), Vector3(4.0f, -5.0f, 6.0f));
        ctx.Check(Ray().Position == Vector3::Zero && Ray().Direction == Vector3::Zero &&
                      authored.Position == Vector3(-1.0f, 2.0f, -3.0f) &&
                      authored.Direction == Vector3(4.0f, -5.0f, 6.0f),
                  "default and authored construction preserve both public vectors");

        const Ray equal(authored.Position, authored.Direction);
        ctx.Check(authored.Equals(equal) && authored.Equals(std::any{equal}) && authored == equal &&
                      !(authored != equal) && !authored.Equals(std::any{17}) &&
                      !authored.Equals(std::any{}),
                  "typed, boxed and operator equality agree and reject other boxed values");

        const Ray moved(Vector3(-1.0f, 2.0f, -2.0f), authored.Direction);
        const Ray turned(authored.Position, Vector3(4.0f, -5.0f, 7.0f));
        ctx.Check(moved != authored && turned != authored && !moved.Equals(authored) &&
                      !turned.Equals(authored),
                  "changing either position or direction makes a Ray unequal");
        ctx.Check(authored.GetHashCode() == equal.GetHashCode(), "equal rays hash equally");

        ctx.Print("authored ray = {}", authored.ToString());
        ctx.Check(authored.ToString() ==
                      "{Position:{X:-1 Y:2 Z:-3} Direction:{X:4 Y:-5 Z:6}}",
                  "ToString uses XNA's single-braced Position/Direction representation");
    }
}

CNA_DEMOS_CONSOLE(RayValueIdentity, {
    .id = "math.ray.value_identity",
    .title = "Ray value identity",
    .description = "Checks Ray default/authored construction, public fields, typed and boxed identity, hashing and "
                   "the exact XNA Position/Direction text form.",
    .apis = {"Ray::Ray", "Ray::Position", "Ray::Direction", "Ray::Equals", "Ray::GetHashCode",
             "Ray::ToString", "Ray::op_Equality", "Ray::op_Inequality"},
    .tags = {"ray", "value-semantics", "formatting"},
})
