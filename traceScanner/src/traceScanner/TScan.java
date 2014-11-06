package traceScanner;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

public class TScan {
	
	public TScan(String in, String marked1, String marked2){
		String fName = in;
		int iCount= 0;
		int slmCount = 0;
		boolean iType = true;
		boolean pass1 = false;
		boolean pass2 = false;
		int before = 0;
		int middle = 0;
		int after = 0;
		ArrayList <String> uniq = new ArrayList <String>();
		
		//set up the reader
		try {
			
			BufferedReader reader;
			reader = new BufferedReader(new FileReader(fName));
			
			
			String Line = null;
			
			while ((Line = reader.readLine()) != null){
				//while there are still lines in the file 
				
				//I SML counts
				String [] seg = Line.split(" ");
				
				
				if (seg[0].equals("I")){
					iCount++;
					iType = true;
				}
				else if(seg[1].equals("S") || seg[1].equals("M") || seg[1].equals("L")){
					slmCount++;
					iType = false;
				}
				
				seg = seg[2].split(",");
				//also count the partitioning
				if(seg[0].equals(marked1)){
					pass1 = true;
					middle--;
				}
				if(seg[0].equals(marked2)){
					pass2 = true;
					after--;
				}
				if (pass1 && pass2){
					after++;
				}
				else if(pass1 && !pass2){
					middle++;
				}
				else if (!pass1 && !pass2){
					before++;
				}
				
				//check for uniqueness
				System.out.println(seg[0]);
				seg[0] = seg[0].substring(0, seg[0].length()-3);
				System.out.println(seg[0]);
				if (uniq.contains(seg[0])){
					if (iType){
						iCount--;
					}
					else{
						slmCount--;
					}
				}
				else{
					uniq.add(seg[0]);
				}

			}
			System.out.println("iCount is: " + iCount);
			System.out.println("slmCount is: " + slmCount);
			System.out.println("before is: " + before);
			System.out.println("after is: " + after);
			System.out.println("middle is: " + middle);
			reader.close();
		} catch (FileNotFoundException e) {
			// Auto-generated catch block
			System.out.println("no file");
			e.printStackTrace();
		} catch (IOException e) {
			// Auto-generated catch block
			e.printStackTrace();
		}		
	}
	
}
