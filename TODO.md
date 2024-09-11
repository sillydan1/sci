### Progress
 - [x] Zeroth things first, let's create a simple CLI application with `--verbosity VAL` and `--help` options.
 - [x] First things first, let's implement something that reacts when some provided file changes (not poll please).
 - [x] Second things second, implement a simple logging system with differing levels of verbosity and configurable
       output file using cli options.
 - [x] Third things third, implement a thing that simultaneously watches two different files (multithreading).
       it should be cancellable with ctrl+c, but it should just contiuously print event notifications.
 - [x] Fourth things fourth, implement a prototype that reads a space-separated file and populates a struct.
 - [x] Fifth things fifth, implement a prototype that spawns a new thread that executes a shell command.
 - [x] Sixth things sixth, daemonize it!
 - [x] Seventh things seventh, package the sucker (arch, debian, alpine, docker)
    - [x] archlinux
         - https://wiki.archlinux.org/title/Creating_packages
    - [x] debian
         - see `/home/agj/documents/Projects/packaging/deb-packaging-tutorial.pdf`
         - just use docker.
    - [-] ~~alpine~~ later.
    - [-] ~~docker~~ later.
 - [x] Eight things eight, try it out! - maybe even write the python webhook extension.
  - [x] Port this document to gitea issue tracking
  - [x] enable PATH-able programs and argv in the command section
  - [x] custom environment variable passing. Something like `-e MY_TOKEN` ala docker-style
  - [x] address sanitizers please.
 - [ ] Ninth things ninth, fix bugs, see https://git.gtz.dk/agj/sci/projects/1
  - [ ] docstring in all header files
 - [ ] Tenth things tenth, write manpages, choose license
 - [ ] Eleventh things Eleventh, polish
 - [ ] Twelveth things last, release!
   - [x] Setup git.gtz.dk (will learn you how to set up subdomains (useful for shop.gtz.dk))
 - [ ] -1th things -1th, write a blog post about the tool (also set up your blog.gtz.dk)

Okay. Now it feels like it's getting complicated. I want to run `sci` in a docker container. But that means
that the build-threads also run in that docker container - meaning the container should have all the build dependencies
installed and we all know where that rabbithole goes. 9-30YiB docker images with about a trillion unique build systems.
Let's not do that.
The only alternative I can see is that the `sci` service is just not dockerized. The pipeline scripts can easily be
dockerized themselves. Just have a `scripts/wget-src-dist-and-sci-sh-dockerized.sh` with `arg1` being the docker image
to use?
```sh
#!/bin/sh
wget "$SCI_PIPELINE_URL"
docker run --rm -it -v .:/src -w /src $@
```
Or something like that... Perhaps we can figure something out with an inline `ADD`, that also extracts the archive in
the container or something. This approach is cleaner IMO. You can also more easily edit the `pipelines.conf` file if you
need to.

The aforementioned rabbithole went like this:
 - Let's say that `sci` is run inside a docker container.
   This would make it very easy to deploy, but:
 - Since pipelines are executed in the same environment as `sci`, either:
   The `sci` container must be all-encompassing. i.e. it contains every single build system and scriptling language that
   could possibly be used by any kind of user or; all pipelines must be run from a docker container themselves, meaning
   that the `sci` container must have `dind`-privileges. Either option is suboptimal and will lock users into one way of
   using `sci`, which is bad.
 - Conclusion: Fuck docker. All environment management is delegated to the user and is not `sci`'s responsibility!
   `sci` will always be run on the ci-machine itself, unless a user has provided a custom docker image, which is fine
   and doesn't burden the `sci` project.

You were getting the following `pipelines.conf` file to work:
```
scih-dev ssh://git@git.gtz.dk:222/agj/scih.git scih-onpush /etc/sci/scripts/git-clone-and-run-sci-sh.sh
scih-release ssh://git@git.gtz.dk:222/agj/scih.git scih-onrelease /etc/sci/scripts/git-clone-and-run-sci-sh.sh
```
To get this to work, you need to change some things regarding the packaging - i.e. the `scripts` directory should be
installed as well. Also, you want `sci` to be containerizable.
For the `.sci.sh` script to `docker login`, it will need secrets. Implement those.

You were reading :Man system.unit and :Man systemd.service as preperation on making a systemd unit file
This will be needed for the .deb package, as well as the arch linux package.
alpine linux is using OpenRC (cool), which complicates things a little bit, but shouldn't be too bad. The wiki is
generally really well written. Otherwise, I am sure that both wiki.gentoo and wiki.archlinux have great pages too
docker is super easy, just make a dockerfile - only concern is the trigger files.

### Note Regarding `inotify` usage
From the manpage:
```
With careful programming, an application can use inotify to efficiently monitor and cache the state of a set of
filesystem  objects.  However, robust applications should allow for the fact that bugs in the monitoring logic or races
of the kind described below may leave the cache inconsistent with the filesystem state.  It is probably wise to do some
consistency checking, and re‐build the cache when inconsistencies are detected.
```
i.e., we should _also_ poll the watched files every once in a while (maybe once per minute? idk) to ensure that we catch
all events.

### Useful manpages
 - `hier(7)`
 - `daemon(7)` and `daemon(3)`
 - `PKGBUILD(5)`
 - `info make`
 - `https://wiki.archlinux.org/title/Creating_packages`
