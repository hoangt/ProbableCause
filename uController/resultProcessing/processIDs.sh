#!/bin/sh
scalePath="../end2end"
resPath="../end2end/results"

# Convert the images to a format where an error is represented as a 1
# Remove the errors that are impossible for this DRAM to expose
javac convertImageDecayToFlips.java
for chip in {1..9}; do
    for temp in 40 50 60; do
	for rel in 99 95 90; do
	    java convertImageDecayToFlips "$scalePath"/scale.txt "$resPath"/"$chip"_"$temp"_imagedecay"$rel".txt > "$chip"_"$temp"_imagedecay"$rel"_flips.txt
	done
    done
done

# Create a version of the image where a 0 bit marks locations in the
# image that can fail given the DRAM architecture
javac markPossibleErrorsInFile.java
java markPossibleErrorsInFile "$scalePath"/scale.txt > image_poss_flips.txt

# Create a fingerprint for the image decay experiments that takes into account impossible bit flips
javac intersectFlips.java
for chip in {1..9}; do
    java intersectFlips image_poss_flips.txt "$chip"_fingerprint.txt > "$chip"_id_fp.txt 2> /dev/null
done

# For each image decay result, calculate the distance from the chip's fingeprint
javac hd_fails_from_1.java
for i in {1..9}; do
    find . -name "*_*0_imagedecay9*_flips.txt" -exec java hd_fails_from_1 "$i"_id_fp.txt {} > "$i"_id_distances.txt \;
done

# Collect the image decay distance results into a single file
rm -rf id_distances.txt
find . -name "*_id_distances.txt" -exec cat {} >> id_distances.txt \;
