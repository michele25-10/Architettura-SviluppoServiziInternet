import java.io.*;
import java.net.*;

public class ClientTDConnReuse
{
    public static void main(String args[])
    {
        if (args.length != 2) {
            System.err.println("Usage: java ClientTDConnReuse hostname port");
            System.exit(1);
        }

        try {
            Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]));

            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(),"UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(theSocket.getOutputStream(),"UTF-8"));

            for (;;) {
                System.out.println("Insert email, 'fine' to terminate: ");
                String email = userIn.readLine();

                /* Exit if the user types 'fine' */
                if (email.equals("fine")) {
                    break;
                }

                System.out.println("Insert the password");
                String password = userIn.readLine();

                networkOut.write(email);
                networkOut.newLine();

                networkOut.write(password);
                networkOut.newLine();

                networkOut.flush();

                /* Read the server's response and print it to the screen */
                String theLine;
                for (;;) {
                    /* Read input from Server line by line */
                    theLine = networkIn.readLine();

                    /* Error check */
                    if (theLine == null) {
                        System.err.println("Error! The Server has closed the connection!");
                        System.exit(2);
                    }

                    /* Print the line read from Server */
                    System.out.println(theLine);

                    /* Move to new request if access denied */
                    if (theLine.equals("--- Access denied ---")) {
                        break;
                    }

                    /* Move to new request once Server input is finished */
                    if (theLine.equals("--- END REQUEST ---")) {
                        break;
                    }
                }
            }

            /* Always remember to close the socket! */
            theSocket.close();
        }
        catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
