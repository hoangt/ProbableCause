import java.io.*;

public class intersectValid
{
    public static void main(String[] args)throws IOException
    {
	if(args.length != 2 && args.length != 3)
	{
	    System.err.println("Usage: java intersectValid file1 file2");
	    System.exit(1);
	}

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
		System.exit(1);
	    }

	    // Ignore lines that don't contain data
	    if(ignoreLine(line1) || ignoreLine(line2))
	    {
		if(ignoreLine(line2) && ignoreLine(line1))
		{
		    System.out.println(line1);
		    continue;
		}

		System.err.println("ERROR: Files do not start or end the same");
		System.exit(1);
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

	    // Diff each byte against that same byte from the other file
	    for(int b = 0; b < bytes1.length; ++b)
	    {
		// Set any bits that differ in the two bytes to 0
		int intersect = byteAND(bytes1[b], bytes2[b]);

		System.out.format("%02X", intersect);

		if(b != (bytes1.length - 1))
		    System.out.print(" ");
	    }

	    System.out.print("\n");
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

    static int byteAND(String pA, String pB)
    {
	return Integer.parseInt(pA, 16) & Integer.parseInt(pB, 16);
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