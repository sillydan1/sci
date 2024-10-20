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
 - [ ] Seventh things seventh, package the sucker (arch, debian, alpine, docker)
    - [x] archlinux
         - https://wiki.archlinux.org/title/Creating_packages
    - [x] debian
         - see `/home/agj/documents/Projects/packaging/deb-packaging-tutorial.pdf`
         - just use docker.
    - [-] ~~alpine~~ later.
    - [-] ~~docker~~ later.
 - [ ] Eight things eight, try it out! - maybe even write the python webhook extension.
 - [ ] Ninth things ninth, fix bugs, see below
 - [ ] Tenth things tenth, write manpages, choose license
 - [ ] Eleventh things Eleventh, polish
 - [ ] Twelveth things last, release!
   - Setup gitea.gtz.dk (will learn you how to set up subdomains (useful for shop.gtz.dk))

BOOKMARK: okay. Now it feels like it's getting complicated. I want to run `sci` in a docker container. But that means
that the build-threads also run in that docker container - meaning the container should have all the build dependencies
installed and we all know where that rabbithole goes. 9-30YB docker images with about a trillion unique build systems.
Let's not do that.
The only alternative I can see is that the `sci` service is just not dockerized. The pipeline scripts can easily be
dockerized themselves. Just have a `scripts/wget-src-dist-and-sci-sh-dockerized.sh` with `arg1` being the docker image
to use?
```sh
#!/bin/sh
wget "$SCI_PIPELINE_URL"
docker run --rm -it --mount type=bind,source="$(pwd)"/thefileyouwgot.tar.gz,target=/thefileyouwgot.tar.gz,readonly --entrypoint sh $2
```
Or something like that... Perhaps we can figure something out with an inline `ADD`, that also extracts the archive in
the container or something. This approach is cleaner IMO. You can also more easily edit the `pipelines.conf` file if you
need to.

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

#### Bugs / Missing Features
 - [x] command output is being inherited. It should be piped into some random log-file
 - [ ] pretty sure that `ctrl+c` / SIGINT is not graceful yet.
 - [ ] missing license (heavily considering GPLv3)
 - [ ] pipeline scripts should be executed in a unique `/tmp` dir
 - [ ] Some way for third parties to see which pipelines are currently running and their status.
    - Could be as simple as looking in the logs directory.
    - How to mark a run as failed / success / warn?
    - Third parties may need to extract artifacts.
      or maybe the scripts themselves would upload artifacts?
 - [ ] I am deliberately not using `Restart=on-failure` in the `scid.service` file because we are using `Type=exec`
       and not `Type=notify` (yet) - which would require a `sd_notify` call of `READY=1` (see `man systemd.service`)
 - [ ] Custom environment variables passed to the pipelines on invokation should be possible.
 - [ ] Listener threads should be killed and restarted (worker pool should just chug along) when pipeline config file
       has changed during runtime. Should be disableable with `--no-hot-reload-config` - i.e. on by default.
 - [ ] `docker stop` is very slow. I am probably not handling signals properly yet.
 - [x] It seems that `-v 4` is segfaulting when running release builds, maybe the logger just cant find the source file?
       Nope. I just wrote some bad code (inverted NULL check).

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
