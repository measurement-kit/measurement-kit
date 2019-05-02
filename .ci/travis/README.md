This code is forked from https://github.com/measurement-kit/ci-common
at 5601b6f397d103433315a7c07d09fe7327d4dabc. We were at the point in
which MK was the sole user of this code. Hence we vendored.

The [script/travis](script/travis) will automatically trigger
an `autotools` build. You can locally run a specific Travis build,
e.g. "vanilla", with:

```
./.ci/travis/script/travis vanilla
```

Of course, this requires you to have the docker daemon running.
