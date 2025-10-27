
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;

public class RemoteDGramStrlen {
    public static void main(String[] args) {
        if(args.length != 2){
            System.err.println("Usage: java RemoteDGramStrlen nodoServer portaServer");
            System.exit(1);
        }

        try {
            // Creo socket
            DatagramSocket ds = new DatagramSocket(); 

            // creo i buffer dei due canali
            byte[] reqBuf = new byte[2048]; 
            byte[] respBuf = new byte[2048]; 

            // creo i packet di req e di res
            DatagramPacket reqPacket = new DatagramPacket(reqBuf, 0, reqBuf.length,
                InetAddress.getByName(args[0]), Integer.parseInt(args[1])); 
            DatagramPacket resPacket = new DatagramPacket(respBuf, respBuf.length); 

            // creo canale per lettura da tastiera
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in)); 
            
            String line = br.readLine();  
            // Esco dal ciclo quanod l'utente scrive fine
            while (!line.equals("fine")){  
                reqPacket.setData(line.getBytes("UTF-8"));
                ds.send(reqPacket); 
                
                ds.receive(resPacket); 

                // la converto in bytes e la invio
                String quote = new String(resPacket.getData(), 0, resPacket.getLength(), "UTF-8"); 
                System.out.println("Il numero di stringhe sono: " + quote);
                
                // Leggo nuova riga da tastiera
                line = br.readLine(); 
            }
            
            // chiudo la socket
            ds.close();
        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(2);
        }
    }
}
