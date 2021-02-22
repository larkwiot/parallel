# parallel

My implementation of parallel

Usage

To echo all lines of a file in parallel:
```
cat text.txt | parallel echo {}
parallel -f text.txt echo {}
```