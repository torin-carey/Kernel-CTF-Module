#!/bin/bash

# Simple bash script to produce a 'flag.h' file containing the flags and the key

genflag() {
	head -c 16 /dev/urandom | xxd -p
}

KEY=`head -c 16 /dev/urandom | xxd -c 8 -i`

FLAG1="flag1{`genflag`}"
FLAG2="flag2{`genflag`}"
FLAG3="flag3{`genflag`}"

cat <<EOF
#ifndef H_FLAG
#define H_FLAG

static unsigned char key[] = {
$KEY
};

static const char flag1[] = "$FLAG1";
static const char flag2[] = "$FLAG2";
static const char flag3[] = "$FLAG3";

#endif // H_FLAG
EOF
