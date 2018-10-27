import java.io.*;
import java.net.*;

public class Server {

    static final int port = 6666;

    public static void main(String[] args) throws IOException {
        Socket from_client = null;

        ServerSocket server = new ServerSocket(port);
        System.out.println("waiting for client");

        Session session = new Session();
        from_client = server.accept();
        System.out.println("new client plugged in");
        session.addPlayer(from_client);
        session.start();
        while (true) {
            from_client = server.accept();
            System.out.println("new client plugged in");
            session.addPlayer(from_client);
        }
    }

}