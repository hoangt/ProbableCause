import java.io.*;

public class FlashDumpToAggregate
{
    private static final int MAX_INCREMENTS = 256;

    public static void main(String args[])throws Exception
    {
	// Print an error message if any parameters are missing
	if(args.length != 3)
	{
	    System.out.println("Usage: java FlashDumpToAggregate flashDump incrementInSecs binSize");
	    System.exit(-1);
	}

	// Parse arguments
	String fileToParse = args[0];
	int increment = Integer.parseInt(args[1]);
	int numResultsPerBin = Integer.parseInt(args[2]);
	
	int resultArray[] = new int[MAX_INCREMENTS];
	int resultCount = 0;

	// Initialize all results to 0
	for(int result = 0; result < MAX_INCREMENTS; ++result)
	{
	    resultArray[result] = 0;
	}

	BufferedReader inFile = new BufferedReader(new FileReader(fileToParse));
	String line;
	
	// Read bytes seperated by spaces in the input file
	// Convert each byte to an integer and use that integer to index the result array
	// Increment the appropriate result
	while((line = inFile.readLine()) != null)
	{
	    if(line.startsWith("@") || line.equals("q"))
		continue;
	    String parts[] = line.split(" ");
	    
	    for(String part : parts)
	    {
		++resultArray[Integer.parseInt(part, 16)];
		++resultCount;
	    }
	}
	inFile.close();

	// Process the results, printing aggregate results to the terminal
	int totalBitsValid = resultCount;
	for(int result = 0; result < MAX_INCREMENTS; result = result + numResultsPerBin)
	{
	    int bitsDecayedThisBin = 0;
	    
	    // Merge together results into bins
	    for(int subResult = result; subResult < (result + numResultsPerBin) && subResult < resultArray.length; ++subResult)
	    {
		bitsDecayedThisBin += resultArray[subResult];
	    }

	    double percentDecayThisBin = bitsDecayedThisBin/(double)resultCount;
	    totalBitsValid -= bitsDecayedThisBin;
	    double percentTotalBitsValid = totalBitsValid/(double)resultCount;

	    // Seconds idle for this bin, numer of bits decayed in this bin, percent decay of this bin, total bits remaining valid, percent total bits valid
	    System.out.println("" + result*increment + " " + bitsDecayedThisBin + " " + percentDecayThisBin + " " + totalBitsValid + " " + percentTotalBitsValid);
	}
    }
}
