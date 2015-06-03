import java.io.*;
import java.util.ArrayList;

public class binify
{
    private static final int MAX_DISTANCE = 256*256*4/10;

    public static void main(String[] args)throws IOException
    {
	if(args.length != 2)
	{
	    System.err.println("Usage: java binify file binSize");
	    System.exit(1);
	}

	BufferedReader file = openFileRead(args[0]);
	double binSize = Double.parseDouble(args[1]);

	int[] bins = new int[(int)(1.0/binSize) + 1];
	double max = -1;
	double min = 999999;
	String line;
	while((line = file.readLine()) != null)
	{
	    double value = Double.parseDouble(line);

	    ++bins[(int)(value/binSize)];

	    if(value > max)
		max = value;

	    if(value < min)
		min = value;
	}

	System.err.println("Max = " + max);
	System.err.println("Min = " + min);
	System.err.println("Bin Size = " + binSize);

	for(int index = 0; index < bins.length; ++index)
	{
	    //if(bins[index] != 0)
	    System.out.format("%.4f\t%d\n", (index * binSize), bins[index]);
	}
    }

    static BufferedReader openFileRead(String pFilename)throws IOException
    {
	return new BufferedReader(new FileReader(pFilename));
    }
}