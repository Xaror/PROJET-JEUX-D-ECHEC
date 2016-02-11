#!/bin/bash

# example: ./match.sh all.pgn 3 gnuchess
xboard -size medium -xexit -thinking -tc 2 -inc 1 -mg 2 -lgf $1 -lgi $2 -fd '.' -fcp 'polyglot protector.ini' -sd '.' -scp "$3" -sgf enginematch.pgn
