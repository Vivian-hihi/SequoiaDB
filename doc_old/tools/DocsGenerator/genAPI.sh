#!/bin/sh
doxygen docs/java/latex;
cd docs/java/latex;
make;
mv refman.pdf ../../java-API.pdf.pdf;