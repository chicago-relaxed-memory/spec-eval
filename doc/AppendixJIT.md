# Our attacks vs. JIT compilers

This brief appendix to the paper --- "Code That Never Ran", found
[here in this repo](doc/paper.pdf) --- details our (so-far-negative) results
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
However, no actual experimentation has yet been done on Cranelift to support
this conclusion.

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
SpiderMonkey performs the `if(y==0)` before any `x=1` in all cases; there is
no load-store reordering that would enable us to potentially observe `z==1`.
In fact, testing seems to show that SpiderMonkey never hoists common statements
from both branches of an `if`, or at least not in the case where the common
statement is an assignment to a global (and the condition is a global as well).
The 3.10 attack fundamentally relies on this hoisting, so it will not be
possible to mount against SpiderMonkey.

In our testing with the code snippet from 4.2, SpiderMonkey doesn't ever
reorder the read of `y` above the write of `x`.
This is relatively unsurprising, as the reordering observed in 4.2 was really
a quirk of `gcc`, in that `gcc` prefers to read variables into registers at the
start of a function, i.e. to move variable reads "up" as much as possible.

### Wasm

TODO

## V8

TODO

## Java (HotSpot)

TODO
