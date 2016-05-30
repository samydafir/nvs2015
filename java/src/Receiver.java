import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.zip.CRC32;

/**
 * Receiver represents a UDP packet receiver. Calculates CRC32 checksum over all received packets.
 * For each received packet an ACK is sent to the Sender.
 * @author Thomas Samy Dafir, 1331483
 * @author Dominik Baumgartner, 0920177
 * @author Vivien Wallner, 1320293
 */
public class Receiver {
    
	private static int LAS;
    private static InetAddress ackAddress;
    private static int ackPort;
	
    /**
     * main handles wrong numbers of cmd-arguments. Wrong numbers lead to a message describing usage being
     * issued, otherwise receive() is called. IOExceptions thrown in the receive-method are handled here.
     * @param args cmd-arguments:
     * 	args[0] port number
     * 	args[1] timeout for the receiving socket
     */
	public static void main(String[] args){
		LAS = -1;
		if(args.length != 5 && args.length != 4){
			System.out.println("Usage: java Receiver <port number> <receiving timeout in ms> <expected packets> <payload size> <-v for verbose>");
		}else{
			try{
				receive(args);
			}catch(IOException e){
				System.err.println("Socket error");
			}
		}
	}
	
	/**
	 * receives UDP packets. The content of the packets is saved in
	 * the buffer byte-array. Also counts and prints the amount of packets received.
	 * @param args cmd-arguments passed from main
	 * @throws IOException exception is forwarded to be handled by calling method
	 */
	private static void receive(String[] args)throws IOException{
        long beforeTime = 0;
        long afterTime = 0;
        //+5 for sequence number and space
        int size = Integer.parseInt(args[3]) * 4;
		//buffer to hold the received message represented as byte-array
		byte[] buffer = new byte[size + 4];
		//create socket with port-number specified in args[0].
		DatagramSocket socket = new DatagramSocket(Integer.parseInt(args[0]));
		
		//create datagram-packet with previously defined buffer. Received packets will be saved in this variable
		DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
		//get expected amount  of packets
        int expec = Integer.parseInt(args[2]);
        
        //Variables used for ACK
        ByteBuffer ackBuffer = ByteBuffer.allocate(4);

        CRC32 checksum = new CRC32();
		int packetCount = 0;
		try{
			//infinite loop to receive packets. Only terminated if no packets arrive for a certain specified time
			//e.g. if a connection problem occurs or no more packets are sent by the Sender.
			socket.receive(packet);
			//set socket-timeout to terminate following loop, if no more packets received
			socket.setSoTimeout(Integer.parseInt(args[1]));
			
			//get Sender-sddress and port
			ackAddress = packet.getAddress();
			ackPort = packet.getPort();
			sendAck(packet, socket);
			checksum.update(packet.getData());
			beforeTime = System.currentTimeMillis();
			packetCount++;
			while(true){
				socket.receive(packet);
				sendAck(packet, socket);
				afterTime = System.currentTimeMillis();
				packetCount++;
				byte[] message = packet.getData();
				/*If packet is not the last packet: use whole packet content to update checksum
				If last packet: dont use last 4 bytes --> contein checksum itself*/
				if(packetCount < expec){
					checksum.update(message);
				}else{
					checksum.update(Arrays.copyOfRange(message, 0, message.length-4));
				}
				//optional verbose mode. prints packet content
				if(args.length == 5 && args[4].equals("-v")){
					print(packet,packetCount,expec);
				}
			}
		//if the socket-timeout is reached the thrown SocketTimeoutException is caught here. Number of received
		//packets is printed.
		}catch(SocketTimeoutException e){}
		socket.close();
		System.out.println("checksum: " + checksum.getValue());
		evaluate(afterTime, beforeTime, packet.getLength(), packetCount, expec);
	}
	
	
	private static void sendAck(DatagramPacket packet, DatagramSocket socket) throws IOException{
        byte[] packetData = packet.getData();
        ByteBuffer buffer = ByteBuffer.wrap(packetData);
        int seqNum = buffer.getInt();
        buffer = ByteBuffer.allocate(4);
        buffer.putInt(seqNum);
        DatagramPacket ack = new DatagramPacket(buffer.array(), 4, ackAddress, ackPort);
        socket.send(ack);
        LAS = seqNum;
	}
	
	
	/**
	 * evaluates the sending process. Computes Transfer-speed based on sent data and transfer-time
	 * @param after timestamp after sending
	 * @param before timestamp before sending
	 * @param size size of a package's Message
	 * @param amnt Amount of packets sent
	 */
	private static void evaluate(long after, long before, int size, int amnt, int expec){
		System.out.println(expec + " packets expected");
		System.out.println(amnt + " packets received");
		long duration = after - before;
		int totalSize = size * expec;
		double speed = (8 * (double)(totalSize)/(double)(duration)) / 1000;
		if(speed != Double.NaN && speed != Double.POSITIVE_INFINITY){
			System.out.println(String.format("%.3f mbit/s", speed));
		}else{
			System.out.println("speed not calculatable");
		}
		
	}
	
	/**
	 * print print the content of a packet
	 * @param packet packet to print
	 * @param current current packetCount
	 * @param expec expected number of packets
	 */
	private static void print(DatagramPacket packet, int current, int expec){
		byte[] message = packet.getData();
		ByteBuffer buf = ByteBuffer.wrap(message);
		if(current < expec){
			while(buf.hasRemaining()){
				System.out.print(buf.getInt());
			}
		}else{
			while(buf.remaining() > 4){
				System.out.print(buf.getInt());
			}
			System.out.println();
			System.out.println("checksum: " + getUnsigned(buf.getInt(buf.array().length-4)));
		}
		System.out.println();
	}
	
	/**
	 * CRC returns a long value storing a 32 bit unsigned integer. Since java does not support
	 * unsigned types, we interpret the integer checksum in out last package as unsigned and
	 * assign it to a long value.
	 * @param checksum
	 * @return checksum interpreted as unsigned value.
	 */
	private static long getUnsigned(int checksum){
		if(checksum >= 0){
			return (long)checksum;
		}else{
			return ((long)(checksum + (Integer.MAX_VALUE + 1)) + Integer.MAX_VALUE + 1);
			
		}
	}
}