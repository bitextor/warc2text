# warc2text
Extracts plain text, language identification and more metadata from WARC records


## Compile
```
mkdir build
cd build
cmake ..
# cmake .. -DCMAKE_BUILD_TYPE=Debug # for debug
make
```
## Usage
```
warc2text -o [output folder] [ WARC ... ]
```

## Included dependencies
HTML Tokenizer by [c-smile](https://www.codeproject.com/Articles/14076/Fast-and-Compact-HTML-XML-Scanner-Tokenizer)

HTML entities decoder by [Christoph Gärtner](https://bitbucket.org/cggaertner/cstuff/src/master/entities.c)
