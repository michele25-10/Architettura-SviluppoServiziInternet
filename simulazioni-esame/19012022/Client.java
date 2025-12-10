
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

class Client{
    public static void main(String[] args) {
        if(args.length != 2){
            System.out.println("Usage: client <server> <porta>");
            System.exit(1); 
        }

        try {
            Socket sd = new Socket(args[0], Integer.parseInt(args[1])); 

            BufferedReader fromUser = new BufferedReader(new InputStreamReader(System.in, "UTF-8")); 

            BufferedReader netIn = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8")); 
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8")); 

            String line; 
            while(true){
                System.out.println("Inserisci il mese nel formato ('YYYYMM') ('fine' per terminare):"); 
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
                netOut.write(line);
                netOut.newLine();
                netOut.flush();


                System.out.println("Inserisci il numero di righe ('fine' per terminare):"); 
                line = fromUser.readLine(); 
                if(line.equals("fine")) break; 
                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                //Lettura risposta dal server
                while(true){
                    line = netIn.readLine(); 
                    if(line == null){
                        System.err.println("Canale di comunicazione chiusto netIN");
                        break; 
                    }

                    System.out.println(line);

                    if(line.equals("--- END REQUEST ---")) break; 
                }

                System.out.println("\n"); 
            }

            System.out.println("Termine esecuzione del programma"); 

        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(2);
        }

    }
}