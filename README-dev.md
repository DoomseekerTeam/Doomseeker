# `README` file for developers

    Copyright (C) 2019 Juuso "Linda" Lapinlampi

    Copying and distribution of this file, with or without modification,
    are permitted in any medium without royalty provided the copyright
    notice and this notice are preserved.  This file is offered as-is,
    without any warranty.

Hello, lost soul. Before flying at our fellow developers with sharp teeth and
potentially frustrating us like Hell on Earth, please familiarize yourself with
our following combat instructions to push forward. Thanks, servant. `:-)`

(These notes are written for people who want to help with development on the
Doomseeker and Wadseeker projects.)

## How you can help

Not all help needed is technical. You can help too!

* Report issues to developers in existing releases.
* Contribute patches to resolve existing issues in releases.
* Write documentation pages.
* Send your thanks to the developers.

Some are detailed in these notes. Finding an issue that looks easy to fix and
then trying to fix it is possibly the easiest way to get involved.

## Reporting and searching issues

> Did we drink yesterday?

----

A list of known issues and software defects is maintained on [Zandronum issue
tracker](https://zandronum.com/tracker/). Those pesky Doomguys keep making a
mess!

You are welcomed to register and contribute to issues `;-)` on our issue
tracker, but we do also recognize not every old school autistic hacker probable
with a neckbeard is willing to do this. (To be fair, some of us don't like
MantisBT issue tracker either.)

## Fetching the latest sources

> MAP01: Entryway

----

The sources for development are available through Git on Bitbucket:
<https://bitbucket.org/Doomseeker/doomseeker>.

A typical way to get started is to clone the Git repository, using a
Git client from command line.

`git clone https://bitbucket.org/Doomseeker/doomseeker.git`

Refer to the manual of your client for information on interfacing with
Git.

A distribution of Doomseeker source code will most often include a copy of
Wadseeker's source code at `src/wadseeker`.

## Contributing guidelines

> MAP08: Tricks and Traps

----

*id*eally you'll do the following before us needing to remind about it. `;-)`

### Licensing

> Those demons never stood a chance...

----

We expect contributions to be made available under a compatible free license,
with the "four" traditional freedoms: to run, study, change, redistribute, make
copies, improve and release your improvements to the public. Similarly, we
expect something similar for non-software contributions such as documentation.

Since Doomseeker 1.2, the preferred license agreement for code contributions is
**GNU Lesser General Public License, version 2.1 of the license or (at your
option) any later version** as published by the Free Software Foundation. See
the `LICENSE` file for more details.

Prior versions had contributions licensed under GNU General Public License,
version 2 (or any later version). The majority of this code was relicensed to
`LGPL-2.1+`. [Issue #3237](https://zandronum.com/tracker/view.php?id=3237) has
more details about this online.

### Copyright and license statement

Include your copyright statement and license header at the top of source code
files.

### C++ coding style

Try to use `uncrustify` to check your C++ changes, or follow the existing
coding style for the source code files to the best of your ability. A
maintainer may also `uncrustify` the source code tree occasionally.

The standard version followed is currently **C++11**, as we look forward to use
more modern C++ standard versions in the future.

## Submitting contributions

> For glory and vodka!

----

We'd love you to bring us *DOOM* by invading our mailboxes with patches. `;-)`

### Bitbucket

> Push - Griefing Made Legal

----

The preferred (or convenient way) way of contributing is to enter into an
agreement with Atlassian Pty Ltd for an Atlassian account to use on
[Bitbucket](https://bitbucket.org/). ([Terms of
Service](https://www.atlassian.com/legal/cloud-terms-of-service) and [Privacy
Policy](https://www.atlassian.com/legal/privacy-policy) may apply.)

Bitbucket has documentation available online: [Tutorial: Learn about Bitbucket
pull
requests](https://confluence.atlassian.com/bitbucket/tutorial-learn-about-bitbucket-pull-requests-774243385.html).
Specifically, [Clone and make a change on a new
branch](https://confluence.atlassian.com/bitbucket/clone-and-make-a-change-on-a-new-branch-774243398.html)
and [Create a pull request to merge your
change](https://confluence.atlassian.com/bitbucket/create-a-pull-request-to-merge-your-change-774243413.html).

### Issue tracker

Contributions are accepted for merging from issue attachments on [Zandronum
issue tracker](https://zandronum.com/tracker/). This requires an account on the
issue tracker.

Use git [format-patch](https://git-scm.com/docs/git-format-patch) to create
patches which include contributor's name and email address. Attach the patches
to a reported issue.

### External communication

At present, Doomseeker does not have an external communication channel such as
IRC or a mailing list for contribution submissions. As a courtesy, one of the
developers or maintainers may be willing to help submit your contribution via
external communication (e.g. email).
