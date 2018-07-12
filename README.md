# Code that never ran: modeling attacks on speculative evaluation

Find our paper [in the doc directory of this repo](doc/paper.pdf).

Abstract:
> This paper studies information flow caused by speculation mechanisms
> in hardware and software.  The Spectre attack shows that there are
> practical information flow attacks which use an interaction of
> dynamic security checks, speculative evaluation and cache timing.
> Previous formal models of program execution have not been designed
> to model speculative evaluation, and so do not capture attacks such
> as Spectre. In this paper, we propose a model based on pomsets which
> is designed to model speculative evaluation. The model provides a
> compositional semantics for a simple shared-memory concurrent
> language, which captures features such as data and control
> dependencies, relaxed memory and transactions. We provide models for
> existing information flow attacks based on speculative evaluation
> and transactions.  We also model new information flow attacks based on
> compiler optimizations. The new attacks are experimentally validated against
> `gcc` and `clang`.  We develop a simple temporal logic that supports
> invariant reasoning.

## Code

The [src directory of this repo](src) contains the code used in the
Experiments section of the paper.
Specifically, the [lsr](src/lsr) directory contains the code for the
load-store reordering attack described in Section 4.2 of the paper, and the
[dse](src/dse) directory contains the code for the dead store elimination
attack described in Section 4.3 of the paper.
To run with default values of parameters, `cd` to the appropriate directory and
`make run`.
To test a broad range of parameter values, `cd` to the appropriate directory
and `make runtune`.
More detailed usage instructions are available by running the executable
(`./lsrattack` or `./dseattack`) directly with no arguments.

By default, the experiments run for 60 seconds per set of parameter values.
To adjust this higher or lower, change the `DURATION_MS` constant in the
appropriate source file.

## License

The artifacts in this repository are distributed under the following licenses:

* All software is licensed under the terms of the Mozilla Public License Version 2.0.
* All documentation is licensed under the terms of the Creative Commons CC-BY-SA v4.0 license.
