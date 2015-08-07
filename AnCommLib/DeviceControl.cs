using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace AnCommLib
{
    public delegate void NewDataReceived(byte[] data,int length);

    public delegate void NewDataReceiveFinished();

    public static class DeviceControl
    {
        public static event NewDataReceived OnNewDataReceived;
        public static event NewDataReceiveFinished OnNewDataReceiveFinished;
        
        private static UdpClient udpClient;

        public static string[] Status
        {
            get { return status; }
            set { status = value; }
        }

        private static Thread hThread;

        public static bool Open()
        {
            try
            {
                IPEndPoint RemoteIpEndPoint = new IPEndPoint(IPAddress.Any, 6000);
                udpClient = new UdpClient(RemoteIpEndPoint);
                udpClient.BeginReceive(EndReceive, udpClient);
                hThread = new Thread(() =>
                {
                    try
                    {
                        while (true)
                        {
                            if (status[0] != "在线")
                            {
                                List<byte> sendBytes =
                                    new List<byte>(new Byte[] {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55});

                                sendBytes.Add(calc_CheckSum(sendBytes.ToArray()));

                                udpClient.Send(sendBytes.ToArray(), sendBytes.Count,
                                    new IPEndPoint(IPAddress.Parse("192.168.1.10"), 4000));
                            }
                        }
                    }
                    catch (Exception)
                    {
                     
                    }

                });
                hThread.Start();

                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        public static void Close()
        {
            if (udpClient != null)
            {
                hThread.Abort();
                hThread = null;
                udpClient.Close();
                udpClient = null;

            }
        }

        public static void SendWaveFile(byte[] filedata)
        {
            List<byte> sendBytes = new List<byte>(new Byte[] { 0x55, 0xaa, 0x00, 0x02 });
        
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(filedata.Length / 4))));
            sendBytes.AddRange(filedata);

            sendBytes.Add(calc_CheckSum(sendBytes.ToArray()));

            udpClient.Send(sendBytes.ToArray(), sendBytes.Count,
                new IPEndPoint(IPAddress.Parse("192.168.1.10"), 4000));
        }


        public static void SetParameters(List<UInt32> paraList)
        {
            List<byte> sendBytes = new List<byte>(new Byte[] { 0x55, 0xaa, 0x00, 0x00 });
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(16))));

            for (int i = 0; i < 13; i++)
            {
                sendBytes.AddRange(swap(BitConverter.GetBytes(paraList[i])));
            }
           
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));

            sendBytes.Add(calc_CheckSum(sendBytes.ToArray()));

            udpClient.Send(sendBytes.ToArray(), sendBytes.Count,
                new IPEndPoint(IPAddress.Parse("192.168.1.10"), 4000));
        }


        private static byte calc_CheckSum(byte[] data)
        {
            byte ret = 0;
            foreach (byte b in data)
            {
                unchecked
                {
                    ret += b;
                }
            }
            return ret;
        }

        private static string[] status = new string[] { "未知", "未知", "未知", "未知", "未知", "未知" };
        private static void EndReceive(IAsyncResult ar)
        {

            UdpClient udpClient = ar.AsyncState as UdpClient;
            try
            {


                IPEndPoint ip = new IPEndPoint(IPAddress.Any, 0);
                Byte[] receiveBytes = udpClient.EndReceive(ar, ref ip);

                if (receiveBytes.Length < 4)
                {
                    goto next;
                }
                byte checksum = 0;

                for (int i = 0; i < receiveBytes.Length - 1; i++)
                {
                    unchecked
                    {
                        checksum += receiveBytes[i];
                    }
                }
                if (checksum != receiveBytes[receiveBytes.Length - 1])
                {
                    goto next;
                }

                // Process Received Data


                switch (swaplong(BitConverter.ToUInt32(receiveBytes, 0)))
                {

                    case 0x55AA1001:
                        if (OnNewDataReceived != null)
                        {
                            OnNewDataReceived(receiveBytes, (int) swaplong(BitConverter.ToUInt32(receiveBytes, 4)));
                        }
                     

                        break;

                    case 0x55AA3001:

                        if (swaplong(BitConverter.ToUInt32(receiveBytes, 4)) == 0x55aa3001)
                        {
                            if (OnNewDataReceiveFinished != null)
                            {
                                OnNewDataReceiveFinished();
                            }
                       
                        }
                        break;

                    case 0x55AA1003:

                        status[1] = BitConverter.ToUInt32(receiveBytes, 8).ToString();
                        status[2] = BitConverter.ToUInt32(receiveBytes, 8).ToString();
                        status[3] = BitConverter.ToUInt32(receiveBytes, 8).ToString();
                        status[4] = BitConverter.ToUInt32(receiveBytes, 8).ToString();
                        status[5] = BitConverter.ToUInt32(receiveBytes, 8).ToString();


                        break;

                    case 0xaa55aa55:
                        if (BitConverter.ToUInt32(receiveBytes, 4) == 0x55aa55aa)
                        {
                            status[0] = "在线";
                        }
                        break;
                    default:
                        break;
                }




            next:

                ;

            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                status[0] = "离线";

            }

            try
            {
                udpClient.BeginReceive(EndReceive, udpClient);
            }
            catch (Exception)
            {

                //  throw;
            }


        }

        private static byte[] swap(byte[] src)
        {
            byte[] ret = new byte[4];
            ret[0] = src[3];
            ret[1] = src[2];
            ret[2] = src[1];
            ret[3] = src[0];
            return ret;
        }

        private static UInt32 swaplong(UInt32 src)
        {
            return BitConverter.ToUInt32(swap(BitConverter.GetBytes(src)), 0);
        }
    }
}
