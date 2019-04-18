#!/bin/bash

genflag() {
	head -c 16 /dev/urandom | xxd -p
}

KEY=$(head -c 16 /dev/urandom | xxd -c 16 -p)

FLAG1="flag1{$(genflag)}"
FLAG2="flag2{$(genflag)}"
FLAG3="flag3{$(genflag)}"

SAMPLE=$((echo $KEY | xxd -p -r && echo -n UNLOCK_FLAG2) | sha256sum - | tr -d ' -')

cat <<EOF
FLAG1=$FLAG1
FLAG2=$FLAG2
FLAG3=$FLAG3
KEY=$KEY
SAMPLE=$SAMPLE
EOF
