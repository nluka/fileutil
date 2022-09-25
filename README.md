# fileutil

## functions
- [x] [repeat](#repeat): repeat the contents of a file
- [x] [sizerank](#sizerank): rank files by size
- [ ] regex searching
- [ ] generate a binary file with a pattern

## repeat

duplicates a file's content N times, writing it as a new file.

| option | type | description |
| - | - | - |
| --in * | pathname | pathname of file to repeat |
| --count * | unsigned int, > 0 | number to times to duplicate content (1 = make copy) |
| --out * | pathname | pathname of resultant file |

\* = required

## sizerank

ranks top N largest files in directory/subdirectories.

| option | type | default | description |
| - | - | - | - |
| --dir | path | cwd | directory to search |
| --recursive | flag | off | enables recusive iteration through all subdirectories |
| --top | unsigned int, > 0 | 10 | how many top entries to rank |
| --minsize | unsigned int | 0 | smallest file size (in bytes) to consider |
| --maxsize | unsigned int | infinity | largest file size (in bytes) to consider |
| --pattern | regular expression | .* | regular expression to test files against |
| --followdirsymlinks | flag | off | enables following directory symlinks |
| --out | pathname | none | pathname of file to save output to |
