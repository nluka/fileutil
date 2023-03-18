# fileutil

```
usage:
  fileutil repeat|sizerank help
  fileutil repeat|sizerank [options...]
```

## functions
- [repeat](#repeat): repeat the contents of a file
- [sizerank](#sizerank): rank files by size

## repeat

Repeats a file's content N times, writing it as a new file.

```
REPEAT OPTIONS:
  -i [ --inpath ] arg
      Path of file to repeat
  -o [ --outpath ] arg
      Path of resultant file
  -n [ --repeats ] arg
      Number of times to duplicate content, 1 = copy
```

## sizerank

Ranks top N largest files in directory/subdirectories.

```
SIZERANK OPTIONS:
  -d [ --dir ] arg
      Search directory, default=cwd
  -r [ --recurse ]
      Enables recursive search through child directories, default=false
  -n [ --top ] arg
      Number of top entries to rank, default=10
  -s [ --sizelim ] arg
      File size limits to consider, inclusive, format: min,max
  -p [ --pattern ] arg
      Regular expression to match file names against, default=.*
  -l [ --followsymlinks ]
      Enables following symbolic links, default=false
  -o [ --outpath ] arg
      Path of output file, default=none
```
