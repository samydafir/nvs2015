import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

/**
 * Sender represents a UDP packet sender. Parameters can be specified via cmd.
 * Packages can be sent to a single recepient via localhost (127.0.0.1) or to a 
 * multicast group (230.0.0.1) (used to send packets to java and c programm 
 * simultaneously)
 * @author Thomas Samy Dafir
 */
public class Sender {
	
	/**
	 * main handles wrong numbers of cmd-arguments.
	 * @param args cmd-arguments:
	 * 	args[0] receiver name or IP-address
 	 * 	args[1] receiver port
 	 * 	args[2] number of packet-sending steps
 	 * 	args[3,4,5,...] number of packets to send in each step
	 */
	public static void main(String[] args) throws IOException{
		if(args.length < 3 || args.length != 3 + Integer.parseInt(args[2])){
			System.out.println("Usage: ./sender <receiver name> <port> <number of sending steps> <number of packages to send in each step>");
		}else{
			startSending(args);
		}
	}
	/**
	 * startSending handles sending of udp packets. Creates a socket on random port (sender-port-number
	 * is not important in our application). Gets the receiver's IP-address and sends packages to the 
	 * receiver (sending process is split into args[2] steps).  
	 * @param args cmd-arguments passed from main
	 * @throws IOException exception is forwarded to be handled by calling method
	 */
	private static void startSending(String[] args) throws IOException{
		DatagramSocket socket = new DatagramSocket();
		InetAddress address = InetAddress.getByName(args[0]);
		int receiverSocket = Integer.parseInt(args[1]);
		
		byte[] buffer = new byte[50];
		
		int sendAmt;
		for(int j = 3; j < args.length; j++){
			sendAmt = Integer.parseInt(args[j]);
			System.out.println("sending " + sendAmt + " packets...");
			for(int i = 0; i < sendAmt; i++){
				String message = String.format("%1$04d", i) + ": Packet Received";
				buffer = message.getBytes();
				DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, receiverSocket);
				socket.send(packet);
			}
		}
		socket.close();
	}
}
