#!/bin/bash

# Group 7
# Michael Pittard
# Bradford Smith
# Paul Yapobi
# Jonathan Morgan

# Preprocessor...
cp $1 output.sh
# Substitutions to fix bash++ into bash
sed -i "s/if \[/if \[ /g" output.sh
sed -i "s/\]/ ]/g" output.sh
sed -i "s/ = /=/g" output.sh
sed -i "s/ then/\nthen/g" output.sh
# Repetition compiler

 
sed -i "s|repeat \(\S*\) times|repeat_var=\1\nwhile \[ itr -lt \$repeat_var \]\ndo\n\$itr=\$\[\$itr-1\]|g" output.sh
sed -i "s/^}$/done/g" output.sh
sed -i "s/^{$//g" output.sh
