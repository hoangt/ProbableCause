import java.io.*;
import java.util.ArrayList;

public class prepareHeatmap
{
    private static final int RESULTS_PER_ROW = 256*4;

    public static void main(String[] args)throws IOException
    {
	if(args.length != 1)
	{
	    System.err.println("Usage: java prepareHeatmap file");
	    System.exit(1);
	}

	BufferedReader file = openFileRead(args[0]);

	String line;
	int resCount = 0;
	while((line = file.readLine()) != null)
	{
	    int value = Integer.parseInt(line);

	    if(resCount == (RESULTS_PER_ROW - 1))
	    {
		System.out.print("" + value + "\n");
		resCount = 0;
	    }
	    else
	    {
		System.out.print("" + value + " ");
		++resCount;
	    }
	}
    }

    static BufferedReader openFileRead(String pFilename)throws IOException
    {
	return new BufferedReader(new FileReader(pFilename));
    }
}