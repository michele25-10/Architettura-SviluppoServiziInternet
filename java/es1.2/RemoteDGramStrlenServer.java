
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class RemoteDGramStrlenServer {
    public static void main(String[] args) {
        if (args.length != 1){
            System.err.println("Usage java RemoteDGramStrlenServer porta");
            System.exit(1);
        }        


        try{
            // Creo la socket
            DatagramSocket ds = new DatagramSocket(Integer.parseInt(args[0])); 

            while(true) {
                // Creo buffer 
                byte[] reqBuf = new byte[2048]; 
                DatagramPacket reqPacket = new DatagramPacket(reqBuf, reqBuf.length); // Pacchetti massimo 1500 byte

                // leggo il messaggio di richiesta
                ds.receive(reqPacket);

                // Devo specificare la lunghezza del messaggio per non prendere anche dati "sporchi"
                String request = new String(reqPacket.getData(),0, reqPacket.getLength(),  "UTF-8"); 
                int length = request.length(); 
    
                // converto la lunghezza da stringa a sequenza di byte
                byte[] respBuf = (""+length).getBytes("UTF-8"); 
                
                // Preparo il datagram packet di risposta
                DatagramPacket respPacket = new DatagramPacket(respBuf, respBuf.length, reqPacket.getAddress(), reqPacket.getPort()); 

                // Invio la risposta al client
                ds.send(respPacket); 
            }

        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(2);
        }
    }
}
