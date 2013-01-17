restartd - A process-restarting daemon
======================================

This is a new upstream for the restartd(8) daemon, which was originally
debian-only. It will host the new developments for this project.

From the man page:

    Restartd is a daemon for checking running and not running processes. It
    reads the /proc directory every n seconds and does a POSIX regexp
    on the process names. You can execute a script or a program if the
    process is or is not running.


Links
-----
* [Debian QA page for the restartd package](http://packages.qa.debian.org/r/restartd.html)
* [Debian packaging repository for the restartd package](https://github.com/ajraymond/restartd-debian)
* [Ubuntu bug tracker for the restartd package](https://launchpad.net/ubuntu/+source/restartd/+bugs)
