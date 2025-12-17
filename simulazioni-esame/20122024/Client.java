import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

public class Client {
    public static void main(String[] args) {
        if(args.length < 2){
            System.err.println("Usage: visualizza_interventi_onerosi server porta");
            System.exit(1);
        }

        try {
            Socket sd = new Socket(args[0], Integer.parseInt(args[1])); 

            BufferedReader from_user = new BufferedReader(new InputStreamReader(System.in, "UTF-8"));

            BufferedReader net_in = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8"));
            BufferedWriter net_out = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8"));

            String line; 
            while(true){
                System.out.println("Nome del cliente ('fine' per terminare): ");
                line = from_user.readLine(); 
                if(line.equals("fine")) break; 
                net_out.write(line);
                net_out.newLine();
                net_out.flush();

                System.out.println("Numero di righe n ('fine' per terminare): ");
                line = from_user.readLine(); 
                if(line.equals("fine")) break; 
                net_out.write(line);
                net_out.newLine();
                net_out.flush();

                System.out.println("Data in formato YYYYMMDD ('fine' per terminare): ");
                line = from_user.readLine(); 
                if(line.equals("fine")) break; 
                net_out.write(line);
                net_out.newLine();
                net_out.flush();

                while(true){
                    line = net_in.readLine();
                    if(line == null){
                        System.err.println("Il server ha chiuso la socket");
                        System.exit(2);
                    }
                    System.out.println(line);
                    if(line.equals("--- END REQUEST ---")) break; 
                } 
            }

            sd.close();
        } catch (IOException e) {
            System.err.println("ERRORE: " + e.getStackTrace());
            System.exit(3);
        }
    }
}
