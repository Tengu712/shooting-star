# exh2imh

## Outline

A converter to transform an internal header file into an external header file.

* `EXPORT` is replaced with `CALLCONV`
* `__declspec(dllexport) ` is removed

## Usage

```
exh2imh <in-filename> <out-filename>
```

* `in-filename`: an input binary file path.
* `out-filename`: an output header file path.
