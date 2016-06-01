import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.zip.CRC32;

/**
 * Sender represents a UDP packet sender. Parameters can be specified via cmd.
 * Packets can be sent to a single recipient. A checksum over all Packets is and
 * sent as part of the last packet.
 * Uses a sliding-window to ensure no packets are lost.
 * @author Thomas Samy Dafir, 1331483
 * @author Dominik Baumgartner, 0920177
 * @author Vivien Wallner, 1320293
 */
public class Sender {
	
	private static int LAR;
	private static int windowSize;
	
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
		LAR = -1;
		if(args.length != 5){
			System.out.println("Usage: java Sender <receiver name> <port> <number of packets to send> <payload size> <window-size>");
		}else{
			try{
				startSending(args);
			}catch(IOException e){
				System.err.println("issue creating socket");
			}catch(InterruptedException e){}
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
	 * @throws InterruptedException 
	 */
	private static void startSending(String[] args) throws IOException, InterruptedException{
        long beforeTime = 0;
        long afterTime = 0;
        windowSize = Integer.parseInt(args[4]);
		//create socket with standard constructor. Sender-port does not have to be known.
		DatagramSocket socket = new DatagramSocket();
		//set timeout for ACK-reception
		socket.setSoTimeout(100);

		//transform the cmd-argument receiver-name into an InetAddress object.
		CRC32 checksum = new CRC32();
		InetAddress address = InetAddress.getByName(args[0]);
		int receiverSocket = Integer.parseInt(args[1]);
		int size = Integer.parseInt(args[3]) * 4;
		ByteBuffer buffer = ByteBuffer.allocate(size + 4);
		ByteBuffer ackBuffer = ByteBuffer.allocate(4);
		byte[] message = getPayload(size);
		//DatagramPacket checkPacket = new DatagramPacket(buffer, buffer.length);
		int sendAmt = Integer.parseInt(args[2]);
		DatagramPacket sendPacket;
		DatagramPacket ackPacket = new DatagramPacket(ackBuffer.array(), ackBuffer.array().length);
		
		//the for-loop handles sending of packets. creates a string-message including the packet-number.
		//Create byte-array from string, receiver-info to generate packet and
		//send the packet using the datagram socket.
		beforeTime = System.currentTimeMillis();
		int ackNumber;
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
			sendPacket = new DatagramPacket(buffer.array(), buffer.array().length, address, receiverSocket);
			socket.send(sendPacket);
			if(i == LAR + windowSize){
				try{
					socket.receive(ackPacket);
					ackBuffer = ByteBuffer.wrap(ackPacket.getData());
					ackNumber = ackBuffer.getInt();
					if(ackNumber != LAR + 1){
						System.out.println(i + " wrong packet " + ackNumber + " " + LAR);
						i = LAR;
						Thread.sleep(10);
					}else{
						LAR = ackNumber;
					}
				}catch(SocketTimeoutException e){
					System.out.println("timeout");
					i = LAR;
					Thread.sleep(10);
				}
				
			}
			buffer.clear();
		}
		afterTime = System.currentTimeMillis();
		socket.close();
		//+4 for one integer --> sequence number
		evaluate(afterTime, beforeTime, size + 4, sendAmt);
		System.out.println("checksum: " + checksum.getValue());
	}
	
	/**
	 * constructs the User-Data for to be sent
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
		double speed = (8 * (double)(totalSize)/(double)(duration)) / 1000;
		System.out.println(String.format("%.3f mbit/s", speed));		
	}
}