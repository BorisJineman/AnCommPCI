using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Data.SqlTypes;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Net;
using System.Text;
using System.Windows.Forms;
//using SharpPcap;
using System.Net.Sockets;
using System.Runtime.CompilerServices;

namespace CommSample
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            //CaptureDeviceList allDevices = SharpPcap.CaptureDeviceList.Instance;

            //foreach (ICaptureDevice dev in allDevices)
            //{
            //    Device cdev = new Device();
            //    cdev.DisplayName = dev.Description;
            //    cdev.CaptureDevice = dev;
            //    comboBox1.Items.Add(cdev);
            //}


            if (comboBox1.Items.Count > 0)
                comboBox1.SelectedIndex = 0;
        }

        private UdpClient udpClient;
        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                IPEndPoint RemoteIpEndPoint = new IPEndPoint(IPAddress.Any, 6000);
                udpClient = new UdpClient(RemoteIpEndPoint);

                udpClient.BeginReceive(EndReceive, udpClient);

       

             


                MessageBox.Show("网络开启成功.");
                timer1.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show("网络开启失败.");
            }
            

        }

        private static string[] status = new string[] {"未知" ,"未知","未知","未知","未知","未知"};
        private static FileStream currentFile;
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
                        if (currentFile == null)
                        {
                          //  Console.WriteLine(filesavepath + "\\" + DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss_fff") + ".bin");
                            currentFile = File.Create(filesavepath + "\\" + DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss_fff") + ".bin");
                       
                        }


                        currentFile.Write(receiveBytes, 0, (int) swaplong(BitConverter.ToUInt32(receiveBytes, 4))*4);

                        break;

                    case 0x55AA3001:

                        if (swaplong(BitConverter.ToUInt32(receiveBytes, 4)) == 0x55aa3001)
                        {
                            currentFile.Flush();
                            currentFile.Close();
                            currentFile = null;
                        }
                        break;

                    case 0x55AA1003:

                        status[1] = BitConverter.ToUInt32(receiveBytes, 8).ToString();
                        status[2]  = BitConverter.ToUInt32(receiveBytes, 8).ToString();
                        status[3]  = BitConverter.ToUInt32(receiveBytes, 8).ToString();
                        status[4]   = BitConverter.ToUInt32(receiveBytes, 8).ToString();
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

        private void MainForm_Load(object sender, EventArgs e)
        {
            comboBox2.SelectedIndex = 0;
            comboBox3.SelectedIndex=0;
        }

        private void button6_Click(object sender, EventArgs e)
        {
            if (udpClient != null)
            {
                timer1.Stop();
                udpClient.Close();
                udpClient = null;
                MessageBox.Show("网络关闭.");
           
            }

        }

        private static string filesavepath;
        private void button3_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK)
            {
                filesavepath = folderBrowserDialog1.SelectedPath;

                MessageBox.Show("设置文件存储路径成功.");
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            //send the wave table file
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                var filename = openFileDialog1.FileName;
                List<byte> sendBytes = new List<byte>(new Byte[] { 0x55, 0xaa, 0x00, 0x02 });
                byte[] filedata = System.IO.File.ReadAllBytes(filename);
                sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(filedata.Length/4))));
                sendBytes.AddRange(filedata);

                sendBytes.Add(calc_CheckSum(sendBytes.ToArray()));

                udpClient.Send(sendBytes.ToArray(), sendBytes.Count,
                    new IPEndPoint(IPAddress.Parse("192.168.1.10"), 4000));
                
                MessageBox.Show("发送波表完成.");
            }
           
      
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            toolStripStatusLabel2.Text = status[0];
            toolStripStatusLabel5.Text = status[1];
            toolStripStatusLabel6.Text = status[2];
            toolStripStatusLabel8.Text = status[3];
            toolStripStatusLabel10.Text = status[4];
            toolStripStatusLabel12.Text = status[5];

            if (status[0] != "在线")
            {
                List<byte> sendBytes = new List<byte>(new Byte[] {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55});

                sendBytes.Add(calc_CheckSum(sendBytes.ToArray()));

                udpClient.Send(sendBytes.ToArray(), sendBytes.Count,
                    new IPEndPoint(IPAddress.Parse("192.168.1.10"), 4000));
            }



        }

        private void button9_Click(object sender, EventArgs e)
        {
            timer1.Stop();
            udpClient.Close();
            udpClient = null;
            this.Close();
        }

        private void button4_Click(object sender, EventArgs e)
        {
            List<byte> sendBytes = new List<byte>(new Byte[] { 0x55,0xaa,0x00, 0x00});
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(16))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox1.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox2.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox3.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox4.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(comboBox2.SelectedIndex))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox5.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(1))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox6.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox7.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox8.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(comboBox3.SelectedIndex))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox9.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox10.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));

            sendBytes.Add(calc_CheckSum(sendBytes.ToArray()));

            udpClient.Send(sendBytes.ToArray(), sendBytes.Count,
                new IPEndPoint(IPAddress.Parse("192.168.1.10"), 4000));

            MessageBox.Show("寄存器写入完成.");
        }

        private void button8_Click(object sender, EventArgs e)
        {
            List<byte> sendBytes = new List<byte>(new Byte[] { 0x55, 0xaa, 0x00, 0x00 });
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(16))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox1.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox2.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox3.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox4.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(comboBox2.SelectedIndex))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox5.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox6.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox7.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox8.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(comboBox3.SelectedIndex))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox9.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(textBox10.Text))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));
            sendBytes.AddRange(swap(BitConverter.GetBytes(Convert.ToUInt32(0))));

            sendBytes.Add(calc_CheckSum(sendBytes.ToArray()));

            udpClient.Send(sendBytes.ToArray(), sendBytes.Count,
                new IPEndPoint(IPAddress.Parse("192.168.1.10"), 4000));

            MessageBox.Show("寄存器写入完成.");
        }

        public static byte[] swap(byte[] src)
        {
            byte[] ret=new byte[4];
            ret[0] = src[3];
            ret[1] = src[2];
            ret[2] = src[1];
            ret[3] = src[0];
            return ret;
        }

        public static UInt32 swaplong( UInt32 src)
        {
            return BitConverter.ToUInt32(swap(BitConverter.GetBytes(src)), 0);
        }

    }
}
