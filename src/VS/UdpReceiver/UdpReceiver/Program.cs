using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace UdpReceiver
{
	class Program
	{
		static void Main(string[] args)
		{
			//specify char code
			System.Text.Encoding enc = System.Text.Encoding.UTF8;

			//locak port number to bind
			int localPort = 9750;

			//binding localPort
			System.Net.Sockets.UdpClient udp =
				new System.Net.Sockets.UdpClient(localPort);

			//データを受信する
			System.Net.IPEndPoint remoteEP = null;
			while (true)
			{
				byte[] rcvBytes = udp.Receive(ref remoteEP);
				string rcvMsg = enc.GetString(rcvBytes);
				OutputString(string.Format("received:{0}", rcvMsg));
				OutputString(string.Format("source address:{0}/port:{1}",
					remoteEP.Address, remoteEP.Port));
			}
			//close UDP connection
			udp.Close();

			//Console.ReadLine();
		}

		static void OutputString(string str)
		{
			Console.WriteLine(str);
			Debug.WriteLine(str);
		}
	}
}
