# Code that never ran: modeling attacks on speculative evaluation

Find our paper [in the doc directory of this repo](doc/paper.pdf).

Abstract:
> This paper studies information flow caused by speculation mechanisms
> in hardware and software.  The Spectre attack shows that there are
> practical information flow attacks which use an interaction of
> dynamic security checks, speculative evaluation and cache timing.
> Previous formal models of program execution are designed to capture
> computer architecture, rather than micro-architecture,
> and so do not capture attacks such
> as Spectre.  In this paper, we propose a model based on pomsets which
> is designed to model speculative evaluation.
> The model is abstract with respect to specific micro-architectural
> features, such as caches and pipelines, yet is powerful enough to express
> known attacks such as Spectre and \textsc{Prime+Abort}.  The model also allows for
> the prediction of new information flow attacks.  We derive two such
> attacks, which exploit compiler optimizations, and validate these
> experimentally against gcc and clang.

## Code

The [src directory of this repo](src) contains the code used in the
Experiments section of the paper.
Specifically, the [lsr](src/lsr) directory contains the code for the
load-store reordering attack described in Section V-B of the paper, and the
[dse](src/dse) directory contains the code for the dead store elimination
attack described in Section V-C of the paper.

To run with default values of parameters, `cd` to the appropriate directory and
`make run`.
To test a broad range of parameter values, `cd` to the appropriate directory
and `make runtune`.
To use a compiler `mycc` other than the default (system `gcc`),
`make CC=mycc [...]`.
More detailed usage instructions are available by running the executable
(`./lsrattack` or `./dseattack`) directly with no arguments.

By default, the experiments run for 60 seconds per set of parameter values.
To adjust this higher or lower, change the `DURATION_MS` constant in the
appropriate source file.

## License

The artifacts in this repository are distributed under the following licenses:

* All software is licensed under the terms of the Mozilla Public License Version 2.0.
* All documentation is licensed under the terms of the Creative Commons CC-BY-SA v4.0 license.
