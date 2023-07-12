#!/bin/bash

for i in {1..4}
do
   ./../src/process "script$i.sh" &
done