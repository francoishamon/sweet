#! /bin/bash

source config.sh


echo "*** OpenMPI ***"
if [ ! -e "$DST_DIR/bin/mpicc"  -o "$1" != "" ]; then
	SRC_LINK="http://www.martin-schreiber.info/pub/sweet_local_software/openmpi-1.10.2.tar.bz2"
	#SRC_LINK="https://www.open-mpi.org/software/ompi/v1.10/downloads/openmpi-1.10.2.tar.bz2"
	FILENAME="`basename $SRC_LINK`"
	BASENAME="openmpi-1.10.2"

	cd "$SRC_DIR"
	if [ ! -e "$FILENAME" ]; then
		echo "Downloading file $FILENAME"
		curl "$SRC_LINK" -o "$FILENAME" || exit 1
	else
		echo "Using existing file $FILENAME"
	fi

	echo "Uncompressing $FILENAME"
	tar xjf "$FILENAME"

	if [ ! -e "$BASENAME" ]; then
		echo "$BASENAME does not exist"
		exit 1
	fi


	##############################
	##############################

	cd "$BASENAME"
	
	export CC=gcc
	export CXX=g++
	export LINK=ld
	./configure --prefix="$DST_DIR" || exit 1

	make || exit 1
	make install || exit 1

	echo "DONE"

else
	echo "OpenMPI already installed"
fi
