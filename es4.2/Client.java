
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

class Client{
    public static void main(String[] args) {
        if(args.length < 2){
            System.err.println("Usage: Client <server> <porta>");
            System.exit(1);
        }

        try {
            Socket sd  = new Socket(args[0], Integer.parseInt(args[1])); 

            BufferedReader fromUser = new BufferedReader(new InputStreamReader(System.in, "UTF-8")); 

            BufferedReader netIn = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8")); 
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8")); 

            String line;
            while(true){
                System.out.println("Inserisci il vino (per uscire 'fine'):");
                line = fromUser.readLine(); 
                
                if(line.equals("fine")) break; 
                
                netOut.write(line); 
                netOut.newLine();
                netOut.flush();

                System.out.println("Inserisci l'annata (per finire 'fine')");
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
            
                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                String res; 
                while(true){
                    res = netIn.readLine(); 

                    if(res == null){
                        System.err.println("Connessione chiusa con il server!");
                        System.exit(3); 
                    }

                    System.out.println(res);

                    if (res.equals("--- END RESPONSE ---")) break; 
                }
            }

        } catch (IOException e) {
        }
    }
}