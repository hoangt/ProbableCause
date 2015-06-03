#!/bin/sh
resPath="../end2end/results"

# Remove the delay from the fingerprint results and reflow the data
javac removeDelayFromFingerprint.java
for res in {0..19}; do
    java removeDelayFromFingerprint "$resPath"/9_"$res"_fingerprint99.txt > 9_40_fingerprint99_noDelay_"$res".txt
done

javac countErrors.java
java countErrors 9_40_fingerprint99_noDelay.txt > 9_40_fingerprint99_conCount.txt
for res in {0..19}; do
    java countErrors 9_40_fingerprint99_noDelay_"$res".txt 9_40_fingerprint99_conCount.txt > 9_40_fingerprint99_conCount_temp.txt
    mv 9_40_fingerprint99_conCount_temp.txt 9_40_fingerprint99_conCount.txt
done

sort 9_40_fingerprint99_conCount.txt | uniq -c > 9_40_fingerprint99_conCount_counts.txt
