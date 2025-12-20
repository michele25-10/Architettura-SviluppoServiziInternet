import java.io.*; 
import java.net.*; 

class Client{
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
                System.out.println("Inserisci il nome del giocatore ('fine' per terminare): "); 
                String nomeGiocatore = from_user.readLine(); 
                if(nomeGiocatore.equals("fine")) break; 

                System.out.println("Inserisci il squadra ('fine' per terminare): "); 
                String squadra = from_user.readLine(); 
                if(squadra.equals("fine")) break;

                System.out.println("Inserisci l'anno ('fine' per terminare): "); 
                String anno = from_user.readLine(); 
                if(anno.equals("fine")) break; 

                net_out.write(nomeGiocatore);
                net_out.newLine();
                net_out.write(squadra);
                net_out.newLine();
                net_out.write(anno);
                net_out.newLine();

                net_out.flush(); 

                String responseLine; 
                while (true) { 
                    responseLine = net_in.readLine(); 

                    if(responseLine==null){
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