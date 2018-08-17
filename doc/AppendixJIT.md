# Our attacks vs. JIT compilers

This brief appendix to the paper --- "Code That Never Ran", found
[here in this repo](paper.pdf) --- details our (so-far-negative) results
attempting to extend the paper's attacks to JIT compilers.
Some justification for why this is a topic worth investigating can be found in
the paper; this document will assume thorough familiarity with the contents of
the paper, specifically Section 4 (Experiments).
In particular, we are interested in leaking the values of not constants, but
rather values which are merely profiled as constant by the JIT (despite not
being declared as such).

Testing was performed on SpiderMonkey 60.0.2, V8 6.9.0, and HotSpot 10.0.2,
all on macOS 10.13.6.

# Dead store elimination

We first study the dead store elimination attack from the paper's Section 4.3.
A necessary precondition for mounting this attack in the JIT context is to
have the JIT perform dead store elimination at all, in particular on stores
to some kind of variable that could be visible across threads.

## SpiderMonkey

### Pure JavaScript

SpiderMonkey does not do dead store elimination at all when compiling
JavaScript.
This has been confirmed experimentally, as well as by examining the source code.
IonMonkey (the most-optimizing compiler that is part of SpiderMonkey) appears
to never remove store instructions except if they're unreachable; it doesn't
appear to ever perform a "must alias" analysis, only a "may alias" analysis, so
it never determines that stores "must" store to the same place.

### Wasm

SpiderMonkey's optimizing compiler for wasm is also IonMonkey, so the same
analysis as done for pure JavaScript applies here as well.
As of this writing it is planned for Cranelift (recently renamed from "Cretonne")
to eventually replace IonMonkey as the optimizing backend for wasm code in
SpiderMonkey, and Cranelift's readme indicates an eventual goal of replacing
the codegen backend of IonMonkey for JS code as well, making Cranelift the
unified codegen stage for both wasm and JS.
Therefore, we investigated Cranelift as well, and a cursory examination of
Cranelift source code at the time of this writing seems to indicate that it,
like IonMonkey, also never removes store instructions except if unreachable.
Cranelift's dead code elimination pass specifically always retains stores.
Finally, we confirmed with Dan Gohman that Cranelift currently does not perform
dead store elimination.

## V8

### Pure JavaScript

V8 does explicitly have a dead store elimination pass, which they call
"store-store elimination".
Unfortunately for the attacker, this pass only operates on V8 `StoreField`
opcodes, which are stores to named object fields; it does not operate on V8
`StoreElement` opcodes, which are stores to array locations by index.
Experimental evidence confirms that dead store elimination happens on stores to
named object fields but not stores to indexed array locations.
We are operating under the assumption that for our dead store elimination attack
to work, we require dead store elimination applied to stores to a
`SharedArrayBuffer`, and it doesn't seem possible to get named object fields to
refer to memory in a `SharedArrayBuffer`.

### Wasm

V8 does not even run its "store-store elimination" pass at all when compiling
wasm.
No dead store elimination takes place.

## Java (HotSpot)

HotSpot is pretty clever --- in fact, _too_ clever --- with redundant stores to
local variables.
In brief, HotSpot has a `schedule_late` function in its codegen phase that
pushes each instruction as far down as possible based on the dominator tree of
its uses; this means that the initial store in our dead store elimination
pattern is pushed inside the `true` branch of the `if`, and is never executed
if the security check fails.
More details and a worked-through example of this are available from
@asajeffrey (ask him for the email from Craig entitled "mid-week progress
report" on July 31, 2018), but it turns out to be irrelevant because of the
issue described below.

(In the above discussion, "HotSpot" refers literally to the JIT itself; the
optimization referred to occurs in the JIT phase and not during the
compilation to bytecode, i.e. `javac`.
The remainder of this discussion, below this note, may be a little less precise
in this respect, and tends to use the term "HotSpot" regardless of whether it
refers to behavior of `javac` or of the actual JIT codenamed HotSpot.)

In order for the dead store elimination to be observable from another thread,
we need dead store elimination not on stores to a local variable but to an
object field --- either a "static variable" or an "instance variable" in Java
terminology.
HotSpot appears to not perform dead store elimination at all on static
variables, and to only perform dead store elimination on instance variables
when the `if` statement between them can be resolved at compile time (e.g., the
condition is a constant or a variable marked `final`).
A possible explanation, consistent with observed behavior, is that HotSpot
resolves and collapses `if` statements with constant conditions _first_, and
then performs dead store elimination on instance (but not static) variables,
but only within a basic block (only within straight-line code).
Furthermore, HotSpot does not appear to "hoist" common statements that appear
on both branches of an if/else statement, including stores to local,
instance, or static variables.
The code `if(x) { y = 1; } else { y = 1; }` is not optimized unless the value
of the condition `x` is known at compile time.
This means that the dynamic condition (security check) in our attack always
remains between the two instance/static stores we want dead store elimination
on, and therefore, HotSpot does not perform this dead store elimination.

# Load-store reordering

We now move on to the load-store reordering attack described in the paper's
Section 4.2 (and in Section 3.10).
It is harder to reach a "clean" impossibility result with respect to this
attack than it is with the dead store elimination attack, because there are a
wide variety of patterns that _could_ cause the compiler to reorder in a way
observable in another thread.
For example, consider how different the paper's 3.10 and 4.2 code snippets are,
as an illustration of the wide design space.
Nonetheless, as a starting point we will analyze the 3.10 and 4.2 snippets
themselves, and leave analysis of the entire design space open for now.

## SpiderMonkey

We note the SpiderMonkey shell's `--ion-sink` and `--ion-instruction-reordering`
command-line options, which are both disabled by default in our version of
SpiderMonkey, and we enable them both in all of our experiments.
We have not investigated whether Firefox sets these options or not in its use of
SpiderMonkey.

### Pure JavaScript

With respect to the code snippet from Section 3.10, testing reveals that
SpiderMonkey performs the `if(y==0)` before any `x=1` in all cases; it does not
perform the load-store reordering that would enable us to potentially observe
`z==1`.
In fact, testing seems to show that SpiderMonkey never hoists common statements
from both branches of an `if`, or at least not in the case where the common
statement is an assignment to a global (and the condition is a global as well).
The 3.10 attack fundamentally relies on this kind of hoisting, so it will not
be possible to mount against SpiderMonkey.

In our testing with the code snippet from 4.2, SpiderMonkey doesn't ever
reorder the read of `y` above the write of `x`.
This is relatively unsurprising, as the reordering observed in 4.2 was really
a quirk of `gcc`, in that `gcc` prefers to read variables into registers at the
start of a function, i.e. to move variable reads "up" as much as possible.

### Wasm

Regarding IonMonkey (the current optimizing wasm backend for SpiderMonkey), I
haven't yet been able to determine whether it's vulnerable, although I suspect
not, as IonMonkey is the same engine which SpiderMonkey uses to optimize JS,
and we saw above that SpiderMonkey wasn't vulnerable to our attacks when using
JS.
Regarding Cranelift (see notes on SpiderMonkey-Wasm for the dead store
elimination attack), Dan Gohman also confirmed that Cranelift doesn't
(currently) perform load-store reordering.

## V8

### Pure JavaScript

For the Section 3.10 attack, experimentation appears to show that V8 does not
hoist common statements from both branches of an `if`, or at least not when
the common statement is an assignment to a global.
This is the same behavior we observed with SpiderMonkey.

For the Section 4.2 attack, V8 does not appear to do any reordering.
This is relatively unsurprising for the same reasons as explained with respect
to SpiderMonkey (regarding the 4.2 attack being pretty specific to `gcc`).

### Wasm

For the Section 3.10 attack, again, V8 appears to never hoist common statements
from both branches of an `if`, or at least not when the common statement is a
store to wasm linear memory.
This was verified to be the case even when compiling the wasm with TurboFan,
which I understand to be V8's most aggressively optimizing wasm compiler.

For the Section 4.2 attack, again V8 does not appear to do any reordering, and
again this is relatively unsurprising.

## Java (HotSpot)

As discussed above in conjunction with dead store elimination, HotSpot does
not appear to hoist common statements from both branches of an `if`, unless the
condition is a compile-time constant, e.g. marked `final`.
Even if HotSpot profiles the `if` statement as always going a certain direction,
and requires a bailout if the `if` statement goes the other direction, the
common statement is still not hoisted --- it is only executed after the bailout
check.
This makes the Section 3.10 attack infeasible against HotSpot (except possibly
for compile-time constant secrets, which are less interesting).

For the Section 4.2 attack, HotSpot actually does reorder the read of y above
the write to x, and this reordering does not appear in the bytecode, so it is
in fact HotSpot itself doing the reordering rather than `javac`.
Unfortunately for the attacker, however, this reordering only occurs when the
security check is known _at compile time_ to be `false`.
Furthermore, when the security check is known at compile time, `javac`
eliminates the security check and access of `SECRET` altogether, and they do
not appear in the bytecode.
Therefore the value of `SECRET` has no effect on the reordering done by HotSpot
--- it happens regardless of the value of `SECRET` --- and there is no way to
infer the secret's value.
When the security check is not known to be `false` at compile time but rather
is only _profiled_ as always `false`, HotSpot does not perform the reordering.
This was tested with operations on both static and instance variables in Java.

The fact that reordering does exist here at least in some cases leaves open the
question, specifically for HotSpot, whether some other pattern might exist
which leads to a successful attack.
The key issue here is that a successful attack requires the presence of
load-store reordering to be _secret-dependent_, i.e. requires that load-store
reordering occurs for some value of the secret but doesn't occur for some other
value.
This in turn requires at least two things which turn out to be problematic
for the attacker: sufficiently aggressive code motion in the appropriate
circumstances, and value-based profiling.

* First we discuss the issue of code motion.
  We note that the load-store reordering has to occur in _live_ code, that is,
  it can't just occur inside the branch of the `if` statement that passes the
  security check --- hereafter referred to as the "security-checked area".
  Since the secret can only be accessed inside the security-checked area, but
  the load-store reordering must be observed outside the security-checked area
  (and in particular, even when failing the security check), this
  means that some load or store (to an instance or static variable) has to,
  in some manner, be moved across/into/out of an `if` statement, or
  alternately, be moved or not moved depending on something that happens inside
  an `if` statement.
  In my experimentation with HotSpot, I haven't found that it ever does either
  of these things (unless, as discussed above, the `if` condition is a
  compile-time constant --- but in that case HotSpot doesn't even see an `if`
  statement in the first place, as `javac` has eliminated it).
  HotSpot may move stores to _local_ variables across/into `if` statements, but
  it never moves stores to instance or static variables in that way, at least
  in my experimentation.
* Second, we need value-based profiling, at least to conduct load-store
  reordering attacks of the form presented in the paper.
  Even though HotSpot will profile the direction of `if` statements and
  optimize for the common case, it appears to never profile the value of an
  instance or static variable, preferring instead to always reload it even on
  the fast path, without making assumptions or optimizations based on its
  commonly-observed value.
  This means that, in effect, the value of the secret (assuming it is a
  "runtime secret" whose value is not known at compile time, i.e. not known to
  `javac`) will never influence any of the decisions or optimizations made by
  HotSpot.
  The only way to avoid this seems to be to use the value of the secret in an
  inner branch condition, much as the dead store elimination attack from the
  paper does.
  However, this seems to make it even less likely that the profiling happening
  inside the security-checked area would influence optimizations happening
  outside it.
  At least, both the 3.10 and 4.2 attacks rely on the _value_ of the secret
  causing an assignment operation to be combined (or not) with another
  assignment operation outside the security-checked area.
  It is very unclear that any pattern would exist where the profiling of an
  inner branch inside the security-checked area would influence a load-store
  reordering outside of, into, or out of the security-checked area.

Both of these issues --- code motion and value-based profiling --- seem to
present severe obstacles for the attacker, leading us to conclude that it seems
impossible for the presence or absence of load-store reordering to leak the
value of a runtime secret in HotSpot.
