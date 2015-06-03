#!/bin/sh
resPath="../end2end/results"

# Create a fingerprint for each chip using the 99% results
# After the fingerprint is created, calculate the distance between the fingerprint and every other fingerprint result
javac intersectFlips.java
javac hd_fails_from_1.java
javac hd.java
for temp in 40 50 60; do
    for i in {1..9}; do
	java intersectFlips "$i"_"$temp"_fingerprint90_noDelay.txt "$i"_"$temp"_fingerprint95_noDelay.txt > temp_fp.txt
	java intersectFlips "$i"_"$temp"_fingerprint99_noDelay.txt temp_fp.txt > "$i"_fingerprint_"$temp".txt
	rm -rf temp_fp.txt
	find . -name "*_$(echo $temp)_fingerprint9*_noDelay.txt" -exec java hd_fails_from_1 "$i"_fingerprint_"$temp".txt {} $(java hd "$resPath"/fingerprint_no_errors.txt "$i"_fingerprint_"$temp".txt) > "$i"_fp_distances_"$temp".txt \;
    done
    
    # Collect all distance results togther in the same file
    rm -rf fp_distances_"$temp".txt
    find . -name "*_fp_distances_$(echo $temp).txt" -exec cat {} >> fp_distances_"$temp".txt \;
done
