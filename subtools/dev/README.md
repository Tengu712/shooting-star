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
* `gn` : generate build.ninja
  * based on ./src and ./sample
* `clean` : clean workspace
  * remove all files that were created by this app or ninja
