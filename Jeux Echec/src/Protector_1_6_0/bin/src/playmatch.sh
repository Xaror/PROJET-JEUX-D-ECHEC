#!/bin/bash

#OPPONENT='gnuchess'
#OPPONENT='polyglot fruit.ini'
OPPONENT='polyglot toga2.ini'
#OPPONENT='crafty'

if [ -f ./enginematch.pgn ]; then
   rm enginematch.pgn
fi

if [ -f ./result.txt ]; then
   rm result.txt
fi

for i in `seq 1 100`;
do
   ./match.sh pgn/all.pgn $i "$OPPONENT" >> result.txt 2>&1
   ./resultcount result.txt
done
