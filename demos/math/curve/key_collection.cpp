// SPDX-License-Identifier: MS-PL
// Managing curve keys: CurveKeyCollection keeps keys sorted by position.
#include <any>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveContinuity.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveKeyCollection.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void CurveKeyCollectionDemo(CnaDemos::DemoContext& ctx)
    {
        const CurveKeyCollection emptyKeys;
        ctx.Check(emptyKeys.getCountProperty() == 0 && !emptyKeys.getIsReadOnlyProperty(),
                  "a directly constructed key collection starts empty and mutable");

        Curve curve;
        CurveKeyCollection& keys = curve.getKeysProperty();

        ctx.Check(curve.getIsConstantProperty() && curve.Evaluate(-5.0f) == 0.0f && curve.Evaluate(5.0f) == 0.0f,
                  "an empty curve is constant and evaluates to zero");

        // Added out of order...
        keys.Add(CurveKey(2.0f, 20.0f));
        keys.Add(CurveKey(0.0f, 0.0f));
        keys.Add(CurveKey(1.0f, 10.0f));

        // ...but stored in position order.
        for (int i = 0; i < keys.getCountProperty(); ++i)
        {
            const CurveKey& key = keys.getItemProperty(i);
            ctx.Print("key {}: position {}, value {}", i, key.getPositionProperty(), key.getValueProperty());
        }
        ctx.Check(keys.getItemProperty(0).getPositionProperty() == 0.0f && keys.getItemProperty(2).getPositionProperty() == 2.0f,
                  "keys are sorted by position");
        ctx.Check(keys.IndexOf(CurveKey(1.0f, 10.0f)) == 1, "IndexOf finds an equal key");
        ctx.Check(keys.Contains(CurveKey(1.0f, 10.0f)) && !keys.Contains(CurveKey(1.0f, 99.0f)),
                  "Contains compares the complete key, not its position alone");

        const CurveKey& middle = keys.getItemProperty(1);
        const CurveKey clonedKey = middle.Clone();
        const CurveKey changedKey(1.0f, 10.0f, 1.0f, 0.0f, CurveContinuity::Smooth);
        ctx.Check(middle.Equals(clonedKey) && middle.Equals(std::any{clonedKey}) &&
                      !middle.Equals(std::any{42}) && !middle.Equals(std::any{}),
                  "a cloned key has typed and boxed value identity");
        ctx.Check(middle == clonedKey && middle != changedKey && middle.GetHashCode() == clonedKey.GetHashCode(),
                  "key operators and hashes include tangents and continuity");
        ctx.Check(middle.CompareTo(keys.getItemProperty(0)) > 0 &&
                      middle.CompareTo(keys.getItemProperty(2)) < 0 &&
                      middle.CompareTo(CurveKey(1.0f, 99.0f)) == 0,
                  "CompareTo orders only by position, independently of the key value");

        const CurveKey sentinel(-1.0f, -1.0f);
        std::vector<CurveKey> copied(5, sentinel);
        keys.CopyTo(copied, 1);
        ctx.Check(copied[0] == sentinel && copied[1] == keys.getItemProperty(0) &&
                      copied[2] == keys.getItemProperty(1) && copied[3] == keys.getItemProperty(2) &&
                      copied[4] == sentinel,
                  "CopyTo writes the ordered keys at the requested destination index");

        // A curve with at most one key is constant.
        ctx.Check(!curve.getIsConstantProperty(), "three keys: not constant");
        const Curve copy = curve.Clone();
        const CurveKeyCollection keyCopy = keys.Clone();
        keys.RemoveAt(2);
        ctx.Check(keys.Remove(CurveKey(1.0f, 10.0f)), "Remove deletes an equal key");
        ctx.Check(curve.getIsConstantProperty(), "one key left: the curve is constant");
        ctx.Check(copy.getKeysProperty().getCountProperty() == 3 && keyCopy.getCountProperty() == 3,
                  "Curve and key-collection clones made independent copies");
        keys.getItemProperty(0).setValueProperty(7.0f);
        ctx.Check(curve.Evaluate(-5.0f) == 7.0f && curve.Evaluate(5.0f) == 7.0f,
                  "a single key supplies its value at every position");
        ctx.Check(copy.getKeysProperty().getItemProperty(0).getValueProperty() == 0.0f,
                  "editing a key value also leaves the cloned curve unchanged");
        keys.Clear();
        ctx.Check(keys.getCountProperty() == 0, "Clear removes every key");
    }
}

CNA_DEMOS_CONSOLE(CurveKeyCollectionDemo, {
    .id = "math.curve.key_collection",
    .title = "Curve key collection",
    .description = "Adds, finds, removes and clears keys in a CurveKeyCollection, which keeps them sorted by "
                   "position, copies a curve, and evaluates empty and single-key curves.",
    .apis = {"CurveKeyCollection::CurveKeyCollection", "CurveKeyCollection::Add", "CurveKeyCollection::Item",
             "CurveKeyCollection::IndexOf", "CurveKeyCollection::Contains", "CurveKeyCollection::CopyTo",
             "CurveKeyCollection::Clone", "CurveKeyCollection::IsReadOnly", "CurveKeyCollection::RemoveAt",
             "CurveKeyCollection::Remove", "CurveKeyCollection::Clear", "CurveKeyCollection::Count",
             "Curve::IsConstant", "Curve::Clone", "CurveKey::Position", "CurveKey::Value", "CurveKey::Clone",
             "CurveKey::CompareTo", "CurveKey::Equals", "CurveKey::GetHashCode", "CurveKey::op_Equality",
             "CurveKey::op_Inequality", "Curve::Keys", "Curve::Evaluate"},
    .tags = {"curve", "collection"},
})
