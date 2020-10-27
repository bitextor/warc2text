# warc2text
Extracts plain text, language identification and more metadata from WARC records

## Install dependencies
On Debian/Ubuntu/Mint:
```
apt-get install uchardet libuchardet-dev
```
On Mac:
```
brew install uchardet
```

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

Charset detection [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/)
=======

___

![Connecting Europe Facility](https://www.paracrawl.eu/images/logo_en_cef273x39.png)

All documents and software contained in this repository reflect only the authors' view. The Innovation and Networks Executive Agency of the European Union is not responsible for any use that may be made of the information it contains.
