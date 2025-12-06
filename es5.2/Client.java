
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

class Client{
    public static void main(String[] args) {
        if(args.length != 2){
            System.out.println("Usage: Client <server> <port>");
            System.exit(1);
        }    

        try {
            Socket sd = new Socket(args[0], Integer.parseInt(args[1]));
            
            BufferedReader fromUser = new BufferedReader(new InputStreamReader(System.in, "UTF-8")); 
            
            BufferedReader netIn = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8")); 
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8")); 

            String line; 
            while(true){
                System.out.println("Inserisci email ('fine' per terminare): ");
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                System.out.println("Inserisci password ('fine' per terminare): ");
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                System.out.println("Inserisci rivista ('fine' per terminare): ");
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                String res; 
                while (true) { 
                    res = netIn.readLine(); 

                    if(res == null){
                        System.err.println("Errore Server ha chiuso la connenssione");
                        System.exit(2);
                    }

                    System.out.println(res);

                    if(res.equals("--- END REQUEST ---")) break; 
                }

                System.out.println("\n");
            }


        } catch (IOException e) {
        }
    }
}