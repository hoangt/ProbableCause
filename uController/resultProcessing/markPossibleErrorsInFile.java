// Takes in an image file and outputs a file where possible
// errors given DRAM architecture and the input image file
// are marked as 0 bits
import java.io.*;
import java.util.ArrayList;

public class markPossibleErrorsInFile
{
    public static void main(String[] args)throws IOException
    {
	if(args.length != 1)
	{
	    System.err.println("Usage: java markPossibleErrorsInFile file1");
	    System.exit(1);
	}

	BufferedReader file1 = openFileRead(args[0]);

	String line1;
	int row = 0;
	int col = 0;

	while((line1 = file1.readLine()) != null)
	{
	    // Ignore lines that don't contain data
	    if(ignoreLine(line1))
	    {
		System.out.println(line1);
		continue;
	    }

	    // Break the lines into individual bytes
	    String[] bytes = line1.split(" ");

	    for(int byteCount = 0; byteCount < bytes.length; ++byteCount)
	    {
		col += 2;

		// odd rows fail to 1
		int value = Integer.parseInt(bytes[byteCount], 16);
		value = (row & 0x1) == 0 ? ~value : value;
		value &= 0xFF;

		System.out.format("%02X", value);
		
		if(byteCount == (bytes.length - 1))
		{
		    System.out.print("\n");
		}
		else
		{
		    System.out.print(" ");
		}

		if(col == 256)
		{
		    ++row;
		    col = 0;
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

    static BufferedReader openFileRead(String pFilename)throws IOException
    {
	return new BufferedReader(new FileReader(pFilename));
    }
}