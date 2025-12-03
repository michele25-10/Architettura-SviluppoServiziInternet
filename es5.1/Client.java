
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

class Client{
    public static void main(String[] args) {
        if(args.length < 2){
            System.err.println("Usage: Client <server> <port>");
            System.exit(1); 
        }   

        try {
            Socket sd = new Socket(args[0], Integer.parseInt(args[1])); 

            BufferedReader fromUser = new BufferedReader(new InputStreamReader(System.in)); 

            BufferedReader netIn = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8")); 
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8")); 

            String line; 
            while(true){
                /* Lettura ed invio della regione */
                System.out.println("Inserisci la regione ('fine' per uscire):");
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
                netOut.write(line);
                netOut.newLine();
                netOut.flush();
                
                /**Lettura ed invio della localita */
                System.out.println("Inserisci numero delle localita ('fine' per uscire):");
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                /**Lettura risposte del server */
                while(true){
                    line = netIn.readLine(); 
                    if(line == null){
                        System.err.println("Connessione con il server chiusa");
                        System.exit(3);
                    }
                    
                    System.out.println(line);

                    if(line.equals("--- END REQUEST ---")) break; 
                }
            }
        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(2);
        }
    }

}