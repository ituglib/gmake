HOW TO CONTRIBUTE TO GMake
============================

Please visit our main page [Main Page][mp] page for other ideas about how to contribute.

  [mp]: https://ituglib.connect-community.org/apps/Ituglib/

Development is done on GitHub in the [ituglib/gmake][gh] repository.

  [gh]: https://github.com/ituglib/gmake

To request new features or report bugs, please open an issue on GitHub

To submit a patch, please open a pull request on GitHub.  If you are thinking
of making a large contribution, open an issue for it before starting work,
to get comments from the community.  Someone may be already working on
the same thing or there may be reasons why that feature isn't implemented.

To make it easier to review and accept your pull request, please follow these
guidelines:

 1. Anything other than a trivial contribution requires a [Contributor License Agreement][cla] (CLA).
    The CLA gives us permission to use your code.
    If your contribution is too small to require a CLA (e.g. fixing a spelling
    mistake), place the text `CLA: trivial` on a line by itself separated by
    an empty line from the rest of the commit message. It is not sufficient to
    only place the text in the GitHub pull request description.
    To amend a missing `CLA: trivial` line after submission, do the following:

  [cla]: policies/cla.md

    ```
        git commit --amend
        [add the line in your editor, save and quit the editor]
        git push -f
    ```

 2. Copyrights for source files must be retained as is without modification. Deleting
    copyright notices will result in your pull request being rejected.

 3. Patches should be based on the main branch using the most recent commit. You
    should expect to have to rebase your branches often. We will not accept
    merge commits so you will need to rebase before your pull request will be
    acceptable.
