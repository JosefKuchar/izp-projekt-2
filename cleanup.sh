#!/bin/sh
# Remove #pragma (end)region lines
awk '!/^#pragma (end)?region.*$/' setcal.c > setcal.final.c
