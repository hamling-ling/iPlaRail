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
			//文字コードを指定する
			System.Text.Encoding enc = System.Text.Encoding.UTF8;

			//データを送信するリモートホストとポート番号
			string remoteHost = "192.168.11.4";
			int remotePort = 9750;
			//バインドするローカルポート番号
			int localPort = 9750;// 0x2616

			//ローカルポート番号localPortにバインドする
			System.Net.Sockets.UdpClient udp =
				new System.Net.Sockets.UdpClient(localPort);

			//送信するデータを読み込む
			//string sendMsg = Console.ReadLine();
			//byte[] sendBytes = enc.GetBytes(sendMsg);
			//リモートホストを指定してデータを送信する
			//udp.Send(sendBytes, sendBytes.Length,
			//	remoteHost, remotePort);

			//データを受信する
			System.Net.IPEndPoint remoteEP = null;
			while (true)
			{
				byte[] rcvBytes = udp.Receive(ref remoteEP);
				string rcvMsg = enc.GetString(rcvBytes);
				OutputString(string.Format("受信したデータ:{0}", rcvMsg));
				OutputString(string.Format("送信元アドレス:{0}/ポート番号:{1}",
					remoteEP.Address, remoteEP.Port));
			}
			//UDP接続を終了
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
