import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.zip.CRC32;

/**
 * Sender represents a UDP packet sender. Parameters can be specified via cmd.
 * Packets can be sent to a single recipient.
 * @author Thomas Samy Dafir, 1331483
 * @author Dominik Baumgartner, 0920177
 * @author Vivien Wallner, 1320293
 */
public class Sender {
	
	/**
	 * main handles wrong numbers of cmd-arguments.
	 * Receiver-name, receiver-port, sending-steps and the amount of packets to send in each step have to be 
	 * specified, otherwise a message is issued informing the user on how to use the application. IOExceptions
	 * thrown in the startSending-method are handled here.
	 * @param args cmd-arguments:
	 * 	args[0] receiver name or IP-address
 	 * 	args[1] receiver port
 	 * 	args[2] number of packets to send
 	 *  args[3] size of each packet's User-Message 	
	 */
	public static void main(String[] args){
		if(args.length != 4){
			System.out.println("Usage: java Sender <receiver name> <port> <number of packets to send> <payload size>");
		}else{
			try{
				startSending(args);
			}catch(IOException e){
				System.err.println("issue creating socket");
			}
		}
	}
	/**
	 * startSending handles sending of UDP packets. Creates a socket on free port (sender-port-number
	 * is not important in our application since the receiver does not send an answer to the sender).
	 * Gets the receiver's IP-address from args[0] and sends packets to the 
	 * receiver (sending process is split into args[2] steps).
	 * in each step the number of packets specified in args[3,4,...] is sent.
	 * @param args cmd-arguments passed from main
	 * @throws IOException exception is forwarded to be handled by calling method
	 */
	private static void startSending(String[] args) throws IOException{
        long beforeTime = 0;
        long afterTime = 0;
        
		//create socket with standard constructor. Sender-port does not have to be known.
		DatagramSocket socket = new DatagramSocket();
		//transform the cmd-argument receiver-name into an InetAddress object.
		CRC32 checksum = new CRC32();
		InetAddress address = InetAddress.getByName(args[0]);
		int receiverSocket = Integer.parseInt(args[1]);
		int size = Integer.parseInt(args[3]) * 4;
		ByteBuffer buffer = ByteBuffer.allocate(size + 4);
		byte[] message = getPayload(size);
		//DatagramPacket checkPacket = new DatagramPacket(buffer, buffer.length);
		int sendAmt = Integer.parseInt(args[2]);
		//the for-loop handles sending of packets. creates a string-message including the packet-number.
		//Create byte-array from string, receiver-info to generate packet and
		//send the packet using the datagram socket.
		beforeTime = System.currentTimeMillis();
		for(int i = 0; i < sendAmt; i++){
			buffer.putInt(i);
			/*If current packet is not the last packet: put the whole message into the bytebuffer and use it
			 * to update the checksum. Otherwise: put message up to length - 4 into the buffer, to leave space
			 * for the checksum + add checksum.*/
			if(i < sendAmt - 1){
				buffer.put(message);
				checksum.update(buffer.array());
			}else{
				buffer.put(Arrays.copyOfRange(message, 0, message.length-4));
				checksum.update(Arrays.copyOfRange(buffer.array(), 0, buffer.array().length-4));
				buffer.putInt((int) checksum.getValue());
			}
			DatagramPacket packet = new DatagramPacket(buffer.array(), buffer.array().length, address, receiverSocket);
			socket.send(packet);
			buffer.clear();
		}
		afterTime = System.currentTimeMillis();
		socket.close();
		//+4 for one integer --> sequence number
		evaluate(afterTime, beforeTime, size + 4, sendAmt);
		System.out.println("checksum: " + checksum.getValue());
	}
	
	/**
	 * constucts the User-Data for to be sent
	 * @param length length of result string
	 * @return string of given length
	 */
	private static byte[] getPayload(int length){
		ByteBuffer bb = ByteBuffer.allocate(length);
		for(int i = 0; i < length/4; i++){
			bb.putInt(9);
		}
		return bb.array();
		
	}
	
	/**
	 * evaluates the sending process. Computes Transfer-speed based on sent data and transfer-time
	 * @param after timestamp after sending
	 * @param before timestamp before sending
	 * @param size size of a package's Message
	 * @param amnt Amount of packets sent
	 */
	private static void evaluate(long after, long before, int size, int amnt){
		long duration = after - before;
		int totalSize = size * amnt;
		double speed = (double)(totalSize)/(double)(duration);
		System.out.println(String.format("%.2f KB/s", speed));		
	}
}