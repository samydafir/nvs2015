import java.io.IOException;
import java.net.DatagramPacket;
import java.net.MulticastSocket;
import java.net.SocketTimeoutException;
import java.net.InetAddress;

/**
 * Receiver represents a UDP packet receiver with multicast capabilities: several receivers can join
 * the multicast group and receive packets intended for the multicast address.
 * @author Thomas Samy Dafir
 */
public class Receiver {
    
    private final static int MAXSIZE = 512;
    private final static String GROUP_IP = "230.0.0.1";

    /**
     * main handles wrong numbers of cmd-arguments and calls the receive-method
     * @param args cmd-arguments:
     * 	args[0] port number
     * 	args[1] timeout for the receiving socket
     * @throws IOException
     */
	public static void main(String[] args) throws IOException {
		if(args.length != 2){
			System.out.println("Usage: java Receiver <port number> <receiving timeout in ms>");
		}else{
			receive(args);
		}
	}
	
	/**
	 * creates a multicast group as well as group address and a socket. joins the socket to
	 * the multicast group and receives UDP packets. The content of the packets is saved in
	 * the buffer byte-array. Also counts the amount of packets received.
	 * @param args cmd-arguments passed from main
	 * @throws IOException exception is forwarded to be handled by calling method
	 */
	private static void receive(String[] args) throws IOException{
		byte[] buffer = new byte[MAXSIZE];
		MulticastSocket socket = new MulticastSocket(Integer.parseInt(args[0]));
        InetAddress group = InetAddress.getByName(GROUP_IP);
        socket.joinGroup(group);
		socket.setSoTimeout(Integer.parseInt(args[1]));
		DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
		int packetCount = 0;
		try{
			while(true){
				socket.receive(packet);
				//print(packet);
				packetCount++;
			}
		}catch(SocketTimeoutException e){
			System.out.println("Receiving of packets terminated");
			System.out.println(packetCount + " packets received");
		}
        socket.leaveGroup(group);
		socket.close();
	}
	
	
	/**
	 * Prints the content of a UDP packet
	 * @param packet packet to be printed
	 */
	@SuppressWarnings("unused")
	private static void print(DatagramPacket packet){
		String message = new String(packet.getData(), 0, packet.getLength());
		System.out.println(message);
	}
}
