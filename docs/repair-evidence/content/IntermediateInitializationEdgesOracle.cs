// ISSUE-089: genuine initialization reentrancy, cycles and exception-cache semantics.
using System;
using System.Reflection;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Content.Pipeline.Serialization.Intermediate;
public class SelfPayload { }
public class LeftPayload { }
public class RightPayload { }
public class FailingPayload { }
public abstract class EdgeSerializer<T> : ContentTypeSerializer<T> where T : new()
{
    public int Calls;
    public ContentTypeSerializer Nested;
    protected override void Serialize(IntermediateWriter output, T value, ContentSerializerAttribute format) { }
    protected override T Deserialize(IntermediateReader input, ContentSerializerAttribute format, T existing) { return new T(); }
}
[ContentTypeSerializer]
public class SelfSerializer : EdgeSerializer<SelfPayload>
{
    protected override void Initialize(IntermediateSerializer owner) { ++Calls; Nested = owner.GetTypeSerializer(typeof(SelfPayload)); }
}
[ContentTypeSerializer]
public class LeftSerializer : EdgeSerializer<LeftPayload>
{
    protected override void Initialize(IntermediateSerializer owner) { ++Calls; Nested = owner.GetTypeSerializer(typeof(RightPayload)); }
}
[ContentTypeSerializer]
public class RightSerializer : EdgeSerializer<RightPayload>
{
    protected override void Initialize(IntermediateSerializer owner) { ++Calls; Nested = owner.GetTypeSerializer(typeof(LeftPayload)); }
}
[ContentTypeSerializer]
public class FailingSerializer : EdgeSerializer<FailingPayload>
{
    public static FailingSerializer Created;
    public static bool Fail = true;
    public IntermediateSerializer Owner;
    public FailingSerializer() { Created = this; }
    protected override void Initialize(IntermediateSerializer owner)
    {
        ++Calls;
        Owner = owner;
        if (Fail) throw new InvalidOperationException("initializer failure");
    }
}
static class IntermediateInitializationEdgesOracle
{
    static int checks, failed;
    static void Check(bool condition, string label)
    {
        ++checks;
        if (!condition) { ++failed; Console.WriteLine("FAIL " + label); }
    }
    static int Main()
    {
        try
        {
            Console.WriteLine(typeof(IntermediateSerializer).Assembly.FullName);
            var singleton = typeof(IntermediateSerializer)
                .GetProperty("SingletonInstance", BindingFlags.NonPublic | BindingFlags.Static);
            Exception observed = null;
            try { singleton.GetValue(null, null); }
            catch (TargetInvocationException error) { observed = error.InnerException; }
            Check(observed != null && observed.GetType() == typeof(InvalidOperationException), "eager custom initialization propagates original exception");
            var failedInstance = FailingSerializer.Created;
            Check(failedInstance != null && failedInstance.Calls == 1, "failed owner invokes initializer once");
            Check(Object.ReferenceEquals(failedInstance, failedInstance.Owner.GetTypeSerializer(typeof(FailingPayload))), "failed owner retains its published serializer without reinitializing");
            FailingSerializer.Fail = false;
            var owner = (IntermediateSerializer)singleton.GetValue(null, null);
            Check(!Object.ReferenceEquals(owner, failedInstance.Owner), "failed singleton construction is not published");
            var recovered = (FailingSerializer)owner.GetTypeSerializer(typeof(FailingPayload));
            Check(!Object.ReferenceEquals(recovered, failedInstance), "new owner creates a fresh custom serializer");
            Check(recovered.Calls == 1, "recovered initializer once");
            Check(failedInstance.Calls == 1, "old failed instance was not retried");
            var self = (SelfSerializer)owner.GetTypeSerializer(typeof(SelfPayload));
            Check(self.Calls == 1, "self initializer once");
            Check(Object.ReferenceEquals(self.Nested, self), "self lookup returns published initializing instance");
            Check(Object.ReferenceEquals(owner.GetTypeSerializer(typeof(SelfPayload)), self), "self cache identity");
            var left = (LeftSerializer)owner.GetTypeSerializer(typeof(LeftPayload));
            var right = (RightSerializer)owner.GetTypeSerializer(typeof(RightPayload));
            Check(left.Calls == 1, "left initializer once");
            Check(right.Calls == 1, "right initializer once");
            Check(Object.ReferenceEquals(left.Nested, right), "left nested identity");
            Check(Object.ReferenceEquals(right.Nested, left), "right nested identity closes cycle");
            Check(Object.ReferenceEquals(recovered, owner.GetTypeSerializer(typeof(FailingPayload))), "recovered cache remains stable");
            Console.WriteLine("checks=" + checks + " failed=" + failed);
            return failed == 0 && checks == 15 ? 0 : 1;
        }
        catch (Exception error) { Console.WriteLine("SETUP/BEHAVIOR " + error); return 2; }
    }
}
