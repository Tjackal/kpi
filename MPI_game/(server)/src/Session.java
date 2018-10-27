import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class Session extends Thread {
    private int N = 120;

    private List<Socket> players = new ArrayList<>();
    private List<BufferedReader> in = new ArrayList<>();
    private List<PrintWriter> out = new ArrayList<>();

    public synchronized void addPlayer(Socket currentPlayer) throws IOException {
        players.add(currentPlayer);
        in.add(new BufferedReader(new InputStreamReader(currentPlayer.getInputStream())));
        out.add(new PrintWriter(currentPlayer.getOutputStream(),true));
    }

    public void run() {
        try {
            String input;
            while (true) {
                for (int i = 0; i < players.size(); i++) {
                    for (int j = 0; j < players.size(); j++) {
                        if (j == i)
                            out.get(j).println("Your turn");
                        else
                            out.get(j).println((i + 1) + " player's turn");
                    }
                    input = in.get(i).readLine();
                    int currN = Integer.parseInt(input);
                    for (int j = 0; j < players.size(); j++) {
                        if (j != i)
                            out.get(j).println("Player " + (i + 1) + "supposes N is " + currN + ". ");
                    }
                    if (currN < N) {
                        for (int j = 0; j < players.size(); j++) {
                            out.get(j).println(input + " is less than N");
                        }
                    }
                    else if (currN > N) {
                        for (int j = 0; j < players.size(); j++)
                            out.get(j).println(input + " is more than N");
                    }
                    else if (currN == N) {
                        out.get(i).println(input + " equals N. You win!");
                        for (int j = 0; j < players.size(); j++) {
                            if (j != i)
                                out.get(j).println("Player " + (i + 1) + " wins. N equals " + currN + ". Better luck next time");
                            out.get(j).println("Have a nice day");
                            players.get(j).close();
                        }
                        System.out.println(i + " player wins. Session dead");
                        System.exit(1);
                    }
                    for (int j = 0; j < players.size(); j++)
                        out.get(j).println("Turn ended");
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }
    }
}
