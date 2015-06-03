import java.io.*;

public class hd
{
    public static void main(String[] args)throws IOException
    {
	if(args.length != 2 && args.length != 3)
	{
	    System.err.println("Usage: java hd [-v] file1 file2");
	    System.exit(1);
	}

	boolean VERBOSE = false;
	if(args.length == 3)
	    VERBOSE = true;

	int totalBytes = 0;
	int totalFlips = 0;

	BufferedReader file1 = openFileRead(args[args.length - 2]);
	BufferedReader file2 = openFileRead(args[args.length - 1]);

	String line1;
	String line2;

	while((line1 = file1.readLine()) != (line2 = file2.readLine()))
	{
	    // Check for unequal length files
	    if(line1 == null || line2 == null)
	    {
		System.err.println("ERROR: Files not the same length");
		break;
	    }

	    // Ignore lines that don't contain data
	    if(ignoreLine(line1) || ignoreLine(line2))
	    {
		if(ignoreLine(line2) && ignoreLine(line1))
		{
		    continue;
		}

		System.err.println("ERROR: Files do not start or end the same");
		break;
	    }

	    // Break the lines into individual bytes
	    String[] bytes1 = line1.split(" ");
	    String[] bytes2 = line2.split(" ");

	    // Check that the lines contain the same amount of data
	    if(bytes1.length != bytes2.length)
	    {
		System.err.println("ERROR: Files do not have the same number of bytes per line");
		System.exit(1);
	    }

	    totalBytes += bytes1.length;
	    // Diff each byte against that same byte from the other file
	    for(int b = 0; b < bytes1.length; ++b)
	    {
		int diff = byteDiff(bytes1[b], bytes2[b]);

		if(diff != 0)
		    totalFlips += countOnes(diff);
	    }
        }

        double totalBits = totalBytes * 8;
	int totalValid = (int)totalBits - totalFlips;
	
	if(VERBOSE)
	{
	    System.out.println("Total bytes: " + totalBytes);
	    System.out.println("Total bits: " + (int)totalBits);
	    System.out.println("Total flips: " + totalFlips);
	    System.out.format("Percent flips: %3.3f\n", ((totalFlips / totalBits) * 100));
	    System.out.println("Total valid: " + totalValid);
	    System.out.format("Percent valid: %3.3f\n", ((totalValid / totalBits) * 100));
	}
	else
	{
	    System.out.println("" + totalFlips);
	    //	    System.out.format("%3.3f\n", ((totalValid / totalBits) * 100));
	}
    }

    static boolean ignoreLine(String pLine)
    {
	// Modified length check because last line varries by is small and not valid data
	if(pLine.startsWith("@") || pLine.trim().length() < 5 || pLine.startsWith("q"))
	{
	    return true;
	}

	return false;
    }

    static int byteDiff(String pA, String pB)
    {
	return Integer.parseInt(pA, 16) ^ Integer.parseInt(pB, 16);
    }

    static int countOnes(int pValue)
    {
	int oneCount = 0;

	while(pValue != 0)
	{
	    if((pValue & 0x1) != 0)
	    {
		++oneCount;
	    }

	    pValue = pValue >> 1;
	}

	return oneCount;
    }

    static BufferedReader openFileRead(String pFilename)throws IOException
    {
	return new BufferedReader(new FileReader(pFilename));
    }
}