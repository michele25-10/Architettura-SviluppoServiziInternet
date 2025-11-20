
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;

public class RemoteHeadServer {
    public static void main(String[] args) {
        if(args.length != 1){
            System.err.println("Usage: java RemoteHeadServer porta");
            System.exit(1); 
        }

        try {
            // Apro una porta sul server e rimane in ascolto
            // di connessioni in ingresso da parte dei client
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0])); 

            for(;;){
                // quando un client si connette, accetta la connessione
                // e ti restituisce un oggetto Socket che rappresenta
                // quella comunicazione.
                Socket s = ss.accept(); 

                BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8")); 
                BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8")); 

                // leggo il filename dalla socket
                String filename = netIn.readLine();
                File f = new File(filename); 

                // controllo se il file esiste
                if(f.exists()){
                    BufferedReader bfr = new BufferedReader(new FileReader(f));     
                    String line;
                    int line_number = 1; 
                    // leggo dal file
                    while((line = bfr.readLine()) != null && line_number <= 5){
                        // mando le righe nella socket
                        netOut.write(line);
                        netOut.newLine();
                        netOut.flush();
                        line_number += 1; 
                    }
                }

                s.close();
            }

        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }
}
