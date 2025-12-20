import java.io.*;
import java.net.*;

public class Client
{
    public static void main(String args[])
    {
        if(args.length != 2)
        {
            System.err.println("Errore! Uso: java Client hostname porta");
            System.exit(1);
        }

        try
        {
            Socket sd = new Socket(args[0], Integer.parseInt(args[1]));

            BufferedReader from_user = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader net_in = new BufferedReader(new InputStreamReader(sd.getInputStream(), "UTF-8"));
            BufferedWriter net_out = new BufferedWriter(new OutputStreamWriter(sd.getOutputStream(), "UTF-8"));

            for(;;)
            {
                System.out.println("username (fine per uscire):");
                String username = from_user.readLine();

                if(username.equals("fine")) {
                    break;
                }

                System.out.println("password (fine per uscire):");
                String password = from_user.readLine();

                if(password.equals("fine")) {
                    break;
                }

                System.out.println("Inserire tipologia di giocattolo (fine per uscire):");
                String tipologia = from_user.readLine();

                if(tipologia.equals("fine")) {
                    break;
                }

                net_out.write(username);
                net_out.newLine();
                net_out.write(password);
                net_out.newLine();
                net_out.write(tipologia);
                net_out.newLine();

                net_out.flush();

                String theLine;
                for(;;)
                {
                    theLine = net_in.readLine();

                    if(theLine == null)
                    {
                        System.err.println("Errore! Il server ha chiuso la connessione");
                        System.exit(2);
                    }

                    System.out.println(theLine);

                    if(theLine.equals("--- END REQUEST ---"))
                    {
                        break;
                    }
                }

            }
            
            sd.close();
        }
        catch(IOException e)
        {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(3);
        }
    }
}
