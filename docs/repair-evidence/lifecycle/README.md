# Prepared lifecycle audit and native witnesses (141–144; not repaired)

Production audit while the geometry checkpoint is frozen. No lifecycle runtime
code or demo assertions have been changed. The fresh `ComponentContractsOracle.cs` now
passes29/29 genuine checks (`repair-lifecycle-contracts-xna{,-build}.log`): hook
dispatch/suppression and identity, repeated explicit disposal/finalizer silence,
collection index ordering/null Clear events, and launch parsing/duplicate Add.
This does not yet validate the separate live-Game removal/ownership path.

The separate `ComponentDisposalOracle.cs` now exercises a genuine live Game
(without Run/device creation): self-removal before repeated Disposed events,
Game's original-component snapshot even when one callback removes another
component, guarded reentrant disposal, and a later Dispose after a handler throws.
See `repair-component-disposal-xna.log` for the measured result. Native lifetime
and sanitizer coverage must still be added before repairing this path.

Current CNA still contains all four reported causes:

- 141: GameComponent::Dispose(bool) returns when disposed_ is set, suppressing
  the next explicit Disposed event. Genuine GameComponent.Dispose(bool) also
  removes the component from Game.Components before dispatch; CNA does not.
  Do not blindly add that removal: CNA's owning shared_ptr collection can destroy
  its last owned component inside Remove. Inspect Game teardown/iteration and
  preserve lifetime through dispatch, with an ASan regression, before changing
  this path. Destruction must not dispatch a managed-style event. Genuine Game's
  Dispose copies its component collection to an array before invoking Dispose;
  CNA currently walks the live shrinking collection forward. A lifetime-holding
  snapshot is therefore a reference-backed companion, not an arbitrary redesign.
- 142: all four property setters raise events directly, then call empty virtual
  hooks. Genuine setters call only the hooks; the base hooks dispatch with `this`
  as sender. An override omitting base suppresses the event. Existing native
  GameComponentTests.cpp and DrawableGameComponentTests.cpp are two-line stubs,
  so new native tests need a real Game on the private display, not a fake reference.
- 143: CNA checks duplicates before Insert's range. Genuine Collection.Insert
  validates its public index before invoking the overridden InsertItem, explaining
  why looking only at the XNA InsertItem decompilation gives the wrong conclusion.
  Clear raises for every slot including null in genuine XNA. CNA skips null there.
  Add/Remove null silence and pre-clear event observation must remain distinct.
- 144: CNA drops flags with no value and ignores duplicate Add. Genuine parser
  splits at the first colon, skips empty keys, preserves empty values, trims all
  leading slash/dash characters, and keeps the first duplicate parsed key.
  It skips argv[0]. CNA's explicit-vector constructor is a documented extension
  taking arguments without an executable entry, whereas its default constructor
  reads the real process argv. Preserve that distinction: do not skip the first
  explicit argument or accidentally insert the executable as a parameter.
  Current native tests explicitly expect the three wrong empty-value behaviors.

References inspected: local genuine Windows Game assembly decompilation's
GameComponent.cs, DrawableGameComponent.cs, GameComponentCollection.cs and
LaunchParameters.cs; matching CNA public headers/sources; native collection and
launch tests; demos game.components.* and game.launch_parameters.parse; permanent
issue141–144 histories. Next: reproduce exact contracts with a small genuine
runtime probe, then add failing native tests before any product edit. Keep this
batch separate from geometry and the prepared Color132 work.

## Native pre-fix witnesses (2026-10-20)

Three added LaunchParameters tests and two added collection tests all fail against
unchanged runtime production; all39 old controls pass (44total). This covers the
full eight-entry parse, one-character/empty keys, duplicate Add atomicity, invalid
duplicate index ordering and null Clear events. Reports:
`repair-runtime-values-before.log`, `repair-runtime-values-corrected-before-build.log`.
The initial strict build found two pre-existing ignored nodiscard Remove results;
the existing tests now additionally assert both return true. No assertion removed.

`ComponentLifecycleCompatibilityTests.cpp` adds six genuine-backed real-Game
tests. All six fail on unchanged code, with zero skips (1.014s), on a private
display: hook order/suppression/sender identity, repeated disposal/finalizer silence,
guarded reentry/throwing handlers, removal before disposal, last owning reference
surviving the entire virtual call, and Game's owning snapshot despite removal by
another callback. The ownership cases retain safe pre-fix cleanup; after repair
they must also run against instrumented production translation units.
Report: `repair-runtime-lifecycle-corrected-before.log`.

The standalone Game probe initially added -Wpedantic beyond CNA's normal runtime
test configuration, which rejects Sharp Runtime Decimal's explicitly required
native unsigned __int128. That build failure and the following missing-executable
launch are retained. The corrected probe uses C++23/Wall/Wextra/Werror, compatible
with that supported extension, and builds without warnings. No dependency change
or behavioral test suppression was made. No lifecycle fix or closure is claimed.

The tests are now also integrated into the real CnaTests target. Its warning-free
13-step rebuild and private-display run reproduce the same result:50tests,
39PASS/eleven new intendedFAIL/zero skips (1.063s;
`repair-lifecycle-native-before{,-build}.log`). Runtime production remains unchanged.

Five further collection tests preserve ownership through an Added callback removing
the object, release old ownership even when Removed throws, preserve ownership of
a new registration created inside Removed, exercise nested removal during Clear,
and retain an owned Clear event argument until dispatch returns. The standalone
values suite now has49tests:39 old controls pass and all10 new witnesses fail on
unchanged production (`repair-collection-ownership-before{,-build}.log`). These five
additional tests are not yet in the previously built native CnaTests executable.

`CollectionMutationOracle.cs` passes all3 genuine XNA checks
(`repair-collection-mutation-xna{,-build}.log`). Clear uses a live index loop:
removing its first item from the first callback produces first,first,third events
and then empties the collection. Neither a snapshot nor fail-fast enumeration
would match. No mutation-version field or associated ABI change is needed.
Game.Dispose's original component snapshot is a separate contract; it must retain
ownership while components remove themselves or each other. Existing GameTest
controls also pass19/19 privately (3.265s; `repair-lifecycle-game-controls-before.log`).

The C forwarding layer currently promises one-shot disposal based on the incorrect
canonical assumption. Its callback adapter, public comment and repeated-disposal
smoke expectation must be reviewed with the underlying fix, not left inconsistent
with corrected C++ events. No C runtime behavior has yet been changed.

`ComponentCDisposalProbe.c` independently verifies this against the actual frozen
external C ABI: strict C17/Wall/Wextra/Wpedantic/Werror build, private display,
11checks/seven passes/four failures. Missing self-removal and second explicit
callback/event are observed; handle cleanup succeeds and destruction does not
invoke extra managed callbacks. Reports `repair-component-c-disposal-corrected-before*`.
The initial probe's cleanup check assumed the earlier count had already reached2;
its five-failure report is retained. The corrected cleanup control compares counts
immediately before and after release, independently of the four actual failures.

`DrawableDisposalOracle.cs` adds4/4 genuine passes: even before Initialize, every
explicit derived disposal invokes UnloadContent before the base event; the false
path is silent, and an UnloadContent exception does not suppress a later attempt.
Two matching native tests fail. Combined standalone native lifecycle/collection/
launch suite now has57tests:39 old controls pass,18 intended new failures,zero skips
(907ms; `repair-drawable-lifecycle-native-before*`). The genuine report is
`repair-drawable-disposal-xna*`. Include this directly related derived disposal
contract in the141 repair; do not preserve initialized_ as a false one-shot guard
for a virtual managed hook. Device-service Initialize/event hookup is a separate
source limitation and is not claimed validated by this no-Initialize probe.
