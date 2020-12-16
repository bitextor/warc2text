# warc2text
Extracts plain text, language identification and more metadata from WARC records

## Download
Clone this repo along with submodules:
```
git clone --recurse-submodules https://github.com/bitextor/warc2text.git
```
Or:
```
git clone https://github.com/bitextor/warc2text.git
git submodules update --init
```

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
* `--output`/`-o` output folder

* `--files`/`-f` list of output files separated by commas (and without `.gz`); `text` and `url` are always written, while `mime` and `html` are optional
* `--pdfpass` WARC file where PDF records will be stored
* `--tag-filters` file containig filters that to eliminate some documents
  
  Filter format is the following: `tag <tab> attribute <tab> value ...`
  
  For example, `meta <tab> name <tab> translation-stats` will remove documents that contain `<meta name="translation-stats" ... >`


## Broader document formats
For a broader document format processing (.docx, .pptx, .odt, .epub, .pdf, etc.) you can use [warc2htmlwarc](https://raw.githubusercontent.com/bitextor/bitextor/snake_performance/bitextor-warc2htmlwarc.py) script with `--only-broader` argument to only output XHTML versions of these special WARC record payloads:

`warc2text -o output <(cat input.warc.gz <(python3 bitextor-warc2htmlwarc.py -i input.warc.gz --only-broader))`

## Included dependencies
HTML Tokenizer by [c-smile](https://www.codeproject.com/Articles/14076/Fast-and-Compact-HTML-XML-Scanner-Tokenizer)

HTML entities decoder by [Christoph Gärtner](https://bitbucket.org/cggaertner/cstuff/src/master/entities.c)

Charset detection [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/)

___

![Connecting Europe Facility](https://www.paracrawl.eu/images/logo_en_cef273x39.png)

All documents and software contained in this repository reflect only the authors' view. The Innovation and Networks Executive Agency of the European Union is not responsible for any use that may be made of the information it contains.
