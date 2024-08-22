# Simple CI (Continous Integration)
Jenkins, Travis, GitHub Actions, GitLab CI. The list goes on.
There are many CI systems.
All of them overcomplicate an extremely simple use case:
"I want to trigger custom scripts (usually just a list of simple shell commands) on a server.
Either automatically, or manually.
And then I want to be follow and see any errors that may happen."

There's no need to invent the wheel 10 different times.
Most of the actions required by the CI use case can be handled with most UNIX tools such as `tail`,
`cat`, `inotify` etc.
At the core of CI must be a triggering system that can be dynamically configured to run a pipeline script.

This program provides that simple triggering system where you simply `touch` a file, and the associated
pipeline will then be executed.
You can then follow the log via `tail`.

The operation of sci is configured through a pipelines.conf configuration file  (see  sci(7)  for
configuration  language  details) and each pipeline will have an associated pipeline trigger file
that can be By default, pipeline triggers are placed in /tmp/sci but this can be overridden  with
the `-T` flag (WIP feature). For more commandline options see the manpage `sci(1)` or use the `--help` option.

## Dependencies
Just GNU/Linux! `sci` is just using the POSIX api, so it should work with *NIX, but it has only been tested on GNU/Linux
systems.
NOTE: Not tested with `muslc` systems yet, `sci` use some libc GNU extensions, but it would be nice to support alpine
linux some time in the future.

## Build
Build the project using `make`:
```sh
make
```

## Install
Install the project:
```sh
make
sudo make install
```

By default, `sci` is installed to `/usr/local` but this can be overridden by providing the `PREFIX` var:
```sh
make
sudo make install PREFIX=/some/path
```

## Package
`sci` can be packaged for multiple distributions:

### Arch Linux
It is recommended that you use an arch linux distribution when building.
<!-- TODO: add a dockerfile for arch building -->
```sh
```

### Debian
It is recommended that you use the `deb-builder.dockerfile` docker image to build the debian image.
```sh
```

#### Building the debian builer docker image
```sh
docker build -t debbuilder deb-builder.dockerfile .
```

## Brainstorm
If you dont want to congest your CI server. Too bad. Write faster ci suites. (TODO: implement runners)

I would like to try to avoid writing a million REST APIs, as that just results in bloat usually.

Could this also be used as a systest replacement? The value of systest is the drivers and the configuration files. Not
really the controllers themselves. But yes, this system could replace the systest runner scripts. But not the drivers or
the configuration files themselves...

Why trigger files instead of IPC (Inter Process Communication)?
It was easier to implement. If you know how to add POSIX IPC (not a fan of System-V IPC), please submit a PR :-)

### Using
`/etc/sci/conf.d/pipelines.conf`
```txt
aaltitoad-release   https://github.com/sillydan1/aaltitoad      onpush-main         /home/sci/pipelines/cpp-dev-git.sh
aaltitoad-develop   https://github.com/sillydan1/aaltitoad      onpush-dev          /home/sci/pipelines/cpp-rel-git.sh
schoolnotes         https://gitlab.com/sillydan/schoolnotes     manual-name         "echo hello"
horse               https://gtz.dk/horse"                       manual-name         "curl -X POST https://example.com"
cool-horse          https://gtz.dk/horse"                       manual-name         "docker run --rm -v $SCI_PIPELINE_DIR:$SCI_PIPELINE_DIR -v /home/sci/pipelines:/pipelines alpine /pipelines/your-script.sh"
rad-horse           https://gtz.dk/horse"                       manual-name         "/home/sci/pipelines/dockerized-horse.sh"
<name>              <url>                                       <trigger-filename>  <pipeline-command>
```
Triggers are just bare files that reside in `/etc/sci/triggers/` - this means that multiple pipelines can use the same
trigger. When a trigger file is being written to or `touch`ed, this triggers the associated pipeline. Trigger files are
automatically created by the `scid` daemon during startup or reconfiguring.

Any webhooks should be managed by shim-APIs / adapter APIs that are very slim (possibly flask) REST APIs that just
listen for webhook event triggers and does a simple `touch <associated file>`. It could even read the same configuration
file as `scid` uses to autogenerate these webhooks. But it is explicity NOT part of the core system, as that would be
unneeded bloat.

How do I get information about a running pipeline? Perhaps a `/var/tmp/sci/<pipeline-name>` is continually updated with
runtime information? Then you can `cat`, or `tail -f` it to get the information.

`scid` manages running subprocesses, so it must be implemented in a language that handles that nicely.

Pipelines - Should pipelines have a custom language? NO! God already gave us shell scripts! Why would we reinvent the
wheel? Take a page out of Jenkins' book. `scid` should just call some command on trigger. That command could / should 
usually be a quick shell script (these calls are executed in the dispatched separate thread, so dont worry about 
sleeping or whatever you need to do). These scripts are equivalent to the Jenkins pipelines, and should update their 
current state in `/var/tmp/scid/$PIPELINE_NAME` by simply writing to the file (when implementing runners, we probably 
need to `rsync` these files automagically). Oh, and dont worry about cleaning up. Each pipeline execution will be placed
in a `/tmp/<pipeline-name>-<uuid>/` directory and will be executed by a gimped user. If you need to interact with the
linux stuff and it might break other pipelines or whatever, then your pipeline-command should just start a docker
container, volume mount the tmpdir and what else you need and run your command in that.

Need multistep pipelines? Then write a multistep shell script! You're the boss of your own CI server man.

```sh
#!/bin/bash
# cpp-dev-git-pipeline,sh
# NOTE: Will be executed from /tmp/$SCI_PIPELINE_NAME-<uuid>/
git clone -b dev "$SCI_PIPELINE_URL" "$SCI_PIPELINE_NAME"
echo "clone success" > "/var/tmp/sci/$SCI_PIPELINE_NAME" # You can have any kind of data in the pipeline status file.
cd "$SCI_PIPELINE_NAME"
cmake -B build
cmake --build build
```

## Secrets
Not planned yet. But they could be as simple as a keystore.

## Development
What language should I implement `scid` in?
 - `c` slow devtime, decent tooling, very minimal and forces you to be minimal also. Maybe
 - `c++` slow devtime, good tooling, very familiar. Can easily get out of hand. Maybe
 - `python` quick, easy, slow performance, terrible multiprocessing capabilities. Maybe
 - `java` slow devtime, bloated. No.
 - `rust` slow devtime (not familiar), great tooling, too many risks for a first prototype. No.
 - `shell` quick, easy, instant spaghetti. No.

I choose `c`!

I also choose `Makefile`s! - Just to force myself to use another build system than CMake!
If you want `compile_commands.json` files, you should use [bear](https://github.com/rizsotto/Bear) as it works well
