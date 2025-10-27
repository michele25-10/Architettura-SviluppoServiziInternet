
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class QuoteServer {
    static final String[] quotations = {
        "Adoro i piani ben riusciti", 
        "Quel tappeto dava veramente un tono all'ambiente", 
        "Se ci riprovi ti stacco un braccio", 
        "Questo è un colpo di genio, Leonard", 
        "La vita è cosi, ma a me piace così"
    }; 
     
    public static void main(String[] args) {
        if (args.length != 1){
            System.err.println("Usage java QuoteServer porta");
            System.exit(1);
        }        

        try {
            // Creo socket
            DatagramSocket ds = new DatagramSocket(Integer.parseInt(args[0])); 

            int index = 0; 

            while(true){
                // Creo buffer 
                byte[] reqBuf = new byte[2048]; 
                DatagramPacket reqPacket = new DatagramPacket(reqBuf, reqBuf.length); // Pacchetti massimo 1500 byte

                // leggo il messaggio di richiesta
                ds.receive(reqPacket);

                // Devo specificare la lunghezza del messaggio per non prendere anche dati "sporchi"
                String request = new String(reqPacket.getData(),0, reqPacket.getLength(),  "UTF-8"); 
                if (request.equals("QUOTE")){
                    // Ottengo prossima quote
                    String quote = quotations[index % quotations.length]; 
                    
                    // converto la quote da stringa a sequenza di byte
                    byte[] respBuf = quote.getBytes("UTF-8"); 
                    
                    // Preparo il datagram packet di risposta
                    DatagramPacket respPacket = new DatagramPacket(respBuf, respBuf.length, reqPacket.getAddress(), reqPacket.getPort()); 

                    // Invio la risposta al client
                    ds.send(respPacket); 
                }

                index += 1; 
            }
        } catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}
