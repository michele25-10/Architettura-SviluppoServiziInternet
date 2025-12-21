import java.io.*; 
import java.net.*; 

public class Client {
    public static void main(String[] args) {
        if(args.length != 2){
            System.err.println("Errore! Usage: java Client hostname porta");
            System.exit(1);
        }

        try {
            Socket sd = new Socket(args[0], Integer.parseInt(args[1])); 

            BufferedReader from_user = new BufferedReader(new InputStreamReader(System.in, "UTF-8")); 

            BufferedReader net_in = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8")); 
            BufferedWriter net_out = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8")); 

            while (true) { 
                System.out.println("Inserisci lo username ('fine' per terminare): ");
                String username = from_user.readLine(); 
                if(username.equals("fine")) break; 

                System.out.println("Inserisci il nome del progetto ('fine' per terminare): ");
                String nomeProgetto = from_user.readLine(); 
                if(nomeProgetto.equals("fine")) break; 

                System.out.println("Inserisci la versione del progetto ('fine' per terminare): ");
                String versione = from_user.readLine(); 
                if(versione.equals("fine")) break; 

                net_out.write(username);
                net_out.newLine();
                net_out.write(nomeProgetto);
                net_out.newLine();
                net_out.write(versione);
                net_out.newLine();
                net_out.flush();

                String responseLine; 
                while (true) { 
                    responseLine = net_in.readLine(); 
                    if(responseLine == null){
                        System.err.println("Errore! il server ha chiuso la connessione");
                        System.exit(2);
                    }

                    System.out.println(responseLine); 

                    if(responseLine.equals("--- END REQUEST ---")) break; 
                }
            }

            sd.close();
        } catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(3); 
        }
    }
}
