// SPDX-License-Identifier: MS-PL
// Managing curve keys: CurveKeyCollection keeps keys sorted by position.
#include <stdexcept>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "Microsoft/Xna/Framework/CurveKey.hpp"
#include "Microsoft/Xna/Framework/CurveKeyCollection.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

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

        const CurveKey sentinel(-1.0f, -1.0f);
        std::vector<CurveKey> copied(5, sentinel);
        keys.CopyTo(copied, 1);
        ctx.Check(copied[0] == sentinel && copied[1] == keys.getItemProperty(0) &&
                      copied[2] == keys.getItemProperty(1) && copied[3] == keys.getItemProperty(2) &&
                      copied[4] == sentinel,
                  "CopyTo writes the ordered keys at the requested destination index");

        std::vector<CurveKey> shortCopy(3, sentinel);
        const std::vector<CurveKey> shortBefore = shortCopy;
        bool negativeCopyRejected = false;
        bool shortCopyRejected = false;
        try
        {
            keys.CopyTo(shortCopy, -1);
        }
        catch (const std::out_of_range&)
        {
            negativeCopyRejected = true;
        }
        try
        {
            keys.CopyTo(shortCopy, 1);
        }
        catch (const std::out_of_range&)
        {
            shortCopyRejected = true;
        }
        ctx.Check(negativeCopyRejected && shortCopyRejected && shortCopy == shortBefore,
                  "C++ CopyTo rejects negative or insufficient destinations atomically");

        CurveKeyCollection orderedKeys;
        orderedKeys.Add(CurveKey(1.0f, 10.0f));
        orderedKeys.Add(CurveKey(0.0f, 0.0f));
        orderedKeys.Add(CurveKey(1.0f, 11.0f));
        orderedKeys.Add(CurveKey(1.0f, 12.0f));
        orderedKeys.Add(CurveKey(2.0f, 20.0f));
        ctx.Check(orderedKeys.getCountProperty() == 5 &&
                      orderedKeys.getItemProperty(1).getValueProperty() == 10.0f &&
                      orderedKeys.getItemProperty(2).getValueProperty() == 11.0f &&
                      orderedKeys.getItemProperty(3).getValueProperty() == 12.0f,
                  "equal-position keys retain their insertion order");

        orderedKeys.setItemProperty(2, CurveKey(1.0f, 99.0f));
        ctx.Check(orderedKeys.getItemProperty(1).getValueProperty() == 10.0f &&
                      orderedKeys.getItemProperty(2).getValueProperty() == 99.0f &&
                      orderedKeys.getItemProperty(3).getValueProperty() == 12.0f,
                  "same-position indexed replacement stays between its duplicate neighbors");

        orderedKeys.setItemProperty(0, CurveKey(1.5f, 15.0f));
        ctx.Check(orderedKeys.getItemProperty(0).getPositionProperty() == 1.0f &&
                      orderedKeys.getItemProperty(2).getPositionProperty() == 1.0f &&
                      orderedKeys.getItemProperty(3) == CurveKey(1.5f, 15.0f) &&
                      orderedKeys.getItemProperty(4) == CurveKey(2.0f, 20.0f),
                  "changing a key position removes and reinserts it in ascending order");

        bool negativeReplacementRejected = false;
        bool pastEndReplacementRejected = false;
        try
        {
            orderedKeys.setItemProperty(-1, CurveKey(-1.0f, -1.0f));
        }
        catch (const std::out_of_range&)
        {
            negativeReplacementRejected = true;
        }
        try
        {
            orderedKeys.setItemProperty(orderedKeys.getCountProperty(), CurveKey(3.0f, 30.0f));
        }
        catch (const std::out_of_range&)
        {
            pastEndReplacementRejected = true;
        }
        ctx.Check(negativeReplacementRejected && pastEndReplacementRejected &&
                      orderedKeys.getCountProperty() == 5 &&
                      orderedKeys.getItemProperty(0) == CurveKey(1.0f, 10.0f) &&
                      orderedKeys.getItemProperty(2) == CurveKey(1.0f, 12.0f) &&
                      orderedKeys.getItemProperty(3) == CurveKey(1.5f, 15.0f),
                  "invalid indexed replacements throw without changing the collection");

        bool negativeReadRejected = false;
        bool pastEndReadRejected = false;
        bool negativeRemoveRejected = false;
        bool pastEndRemoveRejected = false;
        try
        {
            static_cast<void>(orderedKeys.getItemProperty(-1));
        }
        catch (const std::out_of_range&)
        {
            negativeReadRejected = true;
        }
        try
        {
            static_cast<void>(orderedKeys.getItemProperty(orderedKeys.getCountProperty()));
        }
        catch (const std::out_of_range&)
        {
            pastEndReadRejected = true;
        }
        try
        {
            orderedKeys.RemoveAt(-1);
        }
        catch (const std::out_of_range&)
        {
            negativeRemoveRejected = true;
        }
        try
        {
            orderedKeys.RemoveAt(orderedKeys.getCountProperty());
        }
        catch (const std::out_of_range&)
        {
            pastEndRemoveRejected = true;
        }
        ctx.Check(negativeReadRejected && pastEndReadRejected && negativeRemoveRejected &&
                      pastEndRemoveRejected && orderedKeys.getCountProperty() == 5,
                  "C++ Item and RemoveAt reject both invalid index directions without mutation");

        CurveKeyCollection nearbyKeys;
        const float epsilon = MathHelper::MachineEpsilonFloat;
        nearbyKeys.Add(CurveKey(0.0f, 0.0f));
        nearbyKeys.Add(CurveKey(epsilon * 0.5f, 1.0f));
        nearbyKeys.setItemProperty(0, CurveKey(epsilon * 0.75f, 2.0f));
        ctx.Check(nearbyKeys.getItemProperty(0).getPositionProperty() <=
                      nearbyKeys.getItemProperty(1).getPositionProperty(),
                  "a nearby-position indexed replacement also preserves ascending order");

        // A curve with at most one key is constant.
        ctx.Check(!curve.getIsConstantProperty(), "three keys: not constant");
        const Curve copy = curve.Clone();
        const CurveKeyCollection keyCopy = keys.Clone();
        keys.RemoveAt(2);
        ctx.Check(keys.Remove(CurveKey(1.0f, 10.0f)), "Remove deletes an equal key");
        ctx.Check(curve.getIsConstantProperty(), "one key left: the curve is constant");
        ctx.Check(copy.getKeysProperty().getCountProperty() == 3 && keyCopy.getCountProperty() == 3,
                  "CNA's documented C++ Curve and collection clones own independent key values");
        keys.getItemProperty(0).setValueProperty(7.0f);
        ctx.Check(curve.Evaluate(-5.0f) == 7.0f && curve.Evaluate(5.0f) == 7.0f,
                  "a single key supplies its value at every position");
        ctx.Check(copy.getKeysProperty().getItemProperty(0).getValueProperty() == 0.0f,
                  "editing a key value leaves CNA's documented deep Curve clone unchanged");
        keys.Clear();
        ctx.Check(keys.getCountProperty() == 0, "Clear removes every key");
    }
}

CNA_DEMOS_CONSOLE(CurveKeyCollectionDemo, {
    .id = "math.curve.key_collection",
    .title = "Curve key collection",
    .description = "Adds, finds, replaces, removes and clears keys in a CurveKeyCollection, including duplicate "
                   "positions and ordering boundaries, copies a curve, and evaluates empty and single-key curves.",
    .apis = {"CurveKeyCollection::CurveKeyCollection", "CurveKeyCollection::Add", "CurveKeyCollection::Item",
             "CurveKeyCollection::IndexOf", "CurveKeyCollection::Contains", "CurveKeyCollection::CopyTo",
             "CurveKeyCollection::Clone", "CurveKeyCollection::IsReadOnly", "CurveKeyCollection::RemoveAt",
             "CurveKeyCollection::Remove", "CurveKeyCollection::Clear", "CurveKeyCollection::Count",
             "Curve::IsConstant", "Curve::Clone", "CurveKey::Position", "CurveKey::Value",
             "Curve::Keys", "Curve::Evaluate"},
    .tags = {"curve", "collection"},
    .restrictions = "CNA-DEMOS-ISSUE-097 records that a nearby-position indexed replacement can violate the "
                    "collection's promised ascending order.",
    .knownIssue = "CNA-DEMOS-ISSUE-097",
})
