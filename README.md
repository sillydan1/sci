# Suckless/Simple Continous Integration
Jenkins, Travis, GitHub Actions, GitLab CI. The list goes on.
This is a minimal tool for fulfilling the CI (Continous Integration) use case.

## Brainstorm
If you dont want to congest your CI server. Too bad. Write faster ci suites. (TODO: implement runners)

I would like to try to avoid writing a million REST APIs, as that just results in bloat usually.

Could this also be used as a systest replacement? The value of systest is the drivers and the configuration files. Not
really the controllers themselves. But yes, this system could replace the systest runner scripts. But not the drivers or
the configuration files themselves...

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

### Progress
 - [x] Zeroth things first, let's create a simple CLI application with `--verbosity VAL` and `--help` options.
 - [x] First things first, let's implement something that reacts when some provided file changes (not poll please).
 - [ ] Second things second, implement a simple logging system with differing levels of verbosity and configurable 
       output file using cli options.
 - [ ] Third things third, implement a thing that simultaneously watches two different files (multithreading).
 - [ ] Fourth things fourth, implement a prototype that reads a space-separated file and populates a struct.

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
