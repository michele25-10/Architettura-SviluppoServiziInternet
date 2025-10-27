
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class RemoteSquareServer {
    public static void main(String[] args) {
        if(args.length != 1){
            System.err.println("Usage: java RemoteSquareServer port"); 
            System.exit(1);
        }

        try {
            // Creo server in ascolto sulla porta data in input
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0])); 

            for(;;){
                // Accetto connessione
                Socket s = ss.accept();
                
                // Creo canali di comunicazione per la socket accettata
                BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8")); 
                BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8")); 
                
                // Leggo in loop gli input lanciati dal mio client
                String line; 
                while((line = netIn.readLine()) != null) {  
                    int n = Integer.parseInt(line); 
                    int result = n * n;  
    
                    netOut.write(""+result);
                    netOut.newLine();
                    netOut.flush();                
                }            
                
                s.close(); 
            }
        } catch (IOException e) {
            System.err.println("Errore " + e.getMessage()); 
        }
    }
}
