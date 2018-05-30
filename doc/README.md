# Ignoring merge conflicts in paper.pdf

This directory has paper.pdf checked in, which is very handy for people who
want to read the paper without installing LaTeX. But it does mean we're liable
to get lots and lots of merge conflicts.

The fix is to add a merge driver to `.gitattributes`, which is checked in,
but rather annoyingly this needs a configuration which has to be run locally,
and can't be checked in, sigh.

So, to avoid merge conflicts in paper.pdf, run:
```
git config merge.ours.driver true
```