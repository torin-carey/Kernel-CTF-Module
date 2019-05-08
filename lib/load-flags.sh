#!/bin/bash

DIRNAME=$(dirname "$0")

. "$DIRNAME"/../flags
export FLAG1 FLAG2 FLAG3 KEY

"$DIRNAME"/loader
