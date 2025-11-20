
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;


// java RemoteHeadClient    hostname   porta   nomefile
//                          args[0]    args[1] args[2]    
class RemoteHeadClient{
    public static void main (String args[]) {      
        try {  
            Socket s = new Socket(args[0], Integer.parseInt(args[1]));
            
            // Canali per scrivere all'interno della socket
            BufferedReader netIn =new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8")); 
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8")); 

            netOut.write(args[2]); 
            netOut.newLine(); 
            netOut.flush(); 

            // Attendo una risposta dal server
            String line; 
            int line_number = 1; 
            while((line = netIn.readLine()) != null){
                System.out.println(line); 
                line_number += 1;
            }

            // Chiudo la socket
            s.close();
        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }


}