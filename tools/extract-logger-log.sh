#!/bin/bash
filename=$1
while read s; do
  m="at\+send=lora:[0-9A-Fa-f]*:([0-9A-Fa-f]*)[\r]?"
  if [[ $s =~ $m ]]; then
      a+=(${BASH_REMATCH[1]});
  fi
done < $filename

for v in "${a[@]}"
do
     echo -n "$v "
done
