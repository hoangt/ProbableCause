import java.io.*;
import java.util.*;

public class ParseResults
{
    private static final boolean DECIMATE = true;
    private static final int DEC_AMOUNT = 100;

    public static void main(String[] args)throws IOException
    {
	if(args.length != 2)
	{
	    System.out.println("Usage: java ParseResults filename.ext numberOfDataColumns");
	    System.out.println("To toggle decimation and the number of results to combine into a single result, edit the constants at the top of ParseResults.java.");
	    System.exit(-1);
	}
	int numberOfColumns = 0;
	try
	{
	    numberOfColumns = Integer.parseInt(args[1]);
	}
	catch(NumberFormatException nfe)
	{
	    System.out.println("Usage: java ParseResults filename.ext numberOfDataColumns");
	    System.out.println("To toggle decimation and the number of results to combine into a single result, edit the constants at the top of ParseResults.java.");
	    System.exit(-1);
	}

	ArrayList[] results = new ArrayList[numberOfColumns];
	for(int i = 0; i < results.length; ++i)
	    results[i] = new ArrayList<Float>(1000);

	BufferedReader in = new BufferedReader(new FileReader(args[0]));
	
	int resultCount = 0;
	String line = in.readLine();
	while(line != null)
	{
	    String[] parts = line.split(",");

	    // Ignore lines that do not contain data
	    if(parts.length == numberOfColumns)
	    {
		try{
		// Convert the data to a float and add it to the correct result list
		int part = 0;
		for(ArrayList<Float> al : results)
		{
		    al.add(new Float(parts[part]));
		    ++part;
		}
		++ resultCount;

		// See if the list needs to be condensed through decimation
		if(DECIMATE && (resultCount % DEC_AMOUNT) == 0)
		{
		    for(ArrayList<Float> al : results)
		    {
			float avg = 0.0f;
		        // Sum the N most recently added results
			// Remove those N elements from the result list
			for(int i = 0; i < DEC_AMOUNT; ++i)
			{
			    avg += al.remove(al.size() - 1).floatValue();
			}
			// Average the results and add that average back to the result list
			avg = avg / DEC_AMOUNT;
			al.add(new Float(avg));
		    }
		}
		} catch(NumberFormatException nfe)
		{;}
	    }

	    line = in.readLine();
	}

	in.close();

	int count = 0;
	float total = 0f;
	for(int i = 0; i < results[0].size(); ++i)
	{
	    System.out.println(results[1].get(i) + "," + results[3].get(i));
	    if(((Float)results[3].get(i)).floatValue() > 1.0f)
	    {
		++count;
		total += ((Float)results[1].get(i)).floatValue();
	    }
	}
	System.err.println("Avg sense resistor V: " + total/count);
	System.err.println("Avg sense resistor I: " + (total/count)/200);
    }
}
