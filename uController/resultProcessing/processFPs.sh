#!/bin/sh
resPath="../end2end/results"

# Remove the delay from the fingerprint results and reflow the data
#javac removeDelayFromFingerprint.java
#for chip in {1..9}; do
#    for temp in 40 50 60; do
#	for vol in 90 95 99; do
#	    java removeDelayFromFingerprint "$resPath"/"$chip"_"$temp"_fingerprint"$vol".txt > "$chip"_"$temp"_fingerprint"$vol"_noDelay.txt
#	done
#    done
#done

# Create a fingerprint for each chip using the 99% results
# After the fingerprint is created, calculate the distance between the fingerprint and every other fingerprint result
javac intersectFlips.java
javac hd_fails_from_1.java
javac hd.java
for vol in 90 95 99; do
    for i in {1..9}; do
	java intersectFlips "$i"_40_fingerprint"$vol"_noDelay.txt "$i"_50_fingerprint"$vol"_noDelay.txt > temp_fp.txt
	java intersectFlips "$i"_60_fingerprint"$vol"_noDelay.txt temp_fp.txt > "$i"_fingerprint_"$vol".txt
	rm -rf temp_fp.txt
	find . -name "*_*0_fingerprint9*_noDelay.txt" -exec java hd_fails_from_1 "$i"_fingerprint_"$vol".txt {} $(java hd "$resPath"/fingerprint_no_errors.txt "$i"_fingerprint_"$vol".txt) > "$i"_fp_distances_"$vol".txt \;
    done
    
    # Collect all distance results togther in the same file
    rm -rf fp_distances_"$vol".txt
    find . -name "*_fp_distances_\"$vol\".txt" -exec cat {} >> fp_distances_"vol".txt \;
done