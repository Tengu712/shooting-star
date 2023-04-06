# bin2c

## Outline

A compiler from a binary to a C.

## Usage

```
bin2c <in-filename> <out-filename> <symbol>
```

* `in-filename`: an input binary file path.
* `out-filename`: an output header file path.
* `symbol`: bin2c generate following 2 variables:
  * `char <symbol>_data[]`: binary array data.
  * `int <symbol>_size`: binary array size.

## Example

```
$ echo "012abc" > foo.txt
$ bin2c foo.txt bar.h baz
compile 'foo.txt' to 'bar.h'
  data array name: baz_data
  data array size: baz_size
succeeded
$ cat bar.h
char baz_data[] = { 48, 49, 50, 97, 98, 99, 10 };
int baz_size = 7;
```
