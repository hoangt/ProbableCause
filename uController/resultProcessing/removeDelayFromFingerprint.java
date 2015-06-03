import java.io.*;
import java.util.ArrayList;

public class removeDelayFromFingerprint
{
    public static void main(String[] args)throws IOException
    {
	if(args.length != 1)
	{
	    System.err.println("Usage: java removeDelayFromFingerPrint file1");
	    System.exit(1);
	}

	BufferedReader file1 = openFileRead(args[0]);

	String line1;
	ArrayList<String> byteList = new ArrayList<String>(2*256*256);

	while((line1 = file1.readLine()) != null)
	{
	    // Ignore lines that don't contain data
	    if(ignoreLine(line1))
	    {
		continue;
	    }

	    // Break the lines into individual bytes
	    String[] bytes = line1.split(" ");

	    for(String item : bytes)
	    {
		byteList.add(item);
	    }
        }

	// Remove the delay
	byteList.remove(0);
	byteList.remove(0);

	// Print garbage header
	System.out.println("@FFFF");

	// Print the results, 16 bytes per line
	int count = 0;
	for(String item: byteList)
	{
	    System.out.print(item);

	    if(count == 15)
	    {
		System.out.print("\n");
		count = 0;
	    }
	    else
	    {
		System.out.print(" ");
		++count;
	    }
	}

	// Print garbage footer
	System.out.print("FF ");
	System.out.println("FF");
	System.out.println("FF");
	System.out.println("q");
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

    static BufferedReader openFileRead(String pFilename)throws IOException
    {
	return new BufferedReader(new FileReader(pFilename));
    }
}