#!/bin/bash

#
# This script minify and compress all JS, HTML and CSS files using gzip format.
# It also generates the corresponding constants that is added to the "./builds/DATA.h" file for CatchME "data.h".
# @Author Herwono W. Wijaya < @HerwonoWr >
#
# Original work by: @Author Erick B. Tedeschi < erickbt86 [at] gmail [dot] com >
#

outputfile="$(pwd)/builds/DATA.h"

rm $outputfile

function minify_html_css {
    file=$1
    curl -X POST -s --data-urlencode "input@$file" http://html-minifier.com/raw > /tmp/converter_min.temp
    cat /tmp/converter_min.temp | tr -d "\n" | gzip > /tmp/converter.temp
}

function minify_js {
    file=$1
    curl -X POST -s --data-urlencode "input@$file" https://javascript-minifier.com/raw > /tmp/converter_min.temp
    cat /tmp/converter_min.temp | tr -d "\n" | gzip > /tmp/converter.temp
}

function ascii2hexCstyle {
    file_name=$(constFileName $1)
    result=$(cat /tmp/converter.temp | hexdump -ve '1/1 "0x%.2x,"')
    result=$(echo $result | sed 's/,$//')
    echo "const char data_${file_name}[] PROGMEM = {$result};"
}

function constFileName {
   extension=$(echo $1 | egrep -io "(css|js|html)$" | tr "[:lower:]" "[:upper:]")
   file=$(echo $1 | sed 's/\.\///' | sed 's/js\///' | cut -d '.' -f 1)
   echo ${file}_${extension}
}


cd ../CatchME_UI/
file_list=$(find . -type f)

for file in $file_list; do
  echo "Processing: $file"
  if [[ "$file" == *.js ]]; then
    echo "-> JS minifier"
    minify_js $file
    ascii2hexCstyle $file >> $outputfile
  elif [[ "$file" == *.html ]] || [[ "$file" == *.css ]]; then
    echo "-> HTML and CSS minifier"
    minify_html_css $file
    ascii2hexCstyle $file >> $outputfile
  elif [[ "$file" == *.json ]]; then
    echo "-> Skipping json sample file"
  else
    echo "-> Skipping " $file
  fi
  sleep 1
done
