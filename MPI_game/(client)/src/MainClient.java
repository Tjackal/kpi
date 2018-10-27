import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class MainClient {
    static final int port = 6666;

    public static void main(String[] args) throws IOException {
        Socket from_server = new Socket("localhost",port);
        BufferedReader in  = new BufferedReader(new InputStreamReader(from_server.getInputStream()));
        PrintWriter out = new PrintWriter(from_server.getOutputStream(),true);
        BufferedReader inu = new BufferedReader(new InputStreamReader(System.in));
        String fuser,fserver;

//        while (true) {
//            fserver = in.readLine();
//            System.out.println(fserver);
//            if (fserver.equalsIgnoreCase("Your turn")) {
//                fuser = inu.readLine();
//                out.println(fuser);
//            }
            do {
                fserver = in.readLine();
                System.out.println(fserver);
                if (fserver.equalsIgnoreCase("Your turn")) {
                    fuser = inu.readLine();
                    out.println(fuser);
                }
            } while (!(fserver.equalsIgnoreCase("Have a nice day")));
//        }



//        out.close();
//        in.close();
//        inu.close();
//        from_server.close();
    }
}
