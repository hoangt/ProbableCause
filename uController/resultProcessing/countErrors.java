import java.io.*;

public class countErrors
{
    public static void main(String[] args)throws IOException
    {
	if(args.length != 2 && args.length != 1)
	{
	    System.err.println("Usage: java countErrors errorMap [currentCountMap]");
	    System.exit(1);
	}
	boolean existingCount = args.length == 2;

	BufferedReader file1 = openFileRead(args[0]);
	BufferedReader file2 = existingCount ? openFileRead(args[1]) : null;

	String line1;
	while((line1 = file1.readLine()) != null)
	{
	    // Ignore lines that don't contain data
	    if(ignoreLine(line1))
	    {
		continue;
	    }

	    // Break the line into individual bytes
	    String[] bytes1 = line1.split(" ");

	    // For each error bit (0) in a byte, increment count
	    for(int b = 0; b < bytes1.length; ++b)
	    {
		int errorValue = Integer.parseInt(bytes1[b], 16);

		// Each bit of the error byte has its own entry in the count file
		for(int bitIndex = 0; bitIndex < 8; ++bitIndex)
		{
		    int currentCount = existingCount ? Integer.parseInt(file2.readLine()) : 0;

		    // If the bit is a 0 (error) increment the counter
		    if((errorValue & (1 << (7 - bitIndex))) == 0)
		    {
			currentCount += 1;
		    }

		    // Print the current count
		    System.out.println(currentCount);
		}
	    }
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