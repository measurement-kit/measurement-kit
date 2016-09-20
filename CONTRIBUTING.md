# Contributing to MeasurementKit

Open a pull request by branching the `master` branch. Try to keep the pull request
small (a good pull request should only address one specific issue).

Make sure that new code that you add, or code that you modify, is covered by unit tests
and, if not, write unit tests for that. In general, it would be good for pull requests
not to reduce the current code coverage of the project.

Small pull requests should be tagged as `hotfix` and could be self merged. All other
pull request should be reviewed by another core developer who should take responsibility
of the merge. The repository should be configured such that it is not possible to
merge into `master` (or `stable`) if unit tests are not passing.

If the diff is small, squash merge is preferred, otherwise preserve the history. If
possible do not rebase locally but do merge and resolve conflicts locally and push the
merge commit, such that the way in which you solved conflicts is visible to others.

Before a release, review the code base, fixing simple bugs directly and opening issues
to describe more complex required fixes and refactoring opportunities. Also during such
review make sure that the documentation is up to date with the code.
