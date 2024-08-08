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
git submodule update --init --recursive
```

## Install dependencies
On Debian/Ubuntu/Mint:
```
apt-get install build-essential cmake libuchardet-dev libzip-dev libboost-thread-dev libboost-regex-dev libboost-filesystem-dev libboost-log-dev libboost-iostreams-dev libboost-locale-dev libboost-program-options-dev
```
On Mac:
```
brew install uchardet libzip
```

## Compile
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/your/prefix/path ..
# cmake .. -DCMAKE_BUILD_TYPE=Debug # for debug
# cmake .. -DICU_ROOT_DIR=(brew --prefix icu4c)/lib # for macOS
make -j
make install
```

## Alternative installation with EasyBuild
On a node with EasyBuild installed you can install warc2text as a module:
```
eb --robot easyconfigs/uchardet-0.0.7-foss-2021a.eb 
eb --robot easyconfigs/nlpl-warc2text-1.2.0-foss-2021a.eb
```

## Usage

**note:** for warcs with many languages you might hit the open file limit quite quickly. It is therefore advised to increase it, e.g. `ulimit -n 8192`.

```
warc2text -o <output_folder> [ -f <output_files> ] [ --pdfpass <output_warc> ]
          [ --paragraph-identification ] [ --tag-filters <filters_file> ] <warc_file>...
```
* `--output`/`-o` output folder
* `--files`/`-f` list of output files separated by commas (and without `.gz`); Options are `text`,`html`,`metadata`, `url`,`mime`,`file` and `date`. Defaults to `text,url`. See [output](#output).
* `--jsonl` Produce JSON Lines for `html` and `text` files instead of base64 encoding.
* `--stdout` Write all the information in JSONLines to stdout. Needs --jsonl option.
* `--pdfpass` WARC file where PDF records will be stored
* `--robotstxtpass` WARC file where robots.txt related records will be stored
* `--encode-urls` Escape non-ascii characters that appear in the record URL with `%dd` encoding.
* `--multilang` Detect multiple languages in the document, and split the document accordingly. Only supported with CLD2 classifier.
* `--paragraph-identification` print the paragraph identifier for each sentence extracted from the HTML
* `--classifier` classifier to use: `cld2`, `fasttext`, or `skip`. When `fasttext` is used, one also has to specify a model using `--fasttext-model`. Use `skip` to skip language identification entirely.
* `--fasttext-model` path to FastText model for fasttext classifier. Models can be any [FastText language identification model](https://fasttext.cc/docs/en/language-identification.html) such as [OpenLID lid201-model.ftz](https://github.com/laurieburchell/open-lid-dataset#quantised-model)
* `--skip-text-extraction` Skip text extraction and output only html. This option is not compatible with "text" value in -f option and also requires to skip language identification.
* `--tag-filters` file containing filters that are used to eliminate matching documents
* `--invert-tag-filters` output only documents that match the filter
* `--url-filters` file containing regular expressions that match urls of documents to eliminate
* `--verbose`/`-v` print progress and filtering information
* `--silent`/`-s` print only warnings and errors

  Tag Filter format is the following: `tag <tab> attribute <tab> regexp`
  
  For example, `meta <tab> name <tab> translation-stats` will remove documents that contain `<meta name="translation-stats" ... >`

  URL Filter format is a single regular expression per line.

  Lines beginning with `#` and empty lines are ignored. Any invalid filter will raise a warning message, but will not prevent other filters from being read.

## Output
When used with `--output`/`-o` (with optionally `--files`/`-f`), warc2text will
produce the following directory structure at the path specified by `--output`:

- `./{lang}/text.gz` will contain the plain text per document as base64 encoded lines. E.g. `gzip -cd en/text.gz | head -n5 | tail -n1 | base64 -d` will give you the 5th document's text.
- `./{lang}/url.gz` contains [the crawled URL](https://iipc.github.io/warc-specifications/specifications/warc-format/warc-1.1/#warc-target-uri) for each record.
- `./{lang}/mime.gz` contains the mimetype as reported by the crawled server
- `./{lang}/html.gz` contains lines of base64 encoded HTML as returned by the server. For ePub, MS Office or ODF files this is the extracted XML.
- `./{lang}/file.gz` contains the `{filename}:{offset}:{length}` pointer to the warc archive the record was extracted from. `{offset}` and `{length}` are of the compressed data, e.g. `tail -c+{offset} < {filename} | head -c{length} | gzip -cd` will give you the original record.
- `./{lang}/date.gz` gives you the original crawl date/time as reported by the crawler. [This should be a UTC timestamp](https://iipc.github.io/warc-specifications/specifications/warc-format/warc-1.1/#warc-date-mandatory).
- `./{lang}/metadata.gz` contains the metadata as explained in the [JSONL section](#jsonl) below. Note that this output file will already contain some of the information described above, so `mime`, `url`, `file` and `date` are not needed when using `metadata`. Also note that this option will write **only** the metadata, so it will not include plain text or HTML.

In every file, each line corresponds to the same record. E.g. the fifth line in `text.gz` and fifth line in `url.gz` together give you the text and url for a single record.

The `{lang}` part of the path is determined by the classifier (see `--classifier`) and may be a two-letter or three-letter code depending on the classifier used. See [this list](https://github.com/CLD2Owners/cld2/blob/b56fa78a2fe44ac2851bae5bf4f4693a0644da7b/internal/generated_language.cc#L647-L1262) for CLD2. When skipping the language identification with `--classifier skip`, all the files will be written directly to output folder without creating language specific folders.

When using `--compression zstd` files suffix will be `.zst` instead of `gz`.

### JSONL
When using `--jsonl`, the output files that previously were encoded in base64, are now written in a single JSON record per line.
With the keys `"h"` and `"p"` for the `html` file and the `text` file respectively.

#### stdout
Instead of the classic Bitextor directory structure and files, the `--jsonl` option can be combined with `--stdout` to write all the output to stdout, with the following keys (always in this order):
```ts
{
  f:  string, # filename of warc file (same as the `{filename}` part in `file.gz`)
  o:  number, # byte offset of record in warc file (same as `{offset}` in `file.gz`)
  s:  number, # warc file record size (same as `{size}` in `file.gz`)
  rs: number, # byte size of record payload (uncompressed)
  ps: number, # byte size of text only payload (so compare this against `rs` and you should get amount of HTML removed)
  l:  string, # identified language by classifier, omitted when language identification is skipped
  u:  string, # url
  c:  string, # content type as reported by the HTTP response header (or warc record header if that isn't present)
  ts: string, # crawl date/time as reported by the crawler
  p:  string, # plain text
}
```

More keys might be added in the future (e.g. the raw HTML is not included in JSONL to stdout now) and you should not expect the order of the keys to stay the same between different versions of warc2text.

## Included dependencies
HTML Tokenizer by [c-smile](https://www.codeproject.com/Articles/14076/Fast-and-Compact-HTML-XML-Scanner-Tokenizer)

HTML entities decoder by [Christoph Gärtner](https://bitbucket.org/cggaertner/cstuff/src/master/entities.c)

Charset detection using [uchardet](https://www.freedesktop.org/wiki/Software/uchardet/)

Zip support for open document format using [libzip](https://libzip.org)
___

![Connecting Europe Facility](https://www.paracrawl.eu/images/logo_en_cef273x39.png)

All documents and software contained in this repository reflect only the authors' view. The Innovation and Networks Executive Agency of the European Union is not responsible for any use that may be made of the information it contains.
