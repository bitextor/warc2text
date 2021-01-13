#!/usr/bin/env bash
# Script to help with figuring out what your --tagfilters filters are doing.
#
# Usage: debug-filters.sh tagfilters.txt *.warc.gz
# This will process the warcs with the tag filters, and save all *filtered*
# documents as html in the current folder. Each HTML file will have a header
# with the URL and the filters that caught it.
#
# Note: This script needs at least Bash 4.0 (Apple users beware!)
#
set -euo pipefail

FILTERLIST=$1
shift

OUTPUT=$(mktemp -d ./textXXXX)

${WARC2TEXT:-warc2text} \
	-o $OUTPUT \
	-f html,url \
	--verbose \
	--invert-tag-filters \
	--tag-filters $FILTERLIST \
	$@ \
	>&${OUTPUT}/log.txt

##
# Parse the logs to figure out which urls got filtered for what reason
##

declare -A MATCHES

URL_PATTERN="Processing HTML document (.+)$"
FILTER_PATTERN="Tag filter ([a-z]+\[.+\]) matched"
NEWLINE=$'\n'

while read line; do
	if [[ $line =~ $URL_PATTERN ]]; then
		URL=${BASH_REMATCH[1]}
	elif [[ $line =~ $FILTER_PATTERN ]]; then
		MATCHES[$URL]+="${BASH_REMATCH[1]}$NEWLINE"
	fi
done < $OUTPUT/log.txt

# for URL in "${!MATCHES[@]}"; do
# 	echo "URL: $URL"
# 	echo "FILTERS: ${MATCHES[$URL]}"
# done

##
# Extract the html and url files for each language into separate html files.
##

N=0

for LANG_PATH in $OUTPUT/*/; do
	paste -d$'\t' \
		<(gzip -cd < $LANG_PATH/url.gz) \
		<(gzip -cd < $LANG_PATH/html.gz) \
	| while IFS=$'\t' read URL HTML_ENCODED; do
		N=$((N + 1))
		FILE=$(printf "%06d.html" $N)
		printf "<!--\nLANG: %s\nURL: %s\nFILTERS: %s-->\n" $(basename "$LANG_PATH") "$URL" "${MATCHES[$URL]:-}" > $FILE
		base64 -d <<< "$HTML_ENCODED" >> $FILE
	done
done
