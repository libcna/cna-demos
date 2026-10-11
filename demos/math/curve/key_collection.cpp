// SPDX-License-Identifier: MS-PL
// Managing curve keys: CurveKeyCollection keeps keys sorted by position.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void CurveKeyCollectionDemo(CnaDemos::DemoContext& ctx)
    {
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

        // A curve with at most one key is constant.
        ctx.Check(!curve.getIsConstantProperty(), "three keys: not constant");
        const Curve copy = curve.Clone();
        keys.RemoveAt(2);
        ctx.Check(keys.Remove(CurveKey(1.0f, 10.0f)), "Remove deletes an equal key");
        ctx.Check(curve.getIsConstantProperty(), "one key left: the curve is constant");
        ctx.Check(copy.getKeysProperty().getCountProperty() == 3, "Clone made an independent copy");
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
    .apis = {"CurveKeyCollection::Add", "CurveKeyCollection::Item", "CurveKeyCollection::IndexOf",
             "CurveKeyCollection::RemoveAt", "CurveKeyCollection::Remove", "CurveKeyCollection::Clear",
             "CurveKeyCollection::Count", "Curve::IsConstant", "Curve::Clone", "CurveKey::Position", "CurveKey::Value",
             "Curve::Keys", "Curve::Evaluate"},
    .tags = {"curve", "collection"},
})
