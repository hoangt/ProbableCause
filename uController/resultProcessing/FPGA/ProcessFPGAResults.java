import java.util.*;
import java.io.*;

public class ProcessFPGAResults
{
    // 10 bits in column address, and each cell is a half-word
    // 1024 / 2 = 512
    private static final int NUM_COLS_PER_ROW = 512;
    
	public static void main(String args[])throws Exception
	{
		if(args.length != 1)
		{
			System.err.println("Usage: java ProcessFPGAResults file");
			System.exit(1);
		}
        
		BufferedReader file = new BufferedReader(new FileReader(args[0]));
        
		String line;
		String currentTime = null;
		HashMap<String,String> resultsMap = new HashMap<String,String>(0x40000);
		while((line = file.readLine()) != null)
		{
			if(line.length() == 0)
				continue;
            
			if(line.endsWith(".000") && line.contains(":"))
			{
				currentTime = "" + (Integer.parseInt(line.substring(0, line.indexOf(':')))*60 + Integer.parseInt(line.substring(line.indexOf(':')+1, line.indexOf('.'))));
			}
			else if(line.length() == 12 & line.startsWith("0x0") && (line.endsWith(" 0") || line.endsWith(" 1")))
			{
				if(!resultsMap.containsKey(line))
				{
                    try
                    {
                        Long.parseLong(line.substring(2, line.indexOf(' ')), 16);
                        resultsMap.put(line, currentTime);
                    }
                    catch(Exception e)
                    {
                        System.err.println("Ignoring line: " + line);
                    }
				}
			}
			else
			{
				System.err.println("Ignoring line: " + line);
			}
		}
        
		System.err.println("\nLast Time: " + currentTime + " seconds");
		long max = 0x0;
		long min = 0x100000000l;
		/*for(String address : resultsMap.keySet())
		{
			long current = Long.parseLong(address.substring(2, address.indexOf(' ')), 16);
            
			if(current > max)
				max = current;
            
			if(current < min)
				min = current;
		}*/
        
        // Hardcode the address range to make sure we don't miss non-decayed cells
        min = 0x40000;
        max = 0x7FFFC;
        
		System.err.format("Min: %X\n", min);
		System.err.format("Max: %X\n", max);
		System.err.format("Diff: %X\n", (max - min) + 4);
		int numDRAMWords = (int)((max - min) >> 1); // go from bytes to half-words
		int numResultsMax = numDRAMWords + 2;
        int maxDelay = 0;
        int minDelay = 0x7FFFFFFF;
		System.err.println("Array Size: " + numResultsMax);
        
		// Order the results by their address
		// 0 means no result yet for that address
		int[] resultsArray = new int[numResultsMax];
		for(int index = 0; index < resultsArray.length; ++index)
			resultsArray[index] = 0;
        
        long lastAddress = 0;
		for(String address : resultsMap.keySet())
		{
            // Get the half-word address offsett from the starting address
			long current = (Long.parseLong(address.substring(2, address.indexOf(' ')), 16) - min) >> 1;
            
            if(current >= numResultsMax || current < 0)
                continue;
            
            // Add in which half-word we are addressing
			current += address.endsWith("0") ? 0 : 1;
            
            // Save the result in order
			resultsArray[(int)current] = Integer.parseInt(resultsMap.get(address));
            
            if(resultsArray[(int)current] > maxDelay)
                maxDelay = resultsArray[(int)current];
            if(resultsArray[(int)current] < minDelay)
                minDelay = resultsArray[(int)current];
            
            if(lastAddress > current)
                System.err.println("Uh oh!");
            
		}
        
		// Print the results in a 256 column matrix, skipping 0 results
		int printIndex = 0;
		int resultsPrinted = 0;
        int uCount = 0;
		for(int index = 0; index < resultsArray.length; ++index)
		{
            // Skip second 8 results (Report in 16 byte chunks)
            // This may have to do with the burst size, who knows
            if(uCount >> 3 == 0)
            {
                int result = resultsArray[index];
                
                if(result != 0)
                    ++printIndex;
                else
                    result = maxDelay + 5;
                
                if((resultsPrinted + 1) % NUM_COLS_PER_ROW == 0)
                    System.out.print(result + "\n");
                else
                    System.out.print(result + " ");
                
                resultsPrinted++;
            }
            
            ++uCount;
            
            if(uCount == 16)
                uCount = 0;
		}
        
		System.err.println("Words decayed: " + printIndex);
		System.err.format("Percent decay: %.2f\n", (double)printIndex/(numResultsMax/2)*100.0);
        System.err.println("Min delay: " + minDelay);
        System.err.println("Max delay: " + maxDelay);

	}
}