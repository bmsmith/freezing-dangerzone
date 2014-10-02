#!/bin/bash

echo Starting preprocessor...
cp $1 output.sh
# Substitutions to fix bash++ into bash
sed -i.bak "s/if \[/if \[ /g" output.sh
sed -i.bak "s/\]/ ]/g" output.sh
sed -i.bak "s/ = /=/g" output.sh
sed -i.bak "s/ then/\nthen/g" output.sh
# Repetition compiler

