#!/bin/bash
#
# Script extracts data sent by Lora modem from logger log file
#
# Log file:
#
# Файл данных и пакетов в логгера
# Время начала регистрации: 22.02.2022 15:11:39
# at+set_config=device:sleep:0
# at+join
# at+send=lora:74:4A00280002031C140038100F160216000000003981190002
# ...
#
# Usage:
# ./extract-logger-log.sh 1.txt
#  4A00280002031C140038100F160216000000003981190002 4B1C02020006CFAA0101A8000201A8000301A9000401A900 ..
fn=$1
m="at\+send=lora:[0-9A-Fa-f]*:([0-9A-Fa-f]*)[\r]?"
while read s; do
  if [[ $s =~ $m ]]; then
      a+=(${BASH_REMATCH[1]});
  fi
done < $fn

for v in "${a[@]}"
do
     echo -n "$v "
done
echo
