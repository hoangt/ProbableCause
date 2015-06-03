import java.io.*;

public class addErrorsToImage
{
    public static void main(String[] args)throws IOException
    {
	if(args.length != 2)
	{
	    System.err.println("Usage: java addErrorsToImage errorMap exactImage");
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
		System.err.println("" + args[args.length - 2]);
		System.err.println("" + args[args.length - 1]);
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
		// For any 0 bits (error bits) in the error map, flip the bit from the exact image
		int newByte = (~Integer.parseInt(bytes1[b], 16) ^ Integer.parseInt(bytes2[b], 16)) & 0xFF;

		System.out.format("%02X", newByte);

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

    static int byteOR(String pA, String pB)
    {
	return Integer.parseInt(pA, 16) | Integer.parseInt(pB, 16);
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