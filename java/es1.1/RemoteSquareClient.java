
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

class RemoteSquareClient {
    public static void main(String[] args) {
        if(args.length != 2){
            System.err.print("Usage: java RemoteSquareClient hostname porta"); 
            System.exit(1); 
        }

        try {
            // Creo socket
            Socket s = new Socket(args[0], Integer.parseInt(args[1])); 

            // creo i canali per comunicare con la socket
            BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8")); 
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8")); 

            // creo canale per lettura da tastiera
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in)); 
            
            int n = 0;
            String line = br.readLine();  
            // Esco dal ciclo quanod l'utente scrive fine
            while (!line.equals("fine")){ 
                try{
                    n = Integer.parseInt(line); 
                }catch (Exception e){
                    // Non ho ricevuto un numero, termino programma e chiudo socket
                    System.err.println("Errore conversione in integer dell'input "+ line); 
                    s.close(); 
                    System.exit(2); 
                }

                // Invio al server
                netOut.write(line);
                netOut.newLine();
                netOut.flush();

                // condivido risultato
                String lineSocket = netIn.readLine(); 
                System.out.println("Risultato: "+ n + " * "+ n + " = " + lineSocket);  
                
                // Leggo nuova riga da tastiera
                line = br.readLine(); 
            }
            
            // chiudo la socket
            s.close();
        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }
}