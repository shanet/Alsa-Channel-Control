import java.io.*;
import java.net.*;
import java.util.Scanner;

public class client {

   static final int NORMAL_EXIT = 0;

   public static void main(String args[]) {
      if(args.length <= 1) {
         System.out.println("Usage: client [hostname] [port]");
         System.exit(NORMAL_EXIT);
      }

      Socket connSock = null;
      BufferedReader recv = null;
      PrintWriter send = null;
      Scanner scanner = new Scanner(System.in);

      String msgToSend, hostname = args[0];
      int port = new Integer(args[1]);

      try {
         // Connect to server
         System.out.println("Connecting to " + hostname + " on port " + port + "...");
         connSock = new Socket(hostname, port);

         // Open send stream
         send = new PrintWriter(connSock.getOutputStream(), true);

         // Open read stream
         recv = new BufferedReader(new InputStreamReader(connSock.getInputStream()));

         char c;
         while((c = (char)recv.read()) != -1)
            System.out.print(c);
         System.out.println("done");

         while(true) {
            // Get message to send
            System.out.print("Send to server: ");
            msgToSend = scanner.nextLine();

            if(msgToSend.equals("end"))
                break;

            // Send message
            send.println(msgToSend);

            System.out.println("Message sent. Waiting for reply...");

            // Recieve and output reply
            System.out.println("Reply: " + recv.readLine());
         }
      } catch(Exception e) {
         System.out.println("Exception " + e + " thrown.");
      } finally {
         System.out.println("Closing connection.");
         try {
            recv.close();
            send.close();
            connSock.close();
         } catch(IOException ioe) {}
      }

      System.exit(NORMAL_EXIT);
   }
}
