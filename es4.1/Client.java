
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

public class Client {
    public static void main(String[] args) {

        if(args.length != 2){
            System.err.println("Usage: Client <server> <port>");
            System.exit(1);
        }

        try {           
            Socket sd = new Socket(args[0], Integer.parseInt(args[1])); 
            
            BufferedReader fromUser = new BufferedReader(new InputStreamReader(System.in)); 

            BufferedReader netIn = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8")); 
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8")); 
  
            String line;
            for(;;){

                System.out.println("Inserisci categoria:");
                line = fromUser.readLine();
                
                if (line == null){
                    continue;
                }
                
                if(line.equals("fine")){
                    break; 
                }

                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                String response; 
                for(;;){
                    response = netIn.readLine(); 
                    System.out.println(response);

                    if(response == null){
                        System.err.println("Errore, il server ha chiuso la connessione");
                        System.exit(2);
                    }

                    if(response.equals("--- END RESPONSE ---")){
                        break; 
                    }
                }
            }            
        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(1);
        }
    }
}
