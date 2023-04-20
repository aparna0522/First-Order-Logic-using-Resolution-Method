#!/bin/sh
cp input/input0.txt input.txt;

i=0

while true
do
    i=$((i+1))
    if [ $i -gt 163 ]
    then
        break
    fi
    outputA=$(timeout 80s ./aparna.out);
    outputB=$(cat output/output$((i-1)).txt);
    echo "$((i-1)): Got ${outputA} Expected ${outputB}";
    cp input/input"$i".txt input.txt;
done