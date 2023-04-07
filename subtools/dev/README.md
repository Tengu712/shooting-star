# dev

## Outline

A development manager.

## Usage

**Build this app to Shooting Star root directory.**

```
Shooting-Star$ rustc -o ./dev ./subtools/dev/dev.rs
```

To run this app:

```
dev <option>
```

`<option>` is below:

* `init` : init workspace
  * build or rebuild subtools
  * generate build.ninja
* `build` : build project
  * generate build.ninja
  * run ninja
* `clean` : clean workspace
  * remove all files that were created by this app
* `sample <name>` : build sample whose name is `<name>`
